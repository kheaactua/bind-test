#ifndef LOGGING_HPP_EDKP8OLK
#define LOGGING_HPP_EDKP8OLK

#include <iostream>
#include <sstream>
#include <string>

#ifdef __ANDROID__
#include <android/log.h>
#define LOG_TAG "bind-test"
#define ALOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#endif

#include "components.hpp"

#define ANSI_RED "\033[31;1m"
#define ANSI_YELLOW "\033[33m"
#define ANSI_BLUE "\033[34m"
#define ANSI_GREEN "\033[32m"
#define ANSI_MAGENTA "\033[35m"
#define ANSI_CLEAR "\033[0m"

auto print_msg(std::string&& msg) -> void;

template <typename T> auto exit_on_error(T error, Component c, std::string&& msg) -> void
{
    if (error < 0)
    {
        std::stringstream ss;
        ss << "[" << ANSI_RED "Error" << ANSI_CLEAR << "] ";
        ss << component_to_str(c, true);
        ss << ": " << ANSI_RED << msg << ANSI_CLEAR;
        print_msg(ss.str());
#ifdef __ANDROID__
        ALOGE("%s: %s", component_to_str(c, false), msg.c_str());
#endif
        exit(1);
    }
}

auto info(Component c, std::string&& msg) -> void;

#endif /* end of include guard: LOGGING_HPP_EDKP8OLK */
