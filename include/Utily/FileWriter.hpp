#pragma once

#include <cstdint>
#include <filesystem>
#include <vector>

#include "Utily/Error.hpp"
#include "Utily/Result.hpp"

namespace Utily {
    class FileWriter
    {
    public:
        static auto dump_to_file(std::filesystem::path file_path, std::span<const uint8_t> data)
            -> Utily::Result<void, Utily::Error>;
    };
}