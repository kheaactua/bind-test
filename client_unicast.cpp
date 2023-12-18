#include "components.hpp"

#include <condition_variable>
#include <sstream>

#include <boost/asio/ip/address.hpp>

#include "logging.hpp"
#include "binding_functions.hpp"

// Playing with code from:
// https://www.geeksforgeeks.org/udp-server-client-implementation-c/


// TODO re-test this.. The git history should have a working version
auto client_unicast(
    boost::asio::ip::address const& if_addr,
    std::string const& if_name,
    boost::asio::ip::address const& mc_addr,
    int port) -> int
{
    int sock_fd = 0;
    struct sockaddr_in serv_addr
    {
        0
    }, client_addr{0};

    {
        sock_fd = ::socket(AF_INET, SOCK_DGRAM, 0);
        exit_on_error(sock_fd, Component::client, "Couldn't create socket");
    }

    serv_addr.sin_family = AF_INET;
    address2in_addr(if_addr, serv_addr.sin_addr.s_addr);
    serv_addr.sin_port = htons(port);

    {
        std::string hello;
        hello = "hello from client (1)";
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
        info(Component::client, "Hello message sent");
        std::this_thread::sleep_for(500ms);
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

        std::stringstream ss;
        ss << "Read: " << buffer.data();
        info(Component::client, ss.str());
    }

    info(Component::client, "Closing");
    return 0;
}
