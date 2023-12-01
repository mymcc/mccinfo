#pragma once

#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#include <krabs/krabs.hpp> //#include <windows.h>
#undef NOMINMAX
#endif

#include "predicates.hpp"
#include <string>

namespace mccinfo {
namespace fsm {
namespace filters {
inline constexpr auto make_process_filter = []() {
    return krabs::event_filter{krabs::predicates::any_of(
        {&predicates::is_launcher, &predicates::is_eac, &predicates::is_mcc})};
};

inline constexpr auto make_image_filter = []() {
    return krabs::event_filter{
        krabs::predicates::all_of({&predicates::is_launcher_image, &predicates::image_load})};
};

inline constexpr auto make_fiio_filter = []() {
    return krabs::event_filter{
        krabs::predicates::any_of({
            &predicates::accepted_file_reads,
            &predicates::accepted_file_creates
        })};
};
} // namespace filters
} // namespace fsm2
} // namespace mccinfo