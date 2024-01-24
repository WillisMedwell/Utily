#include "Utily/FileReader.hpp"
#include <benchmark/benchmark.h>
#include <filesystem>
#include <fstream>

#if 1

const static auto STANFORD_BUNNY_PATH = std::filesystem::path { "resources/stanford_bunny.ply" };
const static auto SMALL_TEXT_PATH = std::filesystem::path { "resources/small.txt" };

static std::vector<uint8_t> readFileToVector(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Unable to open file: " + path.string());
    }

    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg); 

    std::vector<uint8_t> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        throw std::runtime_error("Error occurred while reading the file: " + path.string());
    }
    return buffer;
}

static void BM_Utily_FileReader(benchmark::State& state) {
    for (auto _ : state) {
        auto data = Utily::FileReader::load_entire_file(STANFORD_BUNNY_PATH);
        benchmark::DoNotOptimize(data);
    }
}
BENCHMARK(BM_Utily_FileReader);

static void BM_Std_FileReader(benchmark::State& state) {
    for (auto _ : state) {
        auto data = readFileToVector(STANFORD_BUNNY_PATH);
        benchmark::DoNotOptimize(data);
    }
}
BENCHMARK(BM_Std_FileReader);

#endif
