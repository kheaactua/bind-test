#include "components.hpp"
#include "logging.hpp"

#include <sstream>
#include <string>

auto component_to_str(Component c, bool const decorate) -> std::string
{
    std::stringstream ss;
    switch (c)
    {
        case Component::main:
            ss << "main";
            break;
        case Component::server:
            if (decorate)
                ss << ANSI_MAGENTA;
            ss << "service";
            if (decorate)
                ss << ANSI_CLEAR;
            break;
        case Component::client:
            if (decorate)
                ss << ANSI_GREEN;
            ss << "client";
            if (decorate)
                ss << ANSI_CLEAR;
            break;
    }
    return ss.str();
}
