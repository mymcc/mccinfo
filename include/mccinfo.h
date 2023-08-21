#include <windows.h>
#include <tlhelp32.h>

#include <string>
#include <optional>
#include <iostream>
#include <fstream>
#include <filesystem>

#include <wil/registry.h>

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
/**
 * @brief The Microsoft Store&trade; executable basename + extension for Halo: The Master Chief
 * Collection&trade;.
 */
constexpr std::wstring_view MicrosoftStoreMCCexe = L"MCCWinStore-Win64-Shipping.exe";
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
std::optional<std::wstring> LookForMCCSteamInstall(void);
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
std::optional<std::wstring> LookForMCCMicrosoftStoreInstall(void);
/**
 * @brief Retrieves the process id of the first found instance of Halo: The Master
 * Chief Collection&trade;.
 *
 * Attempts to find the process id of a running instance Halo: The Master Chief Collection&trade;,
 * with no distinction on whether or not the instance is hosting an Easy Anti-Cheat client. As far
 * as instance version precedence, this function first checks for Microsoft Store instances, then
 * checks for Steam instances. If no instances are found, then the optional will be empty.
 *
 * @return An optional containing the process of the first found instance of Halo: The Master
 * Chief Collection&trade; if one is found; otherwise, an empty optional.
 */
std::optional<size_t> LookForMCCProcessID(void);
} // namespace mccinfo