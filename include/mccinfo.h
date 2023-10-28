#pragma once

#include <krabs/krabs.hpp> //#include <windows.h>
#include <tlhelp32.h>

#include <queue>
#include <string>
#include <optional>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <vector>
#include <array>
#include <unordered_map>
#include <type_traits>

#include <wil/filesystem.h>
#include <wil/registry.h>
#include <fsm.hpp>

#pragma warning(push)
#pragma warning(disable : 4996)
#include <vdf_parser.hpp>
#pragma warning(pop)



namespace mccinfo {
/**
 * @brief The Steam&trade; Application ID for Halo: The Master Chief Collection&trade;.
 *
 * This ID is used by Steam to uniquely identify each game or application in its store.
 */
constexpr size_t MCCSteamAppID = 976730;
/**
 * @brief The Steam&trade; executable basename + extension for Halo: The Master Chief
 * Collection&trade;.
 */
constexpr std::wstring_view SteamMCCexe = L"MCC-Win64-Shipping.exe";
constexpr std::string_view bSteamMCCexe = "MCC-Win64-Shipping.exe";
/**
 * @brief The Microsoft Store&trade; executable basename + extension for Halo: The Master Chief
 * Collection&trade;.
 */
constexpr std::wstring_view MicrosoftStoreMCCexe = L"MCCWinStore-Win64-Shipping.exe";
constexpr std::string_view bMicrosoftStoreMCCexe = "MCCWinStore-Win64-Shipping.exe";
/**
 * @brief The relative directory path to the main Halo: The Master Chief Collection&trade;
 * executable within its installation path. That means the absolute path to the executable in the
 * filesystem is:
 * - {Steam MCC Install Path} \ {MCCexeRelativePath} \ {SteamMCCexe}
 * - {MS Store MCC Install Path} \ {MCCexeRelativePath} \ {MSStoreMCCexe}
 */
constexpr std::wstring_view MCCexeRelativePath = L"mcc\\binaries\\win64";
/**
 * @brief The temporary directory Halo: The Master Chief Collection&trade; uses.
 */
constexpr std::wstring_view MCCTempPath = L"%AppData%\\..\\LocalLow\\MCC";
/**
 * @brief The array of all Halo: The Master Chief Collection&trade;'s individual title basenames.
 */
constexpr std::array<std::wstring_view, 6> GameBasenames = {L"halo1",     L"halo2", L"halo3",
                                                            L"halo3odst", L"halo4", L"haloreach"};

enum class StoreVersion { None, Steam, MicrosoftStore };

const std::unordered_map<StoreVersion, std::wstring_view> StoreVersionToWString{
    {StoreVersion::None, L"None"},
    {StoreVersion::Steam, L"Steam"},
    {StoreVersion::MicrosoftStore, L"Microsoft Store"}};

struct MCCInstallInfo {
public:
    StoreVersion Kind{StoreVersion::None};
    std::wstring RootPath;
    std::wstring BuildTag;
    std::wstring BuildVersion;

    std::vector<std::wstring> InstalledGames;

    friend std::wostream &operator<<(std::wostream &os, const MCCInstallInfo &ii);
    friend std::optional<MCCInstallInfo> LookForInstallInfoImpl(const std::wstring& install_path, StoreVersion store_version);
private:
    MCCInstallInfo() {}
};
/**
 * @brief Retrieves the installation path of Steam&trade;.
 *
 * Attempts to locate the Steam&trade; installation directory, which is typically found at
 * <b>C:\\Program Files (x86)\\Steam</b>. If the installation path cannot be determined,
 * the returned optional will be empty.
 *
 * @return An optional containing the installation path of Steam&trade; if found; otherwise, an
 * empty optional.
 */
std::optional<std::wstring> LookForSteamInstallPath(void);
/**
 * @brief Retrieves the installation path of the Steam&trade; version of Halo: The Master Chief
 * Collection&trade;.
 *
 * Attempts to locate the installation directory of the Steam&trade; version of Halo: The Master
 * Chief Collection&trade;, which is typically found at <b>C:\\Program Files
 * (x86)\\Steam\\steamapps\\common\\Halo The Master Chief Collection\\</b>. If the installation path
 * cannot be determined, the returned optional will be empty.
 *
 * @return An optional containing the installation path of Steam&trade; if found; otherwise, an
 * empty optional.
 */
std::optional<std::wstring> LookForMCCSteamInstallPath(void);
/**
 * @brief Retrieves the installation path of the Microsoft Store&trade; version of Halo: The Master
 * Chief Collection&trade;.
 *
 * Attempts to locate the installation directory of the Microsoft Store&trade; version of Halo: The
 * Master Chief Collection&trade;, which is typically found at <b>C:\\XboxGames\\Halo The Master
 * Chief Collection\\content\\</b>. If the installation path cannot be determined, the returned
 * optional will be empty.
 *
 * @return An optional containing the installation path of Microsoft Store&trade; if found;
 * otherwise, an empty optional.
 */
std::optional<std::wstring> LookForMCCMicrosoftStoreInstallPath(void);
/**
 * @brief Retrieves the process id of the first found instance of Halo: The Master
 * Chief Collection&trade;.
 *
 * Attempts to find the process id of a running instance Halo: The Master Chief Collection&trade;,
 * with no distinction on whether or not the instance is hosting an Easy Anti-Cheat client. As far
 * as instance version precedence, this function first checks for Microsoft Store instances, then
 * checks for Steam instances. If no instances are found, then the optional will be empty.
 *
 * @return An optional containing the process id of the first found instance of Halo: The Master
 * Chief Collection&trade; if one is found; otherwise, an empty optional.
 */
std::optional<size_t> LookForMCCProcessID(void);
std::optional<std::wstring> LookForMCCTempPath(void);

std::optional<std::wstring> GetFileVersion(const std::wstring &path);

std::optional<std::wstring> ConvertBytesToWString(const std::string& bytes);
std::optional<std::string> ConvertWStringToBytes(const std::wstring& wstr);

std::optional<std::wstring> LookForMCCBuildTag(const std::wstring &install_path);
std::optional<std::wstring> LookForMCCBuildVersion(const std::wstring &install_path);
std::optional<std::vector<std::wstring>> LookForInstalledGameDLLs(const std::wstring &install_path);
std::optional<StoreVersion> LookForMCCKind(const std::wstring &install_path);

std::optional<MCCInstallInfo> LookForInstallInfo(StoreVersion store_version);
std::optional<MCCInstallInfo> LookForInstallInfo(const std::wstring &install_path);
std::optional<MCCInstallInfo> LookForSteamInstallInfo(void);
std::optional<MCCInstallInfo> LookForMicrosoftStoreInstallInfo(void);

bool StartETW(void);
void FlushEventQueue();
//bool StartTempWatchdog(void);
} // namespace mccinfo