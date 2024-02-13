#pragma once

#include <fstream>
#include <iostream>
#include <filesystem>
#include <optional>
#include <chrono>

namespace mccinfo {
namespace file_readers {

inline bool IsLeapSecondsEnabled(void) {
    std::wstring regSubKey = L"SYSTEM\\CurrentControlSet\\Control\\LeapSecondInformation";
    std::wstring regValue(L"Enabled");
    DWORD val;
    DWORD dataSize = sizeof(val);
    if (ERROR_SUCCESS == RegGetValueW(HKEY_LOCAL_MACHINE, regSubKey.c_str(), regValue.c_str(),
                                      RRF_RT_DWORD, nullptr /*type not required*/, &val, &dataSize))
        return val == (DWORD)1;
    return false;
}

inline unsigned int GetSystemAccountedLeapSeconds(void) {
    if (IsLeapSecondsEnabled()) {
        std::wstring regSubKey = L"SYSTEM\\CurrentControlSet\\Control\\LeapSecondInformation";
        std::wstring regValue(L"LeapSeconds");
        DWORD val = 0;
        DWORD dataSize = sizeof(val);
        if (ERROR_SUCCESS == RegGetValueW(HKEY_LOCAL_MACHINE, regSubKey.c_str(), regValue.c_str(),
                                          RRF_RT_REG_BINARY, nullptr /*type not required*/, &val,
                                          &dataSize))
            return val;
    }
    return 0;
}

inline std::optional<std::filesystem::file_time_type> GetTheaterFileTimestamp(
    const std::filesystem::path &theater_file) {
    static const unsigned long long timestamp_offset = 0x00000068; // read at least 128 bytes?
    static const unsigned long long duration_offset = 0x00000118;  // read two bytes

    // this monstrosity believe it or not works.
    // will refactor some other time that is explicitly not now
    try {
        if (std::filesystem::is_regular_file(theater_file)) {
            std::ifstream ifs;
            ifs.open(theater_file, std::ios::binary);
            ifs.seekg(duration_offset);

            char buffer[3];
            buffer[2] = '\0';
            ifs.read(&buffer[0], 1);
            ifs.read(&buffer[1], 1);
            char t = buffer[0];
            buffer[0] = buffer[1];
            buffer[1] = t;
            unsigned char fixed[3];
            fixed[2] = '\0';
            if (0 > buffer[0]) {
                fixed[0] = 128 + abs((-128 + (-1 * buffer[0])));
            } else
                fixed[0] = buffer[0];
            if (0 > buffer[1]) {
                fixed[1] = 128 + abs((-128 + (-1 * buffer[1])));
            } else
                fixed[1] = buffer[1];

            std::ostringstream ret{};

            for (size_t i = 0; i < 2; ++i)
                ret << std::hex << std::setfill('0') << std::setw(2) << (int)fixed[i];

            std::cout << "fixed[0]: " << (int)fixed[0] << std::endl;
            std::cout << "fixed[1]: " << (int)fixed[1] << std::endl;
            auto match_length = std::chrono::seconds(strtoul(ret.str().c_str(), NULL, 16));

            ifs.seekg(timestamp_offset);
            char buffer2[128];
            ifs.read(buffer2, 128);
            ifs.close();

            std::tm tm = {};
            std::string dirty(buffer2, 128);

            dirty = dirty.substr(dirty.find(','));
            dirty.replace(0, 2, "");

            std::istringstream ss(dirty);

            ss >> std::get_time(&tm, "%A %B %d, %Y %H:%M:%S");
            if (ss.fail())
                return std::nullopt;

            std::time_t local_time = mktime(&tm);
            int dst = 0;
            _get_daylight(&dst);
            std::cout << "match length: " << match_length << std::endl;
            std::chrono::seconds dst_offset{dst * 3600};
            std::chrono::utc_time<std::chrono::seconds> utc(
                std::chrono::seconds(local_time) + match_length +
                (std::filesystem::_File_time_clock::_Skipped_filetime_leap_seconds -
                 std::chrono::seconds(GetSystemAccountedLeapSeconds())) -
                dst_offset);

            return std::chrono::file_clock::from_utc(utc);
        }
    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    }
    return std::nullopt;
}
inline std::optional<std::string> GetTheaterFileXUID(
    const std::filesystem::path &theater_file) {
    try {
        if (std::filesystem::is_regular_file(theater_file)) {
            static const unsigned long long xuid_offset = 0x00000100;
            std::ifstream ifs;
            ifs.open(theater_file, std::ios::binary);
            ifs.seekg(xuid_offset);
            std::vector<char> buffer;
            buffer.resize(8);

            size_t i = 0;
            for (auto &b : buffer) {
                ifs.read(&buffer.data()[i], 1);
                ++i;
            }

            std::reverse(buffer.begin(), buffer.end());

            std::ostringstream ret{};
            for (const auto &b : buffer) {
                ret << std::hex << std::setfill('0') << std::setw(2) << (int)((unsigned char)b);
            }
            auto tu = [](std::string s) -> std::string {
                for (auto &c : s)
                    c = toupper(c);
                return s;
            };
            std::string xuid("0x" + tu(ret.str()));
            return xuid;
        }
    }
    catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    }
    return "";
}

inline std::optional<std::string> GetTheaterFileGameType(
    const std::filesystem::path &theater_file) {
    try {
        if (std::filesystem::is_regular_file(theater_file)) {
            static const unsigned long long desc_offset = 0x00000048;

            std::ifstream ifs;
            ifs.open(theater_file, std::ios::binary);
            ifs.seekg(desc_offset);
            std::vector<char> buffer;
            buffer.resize(184);

            ifs.read(&buffer.data()[0], 184);
            ifs.close();

            std::wstring wstr(reinterpret_cast<const wchar_t *>(buffer.data()), buffer.size());

            auto bytes = utility::ConvertWStringToBytes(wstr);
            if (bytes.has_value()) {
                return bytes.value();
            }

        }
    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    }
    return "";
}

inline std::optional<std::string> GetTheaterFileGameDescription(
    const std::filesystem::path &theater_file) {
    try {
        if (std::filesystem::is_regular_file(theater_file)) {
            static const unsigned long long desc_offset = 0x00000048;

            std::ifstream ifs;
            ifs.open(theater_file, std::ios::binary);
            ifs.seekg(desc_offset);

            bool offset_found = false;
            unsigned long long rel_offset = 0x0;
            while (!offset_found) {
                std::vector<char> tbuffer;
                tbuffer.resize(2);
                ifs.read(&tbuffer.data()[0], 2);
                rel_offset += 2;
                if (tbuffer[0] == '\0') {
                    offset_found = true;
                }
            }


            unsigned long long desc_length = 184 - rel_offset;
            std::vector<char> buffer;
            buffer.resize(desc_length);

            ifs.read(&buffer.data()[0], desc_length);
            ifs.close();

            std::string str(buffer.data());
            return str;
        }
    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    }
    return "";
}

struct theater_file {
    std::string xuid_;
    std::filesystem::file_time_type utc_timestamp_;
};

}
}