#include <arpa/inet.h>
#include <sys/socket.h>

#include <condition_variable>
#include <iostream>
#include <sstream>
#include <thread>

#include <boost/asio/ip/address.hpp>

#include "components.hpp"
#include "logging.hpp"

#ifndef INTERFACE_IP
#error "Please define INTERFACE_IP"
#endif

#ifndef INTERFACE_NAME
#error "Please define INTERFACE_NAME"
#endif

#ifndef MULTICAST_ADDR
#error "Please define MULTICAST_ADDR"
#endif


auto main() -> int
{
    auto const if_addr = boost::asio::ip::make_address(INTERFACE_IP);
    std::string if_name{INTERFACE_NAME};
    auto const mc_addr        = boost::asio::ip::make_address(MULTICAST_ADDR);
    static short unsigned int constexpr port = 30512;

    auto server_started = false;
    std::mutex server_started_mutex;
    std::condition_variable server_started_cv;

    std::stringstream ss;
    ss << "Input: "
       << "if=" << if_name << ", "
       << "ipv4=" << if_addr << ", "
       << "maddr=" << mc_addr;
    info(Component::main, ss.str());

    auto service_thread = std::thread(
        &multicast_server,
        if_addr,
        if_name,
        mc_addr,
        port,
        std::ref(server_started),
        std::ref(server_started_mutex),
        std::ref(server_started_cv));

    auto client_thread = std::thread(
        &client_multicast,
        if_addr,
        if_name,
        mc_addr,
        port,
        std::ref(server_started),
        std::ref(server_started_mutex),
        std::ref(server_started_cv));

    service_thread.join();
    client_thread.join();

    return 0;
}
