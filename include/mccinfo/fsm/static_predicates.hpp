#pragma once

#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#include <krabs/krabs.hpp> //#include <windows.h>
#undef NOMINMAX
#endif

#include <string>

namespace mccinfo {
namespace fsm2 {
namespace predicates {
static auto is_launcher =
    krabs::predicates::property_is(L"ImageFileName", std::string("mcclauncher.exe"));
static auto is_eac =
    krabs::predicates::property_is(L"ImageFileName", std::string("EasyAntiCheat.exe"));
static auto is_mcc =
    krabs::predicates::property_is(L"ImageFileName", std::string("MCC-Win64-Shipping.exe"));

static auto process_start = krabs::predicates::opcode_is(1);
static auto process_terminate = krabs::predicates::opcode_is(2);

static krabs::predicates::all_of launcher_start_pred({
    &is_launcher,
    &process_start,
});

static krabs::predicates::all_of launcher_end_pred({
    &is_launcher,
    &process_terminate,
});

static krabs::predicates::all_of eac_start_pred({
    &is_eac,
    &process_start,
});

static krabs::predicates::all_of mcc_process_lossed({
    &is_mcc,
    &process_terminate,
});
} // namespace predicates
} // namespace fsm2
} // namespace mccinfo