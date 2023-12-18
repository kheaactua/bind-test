#include <sys/socket.h>

#include <iostream>
#include <string_view>

#ifndef INTERFACE_IP
#define INTERFACE_IP "192.168.2.53"
#endif

#ifndef MULTICAST_ADDR
#define MULTICAST_ADDR "224.2.127.254"
#endif

#ifndef INTERFACE_NAME
#define INTERFACE_NAME "eno1"
#endif

// Playing with code from:
// https://stackoverflow.com/questions/12681097/c-choose-interface-for-udp-multicast-socket

auto main() -> int
{
    struct sockaddr_in addr;
    int fd=0, nbytes=0;
    socklen_t addrlen;
    struct ip_mreq mreq;

    static constexpr std::string_view if_addr = INTERFACE_IP;
    static constexpr std::string_view if_name = INTERFACE_NAME;
    static constexpr std::string_view mc_addr = MULTICAST_ADDR;

    std::cout << "Input: "
        << "if=" << if_name << ", "
        << "ipv4=" << if_addr << ", "
        << "maddr=" << mc_addr << "\n";

    // if_addr equals current IP as String, e.g. "89.89.89.89"

    // create what looks like an ordinary UDP socket */
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket");
        exit(1);
    }

    // set up addresses
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    // [-]    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_addr.s_addr = inet_addr(if_addr);
    addr.sin_port        = htons(port);

    // bind socket
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        exit(1);
    }

    // use setsockopt() to request that the kernel join a multicast group
    mreq.imr_multiaddr.s_addr = inet_addr(group);
    // [-]    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    mreq.imr_interface.s_addr = inet_addr(if_addr);
    if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
    {
        perror("setsockopt");
        exit(1);
    }
}
