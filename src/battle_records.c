#include "global.h"
#include "battle.h"
#include "bg.h"
#include "event_data.h"
#include "gpu_regs.h"
#include "link.h"
#include "malloc.h"
#include "menu.h"
#include "overworld.h"
#include "palette.h"
#include "scanline_effect.h"
#include "sound.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text_window.h"
#include "trainer_card.h"
#include "trainer_pokemon_sprites.h"
#include "trainer_tower.h"
#include "constants/battle.h"
#include "constants/maps.h"
#include "constants/songs.h"

static void CB2_ShowBattleRecords(void);
static void Task_CloseBattleRecordsOnButton(u8 taskId);
static void Task_BeginPaletteFade(u8 taskId);
static void Task_ExitBattleRecords(u8 taskId);
static void RemoveBattleRecordsWindow(u8 windowId);

EWRAM_DATA u8 gRecordsWindowId = 0;
EWRAM_DATA static u16 * sTilemapBuffer = NULL;

static const u16 sBattleRecordsWindowTiles[] = INCBIN_U16("graphics/battle_records/bg_tiles.4bpp");
static const u16 sBattleRecordsWindowPalette[] = INCBIN_U16("graphics/battle_records/bg_tiles.gbapal");
static const u16 sBattleRecordsWindowTilemap[] = INCBIN_U16("graphics/battle_records/bg_tiles.bin");

static const struct BgTemplate sBattleRecordsBgTemplates[] =
{
    {
        .bg = 0,
        .charBaseIndex = 0,
        .mapBaseIndex = 31,
        .screenSize = 0,
        .paletteMode = 0, // 4bpp
        .priority = 0,
        .baseTile = 0x000
    }, {
        .bg = 3,
        .charBaseIndex = 1,
        .mapBaseIndex = 30,
        .screenSize = 0,
        .paletteMode = 0, // 4bpp
        .priority = 3,
        .baseTile = 0x000
    }
};

static const struct WindowTemplate sBattleRecordsWindowTemplates[] =
{
    {
        .bg = 0,
        .tilemapLeft = 2,
        .tilemapTop = 1,
        .width = 27,
        .height = 18,
        .paletteNum = 15,
        .baseBlock = 0x014
    }, DUMMY_WIN_TEMPLATE
};

static const u8 sTextColor[3] = {
    TEXT_COLOR_TRANSPARENT, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY
};

static u8 *const sStringVars[3] = {
    gStringVar1,
    gStringVar2,
    gStringVar3
};

// code
#if FREE_LINK_BATTLE_RECORDS == FALSE
static void ClearLinkBattleRecord(struct LinkBattleRecord *record)
{
    CpuFill16(0, record, sizeof(*record));
    record->name[0] = EOS;
    record->trainerId = 0;
    record->wins = 0;
    record->losses = 0;
    record->draws = 0;
}

static void ClearLinkBattleRecords(struct LinkBattleRecords *records)
{
    s32 i;

    for (i = 0; i < LINK_B_RECORDS_COUNT; i++)
        ClearLinkBattleRecord(&records->entries[i]);
    SetGameStat(GAME_STAT_LINK_BATTLE_WINS, 0);
    SetGameStat(GAME_STAT_LINK_BATTLE_LOSSES, 0);
    SetGameStat(GAME_STAT_LINK_BATTLE_DRAWS, 0);
}

static s32 GetLinkBattleRecordTotalBattles(struct LinkBattleRecord *record)
{
    return record->wins + record->losses + record->draws;
}

static s32 FindLinkBattleRecord(struct LinkBattleRecords *records, const u8 *name, u16 trainerId)
{
    s32 i;

    for (i = 0; i < LINK_B_RECORDS_COUNT; i++)
    {
        if (StringCompareN(records->entries[i].name, name, PLAYER_NAME_LENGTH) == 0 && records->entries[i].trainerId == trainerId)
            return i;
    }

    return LINK_B_RECORDS_COUNT;
}

