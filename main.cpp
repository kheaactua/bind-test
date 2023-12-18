#include <arpa/inet.h>
#include <sys/socket.h>

#include <iostream>
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

// Playing with code from:
// https://stackoverflow.com/questions/12681097/c-choose-interface-for-udp-multicast-socket

auto main() -> int
{
    struct sockaddr_in addr;
    int server_fd = 0, new_socket = 0;
    socklen_t addrlen = sizeof(addr);
    struct ip_mreq mreq;

    auto const if_addr = boost::asio::ip::make_address(INTERFACE_IP);
    std::string if_name{INTERFACE_NAME};
    auto const mc_addr = boost::asio::ip::make_address(MULTICAST_ADDR);
    int const port     = 30511;

    std::cout << "Input: "
              << "if=" << if_name << ", "
              << "ipv4=" << if_addr << ", "
              << "maddr=" << mc_addr << "\n";

    // create what looks like an ordinary UDP socket
    if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket");
        exit(1);
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
        if (0 != err)
        {
            std::cerr << "setsockopt could not specify REUSEADDR\n";
            exit(EXIT_FAILURE);
        }
    }

    {
        auto const err = set_mc_bound_2(server_fd, mc_addr, if_addr, if_name);
        if (err != 0)
        {
            std::cout << "Could not bind mc socket to " << if_name
                      << ", errno=" << std::to_string(errno) << ":" << strerror(errno) << "\n";
        }
    }

    // set up addresses
    {
        boost::system::error_code ec;
        bzero(&addr, sizeof(addr));
        addr.sin_family = AF_INET;
        // [-]    addr.sin_addr.s_addr = htonl(INADDR_ANY);
        address2in_addr(if_addr, addr.sin_addr.s_addr);
        addr.sin_port = ::htons(port);
    }

    // bind socket
    {
        auto const err = ::bind(server_fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr));
        if (err < 0)
        {
            std::cerr << "bind error\n";
            exit(1);
        }
    }

    // Listen
    {
        auto const err = listen(server_fd, 3);
        if (err < 0)
        {
            std::cerr << "listen error\n";
            exit(1);
        }
    }

    // Accept
    {
        // clang-format off
        new_socket = accept(
            server_fd,
            reinterpret_cast<struct sockaddr*>(&addr),
            reinterpret_cast<socklen_t*>(&addrlen)
        );
        if (new_socket < 0)
        {
            std::cerr << "accept error\n";
            exit(1);
        }
    }

    {
        std::array<char, 1024> buffer = {0};
        auto const valread = ::read(new_socket, buffer.data(), buffer.size());
        std::cout << "Read: " << buffer.data() << "\n";
    }

    {
        std::string hello{"hello"};
        ::send(new_socket, hello.c_str(), hello.size(), 0);
        std::cout << "Hello message sent\n";
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
