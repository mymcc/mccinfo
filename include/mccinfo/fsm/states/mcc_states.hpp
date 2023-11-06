#pragma once

#include "..\static_predicates.hpp"
#include "..\predicate_sequence.hpp"
#include "..\transitions\sequences.hpp"
namespace mccinfo {
namespace fsm {
namespace states {
class off;
class launching;
class on;

//std::unordered_map<std::type_index,>

//static constexpr predicate_sequence<events::launcher_start, 2> seq{
//        &predicates::launcher_start_pred, &predicates::eac_start_pred};
//std::unordered_map<std::type_index, fsm2::predicate_sequence> preds;

/*
* {"on" : { "mcc_terminate" : {..., ..., ...}}}
* 
* 
* 
* 
*/
} // namespace states
} // namespace fsm
} // namespace mccinfo