#include "components.hpp"

#include <condition_variable>
#include <sstream>

#include <boost/asio/ip/address.hpp>

#include "binding_functions.hpp"
#include "logging.hpp"

// Playing with code from:
// http://www.cs.tau.ac.il/~eddiea/samples/Multicast/multicast-listen.c.html

auto client_multicast(
    boost::asio::ip::address const& if_addr,
    std::string const& if_name,
    boost::asio::ip::address const& mc_addr,
    int port,
    bool const& server_started,
    std::mutex& server_started_mutex,
    std::condition_variable& server_started_cv) -> int
{
    // http://www.cs.tau.ac.il/~eddiea/samples/Multicast/multicast-listen.c.html
    int sock_fd = 0;
    struct sockaddr_in mcast_group
    {
        0
    }, client_addr{0};

    {
        sock_fd = ::socket(AF_INET, SOCK_DGRAM, 0);
        exit_on_error(sock_fd, Component::client, "Couldn't create socket");
    }

    {
        std::unique_lock<std::mutex> lk(server_started_mutex);
        server_started_cv.wait(lk, [&server_started] { return server_started; });
        info(Component::server, "Server started");
    }

    {
        int const opt = 1; // Positive value for re-use
        // clang-format off
        auto const err = ::setsockopt(
            sock_fd,
            SOL_SOCKET,
            SO_REUSEADDR | SO_REUSEPORT,
            &opt,
            sizeof(opt)
        );
        // clang-format on
        exit_on_error(err, Component::server, "setsockopt could not specify REUSEADDR");
    }

    // {
    //     // clang-format off
    //     auto const err = setsockopt(
    //         sock_fd,
    //         SOL_SOCKET,
    //         SO_BINDTODEVICE,
    //         if_name.c_str(),
    //         static_cast<socklen_t>(if_name.size())
    //     );
    //     // clang-format on
    //     std::stringstream ss;
    //     ss << "Could not bind multicast to \"" << if_name << "\": errno=" << std::to_string(errno)
    //        << ":" << strerror(errno);
    //     exit_on_error(err, Component::client, ss.str());

    //     ss << "Bound to interface \"" << if_name << "\"";
    //     info(Component::server, ss.str());
    // }

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

        std::stringstream ss;
        ss << "Could not specify " << mreq_str
           << " as the associated address.  Error: " << strerror(errno);
        exit_on_error(err, Component::server, ss.str());
        ss.str("");

        ss << "Successful call to setsockopt(IP_MULTICAST_IF) req=" << mreq_str;
        info(Component::client, ss.str());
    }

    {
        mcast_group.sin_family = AF_INET;
        address2in_addr(mc_addr, mcast_group.sin_addr.s_addr);
        mcast_group.sin_port = htons(port);

        // setsockopt(sock_fd, SOL_SOCKET, SO_BINDTODEVICE, if_name.c_str(), if_name.size());

        // clang-format off
        auto const err = ::bind(
            sock_fd,
            reinterpret_cast<struct sockaddr*>(&mcast_group),
            sizeof(mcast_group)
        );
        // clang-format on
        exit_on_error(err, Component::server, "bind error");
        std::stringstream ss;
        ss << "Bound to " << ::inet_ntoa(mcast_group.sin_addr) << ":"
           << ::ntohs(mcast_group.sin_port);
        info(Component::client, ss.str());
    }

    {
        // Preparatios for using Multicast

        ip_mreqn req;
        req.imr_multiaddr = mcast_group.sin_addr;
        req.imr_ifindex   = 0;

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

    // {
    //     std::string hello;
    //     hello = "hello from client (1)";
    //     // clang-format off
    //     auto const err = ::sendto(
    //         sock_fd,
    //         hello.c_str(),
    //         hello.size(),
    //         MSG_CONFIRM,
    //         reinterpret_cast<const struct sockaddr *>(&mcast_group),
    //         sizeof(mcast_group)
    //     );
    //     // clang-format on
    //     exit_on_error(err, Component::client, "Could not send hello message");
    //     info(Component::client, "Hello message sent");
    //     std::this_thread::sleep_for(500ms);
    // }

    auto len = static_cast<socklen_t>(sizeof(client_addr));
    {
        std::array<char, 1024> buffer = {0};
        // clang-format off
        auto const n = ::recvfrom(
            sock_fd,
            reinterpret_cast<char *>(buffer.data()),
            buffer.size()-1,
            MSG_WAITALL,
            reinterpret_cast<struct sockaddr *>(&mcast_group),
            reinterpret_cast<socklen_t*>(&len)
        );
        // clang-format on
        buffer[n] = '\0';

        std::stringstream ss;
        ss << "Read: " << buffer.data();
        info(Component::client, ss.str());
    }

    info(Component::client, "Closing");
    close(sock_fd);

    return 0;
}
