#include "Utily/FileReader.hpp"

#include <format>

#if defined(_WIN32)
#include <Windows.h>
#include <bitset>

namespace Utily {
    auto FileReader::load_entire_file(std::filesystem::path file_path)
        -> Utily::Result<std::vector<uint8_t>, Utily::Error> {

        const wchar_t* fp = file_path.c_str();

        void* handle = CreateFileW(
            fp,
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL);

        if (handle == INVALID_HANDLE_VALUE) {
            auto fp_string = file_path.string();
            return Utily::Error { std::format("The file {} does not exist.", fp_string) };
        }

        static_assert(sizeof(unsigned long) == sizeof(uint32_t));
        unsigned long size_low = 0;
        unsigned long size_high = 0;
        size_low = GetFileSize(handle, &size_high);
        const uint64_t size = ((std::bitset<64>(size_high) << 32) | std::bitset<64>(size_low)).to_ullong();

        std::vector<uint8_t> buffer(size);

        if (size > buffer.max_size()) [[unlikely]] {
            auto fp_string = file_path.string();
            return Utily::Error {
                std::format(
                    "Failed to read {} as the file's size ({}) cannot fit into a vector<uint8_t> ({})",
                    fp_string,
                    size,
                    buffer.max_size())
            };
        }

        // necessary to read stuff over 4GB
        for (uint64_t i = 0; i < size;) {
            unsigned long bytes_read = 0;
            unsigned long bytes_to_read = static_cast<unsigned long>(buffer.size() - i);

            bool is_good_read = ReadFile(
                handle,
                buffer.data() + i,
                bytes_to_read,
                &bytes_read,
                nullptr);

            i += bytes_read;

            if (!is_good_read || bytes_read == 0) [[unlikely]] {
                auto fp_string = file_path.string();
                return Utily::Error {
                    std::format(
                        "The file {} had a bad read. Need to read {} bytes, actually read {} bytes",
                        fp_string,
                        bytes_to_read,
                        bytes_read)
                };
            }
        }

        CloseHandle(handle);

        return buffer;
    }
}

#else
#warning "Platform is not optimised for file reading."

#include <cstdio>

namespace Utily {
    auto FileReader::load_entire_file(std::filesystem::path file_path)
        -> Utily::Result<std::vector<uint8_t>, Utily::Error> {

        auto fp = file_path.c_str();

        if (!std::filesystem::exists(file_path)) {
            return Utily::Error { std::format("The file {} does not exist.", fp) };
        }

        constexpr bool NeedsPathConversion = std::same_as<std::filesystem::path::value_type, char>;

        FILE* handle = nullptr;

        if constexpr (NeedsPathConversion) {
            std::string utf8 = file_path.string();
            handle = fopen(utf8.c_str(), "rb");
        } else {
            handle = fopen(fp, "rb");
        }

        if (handle == nullptr) {
            return Utily::Error { std::format("The file {} could not be opened.", fp) };
        }

        fseek(handle, 0, SEEK_END);
        size_t file_size = static_cast<size_t>(ftell(handle));
        fseek(handle, 0, SEEK_SET);

        std::vector<uint8_t> buffer(file_size);
        size_t bytes_read = fread(buffer.data(), sizeof(uint8_t), file_size, handle);

        if (bytes_read != file_size) {
            return Utily::Error { std::format("The file {} opened but failed whilst reading.", fp) };
        }
        fclose(handle);
        return buffer;
    }
}

#endif
