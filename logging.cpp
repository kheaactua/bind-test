
#include "logging.hpp"

#include <string>
#include <sstream>
#include <iostream>
#include <mutex>

#include "components.hpp"

std::mutex log_mutex;

auto print_msg(std::string&& msg) -> void
{
    std::lock_guard<std::mutex> const lock(log_mutex);
    std::cout << msg << "\n";
}

auto info(Component c, std::string&& msg) -> void
{
    std::stringstream ss;
    ss << "[" << ANSI_BLUE << "Info" << ANSI_CLEAR << "] ";
    ss << component_to_str(c, true);
    ss << ": " << ANSI_BLUE << msg << ANSI_CLEAR;

    print_msg(ss.str());
}
