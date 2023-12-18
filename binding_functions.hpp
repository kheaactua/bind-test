#ifndef BINDING_FUNCTIONS_HPP_PDKYFOSL
#define BINDING_FUNCTIONS_HPP_PDKYFOSL

#include <string>

namespace boost::asio::ip
{
    class address;
}

auto set_mc_bound_2(
    int sockfd,
    boost::asio::ip::address mc_addr,
    boost::asio::ip::address local,
    std::string const& if_name
) -> int;

auto get_bound_device(int sockfd) -> std::string;

auto get_ifname(int if_index, std::string& if_name) -> int;
auto get_ifindex(std::string const& if_name, int* const if_index) -> unsigned int;

#endif /* end of include guard: BINDING_FUNCTIONS_HPP_PDKYFOSL */
