#include "components.hpp"

#include <condition_variable>
#include <sstream>

#include <boost/asio/ip/address.hpp>

#include "types.hpp"
#include "binding_functions.hpp"
#include "logging.hpp"

// Playing with code from:
// http://www.cs.tau.ac.il/~eddiea/samples/Multicast/multicast-listen.c.html

auto client_multicast(
    boost::asio::ip::address const& if_addr,
    std::string const& if_name,
    boost::asio::ip::address const& mc_addr,
    short unsigned int port,
    bool const& server_started,
    std::mutex& server_started_mutex,
    std::condition_variable& server_started_cv) -> int
{
    // http://www.cs.tau.ac.il/~eddiea/samples/Multicast/multicast-listen.c.html
    int sock_fd = 0;
    struct sockaddr_in mcast_group, client_addr;

    std::memset(&mcast_group, 0, sizeof(mcast_group));
    std::memset(&client_addr, 0, sizeof(client_addr));


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
#ifdef __QNX__
            // TODO not sure why I appear to need this
            SO_REUSEPORT,
#else
            SO_REUSEADDR | SO_REUSEPORT,
#endif
            &opt,
            sizeof(opt)
        );
        // clang-format on
        exit_on_error(err, Component::client, "setsockopt could not specify REUSEADDR");
    }

    {
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
        exit_on_error(err, Component::client, ss.str());
        ss.str("");

        ss << "Bound to interface \"" << if_name << "\"";
        info(Component::client, ss.str());
    }

    {
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

        std::stringstream ss;
        ss << "Could not specify " << ::inet_ntoa(mc_if_addr)
           << " as the associated address.  Error: " << strerror(errno);
        exit_on_error(err, Component::server, ss.str());
        ss.str("");

        ss << "Successful call to setsockopt(IP_MULTICAST_IF) req=" << ::inet_ntoa(mc_if_addr);
        info(Component::client, ss.str());
    }

    {
        mcast_group.sin_family = AF_INET;
        address2in_addr(mc_addr, mcast_group.sin_addr);
        mcast_group.sin_port = htons(port);

        // clang-format off
        auto const err = ::bind(
            sock_fd,
            reinterpret_cast<struct sockaddr*>(&mcast_group),
            sizeof(mcast_group)
        );
        // clang-format on
        auto const errno_b = errno;

        std::stringstream ss;
        ss << "Could not bind to " << ::inet_ntoa(mcast_group.sin_addr)
           << " : Error: " << strerror(errno_b);
        exit_on_error(err, Component::client, ss.str());
        ss.str("");

        ss << "Bound to " << ::inet_ntoa(mcast_group.sin_addr) << ":"
           << ntohs(mcast_group.sin_port);
        info(Component::client, ss.str());
    }

    {
        // Preparatios for using Multicast
        IP_REQ req;
        address2in_addr(mc_addr, req.imr_multiaddr);
#ifdef __QNX__
        address2in_addr(if_addr, req.imr_interface);
#else
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
            buffer.data(),
            buffer.size()-1,
            MSG_WAITALL,
            reinterpret_cast<struct sockaddr *>(&mcast_group),
            &len
        );
        // clang-format on
        buffer[static_cast<decltype(buffer)::size_type>(n)] = '\0';

        std::stringstream ss;
        ss << "Read: " << buffer.data();
        info(Component::client, ss.str());
    }

    info(Component::client, "Closing");
    close(sock_fd);

    return 0;
}
