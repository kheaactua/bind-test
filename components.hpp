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
    short unsigned int,
    bool& server_started,
    std::mutex& server_started_mutex,
    std::condition_variable& server_started_cv) -> void;

auto unicast_server(
    boost::asio::ip::address const& if_addr,
    std::string const& if_name,
    boost::asio::ip::address const& mc_addr,
    short unsigned int,
    bool& server_started,
    std::mutex& server_started_mutex,
    std::condition_variable& server_started_cv) -> void;

auto client_unicast(
    boost::asio::ip::address const& if_addr,
    std::string const& if_name,
    boost::asio::ip::address const& mc_addr,
    short unsigned int,
    bool const& server_started,
    std::mutex& server_started_mutex,
    std::condition_variable& server_started_cv) -> int;

auto client_multicast(
    boost::asio::ip::address const& if_addr,
    std::string const& if_name,
    boost::asio::ip::address const& mc_addr,
    short unsigned int,
    bool const& server_started,
    std::mutex& server_started_mutex,
    std::condition_variable& server_started_cv) -> int;

#endif /* end of include guard: COMPONENTS_HPP_S0EML3DC */
