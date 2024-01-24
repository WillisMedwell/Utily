#pragma once

#include <cstdint>
#include <filesystem>
#include <vector>

#include "Utily/Error.hpp"
#include "Utily/Result.hpp"

namespace Utily {
    class FileReader
    {
    public:
        static auto load_entire_file(std::filesystem::path file_path)
            -> Utily::Result<std::vector<uint8_t>, Utily::Error>;
    };
}