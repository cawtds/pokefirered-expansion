#include "global.h"
#include "battle_records.h"
#include "event_data.h"
#include "field_specials.h"
#include "frontier_util.h"
#include "international_string_util.h"
#include "malloc.h"
#include "menu.h"
#include "party_menu.h"
#include "record_mixing.h"
#include "string_util.h"
#include "strings.h"
#include "window.h"
#include "constants/battle_frontier.h"
#include "constants/event_objects.h"
#include "constants/frontier_util.h"

struct FrontierBrainMon
{
    u16 species;
    u16 heldItem;
    u8 fixedIV;
    u8 nature;
    u8 evs[NUM_STATS];
    u16 moves[MAX_MON_MOVES];
};

struct FrontierBrain
{
    u16 trainerId;
    u8 objEventGfx;
    u8 isFemale;
    const u8 *lostTexts[2];
    const u8 *wonTexts[2];
    u16 battledBit[2];
    u8 streakAppearances[4];
};

// This file's functions.
static void GetChallengeStatus(void);
static void SetFrontierData(void);

// battledBit: Flags to change the conversation when the Frontier Brain is encountered for a battle
// First bit is has battled them before and not won yet, second bit is has battled them and won (obtained a Symbol)
const struct FrontierBrain gFrontierBrainInfo[NUM_FRONTIER_FACILITIES] =
{
    [FRONTIER_FACILITY_TOWER] =
    {
        .trainerId = TRAINER_ANABEL,
        .objEventGfx = OBJ_EVENT_GFX_ANABEL,
        .isFemale = TRUE,
        .lostTexts = {
            COMPOUND_STRING("Okay, I understand…"), //Silver
            COMPOUND_STRING("Thank you…")           //Gold
        },
        .wonTexts = {
            COMPOUND_STRING("It's very disappointing…"), //Silver
            COMPOUND_STRING("I'm terribly sorry…")       //Gold
        },
        .battledBit = {1 << 0, 1 << 1},
        .streakAppearances = {35, 70, 35, 1},
    },
    [FRONTIER_FACILITY_DOME] =
    {
        .trainerId = TRAINER_TUCKER,
        .objEventGfx = OBJ_EVENT_GFX_TUCKER,
        .isFemale = FALSE,
        .lostTexts = {
            COMPOUND_STRING(
                "Grr…\n"
                "What the…"),        //Silver
            COMPOUND_STRING(
                "Ahahaha!\n"
                "You're inspiring!") //Gold
        },
        .wonTexts = {
            COMPOUND_STRING(
                "Ahahaha! Aren't you embarrassed?\n"
                "Everyone's watching!"),                              //Silver
            COMPOUND_STRING("My DOME ACE title isn't just for show!") //Gold
        },
        .battledBit = {1 << 2, 1 << 3},
        .streakAppearances = {1, 2, 5, 0},
    },
    [FRONTIER_FACILITY_PALACE] =
    {
        .trainerId = TRAINER_SPENSER,
        .objEventGfx = OBJ_EVENT_GFX_SPENSER,
        .isFemale = FALSE,
        .lostTexts = {
            COMPOUND_STRING(
                "Ah…\n"
                "Now this is something else…"), //Silver
            COMPOUND_STRING(
                "Gwah!\n"
                "Hahahaha!")                    //Gold
        },
        .wonTexts = {
            COMPOUND_STRING(
                "Your POKéMON are wimpy because\n"
                "you're wimpy as a TRAINER!"),           //Silver
            COMPOUND_STRING(
                "Gwahahaha!\n"
                "My brethren, we have nothing to fear!") //Gold
        },
        .battledBit = {1 << 4, 1 << 5},
        .streakAppearances = {21, 42, 21, 1},
    },
    [FRONTIER_FACILITY_ARENA] =
    {
        .trainerId = TRAINER_GRETA,
        .objEventGfx = OBJ_EVENT_GFX_GRETA,
        .isFemale = TRUE,
        .lostTexts = {
            COMPOUND_STRING(
                "No way!\n"
                "Good job!"),        //Silver
            COMPOUND_STRING(
                "Huh?\n"
                "Are you serious?!") //Gold
        },
        .wonTexts = {
            COMPOUND_STRING(
                "Oh, come on!\n"
                "You have to try harder than that!"), //Silver
            COMPOUND_STRING(
                "Heheh!\n"
                "What did you expect?")               //Gold
        },
        .battledBit = {1 << 6, 1 << 7},
        .streakAppearances = {28, 56, 28, 1},
    },
    [FRONTIER_FACILITY_FACTORY] =
    {
        .trainerId = TRAINER_NOLAND,
        .objEventGfx = OBJ_EVENT_GFX_NOLAND,
        .isFemale = FALSE,
        .lostTexts = {
            COMPOUND_STRING(
                "Good job!\n"
                "You know what you're doing!"),    //Silver
            COMPOUND_STRING("What happened here?") //Gold
        },
        .wonTexts = {
            COMPOUND_STRING(
                "Way to work!\n"
                "That was a good lesson, eh?"), //Silver
            COMPOUND_STRING(
                "Hey, hey, hey!\n"
                "You're finished already?")     //Gold
        },
        .battledBit = {1 << 8, 1 << 9},
        .streakAppearances = {21, 42, 21, 1},
    },
    [FRONTIER_FACILITY_PIKE] =
    {
        .trainerId = TRAINER_LUCY,
        .objEventGfx = OBJ_EVENT_GFX_LUCY,
        .isFemale = TRUE,
        .lostTexts = {
            COMPOUND_STRING("Urk…"), //Silver
            COMPOUND_STRING("Darn!") //Gold
        },
        .wonTexts = {
            COMPOUND_STRING("Humph…"), //Silver
            COMPOUND_STRING("Hah!")    //Gold
        },
        .battledBit = {1 << 10, 1 << 11},
        .streakAppearances = {28, 140, 56, 1},
    },
    [FRONTIER_FACILITY_PYRAMID] =
    {
        .trainerId = TRAINER_BRANDON,
        .objEventGfx = OBJ_EVENT_GFX_BRANDON,
        .isFemale = FALSE,
        .lostTexts = {
            COMPOUND_STRING(
                "That's it! You've done great!\n"
                "You've worked hard for this!"), //Silver
            COMPOUND_STRING(
                "That's it! You've done it!\n"
                "You kept working for this!")    //Gold
        },
        .wonTexts = {
            COMPOUND_STRING(
                "Hey! What's wrong with you!\n"
                "Let's see some effort! Get up!"),       //Silver
            COMPOUND_STRING(
                "Hey! Don't you give up now!\n"
                "Get up! Don't lose faith in yourself!") //Gold
        },
        .battledBit = {1 << 12, 1 << 13},
        .streakAppearances = {21, 70, 35, 0},
    },
};


