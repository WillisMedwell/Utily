#include "Utily/AsyncFileReader.hpp"

#if defined(_WIN32)
#include <windows.h>

#elif defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)
#include <emscripten/emscripten.h>
#include <format>
#include <iostream>
#include <span>



#elif defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
//#error "not implemented unix/apple system"
#endif

#if 0

namespace Utily {
#if defined(_WIN32)
    using FileHandle = HANDLE;

    struct AsyncFileReader::FileDataUnfulfilled {
    private:
        FileHandle stream = nullptr;

    public:

        auto get_stream() -> FileHandle& {
            return stream;
        }

        std::vector<char> contents;
        OVERLAPPED overlapped;

        FileDataUnfulfilled() = default;

        FileDataUnfulfilled(const FileDataUnfulfilled&) = delete;

        FileDataUnfulfilled(FileDataUnfulfilled&& other)
            : stream(std::exchange(other.stream, nullptr))
            , contents(std::move(other.contents))
            , overlapped(std::exchange(other.overlapped, OVERLAPPED {})) { }

        ~FileDataUnfulfilled() {
            if (stream != nullptr) {
                CloseHandle(stream);
                stream = nullptr;
            }
        }
    };

#elif defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)
    struct AsyncFileReader::FileDataUnfulfilled {
        std::monostate dummy;
    };

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

        file_data.get_stream() = CreateFile(
            file_str.c_str(),
            GENERIC_READ,
            FILE_SHARE_READ,
            nullptr,
            OPEN_EXISTING,
            FILE_FLAG_OVERLAPPED,
            nullptr);

        if (file_data.get_stream() == INVALID_HANDLE_VALUE) {
            file_data.get_stream() = nullptr;
            return Utily::Error {
                std::format(
                    "File \"{}\" does not exist.",
                    file_str)
            };
        }

        if (LARGE_INTEGER temp; !GetFileSizeEx(file_data.get_stream(), &temp)) {
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
            file_data_ref.get_stream(),
            file_data_ref.contents.data(),
            static_cast<DWORD>(file_data_ref.contents.size()),
            &file_data_ref.overlapped,
            nullptr);

        if (!has_started_reading) {
            CloseHandle(file_data_ref.get_stream());
            file_data_ref.get_stream() = nullptr;
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
            GetOverlappedResult(file_data.get_stream(), &file_data.overlapped, &bytes_transferreed, true);
            CloseHandle(file_data.get_stream());
            file_data.get_stream() = nullptr;
            files_fulfilled.emplace(path, std::move(file_data.contents));
        }
        files_unfulfilled.clear();
    }

    auto AsyncFileReader::wait_pop(std::filesystem::path file_path) -> Utily::Result<std::vector<char>, Utily::Error> {
        if (files_unfulfilled.contains(file_path)) {
            DWORD bytes_transferreed;
            auto& file_data = files_unfulfilled[file_path];
            GetOverlappedResult(file_data.get_stream(), &file_data.overlapped, &bytes_transferreed, true);
            
            CloseHandle(file_data.get_stream());
            file_data.get_stream() = nullptr;
            auto contents = std::move(file_data.contents);
            files_unfulfilled.erase(file_path);
            return contents;
        }
        return pop(file_path);
    }

#elif defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)

    void AsyncFileReader::file_load_callback(void* arg, void* buffer_begin, int buffer_size) {
        const std::filesystem::path& path = *reinterpret_cast<std::filesystem::path*>(arg);

        std::vector<char> copy(static_cast<size_t>(buffer_size));
        std::span<char> buffer {
            reinterpret_cast<char*>(buffer_begin),
            static_cast<size_t>(buffer_size)
        };
        std::copy(buffer.begin(), buffer.end(), copy.data());

        AsyncFileReader::files_fulfilled.emplace(path, std::move(copy));
        AsyncFileReader::files_unfulfilled.erase(path);
    }

    void AsyncFileReader::file_fail_callback(void* arg) {
        const std::filesystem::path& path = *reinterpret_cast<std::filesystem::path*>(arg);
        std::cerr << std::format("File \"{}\" failed to load.", path.string());
        files_unfulfilled.erase(path);
    }

    auto AsyncFileReader::push(std::filesystem::path file_path) -> Utily::Result<void, Utily::Error> {
        if (files_unfulfilled.contains(file_path) || files_fulfilled.contains(file_path)) {
            return {};
        }

        auto file_str = file_path.string();

        if (!std::filesystem::exists(file_path)) {
            return Utily::Error {
                std::format(
                    "File \"{}\" does not exist.",
                    file_str)
            };
        }

        return {};
    }

    auto AsyncFileReader::pop(std::filesystem::path file_path [[maybe_unused]]) -> Utily::Result<std::vector<char>, Utily::Error> {
        return Utily::Error { "Not implemented" };
    }

    void AsyncFileReader::wait_for_all() {
        return;
    }

    auto AsyncFileReader::wait_pop(std::filesystem::path file_path) -> Utily::Result<std::vector<char>, Utily::Error> {
        return Utily::Error("Not implemented");
    }

    

#elif defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))

#endif

}

#endif