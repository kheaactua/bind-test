#ifndef COMPONENTS_HPP_S0EML3DC
#define COMPONENTS_HPP_S0EML3DC

#include <condition_variable>
#include <mutex>
#include <string>
#include <iostream>


namespace boost::asio::ip
{
class address;
}

enum class Component
{
    main = 0,
    server,
    client
};

auto component_to_str(Component c) -> std::string;

auto multicast_server(
    boost::asio::ip::address const& if_addr,
    std::string const& if_name,
    boost::asio::ip::address const& mc_addr,
    short unsigned int port,
    std::mutex& component_ready,
    bool& server_ready,
    std::condition_variable& server_ready_cv,
    bool const& client_ready,
    std::condition_variable& client_ready_cv) -> void;

// auto unicast_server(
//     boost::asio::ip::address const& if_addr,
//     std::string const& if_name,
//     boost::asio::ip::address const& mc_addr,
//     short unsigned int,
//     bool& server_ready,
//     std::mutex& component_ready,
//     std::condition_variable& server_ready_cv) -> void;

// auto client_unicast(
//     boost::asio::ip::address const& if_addr,
//     std::string const& if_name,
//     boost::asio::ip::address const& mc_addr,
//     short unsigned int,
//     std::mutex& component_ready,
//     bool const& server_ready,
//     std::condition_variable& server_ready_cv,
//     bool const& client_ready,
//     std::condition_variable& client_ready_cv) -> int;

auto multicast_client(
    boost::asio::ip::address const& if_addr,
    std::string const& if_name,
    boost::asio::ip::address const& mc_addr,
    short unsigned int port,
    std::mutex& component_ready,
    bool const& server_ready,
    std::condition_variable& server_ready_cv,
    bool& client_ready,
    std::condition_variable& client_ready_cv) -> void;

#endif /* end of include guard: COMPONENTS_HPP_S0EML3DC */
