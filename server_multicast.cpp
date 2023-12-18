#include "components.hpp"

#include <chrono>
#include <condition_variable>
#include <sstream>

#include <boost/asio/ip/address.hpp>

#include "binding_functions.hpp"
#include "logging.hpp"

// Playing with code from:
// https://www.geeksforgeeks.org/udp-server-client-implementation-c/
// https://gist.github.com/hostilefork/f7cae3dc33e7416f2dd25a402857b6c6

using namespace std::chrono_literals;

auto multicast_server(
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
    int sock_fd = 0;

    {
        sock_fd = ::socket(AF_INET, SOCK_DGRAM, 0);
        exit_on_error(sock_fd, Component::server, "socket");
    }

    {
        int const opt = 1; // Not sure what this is
        // clang-format off
        auto const err = ::setsockopt(
            sock_fd, SOL_SOCKET,
            SO_REUSEADDR | SO_REUSEPORT,
            &opt,
            sizeof(opt)
        );
        // clang-format on
        exit_on_error(err, Component::server, "setsockopt could not specify REUSEADDR");
    }

    {
        ip_mreqn req;
        address2in_addr(mc_addr, req.imr_multiaddr.s_addr);
        req.imr_ifindex = 0;

        // clang-format off
        auto const err = setsockopt(
            sock_fd,
            IPPROTO_IP,
            IP_ADD_MEMBERSHIP,
            &req,
            sizeof(req)
        );
        // clang-format on
        exit_on_error(err, Component::server, "Add membership error");
    }

    // Right now, doing this blocks the client from receiving anything!
    // {
    //     auto const err = set_mc_bound_2(sock_fd, mc_addr, if_addr, if_name);
    //     std::stringstream ss;
    //     ss << "Could not bind mc socket to " << if_name << ", errno=" << std::to_string(errno)
    //        << ":" << strerror(errno);
    //     exit_on_error(err, Component::server, ss.str());
    // }

    {
        // Bind to device.  Right now the effect is that the client won't
        // receive messages.

        // clang-format off
        auto const err = setsockopt(
            sock_fd,
            SOL_SOCKET,
            SO_BINDTODEVICE,
            if_name.c_str(),
            static_cast<socklen_t>(if_name.size())
        );
        // clang-format on
        std::stringstream ss;
        ss << "Could not bind multicast to \"" << if_name << "\": errno=" <<
        std::to_string(errno)
           << ":" << strerror(errno);
        exit_on_error(err, Component::server, ss.str());

        ss << "Bound to interface \"" << if_name << "\"";
        info(Component::server, ss.str());
    }

    {
        ip_mreqn req;
        address2in_addr(if_addr, req.imr_address.s_addr);
        address2in_addr(mc_addr, req.imr_multiaddr.s_addr);
        get_ifindex(if_name, &req.imr_ifindex);
        auto const mreq_str = ip_mreqn2str(req);

        // clang-format off
        auto const err = ::setsockopt(
            sock_fd,
            IPPROTO_IP,
            IP_MULTICAST_IF,
            &req,
            sizeof(req)
        );
        // clang-format on
        auto const errno_b = errno;

        std::stringstream ss;
        ss << "Could not specify " << mreq_str
           << " as the associated address.  Error: " << strerror(errno_b);
        exit_on_error(err, Component::server, ss.str());
        ss.str("");

        ss << "Successful call to setsockopt(IP_MULTICAST_IF) req=" << mreq_str;
        info(Component::server, ss.str());
    }

    // bind socket
    {
        // clang-format off
        auto const err = ::bind(
            sock_fd,
            reinterpret_cast<struct sockaddr*>(&serv_addr),
            sizeof(serv_addr)
        );
        // clang-format on
        exit_on_error(err, Component::server, "bind error");
    }

    {
        server_started = true;
        server_started_cv.notify_all();
        info(Component::server, "Notifying that service is bound");
    }

    {
        client_addr.sin_family = AF_INET;
        address2in_addr(mc_addr, client_addr.sin_addr.s_addr);
        client_addr.sin_port = ::htons(port);

        std::string hello;
        for (int i = 0; i < 5; i++)
        {
            hello = "hello from server (" + std::to_string(i) + ")";
            // clang-format off
            auto const err = ::sendto(
                sock_fd,
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
    }

    // use setsockopt() to request that the kernel join a multicast group
    // mreq.imr_multiaddr.s_addr = mc_addr.to_v4().to_uint();
    // [-]    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    // mreq.imr_interface.s_addr = inet_addr(if_addr);
    // if (setsockopt(sock_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
    // {
    //     perror("setsockopt");
    //     exit(1);
    // }

    info(Component::server, "Closing");
    close(sock_fd);
}
