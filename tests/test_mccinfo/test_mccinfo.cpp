#include <iostream>
#include "mccinfo.h"

int main(int argc, char** argv) {
	auto paths = mccinfo::LookForMCCSteamInstall();
	if (paths.has_value()) {
		std::wcout << L"MCC Steam Install: " << paths.value() << std::endl;
	}

	auto pathw = mccinfo::LookForMCCMicrosoftStoreInstall();
	if (pathw.has_value()) {
		std::wcout << L"MCC Microsoft Store Install: " << pathw.value() << std::endl;
	}

	auto pid = mccinfo::LookForMCCProcessID();
	if (pid.has_value()) {
		std::wcout << L"MCC pid: " << pid.value() << std::endl;
	}
	
	return 0;
}