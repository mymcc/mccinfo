#pragma once

#include <fstream>
#include <iostream>
#include <filesystem>
#include <optional>
#include <chrono>
#include <set>

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



using player_info = std::pair<int, std::string>;

struct theater_file_data {
    std::string gametype_;
    std::string desc_;
    std::string author_xuid_;
    std::string author_;
    std::set<player_info> player_set_;
    std::filesystem::file_time_type utc_timestamp_;
};

class theater_file_reader {
  public:
    std::optional<theater_file_data> Read(const std::filesystem::path &theater_file) {
        try {
            // should be openable and of explicit size
            if (std::filesystem::is_regular_file(theater_file)) {
                std::ifstream ifs;
                ifs.open(theater_file, std::ios::binary);

                return this->ReadImpl(theater_file, ifs);
            }
        }

        catch (std::exception &e) {
            std::cout << e.what() << std::endl;
        }
        
        return std::nullopt;
    }

  private:
    virtual std::optional<theater_file_data> ReadImpl(
        const std::filesystem::path &theater_file,
        std::ifstream &ifs) = 0;
    virtual std::optional<std::string> ReadTheaterFileGameType(std::ifstream &ifs) = 0;
    virtual std::optional<std::string> ReadTheaterFileGameDescription(std::ifstream &ifs) = 0;
    virtual std::optional<std::string> ReadTheaterFileAuthorXUID(std::ifstream &ifs) = 0;
    virtual std::optional<std::string> ReadTheaterFileAuthor(std::ifstream &ifs) = 0;
    virtual std::optional<std::filesystem::file_time_type> ReadTheaterFileUTCTimestamp(
        std::ifstream &ifs) = 0;
    virtual std::optional<std::set<player_info>> ReadTheaterFilePlayerSet(std::ifstream &ifs) = 0;
};

