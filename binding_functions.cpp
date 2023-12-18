#include "binding_functions.hpp"

#include <iostream>
#include <sstream>
#include <string>

#include <boost/asio/ip/address.hpp>

auto address2in_addr(boost::asio::ip::address const& addr, in_addr_t& dest) -> void
{
    auto const a = addr.to_v4().to_bytes();
    std::memcpy(&dest, a.data(), a.size());
}

auto ip_mreqn2str(ip_mreqn const& req) -> std::string
{
    std::stringstream rss;
    // clang-format off
    rss << "req{"
       << "multiaddr=\"" << ::inet_ntoa(req.imr_multiaddr) << "\","
       << "addr=\"" << ::inet_ntoa(req.imr_address) << "\","
       << "index=\"" << req.imr_ifindex << "\""
       << "}";
    // clang-format on
    return rss.str();
}

auto set_mc_bound_2(
    int sockfd,
    boost::asio::ip::address mc_addr,
    boost::asio::ip::address if_addr,
    std::string const& if_name) -> int
{
    // Following
    // https://stackoverflow.com/a/23718680/1861346

    // """
    // you can send out packets through the interface ETH1,
    // but you can only recv packets send out from the ip associated with ETH1,
    // you can't recv any packets from other clients.
    // """
    // clang-format off
    auto const err = setsockopt(
        sockfd,
        SOL_SOCKET,
        SO_BINDTODEVICE,
        if_name.c_str(),
        static_cast<socklen_t>(if_name.size())
    );
    // clang-format on
    if (err == 0)
    {
        std::cout << __func__
                  << " Successful call to setsockopt(SO_BINDTODEVICE) if_name=" << if_name << "\n";
    }
    else
    {
        std::cout << __func__ << " Could not bind multicast to \"" << if_name
                  << "\": errno=" << std::to_string(errno) << ":" << strerror(errno) << "\n";
        return errno;
    }

    {
        // """
        // you can send out packets through the interface associated with my_ipv4addr,
        // also you can recv any packets from any clients in the multicast group.
        // """
        ip_mreqn req;

        // TODO replace with address2in_addr
        {
            auto const a = if_addr.to_v4().to_bytes();
            std::memcpy(&req.imr_address.s_addr, a.data(), a.size());
        }
        {
            auto const a = mc_addr.to_v4().to_bytes();
            std::memcpy(&req.imr_multiaddr.s_addr, a.data(), a.size());
        }
        get_ifindex(if_name, &req.imr_ifindex);

        // clang-format off
        auto const err = ::setsockopt(
            sockfd,
            IPPROTO_IP,
            IP_MULTICAST_IF,
            &req,
            sizeof(req)
        );
        // clang-format on

        if (err == 0)
        {
            std::cout << __func__ << ": Successful call to setsockopt(IP_MULTICAST_IF) req="
                      << ip_mreqn2str(req) << "\n";
        }
        else
        {
            auto const errno_b = errno;

            std::cout << __func__ << ": Could not specify " << ip_mreqn2str(req)
                      << " as the associated address.  Error: ";

            switch (errno_b)
            {
                case EBADF:
                {
                    std::cout << "Invalid socket descriptor";
                    break;
                }
                case EFAULT:
                {
                    std::cout << "invalid optval";
                    break;
                }
                case ENOPROTOOPT:
                {
                    std::cout << "optlen invalid";
                    break;
                }
                case ENOTSOCK:
                {
                    std::cout << "fd is not a socket";
                    break;
                }
                default:
                {
                    std::cout << strerror(errno_b);
                }
            }
            std::cout << "\n";
            return errno;
        }
    }

    return 0;
}

auto get_bound_device(int sockfd) -> std::string
{
    std::array<char, 10> dev_name;
    socklen_t dev_name_len = 0;
    auto err = ::getsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, &dev_name[0], &dev_name_len);
    std::string dev_name_str(std::begin(dev_name), std::begin(dev_name) + dev_name_len);
    if (err < 0)
    {
        std::stringstream ss;
        ss << "<errno=" << std::to_string(errno) << ":" << strerror(errno) << ">";
        dev_name_str = ss.str();
    }
    else if (0 == dev_name_len)
    {
        dev_name_str = "<blank>";
    }
    return dev_name_str;
}

auto get_ifname(int if_index, std::string& if_name) -> int
{
    struct if_nameindex *if_ni = nullptr, *i = nullptr;

    if_ni = ::if_nameindex();
    if (nullptr == if_ni)
    {
        perror("if_nameindex");
        exit(EXIT_FAILURE);
        return -1;
    }

    for (i = if_ni; !(i->if_index == 0 && nullptr == i->if_name); i++)
    {
        std::cout << "get_ifname(search=" << if_index << "): idx=" << i->if_index
                  << " name=" << i->if_name << "\n";
        if (i->if_index == if_index)
        {
            if_name = i->if_name;
            break;
        }
    }

    if_freenameindex(if_ni);

    return 0;
}

auto get_ifindex(std::string const& if_name, int* const if_index) -> unsigned int
{
#if 0
    ifaddrs *if_arr=nullptr, *if_int=nullptr;
    auto const err = ::getifaddrs(&if_arr);
    if (err != 0)
    {
        return err;
    }

    unsigned int index=0;
    for(if_int = if_arr; if_int != nullptr; if_int = if_int->ifa_next)
    {
        std::cout << "get_ifindex(search=" << if_name << "): idx=" << i->if_index << " name=" << i->if_name;
        if (0 == std::strncmp(if_int->ifa_name, if_name.c_str(), if_name.size()))
        {
            *if_index = index;
            break;
        }
        index++;
    }
    freeifaddrs(if_arr);
    if_arr = nullptr;
    if_int = nullptr;
#else
    struct if_nameindex *if_ni = nullptr, *i = nullptr;

    if_ni = ::if_nameindex();
    if (nullptr == if_ni)
    {
        perror("if_nameindex");
        exit(EXIT_FAILURE);
        return -1;
    }

    for (i = if_ni; !(i->if_index == 0 && nullptr == i->if_name); i++)
    {
        // std::cout << "get_ifindex(search=" << if_name << "): idx=" << i->if_index
        //           << " name=" << i->if_name << "\n";
        if (0 == std::strncmp(i->if_name, if_name.c_str(), if_name.size()))
        {
            *if_index = i->if_index;
            break;
        }
    }

    if_freenameindex(if_ni);
#endif

    return 0;
}