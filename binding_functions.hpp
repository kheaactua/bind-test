#ifndef BINDING_FUNCTIONS_HPP_PDKYFOSL
#define BINDING_FUNCTIONS_HPP_PDKYFOSL

#include <string>

#include "types.hpp"

namespace boost::asio::ip
{
    class address;
}

auto address2in_addr(boost::asio::ip::address const& addr, in_addr& dest) -> void;

auto ip_mreq2str(IP_REQ const& req) -> std::string;

auto set_mc_bound_2(
    int sockfd,
    boost::asio::ip::address const& mc_addr,
    boost::asio::ip::address const& if_addr,
    std::string const& if_name
) -> int;

auto get_bound_device(int sockfd) -> std::string;

auto get_ifname(unsigned int if_index, std::string& if_name) -> int;
auto get_ifname(IP_REQ req) -> std::string;
#ifndef __QNX__
auto get_ifindex(std::string const& if_name) -> decltype(IP_REQ::imr_ifindex);
#endif

#endif /* end of include guard: BINDING_FUNCTIONS_HPP_PDKYFOSL */
