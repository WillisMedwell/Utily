#pragma once

#include <array>
#include <cassert>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <queue>
#include <string>
#include <unordered_map>
#include <variant>

#include "Utily/Error.hpp"
#include "Utily/Result.hpp"



namespace Utily {
    class AsyncFileReader
    {
        struct FileDataUnfulfilled;

        struct FileDataFulfilled {
            std::vector<char> contents;
        };

        static std::unordered_map<std::filesystem::path, FileDataUnfulfilled> files_unfulfilled;
        static std::unordered_map<std::filesystem::path, FileDataFulfilled> files_fulfilled;

    public:
        static auto push(std::filesystem::path file_path) -> Utily::Result<void, Utily::Error>;
        static auto pop(std::filesystem::path file_path) -> Utily::Result<std::vector<char>, Utily::Error>;

        static void wait_for_all();
    };


}