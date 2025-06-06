@ Unused
EventScript_CheckFallDownHole::
	map_script_2 VAR_ICE_STEP_COUNT, 0, EventScript_FallDownHole
	.2byte 0

@ Unused
EventScript_FixCrackedGround::
	copyvar VAR_ICE_STEP_COUNT, 1
	end

EventScript_FallDownHole::
	lockall
	delay 20
	applymovement LOCALID_PLAYER, Movement_SetInvisible
	waitmovement 0
	playse SE_FALL
	delay 60
	warphole MAP_UNDEFINED
	waitstate
	end

@ Differs from the script above by assuming the warp destination has already been set
EventScript_DoFallWarp::
	lockall
	delay 20
	applymovement LOCALID_PLAYER, Movement_SetInvisible
	waitmovement 0
	playse SE_FALL
	delay 60
	special DoFallWarp
	waitstate
	end

Movement_SetInvisible:
	set_invisible
	step_end
