#pragma once

#include <array>
#include <string>

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
constexpr std::array<std::wstring_view, 6> GameBasenames = { L"halo1",     L"halo2", L"halo3",
														L"halo3odst", L"halo4", L"haloreach" };
}