#include "global.h"
#include "battle_frontier.h"
#include "battle_setup.h"
#include "event_data.h"
#include "frontier_util.h"
#include "constants/battle_palace.h"

// This file's functions.
static void BufferOpponentIntroSpeech(void);

// Const rom data.
static void (*const sBattlePalaceFunctions[])(void) =
{
    // [BATTLE_PALACE_FUNC_INIT]               = InitPalaceChallenge,
    // [BATTLE_PALACE_FUNC_GET_DATA]           = GetPalaceData,
    // [BATTLE_PALACE_FUNC_SET_DATA]           = SetPalaceData,
    // [BATTLE_PALACE_FUNC_GET_COMMENT_ID]     = GetPalaceCommentId,
    // [BATTLE_PALACE_FUNC_SET_OPPONENT]       = SetPalaceOpponent,
    [BATTLE_PALACE_FUNC_GET_OPPONENT_INTRO] = BufferOpponentIntroSpeech,
    // [BATTLE_PALACE_FUNC_INCREMENT_STREAK]   = IncrementPalaceStreak,
    // [BATTLE_PALACE_FUNC_SAVE]               = SavePalaceChallenge,
    // [BATTLE_PALACE_FUNC_SET_PRIZE]          = SetRandomPalacePrize,
    // [BATTLE_PALACE_FUNC_GIVE_PRIZE]         = GivePalacePrize,
};

// code
void CallBattlePalaceFunction(void)
{
    sBattlePalaceFunctions[gSpecialVar_0x8004]();
}

static void BufferOpponentIntroSpeech(void)
{
    if (TRAINER_BATTLE_PARAM.opponentA < FRONTIER_TRAINERS_COUNT)
        FrontierSpeechToString(gFacilityTrainers[TRAINER_BATTLE_PARAM.opponentA].speechBefore);
}