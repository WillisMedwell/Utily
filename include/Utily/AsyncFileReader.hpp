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

/*
// namespace Utily {

//     class FileReaderAysncQueue
//     {
// #if defined(_WIN32)

// #elif defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))

// #elif defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)

// #endif

//         FileReaderAysncQueue();
//         auto push(std::filesystem::path&& file_path) -> void;
//         auto pop() -> std::vector<char>;
//     };

// }

// #if defined(_WIN32)

// auto Utily::FileReaderAysncQueue::push(std::filesystem::path&& file_path) -> void {
// }

// auto Utily::FileReaderAysncQueue::pop() -> std::vector<char> {
//     return { };
// }

// #elif defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))

// #elif defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)

// #endif

// namespace Utily {

//     class File
//     {
//         constexpr static size_t page_size = 4096; // 4KB

//         size_t _bytes_size;
//         size_t _bytes_read;
//         FILE* _stream;
//         std::array<char, page_size> _buffer;

//         void stop() {
//             if (_stream) {
//                 std::fclose(_stream);
//                 _stream = nullptr;
//             }
//         }
//         auto init(const std::filesystem::path& path) -> Utily::Result<void, Utily::Error> {
//             stop();

//             if (!std::filesystem::exists(path)) {
//                 return Utily::Error { "File does not exists." };
//             }
//             std::error_code error_code [[maybe_unused]];
//             const size_t file_size = std::filesystem::file_size(path, error_code);
//             if (error_code) {
//                 return Utily::Error { "Failed to read file size." };
//             }

//             if (_stream = std::fopen(path.string().c_str(), "r"); _stream)
//             {
//                 return Utily::Error { "Failed to open file. Probably in use." };
//             }

//             if (std::setvbuf(_stream, _buffer.data(), _IOFBF, page_size)) {
//                 std::fclose(_stream);
//                 _stream = nullptr;
//                 return Utily::Error { "Failed to set internal buffer. " };
//             }
//         }
//         auto read_page(char* ptr) {
//             bool has_failed = std::fgets(ptr, static_cast<int>(page_size), _stream) == nullptr;
//             if (has_failed) {
//             }
//         }

//         File()
//             : _bytes_size(0)
//             , _bytes_read(0)
//             , _stream(nullptr)
//             , _buffer {} { }

//         File(const File&) = delete;
//         File& operator=(const File&) = delete;

//         File(File&& other)
//             : _bytes_size(std::exchange(other._bytes_size, 0))
//             , _bytes_read(std::exchange(other._bytes_read, 0))
//             , _stream(std::exchange(other._stream, nullptr))
//             , _buffer(other._buffer) { }
//         File& operator=(File&& other) {
//             std::swap(_bytes_size, other._bytes_size);
//             std::swap(_bytes_read, other._bytes_read);
//             std::swap(_stream, other._stream);
//             std::swap(_buffer, other._buffer);
//         }

//         ~File() {
//             stop();
//         }
//     };

//     class FileReader
//     {
//         using Contents = std::vector<char>;

//         using ContentsIter = std::decay_t<decltype(Contents {}.begin())>;

//         struct StateUninitialised {
//         };

//         constexpr static size_t buffer_read_amount = 4096; // read 4KB per read_some.

//         struct StateAsyncReading {
//             Contents contents;
//             std::ifstream infile_stream;
//             ContentsIter iter;

//             StateAsyncReading(std::ifstream&& infile, std::ptrdiff_t n)
//                 : contents(n)
//                 , infile_stream(std::move(infile))
//                 , iter(contents.begin()) { }
//         };

//         struct StateComplete {
//             Contents contents;

//             StateComplete(StateAsyncReading&& sar)
//                 : contents(std::move(sar.contents)) {
//             }
//         };

//         std::variant<StateUninitialised, StateAsyncReading, StateComplete> _state;

//     public:
//         constexpr FileReader() noexcept
//             : _state(StateUninitialised {}) { }

//         auto init(std::filesystem::path path) noexcept -> Utily::Result<void, Utily::Error> {
//             if (!std::filesystem::exists(path)) {
//                 return Utily::Error { "File does not exists." };
//             }
//             std::error_code error_code [[maybe_unused]];
//             const size_t file_size = std::filesystem::file_size(path, error_code);

//             std::ifstream infile_stream { path, std::ios::binary };
//             if (!infile_stream.is_open()) {
//                 return Utily::Error { "File cannot be opened. Probably in use." };
//             }

//             // switch into reading.
//             _state = StateAsyncReading(std::move(infile_stream), file_size);

//             return {};
//         }

//         [[nodiscard]] bool is_done() const noexcept {
//             return std::holds_alternative<StateComplete>(_state);
//         }

//         [[nodiscard]] auto percent_complete() const noexcept -> float {
//             struct GetPercentComplete {
//                 float operator()(const StateUninitialised& s [[maybe_unused]]) const {
//                     return 0.0f;
//                 }
//                 float operator()(const StateAsyncReading& s) const {
//                     return static_cast<float>(std::distance(s.contents.begin(), Contents::const_iterator { s.iter }))
//                         / static_cast<float>(s.contents.size());
//                 }
//                 float operator()(const StateComplete& s [[maybe_unused]]) const {
//                     return 1.0f;
//                 }
//             };

//             return std::visit(GetPercentComplete {}, _state);
//         }

//     private:
//         void read_page_impl() noexcept {
//             auto& sar = std::get<StateAsyncReading>(_state);
//             assert(sar.infile_stream.is_open());
//             std::advance(sar.iter, sar.infile_stream.readsome(&(*sar.iter), buffer_read_amount));
//         }

//         bool is_done_impl() noexcept {
//             auto& sar = std::get<StateAsyncReading>(_state);
//             return sar.iter == sar.contents.end();
//         }

//         void change_state_to_completed() noexcept {
//             auto& sar = std::get<StateAsyncReading>(_state);
//             _state = StateComplete { std::move(sar) };
//         }

//     public:
//         void read_page() noexcept {
//             assert(std::holds_alternative<StateAsyncReading>(_state));
//             read_page_impl();
//             if (is_done_impl()) {
//                 change_state_to_completed();
//             }
//         }
//         void read_pages_for(std::chrono::duration<float> duration) noexcept {
//             assert(std::holds_alternative<StateAsyncReading>(_state));

//             const auto start_time = std::chrono::high_resolution_clock::now();

//             auto get_duration = [&]() {
//                 auto curr_time = std::chrono::high_resolution_clock::now();
//                 return std::chrono::duration_cast<std::chrono::duration<float>>(curr_time - start_time);
//             };

//             while (get_duration() < duration && !is_done_impl()) {
//                 read_page_impl();
//             }
//             if (is_done_impl()) {
//                 change_state_to_completed();
//             }
//         }

//         auto take_contents() -> Contents {
//             assert(std::holds_alternative<StateComplete>(_state));
//             auto contents = std::move(std::get<StateComplete>(_state).contents);
//             _state = StateUninitialised {};
//             return contents;
//         }
//     };
// }
*/

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

        static void CALLBACK fileIOCompletionRoutine(
            DWORD dwErrorCode,
            DWORD dwNumberOfBytesTransfered,
            LPOVERLAPPED lpOverlapped) {
            //std::cout << "Read " << dwNumberOfBytesTransfered << " bytes successfully." << std::endl;
        }

        struct FileDataUnfulfilled {
            FileHandle stream;
            std::vector<char> contents;
            OVERLAPPED overlapped;
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
            return {}; /*Utily::Error {
                std::format(
                    "File \"{}\" is already queued.",
                    file_path.string())
            };*/
        }

        if (!std::filesystem::exists(file_path)) {
            return Utily::Error {
                std::format(
                    "File \"{}\" does not exist.",
                    file_path.string())
            };
        }
        FileDataUnfulfilled file_data {};

        file_data.stream = CreateFile(
            file_path.string().c_str(),
            GENERIC_READ,
            FILE_SHARE_READ,
            nullptr,
            OPEN_EXISTING,
            FILE_FLAG_OVERLAPPED,
            nullptr);

        if (file_data.stream == INVALID_HANDLE_VALUE) {
            return Utily::Error {
                std::format(
                    "File \"{}\" was not able to be opened. But it does exist.",
                    file_path.string())
            };
        }

        if (LARGE_INTEGER temp; !GetFileSizeEx(file_data.stream, &temp)) {
            CloseHandle(file_data.stream);
            return Utily::Error {
                std::format(
                    "File \"{}\" did not allow us to read the file's size.",
                    file_path.string())
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
                    file_path.string())
            };
        }

        auto& [path, file_data_ref] = *iter;

        bool has_started_reading = ReadFileEx(
            file_data_ref.stream,
            file_data_ref.contents.data(),
            static_cast<DWORD>(file_data_ref.contents.size()),
            &file_data_ref.overlapped,
            nullptr);
            //AsyncFileReader::fileIOCompletionRoutine);

        if (!has_started_reading) {
            CloseHandle(file_data_ref.stream);
            files_unfulfilled.erase(path);
            return Utily::Error {
                std::format(
                    "File \"{}\" was opened but able to be read.",
                    file_path.string())
            };
        }

        // SleepEx(INFINITE, TRUE);

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