static const u8 *const sHallFacilityToRecordsText[] =
{
    [RANKING_HALL_TOWER_SINGLES] = gText_FrontierFacilityWinStreak,
    [RANKING_HALL_TOWER_DOUBLES] = gText_FrontierFacilityWinStreak,
    [RANKING_HALL_TOWER_MULTIS]  = gText_FrontierFacilityWinStreak,
    [RANKING_HALL_DOME]          = gText_FrontierFacilityClearStreak,
    [RANKING_HALL_PALACE]        = gText_FrontierFacilityWinStreak,
    [RANKING_HALL_ARENA]         = gText_FrontierFacilityKOsStreak,
    [RANKING_HALL_FACTORY]       = gText_FrontierFacilityWinStreak,
    [RANKING_HALL_PIKE]          = gText_FrontierFacilityRoomsCleared,
    [RANKING_HALL_PYRAMID]       = gText_FrontierFacilityFloorsCleared,
    [RANKING_HALL_TOWER_LINK]    = gText_FrontierFacilityWinStreak,
};

static const u8 *const sRecordsWindowChallengeTexts[][2] =
{
    [RANKING_HALL_TOWER_SINGLES] = {gText_BattleTower2,  gText_FacilitySingle},
    [RANKING_HALL_TOWER_DOUBLES] = {gText_BattleTower2,  gText_FacilityDouble},
    [RANKING_HALL_TOWER_MULTIS]  = {gText_BattleTower2,  gText_FacilityMulti},
    [RANKING_HALL_DOME]          = {gText_BattleDome,    gText_FacilitySingle},
    [RANKING_HALL_PALACE]        = {gText_BattlePalace,  gText_FacilitySingle},
    [RANKING_HALL_ARENA]         = {gText_BattleArena,   gText_Facility},
    [RANKING_HALL_FACTORY]       = {gText_BattleFactory, gText_FacilitySingle},
    [RANKING_HALL_PIKE]          = {gText_BattlePike,    gText_Facility},
    [RANKING_HALL_PYRAMID]       = {gText_BattlePyramid, gText_Facility},
    [RANKING_HALL_TOWER_LINK]    = {gText_BattleTower2,  gText_FacilityLink},
};

