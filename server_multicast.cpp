#include "components.hpp"

#include <arpa/inet.h>

#include <chrono>
#include <condition_variable>
#include <sstream>
#include <thread>

#include <boost/asio/ip/address.hpp>

#include "binding_functions.hpp"
#include "logging.hpp"
#include "types.hpp"

// Playing with code from:
// https://www.geeksforgeeks.org/udp-server-client-implementation-c/
// https://gist.github.com/hostilefork/f7cae3dc33e7416f2dd25a402857b6c6

using namespace std::chrono_literals;

auto multicast_server(
    boost::asio::ip::address const& if_addr,
    std::string const& if_name,
    boost::asio::ip::address const& mc_addr,
    short unsigned int port,
    std::mutex& component_ready,
    bool& server_ready,
    std::condition_variable& server_ready_cv,
    bool const& client_ready,
    std::condition_variable& client_ready_cv) -> void
{
    struct sockaddr_in serv_addr;
    int sock_fd = 0;

    std::memset(&serv_addr, 0, sizeof(serv_addr));

    {
        sock_fd = ::socket(AF_INET, SOCK_DGRAM, 0);
        exit_on_error(sock_fd, Component::server, "socket");
    }

    {
        // QNX seems to require that I set these separately
        int const opt = 1; // Positive value for re-use
        // clang-format off
        auto const err1 = ::setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        exit_on_error(err1, Component::server, "setsockopt could not specify REUSEADDR");
        auto const err2 = ::setsockopt(sock_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
        exit_on_error(err2, Component::server, "setsockopt could not specify REUSEPORT");
        // clang-format on
    }

    {
        IP_REQ req;
        address2in_addr(mc_addr, req.imr_multiaddr);
#ifdef __QNX__
        address2in_addr(if_addr, req.imr_interface);
#else
        // TODO arguably this shouldn't be set here
        // req.imr_ifindex = 0; // ANY interface!
        req.imr_ifindex = get_ifindex(if_name);
#endif

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

        std::stringstream ss;
        ss << "Added to multicast group (IP_ADD_MEMBERSHIP) " << ::inet_ntoa(req.imr_multiaddr) << " on";
#ifdef __QNX__
        ss << " interface with IP " << ::inet_ntoa(req.imr_interface);
#else
        ss << " interface " << get_ifname(req);
#endif
        info(Component::server, ss.str());
    }

    {
        // Bind to device.  Right now the effect is that the client won't
        // receive messages.

        std::stringstream ss;
        // clang-format off
#ifdef __QNX__
        ifreq req;
        std::strcpy(req.ifr_name, if_name.c_str());
        auto const err = setsockopt(
            sock_fd,
            SOL_SOCKET,
            SO_BINDTODEVICE,
            &req,
            static_cast<socklen_t>(sizeof(req))
        );
        ss << "Could not bind multicast to \"" << req.ifr_name;
#else
        auto const err = setsockopt(
            sock_fd,
            SOL_SOCKET,
            SO_BINDTODEVICE,
            if_name.c_str(),
            static_cast<socklen_t>(if_name.size())
        );
        ss << "Could not bind multicast to \"" << if_name;
#endif
        // clang-format on
        ss << "\": errno=" << std::to_string(errno) << ":" << strerror(errno);
        exit_on_error(err, Component::server, ss.str());
        ss.str("");

        ss << "Bound to interface (SO_BINDTODEVICE) \"" << if_name << "\"";
        info(Component::server, ss.str());
    }

    {
        // IP_REQ req;
        in_addr mc_if_addr;
        address2in_addr(if_addr, mc_if_addr);

        // clang-format off
        auto const err = ::setsockopt(
            sock_fd,
            IPPROTO_IP,
            IP_MULTICAST_IF,
            &mc_if_addr,
            sizeof(mc_if_addr)
        );
        // clang-format on
        auto const errno_b = errno;

        std::stringstream ss;
        ss << "Could not specify " << ::inet_ntoa(mc_if_addr)
           << " as the associated address.  Error: " << strerror(errno_b);
        exit_on_error(err, Component::server, ss.str());
        ss.str("");

        ss << "Associated with interface (IP_MULTICAST_IF) req=" << ::inet_ntoa(mc_if_addr);
        info(Component::server, ss.str());
    }

    // bind socket
    {
        serv_addr.sin_family = AF_INET;
        address2in_addr(mc_addr, serv_addr.sin_addr);
        serv_addr.sin_port = htons(port);

        // clang-format off
        auto const err = ::bind(
            sock_fd,
            reinterpret_cast<struct sockaddr*>(&serv_addr),
            sizeof(serv_addr)
        );
        // clang-format on
        auto const errno_b = errno;

        std::stringstream ss;
        ss << "Could not bind to " << ::inet_ntoa(serv_addr.sin_addr)
           << " : Error: " << strerror(errno_b);
        exit_on_error(err, Component::server, ss.str());
        ss.str("");

        ss << "Bound (::bind) to " << ::inet_ntoa(serv_addr.sin_addr) << ":" << ntohs(serv_addr.sin_port);
        info(Component::server, ss.str());
    }

    {
        server_ready = true;
        server_ready_cv.notify_all();
        info(Component::server, "Notifying that service is bound");
    }

    {
        std::unique_lock<std::mutex> lk(component_ready);
        client_ready_cv.wait(lk, [&client_ready] { return client_ready; });
        info(Component::server, "Client ready");
        std::this_thread::sleep_for(200ms);
    }

    {
        std::string hello;
        for (int i = 0; i < 5; i++)
        {
            hello = "hello from server (" + std::to_string(i) + ")";
            // clang-format off
            auto const err = ::sendto(
                sock_fd,
                hello.c_str(),
                hello.size(),
#ifdef __QNX__
                0,
#else
                MSG_CONFIRM,
#endif
                reinterpret_cast<const struct sockaddr *>(&serv_addr),
                sizeof(serv_addr)
            );
            // clang-format on
            exit_on_error(err, Component::server, "Could not send hello message");
            std::stringstream ss;
            ss << "Sent " << err << " bytes: " << hello;
            info(Component::server, ss.str());
            std::this_thread::sleep_for(200ms);
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
