#pragma once

#include "Utily/Error.hpp"

#include <iostream>
#include <cstdlib>

namespace Utily {
    namespace ErrorHandler {
        static inline auto print_then_quit = [](const auto& error)
        {   
            std::cout << error.what();
            exit(EXIT_FAILURE);
        };
    }
} 