static const u8 *const sLevelModeText[] =
{
    [FRONTIER_LVL_50]   = gText_RecordsLv50,
    [FRONTIER_LVL_OPEN] = gText_RecordsOpenLevel,
};

static const struct WindowTemplate sRankingHallRecordsWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 2,
    .tilemapTop = 1,
    .width = 26,
    .height = 17,
    .paletteNum = 15,
    .baseBlock = 1
};

static void (*const sFrontierUtilFuncs[])(void) =
{
    [FRONTIER_UTIL_FUNC_GET_STATUS]            = GetChallengeStatus,
    // [FRONTIER_UTIL_FUNC_GET_DATA]              = GetFrontierData,
    [FRONTIER_UTIL_FUNC_SET_DATA]              = SetFrontierData,
    // [FRONTIER_UTIL_FUNC_SET_PARTY_ORDER]       = SetSelectedPartyOrder,
    // [FRONTIER_UTIL_FUNC_SOFT_RESET]            = DoSoftReset_,
    // [FRONTIER_UTIL_FUNC_SET_TRAINERS]          = SetFrontierTrainers,
    // [FRONTIER_UTIL_FUNC_SAVE_PARTY]            = SaveSelectedParty,
    // [FRONTIER_UTIL_FUNC_RESULTS_WINDOW]        = ShowFacilityResultsWindow,
    // [FRONTIER_UTIL_FUNC_CHECK_AIR_TV_SHOW]     = CheckPutFrontierTVShowOnAir,
    // [FRONTIER_UTIL_FUNC_GET_BRAIN_STATUS]      = Script_GetFrontierBrainStatus,
    // [FRONTIER_UTIL_FUNC_IS_BRAIN]              = IsTrainerFrontierBrain,
    // [FRONTIER_UTIL_FUNC_GIVE_BATTLE_POINTS]    = GiveBattlePoints,
    // [FRONTIER_UTIL_FUNC_GET_FACILITY_SYMBOLS]  = GetFacilitySymbolCount,
    // [FRONTIER_UTIL_FUNC_GIVE_FACILITY_SYMBOL]  = GiveFacilitySymbol,
    // [FRONTIER_UTIL_FUNC_CHECK_BATTLE_TYPE]     = CheckBattleTypeFlag,
    // [FRONTIER_UTIL_FUNC_CHECK_INELIGIBLE]      = CheckPartyIneligibility,
    // [FRONTIER_UTIL_FUNC_CHECK_VISIT_TRAINER]   = ValidateVisitingTrainer,
    // [FRONTIER_UTIL_FUNC_INCREMENT_STREAK]      = IncrementWinStreak,
    // [FRONTIER_UTIL_FUNC_RESTORE_HELD_ITEMS]    = RestoreHeldItems,
    // [FRONTIER_UTIL_FUNC_SAVE_BATTLE]           = SaveRecordBattle,
    // [FRONTIER_UTIL_FUNC_BUFFER_TRAINER_NAME]   = BufferFrontierTrainerName,
    // [FRONTIER_UTIL_FUNC_RESET_SKETCH_MOVES]    = ResetSketchedMoves,
    // [FRONTIER_UTIL_FUNC_SET_BRAIN_OBJECT]      = SetFacilityBrainObjectEvent,
};

// code
void CallFrontierUtilFunc(void)
{
    sFrontierUtilFuncs[gSpecialVar_0x8004]();
}