static void SortLinkBattleRecords(struct LinkBattleRecords * records)
{
    struct LinkBattleRecord tmp;
    s32 i;
    s32 j;

    for (i = LINK_B_RECORDS_COUNT - 1; i > 0; i--)
    {
        for (j = i - 1; j >= 0; j--)
        {
            if (GetLinkBattleRecordTotalBattles(&records->entries[i]) > GetLinkBattleRecordTotalBattles(&records->entries[j]))
            {
                tmp = records->entries[i];
                records->entries[i] = records->entries[j];
                records->entries[j] = tmp;
            }
        }
    }
}

static void UpdateLinkBattleRecord(struct LinkBattleRecord * record, s32 outcome)
{
    switch (outcome)
    {
    case B_OUTCOME_WON:
        record->wins++;
        if (record->wins > 9999)
            record->wins = 9999;
        break;
    case B_OUTCOME_LOST:
        record->losses++;
        if (record->losses > 9999)
            record->losses = 9999;
        break;
    case B_OUTCOME_DREW:
        record->draws++;
        if (record->draws > 9999)
            record->draws = 9999;
        break;
    }
}

static void UpdateLinkBattleGameStats(s32 outcome)
{
    u8 statId;

    switch (outcome)
    {
    case B_OUTCOME_WON:
        statId = GAME_STAT_LINK_BATTLE_WINS;
        break;
    case B_OUTCOME_LOST:
        statId = GAME_STAT_LINK_BATTLE_LOSSES;
        break;
    case B_OUTCOME_DREW:
        statId = GAME_STAT_LINK_BATTLE_DRAWS;
        break;
    default:
        return;
    }

    if (GetGameStat(statId) < 9999)
        IncrementGameStat(statId);
}

static void UpdateLinkBattleRecords(struct LinkBattleRecords *records, const u8 *name, u16 trainerId, s32 outcome, u32 language)
{
    u8 namebuf[PLAYER_NAME_LENGTH + 1];
    s32 i;
    struct LinkBattleRecord * record;

    if (language == LANGUAGE_JAPANESE)
    {
        namebuf[0] = EXT_CTRL_CODE_BEGIN;
        namebuf[1] = EXT_CTRL_CODE_JPN;
        StringCopy(&namebuf[2], name);
    }
    else
        StringCopy(namebuf, name);
    UpdateLinkBattleGameStats(outcome);
    SortLinkBattleRecords(records);
    i = FindLinkBattleRecord(records, namebuf, trainerId);
    if (i == LINK_B_RECORDS_COUNT)
    {
        i = LINK_B_RECORDS_COUNT - 1;
        record = &records->entries[LINK_B_RECORDS_COUNT - 1];
        ClearLinkBattleRecord(record);
        StringCopyN(record->name, namebuf, PLAYER_NAME_LENGTH);
        record->trainerId = trainerId;
    }
    UpdateLinkBattleRecord(&records->entries[i], outcome);
    SortLinkBattleRecords(records);
}
#endif //FREE_LINK_BATTLE_RECORDS

void ClearPlayerLinkBattleRecords(void)
{
#if FREE_LINK_BATTLE_RECORDS == FALSE
    ClearLinkBattleRecords(&gSaveBlock3Ptr->linkBattleRecords);
#endif //FREE_LINK_BATTLE_RECORDS
}

#if FREE_LINK_BATTLE_RECORDS == FALSE
static void IncTrainerCardWins(enum BattlerId battler)
{
    u16 *wins = &gTrainerCards[battler].rse.linkBattleWins;
    (*wins)++;
    if (*wins > 9999)
        *wins = 9999;
}

static void IncTrainerCardLosses(enum BattlerId battler)
{
    u16 *losses = &gTrainerCards[battler].rse.linkBattleLosses;
    (*losses)++;
    if (*losses > 9999)
        *losses = 9999;
}

static void UpdateTrainerCardWinsLosses(enum BattlerId battler)
{
    switch (gBattleOutcome)
    {
    case B_OUTCOME_WON:
        IncTrainerCardWins(BATTLE_OPPOSITE(battler));
        IncTrainerCardLosses(battler);
        break;
    case B_OUTCOME_LOST:
        IncTrainerCardLosses(BATTLE_OPPOSITE(battler));
        IncTrainerCardWins(battler);
        break;
    }
}
#endif //FREE_LINK_BATTLE_RECORDS

