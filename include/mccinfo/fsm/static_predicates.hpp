#pragma once

#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#include <krabs/krabs.hpp> //#include <windows.h>
#undef NOMINMAX
#endif

#include <string>

namespace mccinfo {
namespace fsm {
namespace predicates {
inline auto is_launcher =
    krabs::predicates::property_is(L"ImageFileName", std::string("mcclauncher.exe"));
inline auto is_eac =
    krabs::predicates::property_is(L"ImageFileName", std::string("EasyAntiCheat.exe"));
inline auto is_mcc =
    krabs::predicates::property_is(L"ImageFileName", std::string("MCC-Win64-Shipping.exe"));

inline auto process_start = krabs::predicates::opcode_is(1);
inline auto process_terminate = krabs::predicates::opcode_is(2);

inline krabs::predicates::all_of launcher_start_pred({
    &is_launcher,
    &process_start,
});

inline krabs::predicates::all_of launcher_end_pred({
    &is_launcher,
    &process_terminate,
});

inline krabs::predicates::all_of eac_start_pred({
    &is_eac,
    &process_start,
});

inline krabs::predicates::all_of mcc_process_start({
    &is_mcc,
    &process_start,
});

inline krabs::predicates::all_of mcc_process_lossed({
    &is_mcc,
    &process_terminate,
});
} // namespace predicates
} // namespace fsm2
} // namespace mccinfo