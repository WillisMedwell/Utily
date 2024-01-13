#include "Utily/AsyncFileReader.hpp"

#if defined(_WIN32)
#include <windows.h>

#elif defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)
#include <emscripten/emscripten.h>

#elif defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
#error "not implemented unix/apple system"
#endif

namespace Utily {
#if defined(_WIN32)
    using FileHandle = HANDLE;

    struct AsyncFileReader::FileDataUnfulfilled {
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

#elif defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)

    struct AsyncFileReader::FlleDataUnfulfilled {
        std::monostate dummy;
    };

    static void file_load_callback(void* arg, void* buffer, int buffer_size) {
        const std::filesystem::path& path = *reinterpret_cast<std::filesystem::path*>(arg);

        std::vector<char> copy { buffer_size };
        std::copy(buffer, buffer + buffer_size, copy.data());

        files_fulfilled.emplace(path, std::move(copy));
        files_unfulfilled.erase(path);
    }

    static void file_fail_callback(void* arg) {
        const std::filesystem::path& path = *reinterpret_cast<std::filesystem::path*>(arg);
        std::cerr << std::format("File \"{}\" failed to load.", path.string());
        files_unfulfilled.erase(path);
    }

#elif defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))

#endif

    std::unordered_map<std::filesystem::path, AsyncFileReader::FileDataUnfulfilled> AsyncFileReader::files_unfulfilled = {};
    std::unordered_map<std::filesystem::path, AsyncFileReader::FileDataFulfilled> AsyncFileReader::files_fulfilled = {};

#if defined(_WIN32)

    auto AsyncFileReader::push(std::filesystem::path file_path) -> Utily::Result<void, Utily::Error> {
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
            nullptr);

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
            files_unfulfilled.erase(iter);
            return Utily::Error {
                std::format(
                    "File \"{}\" was opened but able to be read.",
                    file_str)
            };
        }

        return {};
    }

    auto AsyncFileReader::pop(std::filesystem::path file_path) -> Utily::Result<std::vector<char>, Utily::Error> {
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

    void AsyncFileReader::wait_for_all() {
        for (auto& [path, file_data] : files_unfulfilled) {
            DWORD bytes_transferreed;
            GetOverlappedResult(file_data.stream, &file_data.overlapped, &bytes_transferreed, true);
            CloseHandle(file_data.stream);
            files_fulfilled.emplace(path, std::move(file_data.contents));
        }
        files_unfulfilled.clear();
    }

#elif defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)

    inline auto AsyncFileReader::push(std::filesystem::path file_path) -> Utily::Result<void, Utily::Error> {
        if (files_unfulfilled.contains(file_path) || files_fulfilled.contains(file_path)) {
            return {};
        }

        auto file_str = file_path.string();

        if (!std::filesystem::exist(file_path)) {
            return Utily::Error {
                std::format(
                    "File \"{}\" does not exist.",
                    file_str)
            };
        }

        auto [iter, was_inserted_successfully] = files_unfulfilled.emplace(std::move(path));

        if (!was_inserted_successfully) {
            return Utily::Error {
                std::format(
                    "File \"{}\" is already queued.",
                    file_str)
            };
        }

        const std::filesystem::path& path = iter->first;

        emscripten_async_wget_data(file_str.c_str(), reinterpret_cast<void*>(&path), file_load_callback, file_error_callback);
    }

#elif defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))

#endif
}