void UpdatePlayerLinkBattleRecords(enum BattlerId battler)
{
#if FREE_LINK_BATTLE_RECORDS == FALSE
    if (gSaveBlock1Ptr->location.mapGroup != MAP_GROUP(MAP_UNION_ROOM) || gSaveBlock1Ptr->location.mapNum != MAP_NUM(MAP_UNION_ROOM))
    {
        UpdateTrainerCardWinsLosses(battler);
        UpdateLinkBattleRecords(&gSaveBlock3Ptr->linkBattleRecords, gTrainerCards[battler].rse.playerName, gTrainerCards[battler].rse.trainerId, gBattleOutcome, gLinkPlayers[battler].language);
    }
#endif //FREE_LINK_BATTLE_RECORDS
}

#if FREE_LINK_BATTLE_RECORDS == FALSE
static void PrintLinkBattleWinsLossesDraws(struct LinkBattleRecords *records)
{
    u32 nwins = GetGameStat(GAME_STAT_LINK_BATTLE_WINS);
    u32 nlosses = GetGameStat(GAME_STAT_LINK_BATTLE_LOSSES);
    u32 ndraws = GetGameStat(GAME_STAT_LINK_BATTLE_DRAWS);
    s32 i;
    s32 j;
    bool32 foundEnd;
    u8 * strvar;

    if (nwins > 9999)
        nwins = 9999;
    if (nlosses > 9999)
        nlosses = 9999;
    if (ndraws > 9999)
        ndraws = 9999;

    ConvertIntToDecimalStringN(gStringVar1, nwins, STR_CONV_MODE_LEFT_ALIGN, 4);
    ConvertIntToDecimalStringN(gStringVar2, nlosses, STR_CONV_MODE_LEFT_ALIGN, 4);
    ConvertIntToDecimalStringN(gStringVar3, ndraws, STR_CONV_MODE_LEFT_ALIGN, 4);

    for (i = 0; i < ARRAY_COUNT(sStringVars); i++)
    {
        strvar = sStringVars[i];
        foundEnd = FALSE;
        for (j = 0; j < 4; j++)
        {
            if (!foundEnd && *strvar == EOS)
                foundEnd = TRUE;
            if (foundEnd)
                *strvar = CHAR_SPACE;
            strvar++;
        }
        *strvar = 0xFF;
    }

    StringExpandPlaceholders(gStringVar4, gString_BattleRecords_TotalRecord);
    AddTextPrinterParameterized4(0, FONT_NORMAL, 12, 24, 0, 2, sTextColor, 0, gStringVar4);
}

static void PrintLinkBattleRecord(struct LinkBattleRecord * record, u8 y)
{
    u8 i = 0;
    s32 x;

    if (record->wins == 0 && record->losses == 0 && record->draws == 0)
    {
        AddTextPrinterParameterized4(0, FONT_NORMAL, 0, y, 0, 2, sTextColor, 0, gString_BattleRecords_7Dashes);
        for (i = 0; i < 3; i++)
        {
            if (i == 0)
                x = 0x54;
            else if (i == 1)
                x = 0x84;
            else
                x = 0xB4;
            AddTextPrinterParameterized4(0, FONT_NORMAL, x, y, 0, 2, sTextColor, 0, gString_BattleRecords_4Dashes);
        }
    }
    else
    {
        for (i = 0; i < 4; i++)
        {
            if (i == 0)
            {
                x = 0;
                StringFillWithTerminator(gStringVar1, PLAYER_NAME_LENGTH + 1);
                StringCopyN(gStringVar1, record->name, PLAYER_NAME_LENGTH);
            }
            else if (i == 1)
            {
                x = 0x54;
                ConvertIntToDecimalStringN(gStringVar1, record->wins, STR_CONV_MODE_RIGHT_ALIGN, 4);
            }
            else if (i == 2)
            {
                x = 0x84;
                ConvertIntToDecimalStringN(gStringVar1, record->losses, STR_CONV_MODE_RIGHT_ALIGN, 4);
            }
            else
            {
                x = 0xB4;
                ConvertIntToDecimalStringN(gStringVar1, record->draws, STR_CONV_MODE_RIGHT_ALIGN, 4);
            }
            AddTextPrinterParameterized4(0, FONT_NORMAL, x, y, 0, 2, sTextColor, 0, gStringVar1);
        }
    }
}
#endif //FREE_LINK_BATTLE_RECORDS

