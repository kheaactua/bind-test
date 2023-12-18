#include <arpa/inet.h>
#include <sys/socket.h>

#include <concepts>
#include <iostream>
#include <sstream>
#include <string_view>

#include <boost/asio/ip/address.hpp>

#include "binding_functions.hpp"

#ifndef INTERFACE_IP
#error "Please define INTERFACE_IP"
#endif

#ifndef INTERFACE_NAME
#error "Please define INTERFACE_NAME"
#endif

#ifndef MULTICAST_ADDR
#define "Please define MULTICAST_ADDR"
#endif

#define ANSI_RED "\033[31;1m"
#define ANSI_YELLOW "\033[33m"
#define ANSI_BLUE "\033[34m"
#define ANSI_GREEN "\033[32m"
#define ANSI_MAGENTA "\033[35m"
#define ANSI_CLEAR "\033[0m"

// Playing with code from:
// https://stackoverflow.com/questions/12681097/c-choose-interface-for-udp-multicast-socket

template <typename T>
requires std::integral<T>
auto exit_on_error(T error, std::string&& msg) -> void
{
    if (error < 0)
    {
        std::cerr << "[" << ANSI_RED "Error" << ANSI_CLEAR << "] " << ANSI_RED << msg << ANSI_CLEAR
                  << "\n";
        exit(1);
    }
}

auto main() -> int
{
    struct sockaddr_in addr;
    int server_fd = 0, new_socket = 0;
    socklen_t addrlen = sizeof(addr);

    auto const if_addr = boost::asio::ip::make_address(INTERFACE_IP);
    std::string if_name{INTERFACE_NAME};
    auto const mc_addr = boost::asio::ip::make_address(MULTICAST_ADDR);
    int const port     = 30511;

    std::cout << "[INFO] Input: "
              << "if=" << if_name << ", "
              << "ipv4=" << if_addr << ", "
              << "maddr=" << mc_addr << "\n";

    // create what looks like an ordinary UDP socket
    {
        server_fd = socket(AF_INET, SOCK_DGRAM, 0);
        exit_on_error(server_fd, "socket");
    }

    {
        int const opt = 1; // Not sure what this is
        // clang-format off
        auto const err = ::setsockopt(
            server_fd, SOL_SOCKET,
            SO_REUSEADDR | SO_REUSEPORT,
            &opt,
            sizeof(opt)
        );
        // clang-format on
        exit_on_error(err, "setsockopt could not specify REUSEADDR");
    }

    {
        auto const err = set_mc_bound_2(server_fd, mc_addr, if_addr, if_name);
        if (err != 0)
        {
            std::stringstream ss;
            ss << "Could not bind mc socket to " << if_name
                      << ", errno=" << std::to_string(errno) << ":" << strerror(errno);
            exit_on_error(err, ss.str());
        }
    }

    // bind socket
    {
        // set up addresses
        std::memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        // [-]    addr.sin_addr.s_addr = htonl(INADDR_ANY);
        address2in_addr(if_addr, addr.sin_addr.s_addr);
        addr.sin_port = ::htons(port);

        std::cout << "[INFO] Binding to " << ::inet_ntoa(addr.sin_addr) << ":"
                  << ::ntohs(addr.sin_port) << "\n";

        // clang-format off
        auto const err = ::bind(
            server_fd,
            reinterpret_cast<struct sockaddr*>(&addr),
            sizeof(addr)
        );
        // clang-format on
        exit_on_error(err, "bind error");
    }

    // Listen
    {
        auto const err = ::listen(server_fd, /* backlog */ 3);
        std::stringstream ss;
        ss << "Failed to listen on server fd = " << server_fd;
        exit_on_error(err, ss.str());
    }

    // Accept
    {
        // clang-format off
        new_socket = ::accept(
            server_fd,
            reinterpret_cast<struct sockaddr*>(&addr),
            reinterpret_cast<socklen_t*>(&addrlen)
        );
        // clang-format on
        exit_on_error(new_socket, "accept error");
    }

    {
        std::array<char, 1024> buffer = {0};
        auto const valread            = ::read(new_socket, buffer.data(), buffer.size());
        exit_on_error(valread, "read error");
        std::cout << "[Info] Read: " << buffer.data() << "\n";
    }

    {
        std::string hello{"hello"};
        ::send(new_socket, hello.c_str(), hello.size(), 0);
        std::cout << "[Info] Hello message sent\n";
    }

    // use setsockopt() to request that the kernel join a multicast group
    // mreq.imr_multiaddr.s_addr = mc_addr.to_v4().to_uint();
    // [-]    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    // mreq.imr_interface.s_addr = inet_addr(if_addr);
    // if (setsockopt(server_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
    // {
    //     perror("setsockopt");
    //     exit(1);
    // }

    close(server_fd);
}
