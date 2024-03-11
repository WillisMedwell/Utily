#include "Utily/FileWriter.hpp"

#include <format>

#if defined(_WIN32)
#include <Windows.h>
#include <bitset>

namespace Utily {
    auto FileWriter::dump_to_file(std::filesystem::path file_path, std::span<const uint8_t> data)
        -> Utily::Result<void, Utily::Error> {

        const wchar_t* fp = file_path.c_str();

        void* handle = CreateFileW(
            fp,
            GENERIC_WRITE,
            FILE_SHARE_READ,
            NULL,
            OPEN_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL);

        if (handle == INVALID_HANDLE_VALUE) {
            auto fp_string = file_path.string();
            return Utily::Error { std::format("The file {} could not be created or opened.", fp_string) };
        }

        DWORD bytes_written = 0;

        auto has_wrote_file = WriteFile(
            handle,
            reinterpret_cast<const void*>(data.data()),
            static_cast<unsigned long>(data.size_bytes()),
            &bytes_written,
            nullptr);

        CloseHandle(handle);

        if (!has_wrote_file || bytes_written != data.size_bytes()) {
            auto fp_string = file_path.string();
            return Utily::Error { std::format("The file {} could not write all the data. Only {}/{} bytes written.", fp_string, bytes_written, data.size_bytes()) };
        }

        return {};
    }
}

#else

#include <cstdio>

namespace Utily {
    auto FileWriter::dump_to_file(std::filesystem::path file_path, std::span<const uint8_t> data)
        -> Utily::Result<void, Utily::Error> {

        const auto fp = file_path.c_str();

        FILE* handle = nullptr;
        const char* fp_u8 = nullptr;

        if constexpr (std::same_as<const char*, decltype(fp)>) {
            fp_u8 = reinterpret_cast<const char*>(fp);
            handle = fopen(fp_u8, "wb");
        } else if constexpr(!std::same_as<const int8_t*, decltype(fp)>) {
            const std::string str = file_path.string();
            fp_u8 = str.c_str();
            handle = fopen(fp_u8, "wb");
        }

        if (!handle) {
            return Utily::Error { std::format("The file {} could not be opened/created.", file_path.string()) };
        }

        fwrite(data.data(), 1, data.size_bytes(), handle);

        fclose(handle);
        return {};
    }
}

#endif