static void PrintLinkBattleRecords(void)
{
#if FREE_LINK_BATTLE_RECORDS == FALSE
    u32 left;
    s32 i;

    FillWindowPixelRect(0, PIXEL_FILL(0), 0, 0, 0xD8, 0x90);
    StringExpandPlaceholders(gStringVar4, gString_BattleRecords_PlayersBattleResults);
    left = 208 - GetStringWidth(FONT_NORMAL, gStringVar4, -1);
    AddTextPrinterParameterized4(0, FONT_NORMAL, left / 2, 4, 0, 2, sTextColor, 0, gStringVar4);
    PrintLinkBattleWinsLossesDraws(&gSaveBlock3Ptr->linkBattleRecords);
    AddTextPrinterParameterized4(0, FONT_NORMAL, 0x54, 0x30, 0, 2, sTextColor, 0, gString_BattleRecords_ColumnHeaders);
    for (i = 0; i < LINK_B_RECORDS_COUNT; i++)
        PrintLinkBattleRecord(&gSaveBlock3Ptr->linkBattleRecords.entries[i], 0x3D + 14 * i);

    PutWindowTilemap(0);
    CopyWindowToVram(0, COPYWIN_FULL);
#endif //FREE_LINK_BATTLE_RECORDS
}

void RemoveRecordsWindow(void)
{
    ClearStdWindowAndFrame(gRecordsWindowId, FALSE);
    RemoveWindow(gRecordsWindowId);
}

static void Task_BattleRecordsWaitForPaletteFade(u8 taskId)
{
    if (!gPaletteFade.active)
        gTasks[taskId].func = Task_CloseBattleRecordsOnButton;
}

static void Task_CloseBattleRecordsOnButton(u8 taskId)
{
    struct Task *task = &gTasks[taskId];

    if (JOY_NEW(A_BUTTON) || JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        task->func = Task_BeginPaletteFade;
    }
}

static void Task_BeginPaletteFade(u8 taskId)
{
    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
    gTasks[taskId].func = Task_ExitBattleRecords;
}

static void Task_ExitBattleRecords(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        SetMainCallback2(CB2_ReturnToFieldContinueScriptPlayMapMusic);
        Free(sTilemapBuffer);
        RemoveBattleRecordsWindow(0);
        FreeAllWindowBuffers();
        DestroyTask(taskId);
    }
}

static void RemoveBattleRecordsWindow(u8 windowId)
{
    FillWindowPixelBuffer(windowId, PIXEL_FILL(0));
    ClearWindowTilemap(windowId);
    CopyWindowToVram(windowId, COPYWIN_GFX);
    RemoveWindow(windowId);
}

static void ClearVramOamPlttRegs(void)
{
    DmaClearLarge16(3, (void *)VRAM, VRAM_SIZE, 0x1000);
    DmaClear32(3, (void *)OAM, OAM_SIZE);
    DmaClear16(3, (void *)PLTT, PLTT_SIZE);

    SetGpuReg(REG_OFFSET_DISPCNT, 0);
    SetGpuReg(REG_OFFSET_BG0CNT, 0);
    SetGpuReg(REG_OFFSET_BG0HOFS, 0);
    SetGpuReg(REG_OFFSET_BG0VOFS, 0);
    SetGpuReg(REG_OFFSET_BG1CNT, 0);
    SetGpuReg(REG_OFFSET_BG1HOFS, 0);
    SetGpuReg(REG_OFFSET_BG1VOFS, 0);
    SetGpuReg(REG_OFFSET_BG2CNT, 0);
    SetGpuReg(REG_OFFSET_BG2HOFS, 0);
    SetGpuReg(REG_OFFSET_BG2VOFS, 0);
    SetGpuReg(REG_OFFSET_BG3CNT, 0);
    SetGpuReg(REG_OFFSET_BG3HOFS, 0);
    SetGpuReg(REG_OFFSET_BG3VOFS, 0);
    SetGpuReg(REG_OFFSET_WIN0H, 0);
    SetGpuReg(REG_OFFSET_WIN0V, 0);
    SetGpuReg(REG_OFFSET_WININ, 0);
    SetGpuReg(REG_OFFSET_WINOUT, 0);
    SetGpuReg(REG_OFFSET_BLDCNT, 0);
    SetGpuReg(REG_OFFSET_BLDALPHA, 0);
    SetGpuReg(REG_OFFSET_BLDY, 0);
}

