#include <sys/socket.h>
#include <arpa/inet.h>

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
    int fd=0, nbytes=0;
    socklen_t addrlen;
    struct ip_mreq mreq;

    auto const if_addr = boost::asio::ip::address::from_string(INTERFACE_IP);
    std::string if_name{INTERFACE_NAME};
    auto const mc_addr = boost::asio::ip::address::from_string(MULTICAST_ADDR);
    int const port = 30511;

    std::cout << "Input: "
        << "if=" << if_name << ", "
        << "ipv4=" << if_addr << ", "
        << "maddr=" << mc_addr << "\n";

    // create what looks like an ordinary UDP socket
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket");
        exit(1);
    }

    // set up addresses
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    // [-]    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_addr.s_addr = inet_addr(if_addr.to_string().c_str());
    addr.sin_port        = htons(port);

    // bind socket
    if (::bind(fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0)
    {
        perror("bind");
        exit(1);
    }

    {
        auto const err = set_mc_bound_2(
            fd,
            mc_addr,
            if_addr,
            if_name
        );
        if (err != 0)
        {
            std::cout << "Could not bind mc socket to " << if_name << ", errno=" << std::to_string(errno) << ":" << strerror(errno) << "";
        }
    }

    // use setsockopt() to request that the kernel join a multicast group
    // mreq.imr_multiaddr.s_addr = mc_addr.to_v4().to_uint();
    // [-]    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    // mreq.imr_interface.s_addr = inet_addr(if_addr);
    // if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
    // {
    //     perror("setsockopt");
    //     exit(1);
    // }
}