// VAR_TEMP_CHALLENGE_STATUS is used to react to the status in OnFrame map scripts
static void GetChallengeStatus(void)
{
    VarSet(VAR_TEMP_CHALLENGE_STATUS, 0xFF);
    switch (gSaveBlock2Ptr->frontier.challengeStatus)
    {
    case 0:
        break;
    case CHALLENGE_STATUS_SAVING:
        FrontierGamblerSetWonOrLost(FALSE);
        VarSet(VAR_TEMP_CHALLENGE_STATUS, gSaveBlock2Ptr->frontier.challengeStatus);
        break;
    case CHALLENGE_STATUS_LOST:
        FrontierGamblerSetWonOrLost(FALSE);
        VarSet(VAR_TEMP_CHALLENGE_STATUS, gSaveBlock2Ptr->frontier.challengeStatus);
        break;
    case CHALLENGE_STATUS_WON:
        FrontierGamblerSetWonOrLost(TRUE);
        VarSet(VAR_TEMP_CHALLENGE_STATUS, gSaveBlock2Ptr->frontier.challengeStatus);
        break;
    case CHALLENGE_STATUS_PAUSED:
        VarSet(VAR_TEMP_CHALLENGE_STATUS, gSaveBlock2Ptr->frontier.challengeStatus);
        break;
    }
}

static void SetFrontierData(void)
{
    s32 i;
    u8 facility = VarGet(VAR_FRONTIER_FACILITY);
    u8 hasSymbol = GetPlayerSymbolCountForFacility(facility);
    if (hasSymbol == 2)
        hasSymbol = 1;

    switch (gSpecialVar_0x8005)
    {
    case FRONTIER_DATA_CHALLENGE_STATUS:
        gSaveBlock2Ptr->frontier.challengeStatus = gSpecialVar_0x8006;
        break;
    case FRONTIER_DATA_LVL_MODE:
        gSaveBlock2Ptr->frontier.lvlMode = gSpecialVar_0x8006;
        break;
    case FRONTIER_DATA_BATTLE_NUM:
        gSaveBlock2Ptr->frontier.curChallengeBattleNum = gSpecialVar_0x8006;
        break;
    case FRONTIER_DATA_PAUSED:
        gSaveBlock2Ptr->frontier.challengePaused = gSpecialVar_0x8006;
        break;
    case FRONTIER_DATA_SELECTED_MON_ORDER:
        for (i = 0; i < MAX_FRONTIER_PARTY_SIZE; i++)
            gSaveBlock2Ptr->frontier.selectedPartyMons[i] = gSelectedOrderFromParty[i];
        break;
    case FRONTIER_DATA_RECORD_DISABLED:
        gSaveBlock2Ptr->frontier.disableRecordBattle = gSpecialVar_0x8006;
        break;
    case FRONTIER_DATA_HEARD_BRAIN_SPEECH:
        gSaveBlock2Ptr->frontier.battledBrainFlags |= gFrontierBrainInfo[facility].battledBit[hasSymbol];
        break;
    }
}

u8 GetPlayerSymbolCountForFacility(u8 facility)
{
    return FlagGet(FLAG_SYS_TOWER_SILVER + facility * 2)
         + FlagGet(FLAG_SYS_TOWER_GOLD + facility * 2);
}

static void Fill1PRecords(struct RankingHall1P *dst, s32 hallFacilityId, s32 lvlMode)
{
#if FREE_RECORD_MIXING_HALL_RECORDS == FALSE
    s32 i, j;
    struct RankingHall1P record1P[HALL_RECORDS_COUNT + 1];
    struct PlayerHallRecords *playerHallRecords = AllocZeroed(sizeof(struct PlayerHallRecords));
    GetPlayerHallRecords(playerHallRecords);

    for (i = 0; i < HALL_RECORDS_COUNT; i++)
        record1P[i] = gSaveBlock2Ptr->hallRecords1P[hallFacilityId][lvlMode][i];

    record1P[HALL_RECORDS_COUNT] = playerHallRecords->onePlayer[hallFacilityId][lvlMode];

    for (i = 0; i < HALL_RECORDS_COUNT; i++)
    {
        s32 highestWinStreak = 0;
        s32 highestId = 0;
        for (j = 0; j < HALL_RECORDS_COUNT + 1; j++)
        {
            if (record1P[j].winStreak > highestWinStreak)
            {
                highestId = j;
                highestWinStreak = record1P[j].winStreak;
            }
        }
        if (record1P[HALL_RECORDS_COUNT].winStreak >= highestWinStreak)
            highestId = HALL_RECORDS_COUNT;

        dst[i] = record1P[highestId];
        record1P[highestId].winStreak = 0;
    }

    Free(playerHallRecords);
#endif //FREE_RECORD_MIXING_HALL_RECORDS
}