static void ClearTasksAndGraphicalStructs(void)
{
    ScanlineEffect_Stop();
    ResetTasks();
    ResetSpriteData();
    ResetAllPicSprites();
    ResetPaletteFade();
    FreeAllSpritePalettes();
}

static void ResetBgCoordinates(void)
{
    ChangeBgX(0, 0, 0);
    ChangeBgY(0, 0, 0);
    ChangeBgX(1, 0, 0);
    ChangeBgY(1, 0, 0);
    ChangeBgX(2, 0, 0);
    ChangeBgY(2, 0, 0);
    ChangeBgX(3, 0, 0);
    ChangeBgY(3, 0, 0);
}

static void SetDispcntReg(void)
{
    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_MODE_0 | DISPCNT_OBJ_1D_MAP | DISPCNT_BG0_ON | DISPCNT_BG3_ON);
}


static void LoadBattleRecordsWindowGfx(u8 bg)
{
    LoadBgTiles(bg, sBattleRecordsWindowTiles, sizeof(sBattleRecordsWindowTiles), 0);
    CopyToBgTilemapBufferRect(bg, sBattleRecordsWindowTilemap, 0, 0, 32, 32);
    LoadPalette(sBattleRecordsWindowPalette, BG_PLTT_ID(0), sizeof(sBattleRecordsWindowPalette));
}

static void VBlankCB_BattleRecords(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void MainCB2_BattleRecords(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

void ShowBattleRecords(void)
{
    SetVBlankCallback(NULL);
    SetMainCallback2(CB2_ShowBattleRecords);
}

static void CB2_ShowBattleRecords(void)
{
    switch (gMain.state)
    {
    case 0:
        SetVBlankCallback(NULL);
        ClearVramOamPlttRegs();
        gMain.state++;
        break;
    case 1:
        ClearTasksAndGraphicalStructs();
        gMain.state++;
        break;
    case 2:
        sTilemapBuffer = AllocZeroed(BG_SCREEN_SIZE);
        ResetBgsAndClearDma3BusyFlags(0);
        InitBgsFromTemplates(0, sBattleRecordsBgTemplates, ARRAY_COUNT(sBattleRecordsBgTemplates));
        SetBgTilemapBuffer(3, sTilemapBuffer);
        ResetBgCoordinates();
        gMain.state++;
        break;
    case 3:
        LoadBattleRecordsWindowGfx(3);
        LoadPalette(GetTextWindowPalette(0), BG_PLTT_ID(15), PLTT_SIZE_4BPP);
        gMain.state++;
        break;
    case 4:
        if (IsDma3ManagerBusyWithBgCopy() != TRUE)
        {
            ShowBg(0);
            ShowBg(3);
            CopyBgTilemapBufferToVram(3);
            gMain.state++;
        }
        break;
    case 5:
        InitWindows(sBattleRecordsWindowTemplates);
        DeactivateAllTextPrinters();
        gMain.state++;
        break;
    case 6:
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        gMain.state++;
        break;
    case 7:
        SetDispcntReg();
        SetVBlankCallback(VBlankCB_BattleRecords);
        if (gSpecialVar_0x8004)
            PrintTrainerTowerRecords();
        else
            PrintLinkBattleRecords();

        CreateTask(Task_BattleRecordsWaitForPaletteFade, 8);
        SetMainCallback2(MainCB2_BattleRecords);
        gMain.state = 0;
        break;
    }
}
