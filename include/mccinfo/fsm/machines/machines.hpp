#pragma once

#include <boost/sml.hpp>
#include "mccinfo/fsm/callback_table.hpp"

#define CALLBACK_ON_ENTRY(state_struct, state_flag)					\
	state_struct + boost::sml::on_entry<_> /						\
		[](callback_table& cbtable) {								\
			cbtable.execute_callbacks(ON_STATE_ENTRY | state_flag);	\
        }

#define CALLBACK_ON_EXIT(state_struct, state_flag)					\
	state_struct + boost::sml::on_exit<_> /							\
		[](callback_table& cbtable) {								\
			cbtable.execute_callbacks(ON_STATE_EXIT | state_flag);	\
        }

#define CALLBACK_ON_TRANSITION(state_struct, state_flag)			\
	CALLBACK_ON_ENTRY(state_struct, state_flag),					\
	CALLBACK_ON_EXIT(state_struct, state_flag)			 

#include "mccinfo/fsm/machines/mcc.hpp"
#include "mccinfo/fsm/machines/user.hpp"
#include "mccinfo/fsm/machines/game_id.hpp"

#undef CALLBACK_ON_ENTRY
#undef CALLBACK_ON_EXIT
#undef CALLBACK_ON_TRANSITION