static void Fill2PRecords(struct RankingHall2P *dst, s32 lvlMode)
{
#if FREE_RECORD_MIXING_HALL_RECORDS == FALSE
    s32 i, j;
    struct RankingHall2P record2P[HALL_RECORDS_COUNT + 1];
    struct PlayerHallRecords *playerHallRecords = AllocZeroed(sizeof(struct PlayerHallRecords));
    GetPlayerHallRecords(playerHallRecords);

    for (i = 0; i < HALL_RECORDS_COUNT; i++)
        record2P[i] = gSaveBlock2Ptr->hallRecords2P[lvlMode][i];

    record2P[HALL_RECORDS_COUNT] = playerHallRecords->twoPlayers[lvlMode];

    for (i = 0; i < HALL_RECORDS_COUNT; i++)
    {
        s32 highestWinStreak = 0;
        s32 highestId = 0;
        for (j = 0; j < HALL_RECORDS_COUNT; j++)
        {
            if (record2P[j].winStreak > highestWinStreak)
            {
                highestId = j;
                highestWinStreak = record2P[j].winStreak;
            }
        }
        if (record2P[HALL_RECORDS_COUNT].winStreak >= highestWinStreak)
            highestId = HALL_RECORDS_COUNT;

        dst[i] = record2P[highestId];
        record2P[highestId].winStreak = 0;
    }

    Free(playerHallRecords);
#endif //FREE_RECORD_MIXING_HALL_RECORDS
}

// Battle Frontier Ranking Hall records.
static void Print1PRecord(s32 position, s32 x, s32 y, struct RankingHall1P *hallRecord, s32 hallFacilityId)
{
    u8 text[32];
    u16 winStreak;

    AddTextPrinterParameterized(gRecordsWindowId, FONT_NORMAL, gText_123Dot[position], x * 8, (8 * (y + 5 * position)) + 1, TEXT_SKIP_DRAW, NULL);
    hallRecord->name[PLAYER_NAME_LENGTH] = EOS;
    if (hallRecord->winStreak)
    {
        TVShowConvertInternationalString(text, hallRecord->name, hallRecord->language);
        AddTextPrinterParameterized(gRecordsWindowId, FONT_NORMAL, text, (x + 2) * 8, (8 * (y + 5 * position)) + 1, TEXT_SKIP_DRAW, NULL);
        winStreak = hallRecord->winStreak;
        if (winStreak > MAX_STREAK)
            winStreak = MAX_STREAK;
        ConvertIntToDecimalStringN(gStringVar2, winStreak, STR_CONV_MODE_RIGHT_ALIGN, 4);
        StringExpandPlaceholders(gStringVar4, sHallFacilityToRecordsText[hallFacilityId]);
        AddTextPrinterParameterized(gRecordsWindowId, FONT_NORMAL, gStringVar4, GetStringRightAlignXOffset(FONT_NORMAL, sHallFacilityToRecordsText[hallFacilityId], 0xC8), (8 * (y + 5 * position)) + 1, TEXT_SKIP_DRAW, NULL);
    }
}

