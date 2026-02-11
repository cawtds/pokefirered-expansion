#include "global.h"
#include "battle_tower.h"
#include "event_data.h"
#include "constants/battle_tower.h"
#include "constants/battle_frontier.h"
#include "constants/frontier_util.h"

// This file's functions.
static void GetTowerData(void);

static void (* const sBattleTowerFuncs[])(void) =
{
    // [BATTLE_TOWER_FUNC_INIT]                = InitTowerChallenge,
    [BATTLE_TOWER_FUNC_GET_DATA]            = GetTowerData,
    // [BATTLE_TOWER_FUNC_SET_DATA]            = SetTowerData,
    // [BATTLE_TOWER_FUNC_SET_OPPONENT]        = SetNextTowerOpponent,
    // [BATTLE_TOWER_FUNC_SET_BATTLE_WON]      = SetTowerBattleWon,
    // [BATTLE_TOWER_FUNC_GIVE_RIBBONS]        = AwardBattleTowerRibbons,
    // [BATTLE_TOWER_FUNC_SAVE]                = SaveTowerChallenge,
    // [BATTLE_TOWER_FUNC_GET_OPPONENT_INTRO]  = GetOpponentIntroSpeech,
    // [BATTLE_TOWER_FUNC_NOP]                 = BattleTowerNop1,
    // [BATTLE_TOWER_FUNC_NOP2]                = BattleTowerNop2,
    // [BATTLE_TOWER_FUNC_LOAD_PARTNERS]       = LoadMultiPartnerCandidatesData,
    // [BATTLE_TOWER_FUNC_PARTNER_MSG]         = ShowPartnerCandidateMessage,
    // [BATTLE_TOWER_FUNC_LOAD_LINK_OPPONENTS] = LoadLinkMultiOpponentsData,
    // [BATTLE_TOWER_FUNC_TRY_CLOSE_LINK]      = TowerTryCloseLink,
    // [BATTLE_TOWER_FUNC_SET_PARTNER_GFX]     = SetMultiPartnerGfx,
    // [BATTLE_TOWER_FUNC_SET_INTERVIEW_DATA]  = SetTowerInterviewData,
};

static const u32 sWinStreakFlags[][2] =
{
    {STREAK_TOWER_SINGLES_50,     STREAK_TOWER_SINGLES_OPEN},
    {STREAK_TOWER_DOUBLES_50,     STREAK_TOWER_DOUBLES_OPEN},
    {STREAK_TOWER_MULTIS_50,      STREAK_TOWER_MULTIS_OPEN},
    {STREAK_TOWER_LINK_MULTIS_50, STREAK_TOWER_LINK_MULTIS_OPEN},
};

// code
void CallBattleTowerFunc(void)
{
    sBattleTowerFuncs[gSpecialVar_0x8004]();
}

static void GetTowerData(void)
{
    enum FrontierLevelMode lvlMode = gSaveBlock2Ptr->frontier.lvlMode;
    u32 battleMode = VarGet(VAR_FRONTIER_BATTLE_MODE);

    switch (gSpecialVar_0x8005)
    {
    case 0:
        break;
    case TOWER_DATA_WIN_STREAK:
        gSpecialVar_Result = GetCurrentBattleTowerWinStreak(lvlMode, battleMode);
        break;
    case TOWER_DATA_WIN_STREAK_ACTIVE:
        gSpecialVar_Result = ((gSaveBlock2Ptr->frontier.winStreakActiveFlags & sWinStreakFlags[battleMode][lvlMode]) != 0);
        break;
    case TOWER_DATA_LVL_MODE:
        gSaveBlock2Ptr->frontier.towerLvlMode = gSaveBlock2Ptr->frontier.lvlMode;
        break;
    }
}

u16 GetCurrentBattleTowerWinStreak(enum FrontierLevelMode lvlMode, u8 battleMode)
{
    u16 winStreak = gSaveBlock2Ptr->frontier.towerWinStreaks[battleMode][lvlMode];

    if (winStreak > MAX_STREAK)
        return MAX_STREAK;
    else
        return winStreak;
}