class halo3_theater_file_reader : public theater_file_reader {
  private:
    virtual std::optional<theater_file_data> ReadImpl(const std::filesystem::path &theater_file,
                                                      std::ifstream &ifs) override final {
        theater_file_data file_data;

        auto gametype = this->ReadTheaterFileGameType(ifs);
        if (gametype.has_value()) {
            file_data.gametype_ = gametype.value();
        }

        auto desc = this->ReadTheaterFileGameDescription(ifs);
        if (desc.has_value()) {
            file_data.desc_ = desc.value();
        }

        auto xuid = this->ReadTheaterFileAuthorXUID(ifs);
        if (xuid.has_value()) {
            file_data.author_xuid_ = xuid.value();
        }

        auto utc_timestamp = this->ReadTheaterFileUTCTimestamp(ifs);
        if (utc_timestamp.has_value()) {
            file_data.utc_timestamp_ = utc_timestamp.value();
        }

        auto player_set = this->ReadTheaterFilePlayerSet(ifs);
        if (player_set.has_value()) {
            file_data.player_set_ = player_set.value();
        }

        ifs.close();
        return file_data;
        // if GameType == "", then Discern if its campaign or firefight
        // could split this into halo3_odst_theater_file_data
    }
    virtual std::optional<std::string> ReadTheaterFileGameType(std::ifstream &ifs) override final {
        static const unsigned long long desc_offset = 0x00000048;

        ifs.seekg(0);
        ifs.seekg(desc_offset);

        std::vector<char> buffer;
        buffer.resize(184);

        ifs.read(&buffer.data()[0], 184);

        std::wstring wstr(reinterpret_cast<const wchar_t *>(buffer.data()));

        auto bytes = utility::ConvertWStringToBytes(wstr);
        if (bytes.has_value()) {
            return bytes.value();
        }

        return std::nullopt;
    }
    virtual std::optional<std::string> ReadTheaterFileGameDescription(std::ifstream &ifs) override final {
        static const unsigned long long desc_offset = 0x00000048;

        ifs.seekg(0);
        ifs.seekg(desc_offset);

        bool offset_found = false;
        bool gap_found = false;
        unsigned long long rel_offset = 0x0;
        while (!offset_found) {
            std::vector<char> tbuffer;
            tbuffer.resize(2);
            ifs.read(&tbuffer.data()[0], 2);
            rel_offset += 2;
            if ((!gap_found) && (tbuffer[0] == '\0')) {
                gap_found = true;
            }
            if ((gap_found) && (tbuffer[0] != '\0')) {
                rel_offset -= 2;
                ifs.seekg(-2, std::ios_base::cur);
                offset_found = true;
            }
        }

        unsigned long long desc_length = 184 - rel_offset;
        std::vector<char> buffer;
        buffer.resize(desc_length);

        ifs.read(&buffer.data()[0], desc_length);

        std::string str(buffer.data());
        return str;
    }
    virtual std::optional<std::string> ReadTheaterFileAuthorXUID(std::ifstream &ifs) override final {
        static const unsigned long long xuid_offset = 0x00000100;

        ifs.seekg(0);
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
    virtual std::optional<std::string> ReadTheaterFileAuthor(std::ifstream &ifs) override final {
        return "";
    }
    virtual std::optional<std::filesystem::file_time_type> ReadTheaterFileUTCTimestamp(
        std::ifstream &ifs) override final {
        static const unsigned long long timestamp_offset = 0x00000068; // read at least 128 bytes?
        static const unsigned long long duration_offset = 0x00000118;  // read two bytes

        ifs.seekg(0);
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

        // std::cout << "fixed[0]: " << (int)fixed[0] << std::endl;
        // std::cout << "fixed[1]: " << (int)fixed[1] << std::endl;
        auto match_length = std::chrono::seconds(strtoul(ret.str().c_str(), NULL, 16));

        ifs.seekg(timestamp_offset);
        char buffer2[128];
        ifs.read(buffer2, 128);
        // ifs.close();

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
        // std::cout << "match length: " << match_length << std::endl;
        std::chrono::seconds dst_offset{dst * 3600};
        std::chrono::utc_time<std::chrono::seconds> utc(
            std::chrono::seconds(local_time) + match_length +
            (std::filesystem::_File_time_clock::_Skipped_filetime_leap_seconds -
             std::chrono::seconds(GetSystemAccountedLeapSeconds())) -
            dst_offset);

        return std::chrono::file_clock::from_utc(utc);
    }
    virtual std::optional<std::set<player_info>> ReadTheaterFilePlayerSet(
        std::ifstream &ifs) override final {
        static const unsigned long long head_offset = 0x000001D8;

        ifs.seekg(0);
        ifs.seekg(head_offset);

        bool player_set_offset_found = false;
        unsigned int ones_found = 0;

        while (!player_set_offset_found) {
            bool found_one_this_it = false;

            char buf[1];
            ifs.read(&buf[0], 1);
            if (buf[0] == '\1') {
                ++ones_found;
                found_one_this_it = true;
            }
            // check if the last 4 bytes, 11 bytes ahead, have the pattern:
            // {not zero, not zero, zero, zero}

            if ((ones_found >= 3) && found_one_this_it) {
                char buf2[11];
                ifs.read(&buf2[0], 11);
                if ((buf2[7] != '\0') && (buf2[8] != '\0') && (buf2[9] == '\0') &&
                    (buf2[10] == '\0')) {
                    ifs.seekg(12, std::ios_base::cur);
                    player_set_offset_found = true;
                } else {
                    // go back
                    ifs.seekg(-11, std::ios_base::cur);
                }
            }
        }

        std::set<player_info> player_set;
        int empty_region_count = 0;
        while (true) {
            // look at current byte, no more players if \0

            bool current_is_empty_region = true;
            char buf[1];
            ifs.read(&buf[0], 1);
            if (buf[0] == '\0')
            {
                ++empty_region_count;
                if (empty_region_count >= 5) {
                    break;
                }
            } else {
                empty_region_count = 0;
                current_is_empty_region = false;
            }

            // reset
            ifs.seekg(-1, std::ios_base::cur);
            if (!current_is_empty_region) {
                // read player name
                std::vector<char> buffer;
                buffer.resize(32);

                ifs.read(&buffer.data()[0], 32);

                std::wstring wstr(reinterpret_cast<const wchar_t *>(buffer.data()));

                bool success = false;
                auto bytes = utility::ConvertWStringToBytes(wstr);
                if (bytes.has_value()) {
                
                    success = true;
                }

                char team_buf[1];
                ifs.seekg(120, std::ios_base::cur);
                ifs.read(&team_buf[0], 1);
                ifs.seekg(-1, std::ios_base::cur);
                ifs.seekg(-120, std::ios_base::cur);

                if (success) {
                    player_set.insert({team_buf[0], bytes.value()});
                }


                // skip to next player
                ifs.seekg(184 - 32, std::ios_base::cur);
            } else 
            {
                ifs.seekg(184, std::ios_base::cur);
            }
        }

        return player_set;
    }
};

class haloreach_theater_file_reader : public theater_file_reader {
  private:
    virtual std::optional<theater_file_data> ReadImpl(const std::filesystem::path &theater_file,
                                                      std::ifstream &ifs) override final {
        theater_file_data file_data;
        
        auto gametype = this->ReadTheaterFileGameType(ifs);
        if (gametype.has_value()) {
            file_data.gametype_ = gametype.value();
        }

        auto desc = this->ReadTheaterFileGameDescription(ifs);
        if (desc.has_value()) {
            file_data.desc_ = desc.value();
        }

        auto author = this->ReadTheaterFileAuthor(ifs);
        if (author.has_value()) {
            file_data.author_ = author.value();
            file_data.player_set_.insert({-1, file_data.author_});
        }

        auto xuid = this->ReadTheaterFileAuthorXUID(ifs);
        if (xuid.has_value()) {
            file_data.author_xuid_ = xuid.value();
        }

        /*
        auto utc_timestamp = this->ReadTheaterFileUTCTimestamp(ifs);
        if (utc_timestamp.has_value()) {
            file_data.utc_timestamp_ = utc_timestamp.value();
        }
        */

        auto player_set = this->ReadTheaterFilePlayerSet(ifs);
        if (player_set.has_value()) {
            file_data.player_set_.insert(player_set.value().begin(), player_set.value().end());
        }

        ifs.close();
        return file_data;
        // if GameType == "", then Discern if its campaign or firefight
        // could split this into halo3_odst_theater_file_data
    }
    virtual std::optional<std::string> ReadTheaterFileGameType(std::ifstream &ifs) override final {
        static const unsigned long long gametype_offset = 0x000000C0;

        ifs.seekg(0);
        ifs.seekg(gametype_offset);

        std::vector<char> buffer;
        buffer.resize(256);

        ifs.read(&buffer.data()[0], 256);

        std::wstring wstr(reinterpret_cast<const wchar_t *>(buffer.data()));

        auto bytes = utility::ConvertWStringToBytes(wstr);
        if (bytes.has_value()) {
            return bytes.value();
        }

        return std::nullopt;
    }
    virtual std::optional<std::string> ReadTheaterFileGameDescription(
        std::ifstream &ifs) override final {
        static const unsigned long long desc_offset = 0x000001C0;

        ifs.seekg(0);
        ifs.seekg(desc_offset);

        unsigned long long desc_length = 256;
        std::vector<char> buffer;
        buffer.resize(desc_length);

        ifs.read(&buffer.data()[0], desc_length);

        std::wstring wstr(reinterpret_cast<const wchar_t *>(buffer.data()), buffer.size());

        auto bytes = utility::ConvertWStringToBytes(wstr);
        if (bytes.has_value()) {
            return bytes.value();
        }

        return std::nullopt;
    }
    virtual std::optional<std::string> ReadTheaterFileAuthorXUID(
        std::ifstream &ifs) override final {
        static const unsigned long long xuid_offset = 0x00000080;

        ifs.seekg(0);
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
    virtual std::optional<std::string> ReadTheaterFileAuthor(std::ifstream &ifs) override final {
        static const unsigned long long author_offset = 0x00000088;

        ifs.seekg(0);
        ifs.seekg(author_offset);
        std::vector<char> buffer;
        buffer.resize(16);

        ifs.read(&buffer.data()[0], 16);
        
        std::string str(buffer.data());
        return str;
    }
    virtual std::optional<std::filesystem::file_time_type> ReadTheaterFileUTCTimestamp(
        std::ifstream &ifs) override final {
        
        static const unsigned long long timestamp_offset = 0x00000068; // read at least 128 bytes?
        static const unsigned long long duration_offset = 0x00000118;  // read two bytes

        ifs.seekg(0);
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

        // std::cout << "fixed[0]: " << (int)fixed[0] << std::endl;
        // std::cout << "fixed[1]: " << (int)fixed[1] << std::endl;
        auto match_length = std::chrono::seconds(strtoul(ret.str().c_str(), NULL, 16));

        ifs.seekg(timestamp_offset);
        char buffer2[128];
        ifs.read(buffer2, 128);
        // ifs.close();

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
        // std::cout << "match length: " << match_length << std::endl;
        std::chrono::seconds dst_offset{dst * 3600};
        std::chrono::utc_time<std::chrono::seconds> utc(
            std::chrono::seconds(local_time) + match_length +
            (std::filesystem::_File_time_clock::_Skipped_filetime_leap_seconds -
             std::chrono::seconds(GetSystemAccountedLeapSeconds())) -
            dst_offset);

        return std::chrono::file_clock::from_utc(utc);

    }
    virtual std::optional<std::set<player_info>> ReadTheaterFilePlayerSet(
        std::ifstream &ifs) override final {

        
        static const unsigned long long head_offset = 0x00000BD0;

        ifs.seekg(0);
        ifs.seekg(head_offset);

        std::set<player_info> player_set;

        int empty_region_count = 0;
        while (true) {
            // look at current byte, no more players if \0
            bool current_is_empty_region = true;
            char buf[1];
            ifs.read(&buf[0], 1);
            if (buf[0] == '\0') {
                ++empty_region_count;
                if (empty_region_count >= 2) {
                    break;
                }
            } else {
                empty_region_count = 0;
                current_is_empty_region = false;
            }

            // reset
            ifs.seekg(-1, std::ios_base::cur);
            
            if (!current_is_empty_region) {
                // read player name
                std::vector<char> buffer;
                buffer.resize(32);

                ifs.read(&buffer.data()[0], 32);

                std::wstring wstr(reinterpret_cast<const wchar_t *>(buffer.data()));
                
                bool success = false;
                auto bytes = utility::ConvertWStringToBytes(wstr);
                if (bytes.has_value()) {
                    
                    success = true;
                }
                char team_buf[1];
                ifs.seekg(88, std::ios_base::cur);
                ifs.read(&team_buf[0], 1);
                ifs.seekg(-1, std::ios_base::cur);
                ifs.seekg(-88, std::ios_base::cur);

                if (success) {
                    player_set.insert({team_buf[0], bytes.value()});
                }

                ifs.seekg(160 - 32, std::ios_base::cur);
            } else {
                ifs.seekg(160, std::ios_base::cur);
            }

            // skip to next player
        }


        return player_set;
    }
};

class halo4_theater_file_reader : public theater_file_reader {
  private:
    virtual std::optional<theater_file_data> ReadImpl(const std::filesystem::path &theater_file,
                                                      std::ifstream &ifs) override final {
        theater_file_data file_data;
        
        if (theater_file.generic_string().find("campaign") != std::string::npos) {
            file_data.gametype_ = "Campaign";
        } 
        else {
            auto gametype = this->ReadTheaterFileGameType(ifs);
            if (gametype.has_value()) {
                file_data.gametype_ = gametype.value();
            }
        }

        auto desc = this->ReadTheaterFileGameDescription(ifs);
        if (desc.has_value()) {
            file_data.desc_ = desc.value();
        }

        auto author = this->ReadTheaterFileAuthor(ifs);
        if (author.has_value()) {
            file_data.author_ = author.value();
            file_data.player_set_.insert({-1, file_data.author_});
        }

        auto xuid = this->ReadTheaterFileAuthorXUID(ifs);
        if (xuid.has_value()) {
            file_data.author_xuid_ = xuid.value();
        }

        /*
        auto utc_timestamp = this->ReadTheaterFileUTCTimestamp(ifs);
        if (utc_timestamp.has_value()) {
            file_data.utc_timestamp_ = utc_timestamp.value();
        }
        */

        auto player_set = this->ReadTheaterFilePlayerSet(ifs);
        if (player_set.has_value()) {
            file_data.player_set_.insert(player_set.value().begin(), player_set.value().end());
        }

        ifs.close();
        return file_data;
        // if GameType == "", then Discern if its campaign or firefight
        // could split this into halo3_odst_theater_file_data
    }
    virtual std::optional<std::string> ReadTheaterFileGameType(std::ifstream &ifs) override final {
        static const unsigned long long gametype_offset = 0x000000C0;

        ifs.seekg(0);
        ifs.seekg(gametype_offset);

        std::vector<char> buffer;
        buffer.resize(256);

        ifs.read(&buffer.data()[0], 256);

        std::wstring wstr(reinterpret_cast<const wchar_t *>(buffer.data()));

        auto bytes = utility::ConvertWStringToBytes(wstr);
        if (bytes.has_value()) {
            return bytes.value();
        }

        return std::nullopt;
    }
    virtual std::optional<std::string> ReadTheaterFileGameDescription(
        std::ifstream &ifs) override final {
        static const unsigned long long desc_offset = 0x000001C0;

        ifs.seekg(0);
        ifs.seekg(desc_offset);

        unsigned long long desc_length = 256;
        std::vector<char> buffer;
        buffer.resize(desc_length);

        ifs.read(&buffer.data()[0], desc_length);

        std::wstring wstr(reinterpret_cast<const wchar_t *>(buffer.data()), buffer.size());

        auto bytes = utility::ConvertWStringToBytes(wstr);
        if (bytes.has_value()) {
            return bytes.value();
        }

        return std::nullopt;
    }
    virtual std::optional<std::string> ReadTheaterFileAuthorXUID(
        std::ifstream &ifs) override final {
        static const unsigned long long xuid_offset = 0x00000080;

        ifs.seekg(0);
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
    virtual std::optional<std::string> ReadTheaterFileAuthor(std::ifstream &ifs) override final {
        static const unsigned long long author_offset = 0x00000088;

        ifs.seekg(0);
        ifs.seekg(author_offset);
        std::vector<char> buffer;
        buffer.resize(16);

        ifs.read(&buffer.data()[0], 16);

        std::string str(buffer.data());
        return str;
    }
    virtual std::optional<std::filesystem::file_time_type> ReadTheaterFileUTCTimestamp(
        std::ifstream &ifs) override final {

        static const unsigned long long timestamp_offset = 0x00000068; // read at least 128 bytes?
        static const unsigned long long duration_offset = 0x00000118;  // read two bytes

        ifs.seekg(0);
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

        // std::cout << "fixed[0]: " << (int)fixed[0] << std::endl;
        // std::cout << "fixed[1]: " << (int)fixed[1] << std::endl;
        auto match_length = std::chrono::seconds(strtoul(ret.str().c_str(), NULL, 16));

        ifs.seekg(timestamp_offset);
        char buffer2[128];
        ifs.read(buffer2, 128);
        // ifs.close();

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
        // std::cout << "match length: " << match_length << std::endl;
        std::chrono::seconds dst_offset{dst * 3600};
        std::chrono::utc_time<std::chrono::seconds> utc(
            std::chrono::seconds(local_time) + match_length +
            (std::filesystem::_File_time_clock::_Skipped_filetime_leap_seconds -
             std::chrono::seconds(GetSystemAccountedLeapSeconds())) -
            dst_offset);

        return std::chrono::file_clock::from_utc(utc);
    }

    virtual std::optional<std::set<player_info>> ReadTheaterFilePlayerSet(
        std::ifstream &ifs) override final {

        static const unsigned long long head_offset = 0x0002C3C0;

        ifs.seekg(0);
        ifs.seekg(head_offset);

        std::set<player_info> player_set;

        int empty_region_count = 0;
        while (player_set.size() < 16) {
            bool current_is_empty_region = true;
            char buf[1];
            ifs.read(&buf[0], 1);
            if (buf[0] == '\0') {
                ++empty_region_count;
                if (empty_region_count >= 2) {
                    break;
                }
            } else {
                empty_region_count = 0;
                current_is_empty_region = false;
            }

            // reset
            ifs.seekg(-1, std::ios_base::cur);

            if (!current_is_empty_region) {
                // read player name
                std::vector<char> buffer;
                buffer.resize(32);

                ifs.read(&buffer.data()[0], 32);

                std::wstring wstr(reinterpret_cast<const wchar_t *>(buffer.data()));

                bool success = false;
                auto bytes = utility::ConvertWStringToBytes(wstr);
                if (bytes.has_value()) {
                    success = true;
                }
                char team_buf[1];
                // go to team
                ifs.seekg(328 - 72, std::ios_base::cur);
                ifs.read(&team_buf[0], 1);

                if (success) {
                    player_set.insert({team_buf[0], bytes.value()});
                }

                ifs.seekg(-1, std::ios_base::cur);
                ifs.seekg(40, std::ios_base::cur);

            } else {
                ifs.seekg(328, std::ios_base::cur);
            }

            // skip to next player
        }

        return player_set;
    }
};

class halo2a_theater_file_reader : public theater_file_reader {
  private:
    virtual std::optional<theater_file_data> ReadImpl(const std::filesystem::path &theater_file,
                                                      std::ifstream &ifs) override final {
        theater_file_data file_data;

        // No film for campaign
        auto gametype = this->ReadTheaterFileGameType(ifs);
        if (gametype.has_value()) {
            file_data.gametype_ = gametype.value();
        }

        auto desc = this->ReadTheaterFileGameDescription(ifs);
        if (desc.has_value()) {
            file_data.desc_ = desc.value();
        }

        auto author = this->ReadTheaterFileAuthor(ifs);
        if (author.has_value()) {
            file_data.author_ = author.value();
            file_data.player_set_.insert({-1, file_data.author_});
        }

        auto xuid = this->ReadTheaterFileAuthorXUID(ifs);
        if (xuid.has_value()) {
            file_data.author_xuid_ = xuid.value();
        }

        /*
        auto utc_timestamp = this->ReadTheaterFileUTCTimestamp(ifs);
        if (utc_timestamp.has_value()) {
            file_data.utc_timestamp_ = utc_timestamp.value();
        }
        */
        auto player_set = this->ReadTheaterFilePlayerSet(ifs);
        if (player_set.has_value()) {
            file_data.player_set_.insert(player_set.value().begin(), player_set.value().end());
        }
        /*
        */
        //can get 
        ifs.close();
        return file_data;
        // if GameType == "", then Discern if its campaign or firefight
        // could split this into halo3_odst_theater_file_data
    }
    virtual std::optional<std::string> ReadTheaterFileGameType(std::ifstream &ifs) override final {
        static const unsigned long long gametype_offset = 0x000000C0;

        ifs.seekg(0);
        ifs.seekg(gametype_offset);

        if (!ifs) {
            return std::nullopt;
        }

        std::vector<char> buffer;
        buffer.resize(256);

        ifs.read(&buffer.data()[0], 256);

        if (ifs.fail()) {
            return std::nullopt;
        }

        std::wstring wstr(reinterpret_cast<const wchar_t *>(buffer.data()));

        auto bytes = utility::ConvertWStringToBytes(wstr);
        if (bytes.has_value()) {
            return bytes.value();
        }

        return std::nullopt;
    }
    virtual std::optional<std::string> ReadTheaterFileGameDescription(
        std::ifstream &ifs) override final {
        static const unsigned long long desc_offset = 0x000001C0;

        ifs.seekg(0);
        ifs.seekg(desc_offset);

        if (!ifs) {
            return std::nullopt;
        }

        unsigned long long desc_length = 256;
        std::vector<char> buffer;
        buffer.resize(desc_length);

        ifs.read(&buffer.data()[0], desc_length);

        if (ifs.fail()) {
            return std::nullopt;
        }

        std::wstring wstr(reinterpret_cast<const wchar_t *>(buffer.data()), buffer.size());

        auto bytes = utility::ConvertWStringToBytes(wstr);
        if (bytes.has_value()) {
            return bytes.value();
        }

        return std::nullopt;
    }
    virtual std::optional<std::string> ReadTheaterFileAuthorXUID(
        std::ifstream &ifs) override final {
        static const unsigned long long xuid_offset = 0x00000080;

        ifs.seekg(0);
        ifs.seekg(xuid_offset);

        if (!ifs) {
            return std::nullopt;
        }

        std::vector<char> buffer;
        buffer.resize(8);

        size_t i = 0;
        for (auto &b : buffer) {
            ifs.read(&buffer.data()[i], 1);
            if (ifs.fail()) {
                return std::nullopt;
            }
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
    virtual std::optional<std::string> ReadTheaterFileAuthor(std::ifstream &ifs) override final {
        static const unsigned long long author_offset = 0x00000088;

        ifs.seekg(0);
        ifs.seekg(author_offset);
        if (!ifs) {
            return std::nullopt;
        }

        std::vector<char> buffer;
        buffer.resize(16);

        if (!ifs) {
            return std::nullopt;
        }

        ifs.read(&buffer.data()[0], 16);

        if (ifs.fail()) {
            return std::nullopt;
        }

        std::string str(buffer.data());
        return str;
    }
    virtual std::optional<std::filesystem::file_time_type> ReadTheaterFileUTCTimestamp(
        std::ifstream &ifs) override final {

        static const unsigned long long timestamp_offset = 0x00000068; // read at least 128 bytes?
        static const unsigned long long duration_offset = 0x00000118;  // read two bytes

        ifs.seekg(0);
        ifs.seekg(duration_offset);
        if (!ifs) {
            return std::nullopt;
        }


        char buffer[3];
        buffer[2] = '\0';
        ifs.read(&buffer[0], 1);
        if (ifs.fail()) {
            return std::nullopt;
        }
        ifs.read(&buffer[1], 1);
        if (ifs.fail()) {
            return std::nullopt;
        }
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

        // std::cout << "fixed[0]: " << (int)fixed[0] << std::endl;
        // std::cout << "fixed[1]: " << (int)fixed[1] << std::endl;
        auto match_length = std::chrono::seconds(strtoul(ret.str().c_str(), NULL, 16));

        ifs.seekg(timestamp_offset);
        if (!ifs) {
            return std::nullopt;
        }

        char buffer2[128];
        ifs.read(buffer2, 128);

        if (ifs.fail()) {
            return std::nullopt;
        }
        // ifs.close();

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
        // std::cout << "match length: " << match_length << std::endl;
        std::chrono::seconds dst_offset{dst * 3600};
        std::chrono::utc_time<std::chrono::seconds> utc(
            std::chrono::seconds(local_time) + match_length +
            (std::filesystem::_File_time_clock::_Skipped_filetime_leap_seconds -
             std::chrono::seconds(GetSystemAccountedLeapSeconds())) -
            dst_offset);

        return std::chrono::file_clock::from_utc(utc);
    }

    virtual std::optional<std::set<player_info>> ReadTheaterFilePlayerSet(
        std::ifstream &ifs) override final {

        static const unsigned long long head_offset = 0x0002C3C0;

        ifs.seekg(0);
        ifs.seekg(head_offset);
        if (!ifs) {
            return std::nullopt;
        }
        std::set<player_info> player_set;

        int empty_region_count = 0;
        while (player_set.size() < 16) {
            bool current_is_empty_region = true;
            char buf[1];
            ifs.read(&buf[0], 1);

            if (ifs.fail()) {
                return std::nullopt;
            }

            if (buf[0] == '\0') {
                ++empty_region_count;
                if (empty_region_count >= 2) {
                    break;
                }
            } else {
                empty_region_count = 0;
                current_is_empty_region = false;
            }

            // reset
            ifs.seekg(-1, std::ios_base::cur);
            if (!ifs) {
                return std::nullopt;
            }
            if (!current_is_empty_region) {
                // read player name
                std::vector<char> buffer;
                buffer.resize(32);

                ifs.read(&buffer.data()[0], 32);

                if (ifs.fail()) {
                    return std::nullopt;
                }

                std::wstring wstr(reinterpret_cast<const wchar_t *>(buffer.data()));

                bool success = false;
                auto bytes = utility::ConvertWStringToBytes(wstr);
                if (bytes.has_value()) {
                    success = true;
                }
                char team_buf[1];
                // go to team
                ifs.seekg(328 - 72, std::ios_base::cur);
                if (!ifs) {
                    return std::nullopt;
                }
                ifs.read(&team_buf[0], 1);
                if (ifs.fail()) {
                    return std::nullopt;
                }
                if (success) {
                    player_set.insert({team_buf[0], bytes.value()});
                }

                ifs.seekg(-1, std::ios_base::cur);
                if (!ifs) {
                    return std::nullopt;
                }
                ifs.seekg(40, std::ios_base::cur);
                if (!ifs) {
                    return std::nullopt;
                }

            } else {
                ifs.seekg(328, std::ios_base::cur);
                if (!ifs) {
                    return std::nullopt;
                }
            }

            // skip to next player
        }

        return player_set;
    }
};

inline theater_file_data ReadTheaterFile(
    const std::filesystem::path &theater_file, mccinfo::game_hint hint) {
    // theater_file_timestamp.str("");
    theater_file_data file_data;

    if (std::filesystem::file_size(theater_file) > 0) {
        switch (hint) {
        case mccinfo::game_hint::HALO2A: {
            mccinfo::file_readers::halo2a_theater_file_reader reader;
            auto file_data_query = reader.Read(theater_file);
            if (file_data_query.has_value()) {
                file_data = file_data_query.value();
                // theater_file_timestamp << file_data.utc_timestamp_;
            }
            break;
        }
        case mccinfo::game_hint::HALO3: {
            mccinfo::file_readers::halo3_theater_file_reader reader;
            auto file_data_query = reader.Read(theater_file);
            if (file_data_query.has_value()) {
                file_data = file_data_query.value();
                // theater_file_timestamp << file_data.utc_timestamp_;
            }
            break;
        }
        case mccinfo::game_hint::HALOREACH: {
            mccinfo::file_readers::haloreach_theater_file_reader reader;
            auto file_data_query = reader.Read(theater_file);
            if (file_data_query.has_value()) {
                file_data = file_data_query.value();
                // theater_file_timestamp << file_data.utc_timestamp_;
            }
            break;
        }
        case mccinfo::game_hint::HALO4: {
            mccinfo::file_readers::halo4_theater_file_reader reader;
            auto file_data_query = reader.Read(theater_file);
            if (file_data_query.has_value()) {
                file_data = file_data_query.value();
                // theater_file_timestamp << file_data.utc_timestamp_;
            }
            break;
        }
        default:
            break;
        }
    } else {
        MI_CORE_WARN("ReadTheaterFile() called with an empty theater file: {0}",
                     theater_file.generic_string().c_str());
    }
    return file_data;
}

}
}