static void Print2PRecord(s32 position, s32 x, s32 y, struct RankingHall2P *hallRecord)
{
    u8 text[32];
    u16 winStreak;

    AddTextPrinterParameterized(gRecordsWindowId, FONT_NORMAL, gText_123Dot[position], x * 8, (8 * (y + 5 * position)) + 1, TEXT_SKIP_DRAW, NULL);
    if (hallRecord->winStreak)
    {
        hallRecord->name1[PLAYER_NAME_LENGTH] = EOS;
        hallRecord->name2[PLAYER_NAME_LENGTH] = EOS;
        TVShowConvertInternationalString(text, hallRecord->name1, hallRecord->language);
        AddTextPrinterParameterized(gRecordsWindowId, FONT_NORMAL, text, (x + 2) * 8, (8 * (y + 5 * position - 1)) + 1, TEXT_SKIP_DRAW, NULL);
        if (IsStringJapanese(hallRecord->name2))
            TVShowConvertInternationalString(text, hallRecord->name2, LANGUAGE_JAPANESE);
        else
            StringCopy(text, hallRecord->name2);
        AddTextPrinterParameterized(gRecordsWindowId, FONT_NORMAL, text, (x + 4) * 8, (8 * (y + 5 * position + 1)) + 1, TEXT_SKIP_DRAW, NULL);

        winStreak = hallRecord->winStreak;
        if (winStreak > MAX_STREAK)
            winStreak = MAX_STREAK;
        ConvertIntToDecimalStringN(gStringVar2, winStreak, STR_CONV_MODE_RIGHT_ALIGN, 4);
        StringExpandPlaceholders(gStringVar4, sHallFacilityToRecordsText[RANKING_HALL_TOWER_LINK]);
        AddTextPrinterParameterized(gRecordsWindowId, FONT_NORMAL, gStringVar4, GetStringRightAlignXOffset(FONT_NORMAL, sHallFacilityToRecordsText[RANKING_HALL_TOWER_LINK], 0xC8), (8 * (y + 5 * position)) + 1, TEXT_SKIP_DRAW, NULL);
    }
}

static void PrintHallRecords(s32 hallFacilityId, s32 lvlMode)
{
    s32 i;
    s32 x;
    struct RankingHall1P records1P[HALL_RECORDS_COUNT];
    struct RankingHall2P records2P[HALL_RECORDS_COUNT];

    StringCopy(gStringVar1, sRecordsWindowChallengeTexts[hallFacilityId][0]);
    StringExpandPlaceholders(gStringVar4, sRecordsWindowChallengeTexts[hallFacilityId][1]);
    AddTextPrinterParameterized(gRecordsWindowId, FONT_NORMAL, gStringVar4, 0, 1, TEXT_SKIP_DRAW, NULL);
    x = GetStringRightAlignXOffset(FONT_NORMAL, sLevelModeText[lvlMode], DISPLAY_WIDTH - 32);
    AddTextPrinterParameterized(gRecordsWindowId, FONT_NORMAL, sLevelModeText[lvlMode], x, 1, TEXT_SKIP_DRAW, NULL);
    if (hallFacilityId == RANKING_HALL_TOWER_LINK)
    {
        gSaveBlock2Ptr->frontier.opponentNames[0][PLAYER_NAME_LENGTH] = EOS;
        gSaveBlock2Ptr->frontier.opponentNames[1][PLAYER_NAME_LENGTH] = EOS;
        Fill2PRecords(records2P, lvlMode);
        for (i = 0; i < HALL_RECORDS_COUNT; i++)
            Print2PRecord(i, 1, 4, &records2P[i]);
    }
    else
    {
        Fill1PRecords(records1P, hallFacilityId, lvlMode);
        for (i = 0; i < HALL_RECORDS_COUNT; i++)
            Print1PRecord(i, 1, 4, &records1P[i], hallFacilityId);
    }
}

void ShowRankingHallRecordsWindow(void)
{
    gRecordsWindowId = AddWindow(&sRankingHallRecordsWindowTemplate);
    DrawStdWindowFrame(gRecordsWindowId, FALSE);
    FillWindowPixelBuffer(gRecordsWindowId, PIXEL_FILL(1));
    PrintHallRecords(gSpecialVar_0x8005, FRONTIER_LVL_50);
    PutWindowTilemap(gRecordsWindowId);
    CopyWindowToVram(gRecordsWindowId, COPYWIN_FULL);
}

void ScrollRankingHallRecordsWindow(void)
{
    FillWindowPixelBuffer(gRecordsWindowId, PIXEL_FILL(1));
    PrintHallRecords(gSpecialVar_0x8005, FRONTIER_LVL_OPEN);
    CopyWindowToVram(gRecordsWindowId, COPYWIN_GFX);
}
