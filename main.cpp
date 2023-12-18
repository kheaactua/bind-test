#include <arpa/inet.h>
#include <sys/socket.h>

#include <concepts>
#include <chrono>
#include <iostream>
#include <sstream>
#include <string_view>
#include <thread>

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
// https://www.geeksforgeeks.org/udp-server-client-implementation-c/

enum class Component {
    main = 0,
    server,
    client
};

template <typename T>
requires std::integral<T>
auto exit_on_error(T error, Component c, std::string&& msg) -> void
{
    if (error < 0)
    {
        std::cerr << "[" << ANSI_RED "Error" << ANSI_CLEAR << "] ";

        switch (c)
        {
            case Component::main:
                std::cerr << "main";
                break;
            case Component::server:
                std::cerr << "service";
                break;
            case Component::client:
                std::cerr << "client";
                break;
        }
        std::cerr << ": " << ANSI_RED << msg << ANSI_CLEAR
                  << "\n";
        exit(1);
    }
}

using namespace std::chrono_literals;

auto server(
    boost::asio::ip::address const& if_addr,
    std::string const& if_name,
    boost::asio::ip::address const& mc_addr,
    int port
) -> void
{
    struct sockaddr_in serv_addr{0}, client_addr{0};
    int server_fd = 0;

    {
        server_fd = ::socket(AF_INET, SOCK_DGRAM, 0);
        exit_on_error(server_fd, Component::server, "socket");
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
        exit_on_error(err, Component::server, "setsockopt could not specify REUSEADDR");
    }

    // {
    //     auto const err = set_mc_bound_2(server_fd, mc_addr, if_addr, if_name);
    //     std::stringstream ss;
    //     ss << "Could not bind mc socket to " << if_name << ", errno=" << std::to_string(errno)
    //        << ":" << strerror(errno);
    //     exit_on_error(err, ss.str());
    // }

    // bind socket
    {
        // set up addresses
        // std::memset(&addr, 0, sizeof(addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        // address2in_addr(if_addr, serv_addr.sin_addr.s_addr);
        serv_addr.sin_port = ::htons(port);

        std::cout << "[INFO] Binding to " << ::inet_ntoa(serv_addr.sin_addr) << ":"
                  << ::ntohs(serv_addr.sin_port) << "\n";

        // clang-format off
        auto const err = ::bind(
            server_fd,
            reinterpret_cast<struct sockaddr*>(&serv_addr),
            sizeof(serv_addr)
        );
        // clang-format on
        exit_on_error(err, Component::server, "bind error");
    }

    auto len = static_cast<socklen_t>(sizeof(client_addr));
    {
        std::array<char, 1024> buffer = {0};
        // clang-format off
        auto const n = ::recvfrom(
            server_fd,
            reinterpret_cast<char *>(buffer.data()),
            buffer.size()-1,
            MSG_WAITALL,
            reinterpret_cast<struct sockaddr *>(&client_addr),
            reinterpret_cast<socklen_t*>(&len)
        );
        // clang-format on
        buffer[n] = '\0';
        std::cout << "[Info] Service: Read: " << buffer.data() << "\n";
    }

    {
        for (int i = 0; i < 2; i++)
        {
            std::string hello;
            hello = "hello" + std::to_string(i);
            // clang-format off
            auto const err = ::sendto(
                server_fd,
                hello.c_str(),
                hello.size(),
                MSG_CONFIRM,
                reinterpret_cast<const struct sockaddr *>(&client_addr),
                sizeof(client_addr)
            );
            // clang-format on
            exit_on_error(err, Component::server, "Could not send hello message");
            std::cout << "[Info] Service: Hello message sent\n";
            std::this_thread::sleep_for(500ms);
        }
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

    std::cout << "[Info] Service: Closing";
    close(server_fd);
}

auto client(
    boost::asio::ip::address const& if_addr,
    std::string const& if_name,
    boost::asio::ip::address const& mc_addr,
    int port) -> int
{
    int sock_fd = 0;
    struct sockaddr_in serv_addr{0}, client_addr{0};

    {
        sock_fd = ::socket(AF_INET, SOCK_DGRAM, 0);
        exit_on_error(sock_fd, Component::client, "Couldn't create socket");
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port   = htons(port);

    {
        for (int i = 0; i < 1; i++)
        {
            std::string hello;
            hello = "hello" + std::to_string(i);
            // clang-format off
            auto const err = ::sendto(
                sock_fd,
                hello.c_str(),
                hello.size(),
                MSG_CONFIRM,
                reinterpret_cast<const struct sockaddr *>(&serv_addr),
                sizeof(serv_addr)
            );
            // clang-format on
            exit_on_error(err, Component::client, "Could not send hello message");
            std::cout << "[Info] Client: Hello message sent\n";
            std::this_thread::sleep_for(500ms);
        }
    }

    auto len = static_cast<socklen_t>(sizeof(client_addr));
    {
        std::array<char, 1024> buffer = {0};
        // clang-format off
        auto const n = ::recvfrom(
            sock_fd,
            reinterpret_cast<char *>(buffer.data()),
            buffer.size()-1,
            MSG_WAITALL,
            reinterpret_cast<struct sockaddr *>(&serv_addr),
            reinterpret_cast<socklen_t*>(&len)
        );
        // clang-format on
        buffer[n] = '\0';
        std::cout << "[Info] Client: Read: " << buffer.data() << "\n";
    }

    std::cout << "[Info] Client: Closing";
    return 0;
}

auto main() -> int
{
    auto const if_addr = boost::asio::ip::make_address(INTERFACE_IP);
    std::string if_name{INTERFACE_NAME};
    auto const mc_addr = boost::asio::ip::make_address(MULTICAST_ADDR);
    int const port     = 30511;

    std::cout << "[INFO] Input: "
              << "if=" << if_name << ", "
              << "ipv4=" << if_addr << ", "
              << "maddr=" << mc_addr << "\n";

    auto service_thread = std::thread(&server, if_addr, if_name, mc_addr, port);
    auto client_thread = std::thread(&client, if_addr, if_name, mc_addr, port);

    service_thread.join();
    client_thread.join();
}
