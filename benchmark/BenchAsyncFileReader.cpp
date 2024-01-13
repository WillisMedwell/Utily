#include "Utily/Utily.hpp"
#include <benchmark/benchmark.h>

#include <cstdint>
#include <ranges>

const static auto STANFORD_BUNNY_PATH = std::filesystem::path { "resources/stanford_bunny.ply" };
const static auto SMALL_TEXT_PATH = std::filesystem::path { "resources/small.txt" };

static void BM_Utily_AsyncFileReader(benchmark::State& state) {
    bool [[maybe_unused]] has_errored = false;
    for (auto _ : state) {

        Utily::AsyncFileReader::push(STANFORD_BUNNY_PATH).on_error([](auto& e) { std::cerr << e.what(); });
        Utily::AsyncFileReader::push(SMALL_TEXT_PATH).on_error([](auto& e) { std::cerr << e.what(); });

        Utily::AsyncFileReader::wait_for_all();

        auto a [[maybe_unused]] = Utily::AsyncFileReader::pop(STANFORD_BUNNY_PATH);
        auto b [[maybe_unused]] = Utily::AsyncFileReader::pop(SMALL_TEXT_PATH);
    }
}
BENCHMARK(BM_Utily_AsyncFileReader);

static void BM_Std_FileReader(benchmark::State& state) {
    bool has_errored = false;
    for (auto _ : state) {
        auto get_contents = [&](std::filesystem::path file_name) -> std::vector<char> {
            std::ifstream file(file_name, std::ios::binary);

            file.seekg(0, std::ios::end);
            size_t fileSize = file.tellg();

            file.seekg(0, std::ios::beg);

            std::vector<char> buffer;
            buffer.reserve(fileSize);
            buffer.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

            return buffer;
        };

        auto a [[maybe_unused]] = std::vector<char> { get_contents(STANFORD_BUNNY_PATH) };
        auto b [[maybe_unused]] = std::vector<char> { get_contents(SMALL_TEXT_PATH) };

        benchmark::DoNotOptimize(a);
        benchmark::DoNotOptimize(b);

        /*for (auto i = 0; i < UINT16_MAX; ++i) {
            benchmark::DoNotOptimize(i);
        }*/
    }
}
BENCHMARK(BM_Std_FileReader);

#if defined(_WIN32)

#include <windows.h>

static void BM_Win32_FileReader(benchmark::State& state) {
    bool has_errored = false;
    for (auto _ : state) {
        auto get_contents = [&](std::filesystem::path file_name) -> std::vector<char> {
            // Open the file
            HANDLE hFile = CreateFile(file_name.string().c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFile == INVALID_HANDLE_VALUE) {
                std::cerr << "Error opening file: " << GetLastError() << std::endl;
                return {};
            }

            // Get the file size
            LARGE_INTEGER fileSize;
            if (!GetFileSizeEx(hFile, &fileSize)) {
                std::cerr << "Error getting file size: " << GetLastError() << std::endl;
                CloseHandle(hFile);
                return {};
            }

            // Create a vector of the appropriate size
            std::vector<char> buffer(static_cast<size_t>(fileSize.QuadPart));

            // Read the file into the vector
            DWORD bytesRead;
            if (!ReadFile(hFile, buffer.data(), static_cast<DWORD>(fileSize.QuadPart), &bytesRead, NULL) || static_cast<LONGLONG>(bytesRead) != fileSize.QuadPart) {
                std::cerr << "Error reading file: " << GetLastError() << std::endl;
                CloseHandle(hFile);
                return {};
            }

            // Close the file handle
            CloseHandle(hFile);

            return buffer;
        };

        auto a [[maybe_unused]] = std::vector<char> { get_contents(STANFORD_BUNNY_PATH) };
        auto b [[maybe_unused]] = std::vector<char> { get_contents(SMALL_TEXT_PATH) };

        benchmark::DoNotOptimize(a);
        benchmark::DoNotOptimize(b);
    }
}
BENCHMARK(BM_Win32_FileReader);

static void BM_Win32_FileMappingReader(benchmark::State& state) {
    bool has_errored = false;
    for (auto _ : state) {
        auto get_contents = [&](std::filesystem::path file_path) -> std::vector<char> {
            // Open the file
        HANDLE h_file = CreateFile(file_path.string().c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (h_file == INVALID_HANDLE_VALUE) {
            std::cerr << "Error opening file: " << GetLastError() << std::endl;
            return {};
        }

        // Create a file mapping object
        HANDLE h_map_file = CreateFileMapping(h_file, NULL, PAGE_READONLY, 0, 0, NULL);
        if (h_map_file == NULL) {
            std::cerr << "Error creating file mapping: " << GetLastError() << std::endl;
            CloseHandle(h_file);
            return {};
        }

        // Map a view of the file into the address space of the calling process
        LPVOID lp_map_address = MapViewOfFile(h_map_file, FILE_MAP_READ, 0, 0, 0);
        if (lp_map_address == NULL) {
            std::cerr << "Error mapping view of file: " << GetLastError() << std::endl;
            CloseHandle(h_map_file);
            CloseHandle(h_file);
            return {};
        }

        // Get file size
        LARGE_INTEGER file_size;
        if (!GetFileSizeEx(h_file, &file_size)) {
            std::cerr << "Error getting file size: " << GetLastError() << std::endl;
            UnmapViewOfFile(lp_map_address);
            CloseHandle(h_map_file);
            CloseHandle(h_file);
            return {};
        }

        // Read data from the mapped view
        std::vector<char> file_contents(static_cast<char*>(lp_map_address), static_cast<char*>(lp_map_address) + file_size.QuadPart);

        // Unmap the file view and close the handles
        UnmapViewOfFile(lp_map_address);
        CloseHandle(h_map_file);
        CloseHandle(h_file);

        return file_contents;
        };

        auto a [[maybe_unused]] = std::vector<char> { get_contents(STANFORD_BUNNY_PATH) };
        auto b [[maybe_unused]] = std::vector<char> { get_contents(SMALL_TEXT_PATH) };

        benchmark::DoNotOptimize(a);
        benchmark::DoNotOptimize(b);
    }
}
BENCHMARK(BM_Win32_FileMappingReader);

#endif