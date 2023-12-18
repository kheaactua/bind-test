#include "components.hpp"

#include <string>

auto component_to_str(Component c) -> std::string
{
    switch (c)
    {
        case Component::main:
            return "main";
        case Component::server:
            return "service";
        case Component::client:
            return "client";
    }
}
