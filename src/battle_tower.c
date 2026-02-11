#include "global.h"
#include "battle_tower.h"
#include "event_data.h"
#include "pokeball.h"
#include "constants/battle_tower.h"
#include "constants/battle_frontier.h"
#include "constants/battle_frontier_mons.h"
#include "constants/event_objects.h"
#include "constants/frontier_util.h"

// This file's functions.
static void GetTowerData(void);

// placeholders
#define FACILITY_CLASS_TUBER_M          FACILITY_CLASS_RUIN_MANIAC
#define FACILITY_CLASS_PKMN_BREEDER_M   FACILITY_CLASS_RUIN_MANIAC

#define FACILITY_CLASS_TUBER_F          FACILITY_CLASS_AROMA_LADY
#define FACILITY_CLASS_PKMN_BREEDER_F   FACILITY_CLASS_AROMA_LADY

#define OBJ_EVENT_GFX_TUBER_M               OBJ_EVENT_GFX_HIKER
#define OBJ_EVENT_GFX_POKEFAN_M             OBJ_EVENT_GFX_HIKER
#define OBJ_EVENT_GFX_CYCLING_TRIATHLETE_M  OBJ_EVENT_GFX_HIKER

#include "data/battle_frontier/battle_frontier_trainer_mons.h"
#include "data/battle_frontier/battle_frontier_trainers.h"
#include "data/battle_frontier/battle_frontier_mons.h"


const u8 gTowerMaleFacilityClasses[30] =
{
    FACILITY_CLASS_RUIN_MANIAC,
    FACILITY_CLASS_TUBER_M,
    FACILITY_CLASS_COOLTRAINER_M,
    FACILITY_CLASS_RICH_BOY,
    FACILITY_CLASS_POKEMANIAC,
    FACILITY_CLASS_SWIMMER_M,
    FACILITY_CLASS_BLACK_BELT,
    FACILITY_CLASS_GUITARIST,
    FACILITY_CLASS_KINDLER,
    FACILITY_CLASS_CAMPER,
    FACILITY_CLASS_BUG_MANIAC,
    FACILITY_CLASS_PSYCHIC_M,
    FACILITY_CLASS_GENTLEMAN,
    FACILITY_CLASS_SCHOOL_KID_M,
    FACILITY_CLASS_POKEFAN_M,
    FACILITY_CLASS_EXPERT_M,
    FACILITY_CLASS_YOUNGSTER,
    FACILITY_CLASS_FISHERMAN,
    FACILITY_CLASS_CYCLING_TRIATHLETE_M,
    FACILITY_CLASS_RUNNING_TRIATHLETE_M,
    FACILITY_CLASS_SWIMMING_TRIATHLETE_M,
    FACILITY_CLASS_DRAGON_TAMER,
    FACILITY_CLASS_BIRD_KEEPER,
    FACILITY_CLASS_NINJA_BOY,
    FACILITY_CLASS_SAILOR,
    FACILITY_CLASS_COLLECTOR,
    FACILITY_CLASS_PKMN_BREEDER_M,
    FACILITY_CLASS_PKMN_RANGER_M,
    FACILITY_CLASS_BUG_CATCHER,
    FACILITY_CLASS_HIKER
};

const u8 gTowerFemaleFacilityClasses[20] =
{
    FACILITY_CLASS_AROMA_LADY,
    FACILITY_CLASS_TUBER_F,
    FACILITY_CLASS_COOLTRAINER_F,
    FACILITY_CLASS_HEX_MANIAC,
    FACILITY_CLASS_LADY,
    FACILITY_CLASS_BEAUTY,
    FACILITY_CLASS_PSYCHIC_F,
    FACILITY_CLASS_SCHOOL_KID_F,
    FACILITY_CLASS_POKEFAN_F,
    FACILITY_CLASS_EXPERT_F,
    FACILITY_CLASS_CYCLING_TRIATHLETE_F,
    FACILITY_CLASS_RUNNING_TRIATHLETE_F,
    FACILITY_CLASS_SWIMMING_TRIATHLETE_F,
    FACILITY_CLASS_BATTLE_GIRL,
    FACILITY_CLASS_PARASOL_LADY,
    FACILITY_CLASS_SWIMMER_F,
    FACILITY_CLASS_PICNICKER,
    FACILITY_CLASS_PKMN_BREEDER_F,
    FACILITY_CLASS_PKMN_RANGER_F,
    FACILITY_CLASS_LASS
};

const u16 gTowerMaleTrainerGfxIds[30] =
{
    OBJ_EVENT_GFX_HIKER,
    OBJ_EVENT_GFX_TUBER_M,
    OBJ_EVENT_GFX_MAN_3,
    OBJ_EVENT_GFX_RICH_BOY,
    OBJ_EVENT_GFX_MANIAC,
    OBJ_EVENT_GFX_RUNNING_TRIATHLETE_M,
    OBJ_EVENT_GFX_BLACK_BELT,
    OBJ_EVENT_GFX_MAN_5,
    OBJ_EVENT_GFX_MAN_5,
    OBJ_EVENT_GFX_CAMPER,
    OBJ_EVENT_GFX_MANIAC,
    OBJ_EVENT_GFX_PSYCHIC_M,
    OBJ_EVENT_GFX_GENTLEMAN,
    OBJ_EVENT_GFX_SCHOOL_KID_M,
    OBJ_EVENT_GFX_POKEFAN_M,
    OBJ_EVENT_GFX_EXPERT_M,
    OBJ_EVENT_GFX_YOUNGSTER,
    OBJ_EVENT_GFX_FISHERMAN,
    OBJ_EVENT_GFX_CYCLING_TRIATHLETE_M,
    OBJ_EVENT_GFX_RUNNING_TRIATHLETE_M,
    OBJ_EVENT_GFX_RUNNING_TRIATHLETE_M,
    OBJ_EVENT_GFX_MAN_3,
    OBJ_EVENT_GFX_MAN_5,
    OBJ_EVENT_GFX_NINJA_BOY,
    OBJ_EVENT_GFX_SAILOR,
    OBJ_EVENT_GFX_MANIAC,
    OBJ_EVENT_GFX_MAN_4,
    OBJ_EVENT_GFX_CAMPER,
    OBJ_EVENT_GFX_BUG_CATCHER,
    OBJ_EVENT_GFX_HIKER
};

const u16 gTowerFemaleTrainerGfxIds[20] =
{
    OBJ_EVENT_GFX_WOMAN_2,
    OBJ_EVENT_GFX_TUBER_F,
    OBJ_EVENT_GFX_WOMAN_5,
    OBJ_EVENT_GFX_HEX_MANIAC,
    OBJ_EVENT_GFX_WOMAN_2,
    OBJ_EVENT_GFX_BEAUTY,
    OBJ_EVENT_GFX_LASS,
    OBJ_EVENT_GFX_GIRL_3,
    OBJ_EVENT_GFX_POKEFAN_F,
    OBJ_EVENT_GFX_EXPERT_F,
    OBJ_EVENT_GFX_CYCLING_TRIATHLETE_F,
    OBJ_EVENT_GFX_RUNNING_TRIATHLETE_F,
    OBJ_EVENT_GFX_RUNNING_TRIATHLETE_F,
    OBJ_EVENT_GFX_GIRL_3,
    OBJ_EVENT_GFX_WOMAN_5,
    OBJ_EVENT_GFX_RUNNING_TRIATHLETE_F,
    OBJ_EVENT_GFX_PICNICKER,
    OBJ_EVENT_GFX_WOMAN_2,
    OBJ_EVENT_GFX_PICNICKER,
    OBJ_EVENT_GFX_LASS
};

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