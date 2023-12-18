
#include "logging.hpp"

#include <iostream>
#include <mutex>
#include <sstream>
#include <string>

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
#ifdef __ANDROID__
    ALOGI("%s: %s", component_to_str(c, false).c_str(), msg.c_str());
#endif
}
