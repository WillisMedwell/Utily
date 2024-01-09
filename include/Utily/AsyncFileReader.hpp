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

#if defined(_WIN32)
#include <windows.h>
#elif defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
#error "not implemented unix/apple system"
#elif defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)
#error "not implemented emscripten system"
#endif

namespace Utily {
    class AsyncFileReader
    {
#if defined(_WIN32)
        using FileHandle = HANDLE;

        struct FileDataUnfulfilled {
            FileHandle stream = nullptr;
            std::vector<char> contents;
            OVERLAPPED overlapped;

            FileDataUnfulfilled() = default;

            FileDataUnfulfilled(const FileDataUnfulfilled&) = delete;

            FileDataUnfulfilled(FileDataUnfulfilled&& other)
                : stream(std::exchange(other.stream, nullptr))
                , contents(std::move(other.contents))
                , overlapped(std::exchange(other.overlapped, OVERLAPPED {})) { }

            ~FileDataUnfulfilled() {
                if (stream) {
                    CloseHandle(stream);
                    stream = nullptr;
                }
            }
        };

        struct FileDataFulfilled {
            std::vector<char> contents;
        };

#elif defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))

#elif defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)

#endif
        static std::unordered_map<std::filesystem::path, FileDataUnfulfilled> files_unfulfilled;
        static std::unordered_map<std::filesystem::path, FileDataFulfilled> files_fulfilled;

    public:
        static auto push(std::filesystem::path file_path) -> Utily::Result<void, Utily::Error>;
        static auto pop(std::filesystem::path file_path) -> Utily::Result<std::vector<char>, Utily::Error>;

        static void wait_for_all();
    };

#if defined(_WIN32)
    inline std::unordered_map<std::filesystem::path, AsyncFileReader::FileDataUnfulfilled> AsyncFileReader::files_unfulfilled = {};
    inline std::unordered_map<std::filesystem::path, AsyncFileReader::FileDataFulfilled> AsyncFileReader::files_fulfilled = {};

    inline auto AsyncFileReader::push(std::filesystem::path file_path) -> Utily::Result<void, Utily::Error> {
        if (files_unfulfilled.contains(file_path) || files_fulfilled.contains(file_path)) {
            return {};
        }

        const auto file_str = file_path.string();

        FileDataUnfulfilled file_data {};

        file_data.stream = CreateFile(
            file_str.c_str(),
            GENERIC_READ,
            FILE_SHARE_READ,
            nullptr,
            OPEN_EXISTING,
            FILE_FLAG_OVERLAPPED,
            nullptr
        );

        if (file_data.stream == INVALID_HANDLE_VALUE) {
            return Utily::Error {
                std::format(
                    "File \"{}\" does not exist.",
                    file_str)
            };
        }

        if (LARGE_INTEGER temp; !GetFileSizeEx(file_data.stream, &temp)) {
            return Utily::Error {
                std::format(
                    "File \"{}\" did not allow us to read the file's size.",
                    file_str)
            };
        } else {
            file_data.contents.resize(static_cast<size_t>(temp.QuadPart));
        }
        auto [iter, was_inserted_successfully] = files_unfulfilled.emplace(
            std::move(file_path),
            std::move(file_data));

        if (!was_inserted_successfully) {
            return Utily::Error {
                std::format(
                    "File \"{}\" is already queued.",
                    file_str)
            };
        }

        auto& [path, file_data_ref] = *iter;

        bool has_started_reading = ReadFileEx(
            file_data_ref.stream,
            file_data_ref.contents.data(),
            static_cast<DWORD>(file_data_ref.contents.size()),
            &file_data_ref.overlapped,
            nullptr);

        if (!has_started_reading) {
            CloseHandle(file_data_ref.stream);
            files_unfulfilled.erase(path);
            return Utily::Error {
                std::format(
                    "File \"{}\" was opened but able to be read.",
                    file_str)
            };
        }

        return {};
    }

    inline auto AsyncFileReader::pop(std::filesystem::path file_path) -> Utily::Result<std::vector<char>, Utily::Error> {
        if (files_fulfilled.contains(file_path)) {
            auto iter = files_fulfilled.find(file_path);
            std::vector<char> value(std::move(iter->second.contents));
            files_fulfilled.erase(iter);
            return value;

        } else if (files_unfulfilled.contains(file_path)) {
            return Utily::Error {
                std::format(
                    "File \"{}\" has not finished loading.",
                    file_path.generic_string())
            };
        } else {
            return Utily::Error {
                std::format(
                    "File \"{}\" was not pushed to be loaded.",
                    file_path.generic_string())
            };
        }
    }

    inline void AsyncFileReader::wait_for_all() {
        for (auto& [path, file_data] : files_unfulfilled) {
            DWORD bytes_transferreed;
            GetOverlappedResult(file_data.stream, &file_data.overlapped, &bytes_transferreed, true);
            CloseHandle(file_data.stream);
            files_fulfilled.emplace(path, std::move(file_data.contents));
        }
        files_unfulfilled.clear();
    }

#elif defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))

#elif defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)

#endif
}