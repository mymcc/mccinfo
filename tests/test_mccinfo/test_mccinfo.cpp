#include <iostream>
#include <iomanip>
#include "mccinfo.h"

constexpr uint8_t align = 18;

int main(int argc, char** argv) {
	auto sii = mccinfo::LookForSteamInstallInfo();
	if (sii.has_value()) {
		std::wcout << sii.value() << std::endl << std::endl;
	}

	auto msii = mccinfo::LookForMicrosoftStoreInstallInfo();
	if (msii.has_value()) {
		std::wcout << msii.value() << std::endl << std::endl;
	}

	auto patht = mccinfo::LookForMCCTempPath();
	if (patht.has_value()) {
		std::wcout << std::left << std::setw(align) << L"MCC Temp Path: " << patht.value() << std::endl;
	}

	auto pid = mccinfo::LookForMCCProcessID();
	if (pid.has_value()) {
		std::wcout << std::left << std::setw(align) << L"MCC PID: " << pid.value() << std::endl;
	}

    mccinfo::StartETW();

	//mccinfo::StartTempWatchdog();

	return 0;
}