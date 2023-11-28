#include <iostream>
#include <iomanip>
#include "mccinfo.hpp"

constexpr uint8_t align = 18;

int main(int argc, char **argv) {
    auto sii = mccinfo::query::LookForSteamInstallInfo();
    if (sii.has_value()) {
        std::wcout << sii.value() << std::endl << std::endl;
    }

    auto msii = mccinfo::query::LookForMicrosoftStoreInstallInfo();
    if (msii.has_value()) {
        std::wcout << msii.value() << std::endl << std::endl;
    }

    auto patht = mccinfo::query::LookForMCCTempPath();
    if (patht.has_value()) {
        std::wcout << std::left << std::setw(align) << L"MCC Temp Path: " << patht.value()
                   << std::endl;
    }

    auto pid = mccinfo::query::LookForMCCProcessID();
    if (pid.has_value()) {
        std::wcout << std::left << std::setw(align) << L"MCC PID: " << pid.value() << std::endl;
    } else {
        std::wcout << std::left << std::setw(align) << L"MCC PID: " << L"NO PID FOUND" << std::endl;
    }

    mccinfo::fsm::provider::StartETW();

    return 0;
}