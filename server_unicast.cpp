#include "components.hpp"

#include <chrono>
#include <condition_variable>
#include <sstream>

#include <boost/asio/ip/address.hpp>

#include "logging.hpp"
#include "binding_functions.hpp"


// Playing with code from:
// https://www.geeksforgeeks.org/udp-server-client-implementation-c/


using namespace std::chrono_literals;

auto unicast_server(
    boost::asio::ip::address const& if_addr,
    std::string const& if_name,
    boost::asio::ip::address const& mc_addr,
    int port,
    bool& server_started,
    std::mutex& server_started_mutex,
    std::condition_variable& server_started_cv) -> void
{
    struct sockaddr_in serv_addr
    {
        0
    }, client_addr{0};
    int server_fd = 0;

    {
        server_fd = ::socket(AF_INET, SOCK_DGRAM, 0);
        exit_on_error(server_fd, Component::server, "socket");
    }

    {
        int const opt = 1; // Positive value for re-use
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

    // bind socket
    {
        // set up addresses
        serv_addr.sin_family = AF_INET;
        address2in_addr(if_addr, serv_addr.sin_addr.s_addr);
        serv_addr.sin_port = ::htons(port);

        std::stringstream ss;
        ss << "Binding to " << ::inet_ntoa(serv_addr.sin_addr) << ":"
           << ::ntohs(serv_addr.sin_port);
        info(Component::server, ss.str());

        // clang-format off
        auto const err = ::bind(
            server_fd,
            reinterpret_cast<struct sockaddr*>(&serv_addr),
            sizeof(serv_addr)
        );
        // clang-format on
        exit_on_error(err, Component::server, "bind error");
    }

    {
        std::unique_lock<std::mutex> lk(server_started_mutex);
        server_started = true;
        server_started_cv.notify_all();
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

        std::stringstream ss;
        ss << "Read: " << buffer.data();
        info(Component::server, ss.str());
    }

    {
        std::string hello;
        hello = "hello from server (1)";
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
        info(Component::server, "Hello message sent");
        std::this_thread::sleep_for(500ms);
    }

    info(Component::server, "Closing");
    close(server_fd);
}
