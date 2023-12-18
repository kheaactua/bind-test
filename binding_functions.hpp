#ifndef BINDING_FUNCTIONS_HPP_PDKYFOSL
#define BINDING_FUNCTIONS_HPP_PDKYFOSL

#include <arpa/inet.h>

#include <string>

namespace boost::asio::ip
{
    class address;
}

struct ip_mreqn;

auto address2in_addr(boost::asio::ip::address const& addr, in_addr_t& dest) -> void;

#ifdef __QNX__
struct ifreq;
auto ip_mreqn2str(ifreq const& req) -> std::string;
#else
auto ip_mreqn2str(ip_mreqn const& req) -> std::string;
#endif

auto set_mc_bound_2(
    int sockfd,
    boost::asio::ip::address mc_addr,
    boost::asio::ip::address if_addr,
    std::string const& if_name
) -> int;

auto get_bound_device(int sockfd) -> std::string;

auto get_ifname(unsigned int if_index, std::string& if_name) -> int;
auto get_ifindex(std::string const& if_name, unsigned int* const if_index) -> unsigned int;

#endif /* end of include guard: BINDING_FUNCTIONS_HPP_PDKYFOSL */
