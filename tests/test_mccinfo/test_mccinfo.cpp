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

    if (pid.has_value()) {
        auto hwnd = mccinfo::query::LookForMCCWindowHandle();
        if (hwnd.has_value()) {
            wchar_t szBuf[64];
            wsprintf(szBuf, L"%p", hwnd.value());
            std::wcout << std::left << std::setw(align) << L"HWND: " << szBuf << std::endl;

            RECT sr;
            GetWindowRect(hwnd.value(), &sr);

            wchar_t topLeft[64];
            wchar_t botRight[64];
            wchar_t winDim[64];
            
            wsprintf(topLeft, L"(%d, %d)", sr.left, sr.top);
            wsprintf(botRight, L"(%d, %d)", sr.right, sr.bottom);
            wsprintf(winDim, L"[%dx%d]", abs(sr.right - sr.left), abs(sr.bottom - sr.top));
            
            std::wcout << std::left << std::setw(align) << L"RECT: " << topLeft << L" - " << botRight << L' ' << winDim << std::endl;
        }
        else {
            std::wcout << std::left << std::setw(align) << L"HWND: " << std::endl;
            std::wcout << std::left << std::setw(align) << L"RECT: " << std::endl;

        }
    } else {
    
    }


    using namespace mccinfo::fsm;

    callback_table cb_table{};

    cb_table.add_callback(ON_STATE_ENTRY | OFF,
                          [] { std::wcout << L"OFF STATE ENTERED!" << std::endl; }
    );

    context ctx(cb_table);

    std::wcout << L" - starting trace" << std::endl;
    ctx.start();

    while (true) {
        std::string input;
        std::getline(std::cin, input);
        if (input == "q") {
            std::wcout << std::endl << L" - stopping trace" << std::endl;
            ctx.stop();
            break;
        }
    }

    return 0;
}