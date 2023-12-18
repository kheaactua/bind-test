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

#ifndef PORT
#error "Please define a PORT"
#endif

#ifdef BOOST_NO_EXCEPTIONS
namespace boost
{

template <class E> inline void throw_exception(E const& e)
{
    std::cout << "Boost exception thrown! " << e;
}

} // namespace boost
#endif

auto main() -> int
{
    auto const if_addr = boost::asio::ip::make_address(INTERFACE_IP);
    std::string if_name{INTERFACE_NAME};
    auto const mc_addr                       = boost::asio::ip::make_address(MULTICAST_ADDR);
    static short unsigned int constexpr port = PORT;

    std::mutex component_ready;

    auto server_ready = false;
    std::condition_variable server_ready_cv;

    auto client_ready = false;
    std::condition_variable client_ready_cv;

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
        std::ref(component_ready),
        std::ref(server_ready),
        std::ref(server_ready_cv),
        std::ref(client_ready),
        std::ref(client_ready_cv));

    auto client_thread = std::thread(
        &multicast_client,
        if_addr,
        if_name,
        mc_addr,
        port,
        std::ref(component_ready),
        std::cref(server_ready),
        std::ref(server_ready_cv),
        std::ref(client_ready),
        std::ref(client_ready_cv));

    service_thread.join();
    client_thread.join();

    return 0;
}
