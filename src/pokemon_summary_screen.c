#include "global.h"
#include "battle_anim.h"
#include "battle_factory.h"
#include "battle_interface.h"
#include "battle_main.h"
#include "data.h"
#include "decompress.h"
#include "dynamic_placeholder_text_util.h"
#include "event_data.h"
#include "field_specials.h"
#include "gpu_regs.h"
#include "graphics.h"
#include "help_system.h"
#include "item.h"
#include "link.h"
#include "malloc.h"
#include "menu_helpers.h"
#include "menu.h"
#include "mon_markings.h"
#include "overworld.h"
#include "palette.h"
#include "party_menu.h"
#include "pokeball.h"
#include "pokemon_icon.h"
#include "pokemon_sprite_visualizer.h"
#include "pokemon_storage_system.h"
#include "pokemon_summary_screen.h"
#include "pokemon.h"
#include "pokerus.h"
#include "region_map.h"
#include "scanline_effect.h"
#include "sound.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "trade.h"
#include "trainer_pokemon_sprites.h"
#include "type_icon_sprite.h"
#include "constants/battle_move_effects.h"
#include "constants/battle.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/party_menu.h"
#include "constants/region_map_sections.h"
#include "constants/songs.h"
#include "constants/sound.h"

#define TAG_CATEGORY_ICONS 30004
#define TAG_MON_SPRITE 30005

enum SummaryWindow
{
    PSS_WIN_PAGE_NAME,
    PSS_WIN_CONTROLS,
    PSS_WIN_LVL_NICK,
    SUMMARY_PRIMARY_WIN_COUNT,

    PSS_SECONDARY_WIN_1 = SUMMARY_PRIMARY_WIN_COUNT,
    PSS_SECONDARY_WIN_2,
    PSS_SECONDARY_WIN_3,
    PSS_SECONDARY_WIN_4,

    SUMMARY_WINDOW_COUNT,
};

#define POKESUM_WIN_RIGHT_PANE   PSS_SECONDARY_WIN_1
#define POKESUM_WIN_TRAINER_MEMO PSS_SECONDARY_WIN_2
#define SUMMARY_SECONDARY_WIN_COUNT (SUMMARY_WINDOW_COUNT - SUMMARY_PRIMARY_WIN_COUNT)


enum EggHatchTime
{
    EGG_HATCH_TIME_LONG,
    EGG_HATCH_TIME_SOME,
    EGG_HATCH_TIME_SOON,
    EGG_HATCH_TIME_ALMOST_READY,
};

enum EggOrigin
{
    EGG_ORIGIN_DAYCARE,
    EGG_ORIGIN_TRADE,
    EGG_ORIGIN_TRAVELING_MAN,
    EGG_ORIGIN_NICE_PLACE,
    EGG_ORIGIN_SPA,
};

enum MoveTextColor
{
    MOVE_TEXT_COLOR_0,
    MOVE_TEXT_COLOR_1,
    MOVE_TEXT_COLOR_2,
    MOVE_TEXT_COLOR_3,
};

static bool32 CurrentMonIsFromGBA(void);
static bool32 IsMultiBattlePartner(void);
static bool32 MapSecIsInKantoOrSevii(u8 mapSec);
static enum Move GetMonMoveBySlotId(struct Pokemon *mon, u8 moveSlot);
static s8 AdvanceMultiBattleMonIndex(s8 direction);
static s8 AdvanceMonIndex(s8 direction);
static u16 GetMonPpByMoveSlot(struct Pokemon *mon, u8 moveSlot);
static u8 PokeSum_BufferOtName_IsEqualToCurrentOwner(struct Pokemon *mon);
static u8 PokeSum_HandleCreateSprites(void);
static u8 PokeSum_HandleLoadBgGfx(void);
static u8 PokeSum_IsPageFlipFinished(u8);
static u8 PokeSum_Setup_BufferStrings(void);
static u8 StatusToAilment(u32 status);
static void BufferMonInfo(void);
static void BufferMonMoveI(u8);
static void BufferMonMoves(void);
static void BufferMonSkills(void);
static void BufferSelectedMonData(struct Pokemon *mon);
static void MainCB2(void);
static void CB2_InitSummaryScreen(void);
static void CommitStaticWindowTilemaps(void);
static void CreateBallIconObj(void);
static void CreateExpBarObjs(u16, u16);
static void CreateHpBarObjs(u16, u16);
static void CreateStatusIconSprite(u16, u16);
static void CreateMoveSelectionCursorSprites(u16, u16);
static void CreatePokerusIconSprite(u16, u16);
static void CreateShinyIconSprite(u16, u16);
static void CreateTypeIconSprites(void);
static void DestroyTypeIconSprites(void);
static void HideMonTypeIcons(void);
static void HideMoveTypeIcons(void);
static void HideShowPokerusIcon(u8 invisible);
static void HideShowShinyStar(u8 invisible);
static void PokeSum_AddWindows(enum PokemonSummaryScreenPage curPageIndex);
static void PokeSum_CopyNewBgTilemapBeforePageFlip_2(void);
static void PokeSum_CopyNewBgTilemapBeforePageFlip(void);
static void PokeSum_CreateMonIconSprite(void);
static void PokeSum_CreateMonMarkingsSprite(void);
static void PokeSum_CreateMonPicSprite(void);
static void PokeSum_CreateWindows(void);
static void PokeSum_DestroyMonMarkingsSprite(void);
static void PokeSum_DestroySprites(void);
static void PokeSum_DrawMoveTypeIcons(void);
static void PokeSum_DrawPageProgressTiles(void);
static void PokeSum_FinishSetup(void);
static void PokeSum_FlipPages_HandleBgHofs(void);
static void PokeSum_HideSpritesBeforePageFlip(void);
static void PokeSum_InitBgCoordsBeforePageFlips(void);
static void PokeSum_PrintAbilityDataOrMoveTypes(void);
static void PokeSum_PrintAbilityNameAndDesc(void);
static void PokeSum_PrintBottomPaneText(void);
static void PokeSum_PrintExpPoints_NextLv(void);
static void PokeSum_PrintMonTypeIcons(void);
static void PokeSum_PrintMoveName(u8 i);
static void PokeSum_PrintPageHeaderText(u8 curPageIndex);
static void PokeSum_PrintPageName(const u8 *str);
static void PokeSum_PrintRightPaneText(void);
static void PokeSum_PrintSelectedMoveStats(void);
static void PokeSum_PrintTrainerMemo_Egg(void);
static void PokeSum_PrintTrainerMemo_Mon_NotHeldByOT(void);
static void PokeSum_PrintTrainerMemo_Mon(void);
static void PokeSum_PrintTrainerMemo(void);
static void PokeSum_RemoveWindows(void);
static void ChangeSummaryPokemon(u8 taskId, s8 direction);
static void PokeSum_SetHelpContext(void);
static void SetMonPicSpriteCallback(u16 spriteId);
static void PokeSum_Setup_InitGpu(void);
static void PokeSum_Setup_SetVBlankCallback(void);
static void PokeSum_Setup_SpritesReset(void);
static void PokeSum_ShowOrHideMonIconSprite(u8 invisible);
static void PokeSum_ShowOrHideMonMarkingsSprite(u8 invisible);
static void PokeSum_ShowOrHideMonPicSprite(u8 invisible);
static void PokeSum_ShowSpritesBeforePageFlip(void);
static void PlayMonCry(void);
static void PokeSum_UpdateBgPriorityForPageFlip(u8 setBg0Priority, u8 keepBg1Bg2PriorityOrder);
static void PokeSum_UpdateMonMarkingsAnim(void);
static void PokeSum_UpdateWin1ActiveFlag(u8 curPageIndex);
static void PrintControlsString(void);
static void PrintInfoPage(void);
static void PrintMonLevelNickOnWindow2(const u8 *str);
static void PrintMovesPage(void);
static void PrintSkillsPage(void);
static void ShowOrHideBallIconObj(u8 invisible);
static void ShowOrHideExpBarObjs(u8 invisible);
static void ShowOrHideHpBarObjs(u8 invisible);
static void ShoworHideMoveSelectionCursor(u8 invisible);
static void ShowOrHideStatusIcon(u8 invisible);
static void ShowPokerusIconObjIfHasOrHadPokerus(void);
static void ShowShinyStarObjIfMonShiny(void);
static void SpriteCB_MoveSelectionCursor(struct Sprite *sprite);
static void SwapBoxMonMoveSlots(void);
static void SwapMonMoveSlots(void);
static void Task_DestroyResourcesOnExit(u8 taskId);
static void Task_FlipPages_FromInfo(u8 taskId);
static void Task_HandleInput_SelectMove(u8 id);
static void Task_InputHandler_SelectOrForgetMove(u8 taskId);
static void Task_PokeSum_FlipPages(u8 taskId);
static void Task_PokeSum_SwitchDisplayedPokemon(u8 taskId);
static void UpdateCurrentMonBufferFromPartyOrBox(struct Pokemon *mon);
static void UpdateExpBarObjs(void);
static void UpdateHpBarObjs(void);
static void UpdateMonStatusIconObj(void);
static void UpdateMonTypeIconSprites(bool32 isInfoPage);
static void UpdateMoveTypeIconSprites(void);
static void CB2_PssChangePokemonNickname(void);

struct PokemonSummaryScreenData
{
    union
    {
        struct Pokemon *mons;
        struct BoxPokemon *boxMons;
    } monList;
    bool32 isEnemyParty;
    enum PokemonSummaryScreenPage curPageIndex;
    enum PokemonSummaryScreenSkillPageMode skillsPageMode:2;
    enum PokemonSummaryScreenState screenState;
    MainCallback savedCallback;
    struct Pokemon currentMon;
    struct Sprite *markingSprite;
    enum Move moveIds[MAX_MON_MOVES + 1];
    enum Type moveTypes[MAX_MON_MOVES + 1];
    s16 flipPagesBgHofs;
    u16 atkStrXpos;
    u16 bg1TilemapBuffer[0x800];
    u16 bg2TilemapBuffer[0x800];
    u16 bg3TilemapBuffer[0x800];
    u16 curHpStrXpos;
    u16 curPpXpos[MAX_MON_MOVES + 1];
    u16 defStrXpos;
    u16 expStrXpos;
    u16 maxPpXpos[MAX_MON_MOVES + 1];
    u16 spAStrXpos;
    u16 spDStrXpos;
    u16 speStrXpos;
    u16 toNextLevelXpos;
    u8 abilityDescStrBuf[52];
    u8 abilityNameStrBuf[ABILITY_NAME_LENGTH + 1];
    u8 ballIconSpriteId;
    u8 bufferStringsStep;
    u8 categoryIconSpriteId;
    u8 curMonStatusAilment;
    u8 dexNumStrBuf[5];
    u8 expPointsStrBuf[9];
    u8 expToNextLevelStrBuf[9];
    u8 flippingPages;
    u8 genderSymbolStrBuf[3];
    u8 infoAndMovesPageBgNum;
    u8 inhibitPageFlipInput;
    u8 inputHandlerTaskId;
    u8 isBadEgg;
    u8 isBoxMon;
    u8 isEgg;
    u8 isSwappingMoves;
    u8 itemNameStrBuf[ITEM_NAME_LENGTH + 1];
    u8 maxMonIndex;
    u8 lastPageFlipDirection;
    u8 levelStrBuf[7];
    u8 loadBgGfxStep;
    u8 lockMovesFlag;
    u8 mode;
    u8 monIconSpriteId;
    u8 monPicSpriteId;
    u8 monTypeIconSpriteIds[3];
    u8 monTypes[3];
    u8 moveAccuracyStrBufs[MAX_MON_MOVES + 1][5];
    u8 moveCurPpStrBufs[MAX_MON_MOVES + 1][11];
    u8 moveMaxPpStrBufs[MAX_MON_MOVES + 1][11];
    u8 moveNameStrBufs[MAX_MON_MOVES + 1][MOVE_NAME_LENGTH + 1];
    u8 movePowerStrBufs[MAX_MON_MOVES + 1][5];
    u8 moveTypeIconSpriteIds[MAX_MON_MOVES + 1];
    u8 nicknameStrBuf[POKEMON_NAME_LENGTH + 1];
    u8 numMonPicBounces;
    u8 numMoves;
    u8 otIdStrBuf[7];
    u8 otNameStrBuf[12];
    u8 otNameStrBufs[2][12];
    u8 pageFlipDirection;
    u8 selectMoveInputHandlerState;
    u8 skillsPageBgNum;
    u8 speciesNameStrBuf[POKEMON_NAME_LENGTH + 1];
    u8 spriteCreationStep;
    u8 statValueStrBufs[NUM_STATS][30];
    u8 switchMonTaskState;
    u8 taskState;
    u8 whichBgLayerToTranslate;
    u8 windowIds[SUMMARY_WINDOW_COUNT];
};

struct ExpBarObjs
{
    struct Sprite *sprites[11];
    u16 xpos[11];
};

struct HpBarObjs
{
    struct Sprite *sprites[10];
    u16 xpos[10];
};

struct MonPicBounceState
{
    u8 animFrame;
    u8 initDelay;
    u8 vigor;
};

EWRAM_DATA u8 gLastViewedMonIndex = 0;
EWRAM_DATA MainCallback gInitialSummaryScreenCallback = NULL; // stores callback from the first time the screen is opened from the party or PC menu
static EWRAM_DATA struct PokemonSummaryScreenData *sMonSummaryScreen = NULL;
static EWRAM_DATA struct Sprite *sMoveSelectionCursorObjs[4] = {};
static EWRAM_DATA struct Sprite *sStatusIcon = NULL;
static EWRAM_DATA struct Sprite *sPokerusIconObj = NULL;
static EWRAM_DATA struct Sprite *sShinyStarObjData = NULL;
static EWRAM_DATA struct HpBarObjs *sHpBarObjs = NULL;
static EWRAM_DATA struct ExpBarObjs *sExpBarObjs = NULL;
static EWRAM_DATA u8 sMoveSelectionCursorPos = 0;
static EWRAM_DATA u8 sMoveSwapCursorPos = 0;
static EWRAM_DATA struct MonPicBounceState *sMonPicBounceState = NULL;

#include "data/pokemon_summary_screen.h"

static const struct StatData sStatData[] = {
    [STAT_HP] =
    {
        .monDataStat            = MON_DATA_HP,
        .monDataEv              = MON_DATA_HP_EV,
        .monDataIv              = MON_DATA_HP_IV,
        .monDataHyperTrained    = MON_DATA_HYPER_TRAINED_HP,
        .pssStat                = PSS_STAT_HP,
    },
    [STAT_ATK] =
    {
        .monDataStat            = MON_DATA_ATK,
        .monDataEv              = MON_DATA_ATK_EV,
        .monDataIv              = MON_DATA_ATK_IV,
        .monDataHyperTrained    = MON_DATA_HYPER_TRAINED_ATK,
        .pssStat                = PSS_STAT_ATK,
    },
    [STAT_DEF] =
    {
        .monDataStat            = MON_DATA_DEF,
        .monDataEv              = MON_DATA_DEF_EV,
        .monDataIv              = MON_DATA_DEF_IV,
        .monDataHyperTrained    = MON_DATA_HYPER_TRAINED_DEF,
        .pssStat                = PSS_STAT_DEF,
    },
    [STAT_SPATK] =
    {
        .monDataStat            = MON_DATA_SPATK,
        .monDataEv              = MON_DATA_SPATK_EV,
        .monDataIv              = MON_DATA_SPATK_IV,
        .monDataHyperTrained    = MON_DATA_HYPER_TRAINED_SPATK,
        .pssStat                = PSS_STAT_SPA,
    },
    [STAT_SPDEF] =
    {
        .monDataStat            = MON_DATA_SPDEF,
        .monDataEv              = MON_DATA_SPDEF_EV,
        .monDataIv              = MON_DATA_SPDEF_IV,
        .monDataHyperTrained    = MON_DATA_HYPER_TRAINED_SPDEF,
        .pssStat                = PSS_STAT_SPD,
    },
    [STAT_SPEED] =
    {
        .monDataStat            = MON_DATA_SPEED,
        .monDataEv              = MON_DATA_SPEED_EV,
        .monDataIv              = MON_DATA_SPEED_IV,
        .monDataHyperTrained    = MON_DATA_HYPER_TRAINED_SPEED,
        .pssStat                = PSS_STAT_SPE,
    },
};

void ShowPokemonSummaryScreen(void *party, u8 cursorPos, u8 maxMonIndex, MainCallback savedCallback, u8 mode)
{
    sMonSummaryScreen = AllocZeroed(sizeof(struct PokemonSummaryScreenData));

    if (sMonSummaryScreen == NULL)
    {
        SetMainCallback2(savedCallback);
        return;
    }

    sMoveSelectionCursorPos = 0;
    sMoveSwapCursorPos = 0;
    sMonSummaryScreen->savedCallback = savedCallback;
    if (gInitialSummaryScreenCallback == NULL)
        gInitialSummaryScreenCallback = savedCallback;

    if (party == gEnemyParty)
        sMonSummaryScreen->isEnemyParty = TRUE;
    else
        sMonSummaryScreen->isEnemyParty = FALSE;

    sMonSummaryScreen->mode = mode;
    if (cursorPos == PC_MON_CHOSEN)
    {
        sMonSummaryScreen->monList.boxMons = GetBoxedMonPtr(gSpecialVar_MonBoxId, 0);
        gLastViewedMonIndex = gSpecialVar_MonBoxPos;
        sMonSummaryScreen->maxMonIndex = IN_BOX_COUNT - 1;
    }
    else
    {
        sMonSummaryScreen->monList.mons = party;
        gLastViewedMonIndex = cursorPos;
        sMonSummaryScreen->maxMonIndex = maxMonIndex;
    }

    if (mode == PSS_MODE_BOX || cursorPos == PC_MON_CHOSEN)
        sMonSummaryScreen->isBoxMon = TRUE;
    else
        sMonSummaryScreen->isBoxMon = FALSE;

    switch (sMonSummaryScreen->mode)
    {
    case PSS_MODE_NORMAL:
    default:
        SetHelpContext(HELPCONTEXT_POKEMON_INFO);
        sMonSummaryScreen->curPageIndex = PSS_PAGE_INFO;
        sMonSummaryScreen->lockMovesFlag = FALSE;
        break;
    case PSS_MODE_BOX:
        SetHelpContext(HELPCONTEXT_POKEMON_INFO);
        sMonSummaryScreen->curPageIndex = PSS_PAGE_INFO;
        sMonSummaryScreen->lockMovesFlag = FALSE;
        break;
    case PSS_MODE_SELECT_MOVE:
    case PSS_MODE_FORGET_MOVE:
        SetHelpContext(HELPCONTEXT_POKEMON_MOVES);
        sMonSummaryScreen->curPageIndex = PSS_PAGE_MOVES_INFO;
        sMonSummaryScreen->lockMovesFlag = TRUE;
        break;
    }

    sMonSummaryScreen->screenState = PSS_STATE_FADEIN;
    sMonSummaryScreen->loadBgGfxStep = 0;
    sMonSummaryScreen->spriteCreationStep = 0;

    sMonSummaryScreen->whichBgLayerToTranslate = 0;
    sMonSummaryScreen->skillsPageBgNum = 2;
    sMonSummaryScreen->infoAndMovesPageBgNum = 1;
    sMonSummaryScreen->flippingPages = FALSE;
    sMonSummaryScreen->categoryIconSpriteId = 0xFF;
    for (u32 i = 0; i < sizeof(sMonSummaryScreen->moveTypeIconSpriteIds); i++)
        sMonSummaryScreen->moveTypeIconSpriteIds[i] = 0xFF;
    for (u32 i = 0; i < sizeof(sMonSummaryScreen->monTypeIconSpriteIds); i++)
        sMonSummaryScreen->monTypeIconSpriteIds[i] = 0xFF;
    sMonSummaryScreen->skillsPageMode = PSS_SKILL_PAGE_STATS;

    BufferSelectedMonData(&sMonSummaryScreen->currentMon);
    sMonSummaryScreen->isBadEgg = GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_SANITY_IS_BAD_EGG);

    if (sMonSummaryScreen->isBadEgg == TRUE)
        sMonSummaryScreen->isEgg = TRUE;
    else
        sMonSummaryScreen->isEgg = GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_IS_EGG);

    sMonSummaryScreen->lastPageFlipDirection = 0xFF;
    SetMainCallback2(CB2_InitSummaryScreen);
}

void ShowSelectMovePokemonSummaryScreen(struct Pokemon *party, u8 cursorPos, MainCallback savedCallback, enum Move move)
{
    ShowPokemonSummaryScreen(party, cursorPos, gPlayerPartyCount - 1, savedCallback, PSS_MODE_SELECT_MOVE);
    sMonSummaryScreen->moveIds[MAX_MON_MOVES] = move;
}

static u8 PageFlipInputIsDisabled(u8 direction)
{
    if (sMonSummaryScreen->inhibitPageFlipInput == TRUE && sMonSummaryScreen->pageFlipDirection != direction)
        return TRUE;

    return FALSE;
}

bool32 IsPageFlipInput(u8 direction)
{
    if (sMonSummaryScreen->isEgg)
        return FALSE;

    if (sMonSummaryScreen->lastPageFlipDirection != 0xff && sMonSummaryScreen->lastPageFlipDirection == direction)
    {
        sMonSummaryScreen->lastPageFlipDirection = 0xff;
        return TRUE;
    }

    if (PageFlipInputIsDisabled(direction))
        return FALSE;

    switch (direction)
    {
    case 1:
        if (JOY_NEW(DPAD_RIGHT))
            return TRUE;

        if (gSaveBlock2Ptr->optionsButtonMode == OPTIONS_BUTTON_MODE_LR && JOY_NEW(R_BUTTON))
            return TRUE;

        break;
    case 0:
        if (JOY_NEW(DPAD_LEFT))
            return TRUE;

        if (gSaveBlock2Ptr->optionsButtonMode == OPTIONS_BUTTON_MODE_LR && JOY_NEW(L_BUTTON))
            return TRUE;

        break;
    }

    return FALSE;
}

static const u8 *const sStatControlStrings[] =
{
    [PSS_SKILL_PAGE_STATS] = sText_PokeSum_Controls_PageStats,
    [PSS_SKILL_PAGE_EVS] = sText_PokeSum_Controls_PageEVs,
    [PSS_SKILL_PAGE_IVS] = sText_PokeSum_Controls_PageIVs,
};

static enum PokemonSummaryScreenSkillPageMode GetNextSkillsPageMode(void)
{
    if (!P_SUMMARY_SCREEN_IV_EV_INFO)
        return PSS_SKILL_PAGE_STATS;

    if (P_SUMMARY_SCREEN_IV_EV_BOX_ONLY && sMonSummaryScreen->mode != PSS_MODE_BOX)
        return PSS_SKILL_PAGE_STATS;

    switch (sMonSummaryScreen->skillsPageMode)
    {
    case PSS_SKILL_PAGE_STATS:
        if (P_SUMMARY_SCREEN_EV_ONLY)
            return PSS_SKILL_PAGE_EVS;
        else
            return PSS_SKILL_PAGE_IVS;
    case PSS_SKILL_PAGE_IVS:
        if (P_SUMMARY_SCREEN_IV_ONLY)
            return PSS_SKILL_PAGE_STATS;
        else
            return PSS_SKILL_PAGE_EVS;
    case PSS_SKILL_PAGE_EVS:
    default:
        return PSS_SKILL_PAGE_STATS;
    }
}

static const u8 *GetStatControlString(void)
{
    if (!P_SUMMARY_SCREEN_IV_EV_INFO)
        return sText_PokeSum_Controls_Page;

    if (!P_SUMMARY_SCREEN_IV_EV_BOX_ONLY)
        return sStatControlStrings[GetNextSkillsPageMode()];

    if (sMonSummaryScreen->mode == PSS_MODE_BOX)
        return sStatControlStrings[GetNextSkillsPageMode()];

    return sText_PokeSum_Controls_Page;
}

static bool32 CanRename(void)
{
    if (P_SUMMARY_SCREEN_RENAME == FALSE)
        return FALSE;
    if (gMain.inBattle)
        return FALSE;
    if (sMonSummaryScreen->isEgg)
        return FALSE;
    if (InBattleFactory())
        return FALSE;
    if (GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_OT_ID) != GetPlayerTrainerId())
        return FALSE;

    return TRUE;
}

static bool32 ShouldShowIvEvPrompt()
{
    if (P_SUMMARY_SCREEN_IV_EV_BOX_ONLY)
    {
        return (P_SUMMARY_SCREEN_IV_EV_INFO || FlagGet(P_FLAG_SUMMARY_SCREEN_IV_EV_INFO))
            && sMonSummaryScreen->isBoxMon;
    }
    else if (!P_SUMMARY_SCREEN_IV_EV_BOX_ONLY)
    {
        return (P_SUMMARY_SCREEN_IV_EV_INFO || FlagGet(P_FLAG_SUMMARY_SCREEN_IV_EV_INFO));
    }
    return FALSE;
}

static void Task_InputHandler_Info(u8 taskId)
{
    switch (sMonSummaryScreen->screenState) {
    case PSS_STATE_FADEIN:
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, 0);
        sMonSummaryScreen->screenState = PSS_STATE_PLAYCRY;
        break;
    case PSS_STATE_PLAYCRY:
        if (!gPaletteFade.active)
        {
            PlayMonCry();
            sMonSummaryScreen->screenState = PSS_STATE_HANDLEINPUT;
            return;
        }

        sMonSummaryScreen->screenState = PSS_STATE_PLAYCRY;
        break;
    case PSS_STATE_HANDLEINPUT:
        if (IsActiveOverworldLinkBusy() == TRUE)
            return;
        if (IsLinkRecvQueueAtOverworldMax() == TRUE)
            return;
        if (FuncIsActiveTask(Task_PokeSum_SwitchDisplayedPokemon))
            return;

        if (sMonSummaryScreen->curPageIndex != PSS_PAGE_MOVES_INFO)
        {
            if (IsPageFlipInput(1) == TRUE)
            {
                if (FuncIsActiveTask(Task_PokeSum_FlipPages))
                {
                    sMonSummaryScreen->lastPageFlipDirection = 1;
                    return;
                }
                else if (sMonSummaryScreen->curPageIndex < PSS_PAGE_MOVES)
                {
                    PlaySE(SE_SELECT);
                    HideBg(0);
                    sMonSummaryScreen->pageFlipDirection = 1;
                    PokeSum_RemoveWindows();
                    sMonSummaryScreen->curPageIndex++;
                    sMonSummaryScreen->screenState = PSS_STATE_FLIPPAGES;
                }
                return;
            }
            else if (IsPageFlipInput(0) == TRUE)
            {
                if (FuncIsActiveTask(Task_PokeSum_FlipPages))
                {
                    sMonSummaryScreen->lastPageFlipDirection = 0;
                    return;
                }
                else if (sMonSummaryScreen->curPageIndex > PSS_PAGE_INFO)
                {
                    PlaySE(SE_SELECT);
                    HideBg(0);
                    sMonSummaryScreen->pageFlipDirection = 0;
                    PokeSum_RemoveWindows();
                    sMonSummaryScreen->curPageIndex--;
                    sMonSummaryScreen->screenState = PSS_STATE_FLIPPAGES;
                }
                return;
            }
        }

        if ((!FuncIsActiveTask(Task_PokeSum_FlipPages)) || FuncIsActiveTask(Task_PokeSum_SwitchDisplayedPokemon))
        {
            if (JOY_NEW(DPAD_UP))
            {
                ChangeSummaryPokemon(taskId, -1);
                return;
            }
            else if (JOY_NEW(DPAD_DOWN))
            {
                ChangeSummaryPokemon(taskId, 1);
                return;
            }
            else if (JOY_NEW(A_BUTTON))
            {
                if (sMonSummaryScreen->curPageIndex == PSS_PAGE_INFO)
                {
                    if (CanRename())
                    {
                        if (sMonSummaryScreen->isBoxMon)
                        {
                            gSpecialVar_0x8004 = PC_MON_CHOSEN;
                            gSpecialVar_MonBoxPos = gLastViewedMonIndex;
                        }
                        else
                        {
                            gSpecialVar_0x8004 = gLastViewedMonIndex;
                        }
                        sMonSummaryScreen->savedCallback = CB2_PssChangePokemonNickname;
                    }

                    PlaySE(SE_SELECT);
                    sMonSummaryScreen->screenState = PSS_STATE_ATEXIT_FADEOUT;
                }
                else if (sMonSummaryScreen->curPageIndex == PSS_PAGE_SKILLS)
                {
                    if (!ShouldShowIvEvPrompt())
                        return;

                    sMonSummaryScreen->skillsPageMode = GetNextSkillsPageMode();
                    BufferMonSkills();
                    RemoveWindow(sMonSummaryScreen->windowIds[POKESUM_WIN_RIGHT_PANE]);
                    AddWindow(&sWindowTemplates_Skills[0]);
                    PokeSum_PrintRightPaneText();
                    PrintControlsString();
                    CopyWindowToVram(sMonSummaryScreen->windowIds[POKESUM_WIN_RIGHT_PANE], 2);
                }
                else if (sMonSummaryScreen->curPageIndex == PSS_PAGE_MOVES)
                {
                    PlaySE(SE_SELECT);
                    sMonSummaryScreen->pageFlipDirection = 1;
                    PokeSum_RemoveWindows();
                    sMonSummaryScreen->curPageIndex++;
                    sMonSummaryScreen->screenState = PSS_STATE_FLIPPAGES;
                }
                return;
            }
            else if (JOY_NEW(B_BUTTON))
            {
                sMonSummaryScreen->screenState = PSS_STATE_ATEXIT_FADEOUT;
            }
            else if (DEBUG_POKEMON_SPRITE_VISUALIZER && JOY_NEW(SELECT_BUTTON) && !gMain.inBattle)
            {
                sMonSummaryScreen->savedCallback = CB2_Pokemon_Sprite_Visualizer;
                // StopPokemonAnimations();
                PlaySE(SE_SELECT);
                sMonSummaryScreen->screenState = PSS_STATE_ATEXIT_FADEOUT;
            }
        }
        break;
    case PSS_STATE_FLIPPAGES:
        switch (sMonSummaryScreen->curPageIndex)
        {
        case PSS_PAGE_INFO:
            HideMoveTypeIcons();
            CreateTask(Task_PokeSum_FlipPages, 0);
            break;
        case PSS_PAGE_SKILLS:
            HideMoveTypeIcons();
            HideMonTypeIcons();
            CreateTask(Task_PokeSum_FlipPages, 0);
            break;
        case PSS_PAGE_MOVES:
            HideMoveTypeIcons();
            HideMonTypeIcons();
            CreateTask(Task_PokeSum_FlipPages, 0);
            break;
        case PSS_PAGE_MOVES_INFO:
            gTasks[sMonSummaryScreen->inputHandlerTaskId].func = Task_FlipPages_FromInfo;
            break;
        case PSS_PAGE_MOVE_DELETER:
            HideMoveTypeIcons();
            HideMonTypeIcons();
            CreateTask(Task_PokeSum_FlipPages, 0);
            break;
        }
        sMonSummaryScreen->screenState = PSS_STATE_HANDLEINPUT;
        break;
    case PSS_STATE_ATEXIT_FADEOUT:
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, 0);
        sMonSummaryScreen->screenState = PSS_STATE_ATEXIT_WAITLINKDELAY;
        break;
    case PSS_STATE_ATEXIT_WAITLINKDELAY:
        if (Overworld_IsRecvQueueAtMax() == TRUE)
            return;
        if (IsLinkRecvQueueAtOverworldMax() == TRUE)
            return;

        sMonSummaryScreen->screenState = PSS_STATE_ATEXIT_WAITFADE;
        break;
    default:
        if (!gPaletteFade.active)
            Task_DestroyResourcesOnExit(taskId);
        break;
    }
}

static void Task_PokeSum_FlipPages(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    switch (data[0])
    {
    case 0:
        PokeSum_HideSpritesBeforePageFlip();
        PokeSum_ShowSpritesBeforePageFlip();
        sMonSummaryScreen->lockMovesFlag = TRUE;
        sMonSummaryScreen->inhibitPageFlipInput = TRUE;
        PokeSum_UpdateWin1ActiveFlag(sMonSummaryScreen->curPageIndex);
        PokeSum_AddWindows(sMonSummaryScreen->curPageIndex);
        break;
    case 1:
        if (sMonSummaryScreen->curPageIndex != PSS_PAGE_MOVES_INFO)
        {
            if (!(sMonSummaryScreen->curPageIndex == PSS_PAGE_MOVES && sMonSummaryScreen->pageFlipDirection == 0))
            {
                FillBgTilemapBufferRect_Palette0(0, 0, 0, 0, 30, 20);
                CopyBgTilemapBufferToVram(0);
            }
        }
        FillBgTilemapBufferRect_Palette0(1, 0, 0, 0, 30, 2);
        FillBgTilemapBufferRect_Palette0(1, 0, 0, 2, 15, 2);
        FillBgTilemapBufferRect_Palette0(2, 0, 0, 0, 30, 2);
        FillBgTilemapBufferRect_Palette0(2, 0, 0, 2, 15, 2);
        break;
    case 2:
        PokeSum_CopyNewBgTilemapBeforePageFlip_2();
        PokeSum_CopyNewBgTilemapBeforePageFlip();
        PokeSum_DrawPageProgressTiles();
        PokeSum_PrintPageHeaderText(sMonSummaryScreen->curPageIndex);
        break;
    case 3:
        CopyWindowToVram(sMonSummaryScreen->windowIds[PSS_WIN_PAGE_NAME], 2);
        CopyWindowToVram(sMonSummaryScreen->windowIds[PSS_WIN_CONTROLS], 2);
        CopyWindowToVram(sMonSummaryScreen->windowIds[PSS_WIN_LVL_NICK], 2);
        break;
    case 4:
        if (!IsDma3ManagerBusyWithBgCopy())
        {
            CopyBgTilemapBufferToVram(3);
            CopyBgTilemapBufferToVram(2);
            CopyBgTilemapBufferToVram(1);
        }
        else
            return;

        break;
    case 5:
        PokeSum_InitBgCoordsBeforePageFlips();
        sMonSummaryScreen->flippingPages = TRUE;
        break;
    case 6:
        if (!PokeSum_IsPageFlipFinished(sMonSummaryScreen->pageFlipDirection))
            return;

        break;
    case 7:
        PokeSum_PrintRightPaneText();
        if (sMonSummaryScreen->curPageIndex != PSS_PAGE_MOVES_INFO)
            PokeSum_PrintBottomPaneText();

        PokeSum_PrintAbilityDataOrMoveTypes();
        PokeSum_PrintMonTypeIcons();
        break;
    case 8:
        CopyWindowToVram(sMonSummaryScreen->windowIds[POKESUM_WIN_RIGHT_PANE], 2);
        CopyWindowToVram(sMonSummaryScreen->windowIds[POKESUM_WIN_TRAINER_MEMO], 2);
        CopyWindowToVram(sMonSummaryScreen->windowIds[5], 2);
        CopyWindowToVram(sMonSummaryScreen->windowIds[6], 2);
        break;
    case 9:
        if (!IsDma3ManagerBusyWithBgCopy())
        {
            CopyBgTilemapBufferToVram(0);
            ShowBg(0);
        }
        else
            return;

        break;
    default:
        PokeSum_SetHelpContext();

        if (sMonSummaryScreen->curPageIndex == PSS_PAGE_MOVES_INFO)
            gTasks[sMonSummaryScreen->inputHandlerTaskId].func = Task_HandleInput_SelectMove;

        DestroyTask(taskId);
        data[0] = 0;
        sMonSummaryScreen->lockMovesFlag = FALSE;
        sMonSummaryScreen->inhibitPageFlipInput = FALSE;
        return;
    }

    data[0]++;
}

static void Task_FlipPages_FromInfo(u8 taskId)
{
    switch (sMonSummaryScreen->taskState)
    {
    case 0:
        sMonSummaryScreen->lockMovesFlag = TRUE;
        sMonSummaryScreen->inhibitPageFlipInput = TRUE;
        PokeSum_AddWindows(sMonSummaryScreen->curPageIndex);
        break;
    case 1:
        if (sMonSummaryScreen->curPageIndex != PSS_PAGE_MOVES_INFO)
        {
            if (!(sMonSummaryScreen->curPageIndex == PSS_PAGE_MOVES && sMonSummaryScreen->pageFlipDirection == 0))
            {
                FillBgTilemapBufferRect_Palette0(0, 0, 0, 0, 30, 20);
                CopyBgTilemapBufferToVram(0);
            }
        }

        FillBgTilemapBufferRect_Palette0(1, 0, 0, 0, 30, 2);
        FillBgTilemapBufferRect_Palette0(1, 0, 0, 2, 15, 2);
        FillBgTilemapBufferRect_Palette0(2, 0, 0, 0, 30, 2);
        FillBgTilemapBufferRect_Palette0(2, 0, 0, 2, 15, 2);
        break;
    case 2:
        PokeSum_HideSpritesBeforePageFlip();
        PokeSum_UpdateWin1ActiveFlag(sMonSummaryScreen->curPageIndex);
        PokeSum_CopyNewBgTilemapBeforePageFlip();
        PokeSum_DrawPageProgressTiles();
        PokeSum_CopyNewBgTilemapBeforePageFlip_2();
        break;
    case 3:
        PokeSum_PrintPageName(sText_PokeSum_PageName_KnownMoves);
        PrintControlsString();
        break;
    case 4:
        CopyWindowToVram(sMonSummaryScreen->windowIds[PSS_WIN_PAGE_NAME], 2);
        CopyWindowToVram(sMonSummaryScreen->windowIds[PSS_WIN_CONTROLS], 2);
        break;
    case 5:
        if (IsDma3ManagerBusyWithBgCopy())
            return;

        CopyBgTilemapBufferToVram(2);
        CopyBgTilemapBufferToVram(1);
        CopyBgTilemapBufferToVram(3);
        break;
    case 6:
        PokeSum_PrintRightPaneText();
        PokeSum_PrintAbilityDataOrMoveTypes();
        CopyWindowToVram(sMonSummaryScreen->windowIds[POKESUM_WIN_RIGHT_PANE], 2);
        CopyWindowToVram(sMonSummaryScreen->windowIds[5], 2);
        break;
    case 7:
        if (IsDma3ManagerBusyWithBgCopy())
            return;

        CopyBgTilemapBufferToVram(0);
        PokeSum_InitBgCoordsBeforePageFlips();
        sMonSummaryScreen->flippingPages = TRUE;
        break;
    case 8:
        if (!PokeSum_IsPageFlipFinished(sMonSummaryScreen->pageFlipDirection))
            return;

        PokeSum_PrintBottomPaneText();
        CopyWindowToVram(sMonSummaryScreen->windowIds[POKESUM_WIN_TRAINER_MEMO], 2);
        break;
    case 9:
        PokeSum_PrintMonTypeIcons();
        PrintMonLevelNickOnWindow2(sText_PokeSum_NoData);
        break;
    case 10:
        PokeSum_ShowSpritesBeforePageFlip();
        CopyWindowToVram(sMonSummaryScreen->windowIds[6], 2);
        CopyWindowToVram(sMonSummaryScreen->windowIds[PSS_WIN_LVL_NICK], 2);
        break;
    case 11:
        if (IsDma3ManagerBusyWithBgCopy())
            return;

        CopyBgTilemapBufferToVram(0);
        CopyBgTilemapBufferToVram(2);
        CopyBgTilemapBufferToVram(1);
        break;
    default:
        PokeSum_SetHelpContext();
        gTasks[sMonSummaryScreen->inputHandlerTaskId].func = Task_HandleInput_SelectMove;
        sMonSummaryScreen->taskState = 0;
        sMonSummaryScreen->lockMovesFlag = FALSE;
        sMonSummaryScreen->inhibitPageFlipInput = FALSE;
        return;
    }

    sMonSummaryScreen->taskState++;
}

static void Task_BackOutOfSelectMove(u8 taskId)
{
    switch (sMonSummaryScreen->taskState)
    {
    case 0:
        sMonSummaryScreen->lockMovesFlag = TRUE;
        sMonSummaryScreen->inhibitPageFlipInput = TRUE;
        PokeSum_AddWindows(sMonSummaryScreen->curPageIndex);
        break;
    case 1:
        if (sMonSummaryScreen->curPageIndex != PSS_PAGE_MOVES_INFO) {
            if (!(sMonSummaryScreen->curPageIndex == PSS_PAGE_MOVES && sMonSummaryScreen->pageFlipDirection == 0))
            {
                FillBgTilemapBufferRect_Palette0(0, 0, 0, 0, 30, 20);
                CopyBgTilemapBufferToVram(0);
            }
        }

        FillBgTilemapBufferRect_Palette0(1, 0, 0, 0, 30, 2);
        FillBgTilemapBufferRect_Palette0(1, 0, 0, 2, 15, 2);
        FillBgTilemapBufferRect_Palette0(2, 0, 0, 0, 30, 2);
        FillBgTilemapBufferRect_Palette0(2, 0, 0, 2, 15, 2);
        break;
    case 2:
        PokeSum_CopyNewBgTilemapBeforePageFlip_2();
        break;
    case 3:
        PokeSum_PrintRightPaneText();
        PokeSum_PrintBottomPaneText();
        PokeSum_PrintAbilityDataOrMoveTypes();
        CopyWindowToVram(sMonSummaryScreen->windowIds[POKESUM_WIN_RIGHT_PANE], 2);
        CopyWindowToVram(sMonSummaryScreen->windowIds[POKESUM_WIN_TRAINER_MEMO], 2);
        CopyWindowToVram(sMonSummaryScreen->windowIds[5], 2);
        CopyBgTilemapBufferToVram(0);
        break;
    case 4:
        PokeSum_PrintPageName(sText_PokeSum_PageName_KnownMoves);
        PrintControlsString();
        break;
    case 5:
        CopyWindowToVram(sMonSummaryScreen->windowIds[PSS_WIN_PAGE_NAME], 2);
        CopyWindowToVram(sMonSummaryScreen->windowIds[PSS_WIN_CONTROLS], 2);
        CopyBgTilemapBufferToVram(2);
        CopyBgTilemapBufferToVram(1);
        break;
    case 6:
        PokeSum_InitBgCoordsBeforePageFlips();
        sMonSummaryScreen->flippingPages = TRUE;
        PokeSum_HideSpritesBeforePageFlip();
        PokeSum_UpdateWin1ActiveFlag(sMonSummaryScreen->curPageIndex);
        PokeSum_PrintMonTypeIcons();
        break;
    case 7:
        break;
    case 8:
        if (PokeSum_IsPageFlipFinished(sMonSummaryScreen->pageFlipDirection) == 0)
            return;

        PrintMonLevelNickOnWindow2(sText_PokeSum_NoData);
        break;
    case 9:
        CopyWindowToVram(sMonSummaryScreen->windowIds[6], 2);
        CopyWindowToVram(sMonSummaryScreen->windowIds[PSS_WIN_LVL_NICK], 2);
        CopyBgTilemapBufferToVram(0);
        CopyBgTilemapBufferToVram(2);
        CopyBgTilemapBufferToVram(1);
        break;
    case 10:
        PokeSum_CopyNewBgTilemapBeforePageFlip();
        PokeSum_DrawPageProgressTiles();
        CopyBgTilemapBufferToVram(3);
        PokeSum_ShowSpritesBeforePageFlip();
        break;
    default:
        PokeSum_SetHelpContext();
        gTasks[sMonSummaryScreen->inputHandlerTaskId].func = Task_InputHandler_Info;
        sMonSummaryScreen->taskState = 0;
        sMonSummaryScreen->lockMovesFlag = FALSE;
        sMonSummaryScreen->inhibitPageFlipInput = FALSE;
        return;
    }

    sMonSummaryScreen->taskState++;
}

static void PokeSum_SetHpExpBarCoordsFullRight(void)
{
    u8 i;
    for (i = 0; i < 11; i++)
    {
        sExpBarObjs->xpos[i] = (8 * i) + 396;
        sExpBarObjs->sprites[i]->x = sExpBarObjs->xpos[i];
        if (i >= 9)
            continue;

        sHpBarObjs->xpos[i] = (8 * i) + 412;
        sHpBarObjs->sprites[i]->x = sHpBarObjs->xpos[i];
    }
}

static void PokeSum_SetHpExpBarCoordsFullLeft(void)
{
    u8 i;
    for (i = 0; i < 11; i++)
    {
        sExpBarObjs->xpos[i] = (8 * i) + 156;
        sExpBarObjs->sprites[i]->x = sExpBarObjs->xpos[i];
        if (i >= 9)
            continue;
        sHpBarObjs->xpos[i] = (8 * i) + 172;
        sHpBarObjs->sprites[i]->x = sHpBarObjs->xpos[i];
    }
}

static void PokeSum_InitBgCoordsBeforePageFlips(void)
{
    s8 pageDelta = 1;

    if (sMonSummaryScreen->pageFlipDirection == 1)
        pageDelta = -1;

    if (sMonSummaryScreen->curPageIndex == PSS_PAGE_MOVES_INFO)
    {
        sMonSummaryScreen->flipPagesBgHofs = 240;
        return;
    }

    if ((sMonSummaryScreen->curPageIndex + pageDelta) == PSS_PAGE_MOVES_INFO)
    {
        PokeSum_UpdateBgPriorityForPageFlip(0, 0);
        sMonSummaryScreen->flipPagesBgHofs = 0;
        SetGpuReg(REG_OFFSET_BG1HOFS, 0);
        SetGpuReg(REG_OFFSET_BG2HOFS, 0);
        return;
    }

    if (sMonSummaryScreen->pageFlipDirection == 1)
    {
        sMonSummaryScreen->flipPagesBgHofs = 0;
        SetGpuReg(REG_OFFSET_BG0HOFS, 0);
        SetGpuReg(REG_OFFSET_BG1HOFS, 0);
        SetGpuReg(REG_OFFSET_BG2HOFS, 0);
        PokeSum_UpdateBgPriorityForPageFlip(1, 1);
    }
    else
    {
        u32 bg1Priority = GetGpuReg(REG_OFFSET_BG1CNT) & 3;
        u32 bg2Priority = GetGpuReg(REG_OFFSET_BG2CNT) & 3;
        sMonSummaryScreen->flipPagesBgHofs = 240;

        if (bg1Priority > bg2Priority)
            SetGpuReg(REG_OFFSET_BG1HOFS, 240);
        else
            SetGpuReg(REG_OFFSET_BG2HOFS, 240);

        SetGpuReg(REG_OFFSET_BG0HOFS, 240);
        PokeSum_UpdateBgPriorityForPageFlip(1, 0);
    }

    if (sMonSummaryScreen->curPageIndex == PSS_PAGE_SKILLS)
    {
        if (sMonSummaryScreen->pageFlipDirection == 1)
            PokeSum_SetHpExpBarCoordsFullLeft();
        else
            PokeSum_SetHpExpBarCoordsFullRight();
    }
    else if (sMonSummaryScreen->curPageIndex == PSS_PAGE_MOVES)
        PokeSum_SetHpExpBarCoordsFullLeft();
}

static enum PokemonSummaryScreenPage GetNewPageIndex(void)
{
    if (sMonSummaryScreen->pageFlipDirection == 1)
        return sMonSummaryScreen->curPageIndex - 1;
    else
        return sMonSummaryScreen->curPageIndex + 1;
}

static void PokeSum_HideSpritesBeforePageFlip(void)
{
    enum PokemonSummaryScreenPage newPage = GetNewPageIndex();

    switch (newPage)
    {
    case PSS_PAGE_INFO:
    case PSS_PAGE_MOVE_DELETER:
        break;
    case PSS_PAGE_SKILLS:
        ShowOrHideHpBarObjs(TRUE);
        ShowOrHideExpBarObjs(TRUE);
        break;
    case PSS_PAGE_MOVES:
        if (sMonSummaryScreen->pageFlipDirection == 1)
        {
            PokeSum_ShowOrHideMonPicSprite(TRUE);
            PokeSum_ShowOrHideMonMarkingsSprite(TRUE);
            ShowOrHideBallIconObj(TRUE);
            ShowOrHideStatusIcon(TRUE);
            HideShowPokerusIcon(TRUE);
            HideShowShinyStar(TRUE);
        }
        break;
    case PSS_PAGE_MOVES_INFO:
        ShoworHideMoveSelectionCursor(TRUE);
        PokeSum_ShowOrHideMonIconSprite(TRUE);
        ShowOrHideStatusIcon(TRUE);
        HideShowPokerusIcon(TRUE);
        HideShowShinyStar(TRUE);
        break;
    }
}

static void PokeSum_ShowSpritesBeforePageFlip(void)
{
    enum PokemonSummaryScreenPage newPage = GetNewPageIndex();

    switch (newPage)
    {
    case PSS_PAGE_INFO:
        ShowOrHideHpBarObjs(FALSE);
        ShowOrHideExpBarObjs(FALSE);
        break;
    case PSS_PAGE_SKILLS:
    case PSS_PAGE_MOVE_DELETER:
        break;
    case PSS_PAGE_MOVES:
        if (sMonSummaryScreen->pageFlipDirection == 0)
        {
            ShowOrHideHpBarObjs(FALSE);
            ShowOrHideExpBarObjs(FALSE);
        }
        else
        {
            ShoworHideMoveSelectionCursor(FALSE);
            HideShowPokerusIcon(FALSE);
            PokeSum_ShowOrHideMonIconSprite(FALSE);
            HideShowShinyStar(FALSE);
        }
        break;
    case PSS_PAGE_MOVES_INFO:
        PokeSum_ShowOrHideMonPicSprite(FALSE);
        PokeSum_ShowOrHideMonMarkingsSprite(FALSE);
        ShowOrHideStatusIcon(FALSE);
        ShowOrHideBallIconObj(FALSE);
        HideShowPokerusIcon(FALSE);
        HideShowShinyStar(FALSE);
        break;
    }
}

static u8 PokeSum_IsPageFlipFinished(u8 a0)
{
    s8 pageDelta = 1;

    if (sMonSummaryScreen->pageFlipDirection == 1)
        pageDelta = -1;

    if (sMonSummaryScreen->curPageIndex == PSS_PAGE_MOVES_INFO)
        if (sMonSummaryScreen->flipPagesBgHofs <= 0)
        {
            sMonSummaryScreen->flipPagesBgHofs = 0;
            sMonSummaryScreen->whichBgLayerToTranslate ^= 1;
            PokeSum_UpdateBgPriorityForPageFlip(0, 0);
            sMonSummaryScreen->flippingPages = FALSE;
            return TRUE;
        }

    if ((sMonSummaryScreen->curPageIndex + pageDelta) == PSS_PAGE_MOVES_INFO)
        if (sMonSummaryScreen->flipPagesBgHofs >= 240)
        {
            sMonSummaryScreen->flipPagesBgHofs = 240;
            sMonSummaryScreen->whichBgLayerToTranslate ^= 1;
            sMonSummaryScreen->flippingPages = FALSE;
            return TRUE;
        }

    if (sMonSummaryScreen->pageFlipDirection == 1)
    {
        if (sMonSummaryScreen->flipPagesBgHofs >= 240)
        {
            sMonSummaryScreen->flipPagesBgHofs = 240;
            sMonSummaryScreen->whichBgLayerToTranslate ^= 1;
            PokeSum_UpdateBgPriorityForPageFlip(0, 0);
            sMonSummaryScreen->flippingPages = FALSE;
            return TRUE;
        }
    }
    else if (sMonSummaryScreen->flipPagesBgHofs <= 0)
    {
        sMonSummaryScreen->whichBgLayerToTranslate ^= 1;
        sMonSummaryScreen->flipPagesBgHofs = 0;
        sMonSummaryScreen->flippingPages = FALSE;
        return TRUE;
    }

    return FALSE;
}

static void PokeSum_UpdateBgPriorityForPageFlip(u8 setBg0Priority, u8 keepBg1Bg2PriorityOrder)
{
    u8 i;
    u32 bg0Priority;
    u32 bg1Priority;
    u32 bg2Priority;

    bg0Priority = GetGpuReg(REG_OFFSET_BG0CNT) & 3;
    bg1Priority = GetGpuReg(REG_OFFSET_BG1CNT) & 3;
    bg2Priority = GetGpuReg(REG_OFFSET_BG2CNT) & 3;

    if (sMonSummaryScreen->pageFlipDirection == 1)
    {
        if (setBg0Priority == 0)
        {
            bg0Priority = 0;

            if (keepBg1Bg2PriorityOrder == 0)
            {
                if (bg1Priority > bg2Priority)
                    bg1Priority = 1, bg2Priority = 2;
                else
                    bg1Priority = 2, bg2Priority = 1;
            }
            else
            {
                if (bg1Priority > bg2Priority)
                    bg1Priority = 2, bg2Priority = 1;
                else
                    bg1Priority = 1, bg2Priority = 2;
            }
        }
        if (setBg0Priority == 1)
        {
            bg0Priority = 1;

            if (keepBg1Bg2PriorityOrder == 0)
            {
                if (bg1Priority > bg2Priority)
                    bg1Priority = 0, bg2Priority = 2;
                else
                    bg1Priority = 2, bg2Priority = 0;
            }
            else
            {
                if (bg1Priority > bg2Priority)
                    bg1Priority = 2, bg2Priority = 0;
                else
                    bg1Priority = 0, bg2Priority = 2;
            }
        }
    }

    if (sMonSummaryScreen->pageFlipDirection == 0)
    {
        bg0Priority = 0;
        if (bg1Priority > bg2Priority)
            bg1Priority = 1, bg2Priority = 2;
        else
            bg1Priority = 2, bg2Priority = 1;
    }

    for (i = 0; i < 11; i++)
    {
        if (sMonSummaryScreen->curPageIndex == PSS_PAGE_SKILLS && sMonSummaryScreen->pageFlipDirection == 1)
            sExpBarObjs->sprites[i]->oam.priority = bg0Priority;
        else
            sExpBarObjs->sprites[i]->oam.priority = bg1Priority;

        if (i >= 9)
            continue;

        if (sMonSummaryScreen->curPageIndex == PSS_PAGE_SKILLS && sMonSummaryScreen->pageFlipDirection == 1)
            sHpBarObjs->sprites[i]->oam.priority = bg0Priority;
        else
            sHpBarObjs->sprites[i]->oam.priority = bg1Priority;
    }

    SetGpuReg(REG_OFFSET_BG0CNT, (GetGpuReg(REG_OFFSET_BG0CNT) & (u16)~3) | bg0Priority);
    SetGpuReg(REG_OFFSET_BG1CNT, (GetGpuReg(REG_OFFSET_BG1CNT) & (u16)~3) | bg1Priority);
    SetGpuReg(REG_OFFSET_BG2CNT, (GetGpuReg(REG_OFFSET_BG2CNT) & (u16)~3) | bg2Priority);
}

static void PokeSum_CopyNewBgTilemapBeforePageFlip_2(void)
{
    const u32 *tilemap;
    u32 bg;
    enum PokemonSummaryScreenPage newPage = GetNewPageIndex();

    switch (newPage)
    {
    case PSS_PAGE_INFO:
        bg = sMonSummaryScreen->infoAndMovesPageBgNum;
        tilemap = gSummaryScreen_PageSkills_Tilemap;
        break;
    case PSS_PAGE_SKILLS:
        bg = sMonSummaryScreen->skillsPageBgNum;
        if (sMonSummaryScreen->pageFlipDirection == 1)
            tilemap = gSummaryScreen_PageMoves_Tilemap;
        else
            tilemap = gSummaryScreen_PageInfo_Tilemap;
        break;
    case PSS_PAGE_MOVES:
        bg = sMonSummaryScreen->infoAndMovesPageBgNum;
        if (sMonSummaryScreen->pageFlipDirection == 1)
            tilemap = gSummaryScreen_PageMovesInfo_Tilemap;
        else
            tilemap = gSummaryScreen_PageSkills_Tilemap;
        break;
    case PSS_PAGE_MOVES_INFO:
        bg = sMonSummaryScreen->skillsPageBgNum;
        tilemap = gSummaryScreen_PageMoves_Tilemap;
        break;
    default:
        return;
    }
    CopyToBgTilemapBuffer(bg, tilemap, 0, 0);
}

static void PokeSum_CopyNewBgTilemapBeforePageFlip(void)
{
    enum PokemonSummaryScreenPage newPage = GetNewPageIndex();

    switch (newPage)
    {
    case PSS_PAGE_INFO:
        CopyToBgTilemapBuffer(sMonSummaryScreen->infoAndMovesPageBgNum, gSummaryScreen_PageSkills_Tilemap, 0, 0);
        break;
    case PSS_PAGE_SKILLS:
    case PSS_PAGE_MOVE_DELETER:
        break;
    case PSS_PAGE_MOVES:
        if (sMonSummaryScreen->pageFlipDirection == 1)
            CopyToBgTilemapBuffer(3, sBgTilemap_MovesPage, 0, 0);
        else
            CopyToBgTilemapBuffer(3, sBgTilemap_MovesInfoPage, 0, 0);

        break;
    case PSS_PAGE_MOVES_INFO:
        CopyToBgTilemapBuffer(3, sBgTilemap_MovesInfoPage, 0, 0);
        break;
    }
}

static void CB2_InitSummaryScreen(void)
{
    switch (gMain.state)
    {
    case 0:
        SetVBlankHBlankCallbacksToNull();
        break;
    case 1:
        PokeSum_Setup_InitGpu();
        break;
    case 2:
        PokeSum_Setup_SpritesReset();
        break;
    case 3:
        if (!PokeSum_HandleLoadBgGfx())
            return;
        break;
    case 4:
        if (!PokeSum_HandleCreateSprites())
            return;
        break;
    case 5:
        PokeSum_CreateWindows();
        break;
    case 6:
        if (!PokeSum_Setup_BufferStrings())
            return;
        break;
    case 7:
        PokeSum_PrintRightPaneText();
        break;
    case 8:
        PokeSum_PrintBottomPaneText();
        break;
    case 9:
        PokeSum_PrintAbilityDataOrMoveTypes();
        PokeSum_PrintMonTypeIcons();
        break;
    case 10:
        if (sMonSummaryScreen->mode == PSS_MODE_SELECT_MOVE || sMonSummaryScreen->mode == PSS_MODE_FORGET_MOVE)
            CopyToBgTilemapBuffer(3, sBgTilemap_MovesPage, 0, 0);
        else
            CopyToBgTilemapBuffer(3, sBgTilemap_MovesInfoPage, 0, 0);

        PokeSum_DrawPageProgressTiles();
        break;
    case 11:
        if (sMonSummaryScreen->isEgg)
            CopyToBgTilemapBuffer(sMonSummaryScreen->skillsPageBgNum, gSummaryScreen_PageEgg_Tilemap, 0, 0);
        else
        {
            if (sMonSummaryScreen->mode == PSS_MODE_SELECT_MOVE || sMonSummaryScreen->mode == PSS_MODE_FORGET_MOVE)
            {
                CopyToBgTilemapBuffer(sMonSummaryScreen->skillsPageBgNum, gSummaryScreen_PageMoves_Tilemap, 0, 0);
                CopyToBgTilemapBuffer(sMonSummaryScreen->infoAndMovesPageBgNum, gSummaryScreen_PageMovesInfo_Tilemap, 0, 0);
            }
            else
            {
                CopyToBgTilemapBuffer(sMonSummaryScreen->skillsPageBgNum, gSummaryScreen_PageInfo_Tilemap, 0, 0);
                CopyToBgTilemapBuffer(sMonSummaryScreen->infoAndMovesPageBgNum, gSummaryScreen_PageSkills_Tilemap, 0, 0);
            }
        }

        break;
    case 12:
        BlendPalettes(0xffffffff, 16, 0);
        PokeSum_PrintPageHeaderText(sMonSummaryScreen->curPageIndex);
        CommitStaticWindowTilemaps();
        break;
    case 13:
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, 0);
        CopyWindowToVram(sMonSummaryScreen->windowIds[PSS_WIN_PAGE_NAME], COPYWIN_GFX);
        CopyWindowToVram(sMonSummaryScreen->windowIds[PSS_WIN_CONTROLS], COPYWIN_GFX);
        CopyWindowToVram(sMonSummaryScreen->windowIds[PSS_WIN_LVL_NICK], COPYWIN_GFX);
        CopyWindowToVram(sMonSummaryScreen->windowIds[PSS_SECONDARY_WIN_4], COPYWIN_GFX);
        CopyWindowToVram(sMonSummaryScreen->windowIds[PSS_SECONDARY_WIN_1], COPYWIN_GFX);
        CopyWindowToVram(sMonSummaryScreen->windowIds[PSS_SECONDARY_WIN_2], COPYWIN_GFX);
        CopyWindowToVram(sMonSummaryScreen->windowIds[PSS_SECONDARY_WIN_3], COPYWIN_GFX);
        break;
    case 14:
        CopyBgTilemapBufferToVram(0);
        CopyBgTilemapBufferToVram(2);
        CopyBgTilemapBufferToVram(1);
        CopyBgTilemapBufferToVram(3);
        break;
    case 15:
        if (sMonSummaryScreen->mode == PSS_MODE_SELECT_MOVE || sMonSummaryScreen->mode == PSS_MODE_FORGET_MOVE)
        {
            PokeSum_ShowOrHideMonIconSprite(FALSE);
            ShoworHideMoveSelectionCursor(FALSE);
        }
        else
        {
            PokeSum_ShowOrHideMonPicSprite(FALSE);
            PokeSum_ShowOrHideMonMarkingsSprite(FALSE);
            ShowOrHideBallIconObj(FALSE);
            ShowOrHideHpBarObjs(FALSE);
            ShowOrHideExpBarObjs(FALSE);
        }

        ShowOrHideStatusIcon(FALSE);
        HideShowPokerusIcon(FALSE);
        HideShowShinyStar(FALSE);
        break;
    default:
        PokeSum_Setup_SetVBlankCallback();
        PokeSum_FinishSetup();
        return;
    }

    gMain.state++;
}

static u8 PokeSum_HandleLoadBgGfx(void)
{
    switch (sMonSummaryScreen->loadBgGfxStep)
    {
    case 0:
        LoadPalette(gSummaryScreen_Bg_Pal, BG_PLTT_ID(0), 5 * PLTT_SIZE_4BPP);
        if (IsMonShiny(&sMonSummaryScreen->currentMon) == TRUE && !sMonSummaryScreen->isEgg)
        {
            LoadPalette(&gSummaryScreen_Bg_Pal[16 * 6], BG_PLTT_ID(0), PLTT_SIZE_4BPP);
            LoadPalette(&gSummaryScreen_Bg_Pal[16 * 5], BG_PLTT_ID(1), PLTT_SIZE_4BPP);
        }
        else
        {
            LoadPalette(&gSummaryScreen_Bg_Pal[16 * 0], BG_PLTT_ID(0), PLTT_SIZE_4BPP);
            LoadPalette(&gSummaryScreen_Bg_Pal[16 * 1], BG_PLTT_ID(1), PLTT_SIZE_4BPP);
        }

        break;
    case 1:
        ListMenuLoadStdPalAt(BG_PLTT_ID(6), 1);
        LoadPalette(sTextHeaderPalette, BG_PLTT_ID(7), PLTT_SIZE_4BPP);
        break;
    case 2:
        ResetTempTileDataBuffers();
        break;
    case 3:
        DecompressAndCopyTileDataToVram(2, gSummaryScreen_Bg_Gfx, 0, 0, 0);
        break;
    case 4:
        if (FreeTempTileDataBuffersIfPossible() == TRUE)
            return FALSE;
        break;

    case 5:
    case 6:
        break;

    default:
        LoadPalette(sTextMovesPalette, BG_PLTT_ID(8), PLTT_SIZE_4BPP);
        return TRUE;
    }

    sMonSummaryScreen->loadBgGfxStep++;
    return FALSE;
}

static u8 PokeSum_Setup_BufferStrings(void)
{
    switch (sMonSummaryScreen->bufferStringsStep)
    {
    case 0:
        BufferMonInfo();
        if (sMonSummaryScreen->isEgg)
        {
            sMonSummaryScreen->bufferStringsStep = 0;
            return TRUE;
        }

        break;
    case 1:
        if (sMonSummaryScreen->isEgg == 0)
            BufferMonSkills();
        break;
    case 2:
        if (sMonSummaryScreen->isEgg == 0)
            BufferMonMoves();
        break;
    default:
        sMonSummaryScreen->bufferStringsStep = 0;
        return TRUE;
    }

    sMonSummaryScreen->bufferStringsStep++;
    return FALSE;
}

static void BufferMonInfo(void)
{
    u8 tempStr[20];
    u16 dexNum;
    u16 gender;
    u16 heldItem;
    u32 otId;

    dexNum = SpeciesToPokedexNum(GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_SPECIES));
    if (dexNum == 0xffff)
        StringCopy(sMonSummaryScreen->dexNumStrBuf, sText_PokeSum_DexNoUnknown);
    else
        ConvertIntToDecimalStringN(sMonSummaryScreen->dexNumStrBuf, dexNum, STR_CONV_MODE_LEADING_ZEROS, IsNationalPokedexEnabled() ? 4 : 3);

    if (!sMonSummaryScreen->isEgg)
    {
        dexNum = GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_SPECIES);
        StringCopy(sMonSummaryScreen->speciesNameStrBuf, GetSpeciesName(dexNum));
    }
    else
    {
        GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_NICKNAME, sMonSummaryScreen->speciesNameStrBuf);
        return;
    }

    sMonSummaryScreen->monTypes[0] = gSpeciesInfo[dexNum].types[0];
    sMonSummaryScreen->monTypes[1] = gSpeciesInfo[dexNum].types[1];
    sMonSummaryScreen->monTypes[2] = GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_TERA_TYPE);

    GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_NICKNAME, tempStr);
    StringCopyN_Multibyte(sMonSummaryScreen->nicknameStrBuf, tempStr, POKEMON_NAME_LENGTH);
    StringGet_Nickname(sMonSummaryScreen->nicknameStrBuf);

    gender = GetMonGender(&sMonSummaryScreen->currentMon);
    dexNum = GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_SPECIES_OR_EGG);

    if (gender == MON_FEMALE)
        StringCopy(sMonSummaryScreen->genderSymbolStrBuf, gText_FemaleSymbol);
    else if (gender == MON_MALE)
        StringCopy(sMonSummaryScreen->genderSymbolStrBuf, gText_MaleSymbol);
    else
        StringCopy(sMonSummaryScreen->genderSymbolStrBuf, gText_EmptyString);

    if (dexNum == SPECIES_NIDORAN_M || dexNum == SPECIES_NIDORAN_F)
        if (StringCompare(sMonSummaryScreen->nicknameStrBuf, gSpeciesInfo[dexNum].speciesName) == 0)
            StringCopy(sMonSummaryScreen->genderSymbolStrBuf, gText_EmptyString);

    GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_OT_NAME, tempStr);
    StringCopyN_Multibyte(sMonSummaryScreen->otNameStrBuf, tempStr, PLAYER_NAME_LENGTH);

    ConvertInternationalString(sMonSummaryScreen->otNameStrBuf, GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_LANGUAGE));

    otId = GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_OT_ID) & 0xffff;
    ConvertIntToDecimalStringN(sMonSummaryScreen->otIdStrBuf, otId, STR_CONV_MODE_LEADING_ZEROS, 5);

    ConvertIntToDecimalStringN(tempStr, GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_LEVEL), STR_CONV_MODE_LEFT_ALIGN, 3);
    StringCopy(sMonSummaryScreen->levelStrBuf, gText_Lv);
    StringAppendN(sMonSummaryScreen->levelStrBuf, tempStr, 4);

    heldItem = GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_HELD_ITEM);

    if (heldItem == ITEM_NONE)
        StringCopy(sMonSummaryScreen->itemNameStrBuf, gText_None);
    else
        CopyItemName(heldItem, sMonSummaryScreen->itemNameStrBuf);
}

#define GetNumberRightAlign63(x) (63 - StringLength((x)) * 6)
#define GetNumberRightAlign27(x) (27 - StringLength((x)) * 6)

static void SetStatXPos(u8 stat, u16 xpos)
{
    switch (stat)
    {
        case STAT_HP:
            sMonSummaryScreen->curHpStrXpos = xpos;
            break;
        case STAT_ATK:
            sMonSummaryScreen->atkStrXpos = xpos;
            break;
        case STAT_DEF:
            sMonSummaryScreen->defStrXpos = xpos;
            break;
        case STAT_SPATK:
            sMonSummaryScreen->spAStrXpos = xpos;
            break;
        case STAT_SPDEF:
            sMonSummaryScreen->spDStrXpos = xpos;
            break;
        case STAT_SPEED:
            sMonSummaryScreen->speStrXpos = xpos;
            break;
    }
}

static void ApplyNatureColor(u8 *str, enum Stat stat)
{
    enum Nature nature = GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_HIDDEN_NATURE);
    u8 tmp[20];

    StringCopy(tmp, str);

    if (!P_SUMMARY_SCREEN_NATURE_COLORS || gNaturesInfo[nature].statUp == gNaturesInfo[nature].statDown)
        StringCopy(str, gText_EmptyString);
    else if (gNaturesInfo[nature].statUp == stat)
        StringCopy(str, sText_ColorRed);
    else if (gNaturesInfo[nature].statDown == stat)
        StringCopy(str, sText_ColorBlue);
    else
        StringCopy(str, gText_EmptyString);
    StringAppend(str, tmp);
}

static void BufferStatString(enum Stat stat)
{
    u8 *dst = sMonSummaryScreen->statValueStrBufs[sStatData[stat].pssStat];
    u16 statValue = GetMonData(&sMonSummaryScreen->currentMon, sStatData[stat].monDataStat);

    ConvertIntToDecimalStringN(dst, statValue, STR_CONV_MODE_LEFT_ALIGN, 3);
    if (stat == STAT_HP)
    {
        u8 tempStr[20];
        StringAppend(dst, gText_Slash);

        statValue = GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_MAX_HP);
        ConvertIntToDecimalStringN(tempStr, statValue, STR_CONV_MODE_LEFT_ALIGN, 3);
        StringAppend(dst, tempStr);
    }

    SetStatXPos(stat, GetNumberRightAlign63(dst));
    if (stat != STAT_HP)
        ApplyNatureColor(dst, stat);
}

static void BufferEVString(u8 stat)
{
    u16 statValue = GetMonData(&sMonSummaryScreen->currentMon, sStatData[stat].monDataEv);
    u8 *dst = sMonSummaryScreen->statValueStrBufs[sStatData[stat].pssStat];
    u8 tmp[20];

    ConvertIntToDecimalStringN(dst, statValue, STR_CONV_MODE_LEFT_ALIGN, 3);
    StringAppend(dst, gText_Slash);
    ConvertIntToDecimalStringN(tmp, MAX_PER_STAT_EVS, STR_CONV_MODE_LEFT_ALIGN, 3);
    StringAppend(dst, tmp);
    SetStatXPos(stat, GetNumberRightAlign63(dst));
    if (stat != STAT_HP)
        ApplyNatureColor(dst, stat);
}

static void BufferIVTextString(u8 *dst, u8 statValue, bool32 isHyperTrained)
{
    if (isHyperTrained)
        StringCopy(dst, sText_JudgeHyperTrained);
    else if (statValue == 31)
        StringCopy(dst, sText_JudgeBest);
    else if (statValue == 30)
        StringCopy(dst, sText_JudgeFantastic);
    else if (statValue > 25)
        StringCopy(dst, sText_JudgeVeryGood);
    else if (statValue > 15)
        StringCopy(dst, sText_JudgePrettyGood);
    else if (statValue > 0)
        StringCopy(dst, sText_JudgeDecent);
    else
        StringCopy(dst, sText_JudgeNoGood);
}

static void BufferIVGradeString(u8 *dst, u8 statValue, bool32 isHyperTrained)
{
    if (isHyperTrained)
        StringCopy(dst, sText_GradeS);
    else if (statValue == 31)
        StringCopy(dst, sText_GradeS);
    else if (statValue == 30)
        StringCopy(dst, sText_GradeA);
    else if (statValue > 25)
        StringCopy(dst, sText_GradeB);
    else if (statValue > 15)
        StringCopy(dst, sText_GradeC);
    else if (statValue > 0)
        StringCopy(dst, sText_GradeD);
    else
        StringCopy(dst, sText_GradeF);
}

static void BufferIVString(enum Stat stat)
{
    bool8 isHyperTrained = P_SUMMARY_SCREEN_IV_HYPERTRAIN ? GetMonData(&sMonSummaryScreen->currentMon, sStatData[stat].monDataHyperTrained) : FALSE;
    u16 statValue = GetMonData(&sMonSummaryScreen->currentMon, sStatData[stat].monDataIv);
    u8 *dst = sMonSummaryScreen->statValueStrBufs[sStatData[stat].pssStat];
    u16 xPos;

    switch (P_SUMMARY_SCREEN_IV_DISPLAY)
    {
        case P_SUMMARY_SCREEN_IV_NUMBER:
            ConvertIntToDecimalStringN(dst, statValue, STR_CONV_MODE_LEFT_ALIGN, 3);
            xPos = GetNumberRightAlign63(dst);
            break;
        case P_SUMMARY_SCREEN_IV_GRADE:
            BufferIVGradeString(dst, statValue, isHyperTrained);
            xPos = GetNumberRightAlign63(dst);
            break;
        case P_SUMMARY_SCREEN_IV_TEXT:
            BufferIVTextString(dst, statValue, isHyperTrained);
            xPos = 0;
            break;
    }

    SetStatXPos(stat, xPos);
    if (stat != STAT_HP)
        ApplyNatureColor(dst, stat);
}

static void BufferStat(enum Stat stat)
{
    switch (sMonSummaryScreen->skillsPageMode)
    {
        case PSS_SKILL_PAGE_STATS:
            BufferStatString(stat);
            break;
        case PSS_SKILL_PAGE_IVS:
            BufferIVString(stat);
            break;
        case PSS_SKILL_PAGE_EVS:
            BufferEVString(stat);
            break;
        default:
            return;
    }
}

static void BufferMonSkills(void)
{
    u8 level;
    enum Ability ability;
    enum Species species;
    u32 exp;
    u32 expToNextLevel;

    BufferStat(STAT_HP);
    BufferStat(STAT_ATK);
    BufferStat(STAT_DEF);
    BufferStat(STAT_SPATK);
    BufferStat(STAT_SPDEF);
    BufferStat(STAT_SPEED);

    exp = GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_EXP);
    ConvertIntToDecimalStringN(sMonSummaryScreen->expPointsStrBuf, exp, STR_CONV_MODE_LEFT_ALIGN, 7);
    sMonSummaryScreen->expStrXpos = GetNumberRightAlign63(sMonSummaryScreen->expPointsStrBuf);

    level = GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_LEVEL);
    expToNextLevel = 0;
    if (level < 100)
    {
        species = GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_SPECIES);
        expToNextLevel = gExperienceTables[gSpeciesInfo[species].growthRate][level + 1] - exp;
    }

    ConvertIntToDecimalStringN(sMonSummaryScreen->expToNextLevelStrBuf, expToNextLevel, STR_CONV_MODE_LEFT_ALIGN, 7);
    sMonSummaryScreen->toNextLevelXpos = GetNumberRightAlign63(sMonSummaryScreen->expToNextLevelStrBuf);

    ability = GetAbilityBySpecies(GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_SPECIES), GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_ABILITY_NUM));
    StringCopy(sMonSummaryScreen->abilityNameStrBuf, gAbilitiesInfo[ability].name);
    StringCopy(sMonSummaryScreen->abilityDescStrBuf, gAbilitiesInfo[ability].description);

    sMonSummaryScreen->curMonStatusAilment = StatusToAilment(GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_STATUS));
    if (sMonSummaryScreen->curMonStatusAilment == AILMENT_NONE)
        if (ShouldPokemonShowActivePokerus(&sMonSummaryScreen->currentMon))
            sMonSummaryScreen->curMonStatusAilment = AILMENT_PKRS;
}

static void BufferMonMoves(void)
{
    u8 i;

    for (i = 0; i < MAX_MON_MOVES; i++)
        BufferMonMoveI(i);

    if (sMonSummaryScreen->mode == PSS_MODE_SELECT_MOVE)
        BufferMonMoveI(MAX_MON_MOVES);
}

#define GetRightAlignXpos_NDigits(a, b) ((6 * (a)) - StringLength((b)) * 6)

static void BufferMonMoveI(u8 i)
{
    if (i < MAX_MON_MOVES)
        sMonSummaryScreen->moveIds[i] = GetMonMoveBySlotId(&sMonSummaryScreen->currentMon, i);

    if (sMonSummaryScreen->moveIds[i] == MOVE_NONE)
    {
        StringCopy(sMonSummaryScreen->moveNameStrBufs[i], sText_PokeSum_OneHyphen);
        StringCopy(sMonSummaryScreen->moveCurPpStrBufs[i], sText_PokeSum_TwoHyphens);
        StringCopy(sMonSummaryScreen->movePowerStrBufs[i], gText_ThreeHyphens);
        StringCopy(sMonSummaryScreen->moveAccuracyStrBufs[i], gText_ThreeHyphens);
        sMonSummaryScreen->curPpXpos[i] = 0xFF;
        sMonSummaryScreen->maxPpXpos[i] = 0xFF;
        return;
    }

    sMonSummaryScreen->numMoves++;
    sMonSummaryScreen->moveTypes[i] = gMovesInfo[sMonSummaryScreen->moveIds[i]].type;
    StringCopy(sMonSummaryScreen->moveNameStrBufs[i], gMovesInfo[sMonSummaryScreen->moveIds[i]].name);

    if (i >= MAX_MON_MOVES && sMonSummaryScreen->mode == PSS_MODE_SELECT_MOVE)
    {
        ConvertIntToDecimalStringN(sMonSummaryScreen->moveCurPpStrBufs[i],
                                   gMovesInfo[sMonSummaryScreen->moveIds[i]].pp, STR_CONV_MODE_LEFT_ALIGN, 3);
        ConvertIntToDecimalStringN(sMonSummaryScreen->moveMaxPpStrBufs[i],
                                   gMovesInfo[sMonSummaryScreen->moveIds[i]].pp, STR_CONV_MODE_LEFT_ALIGN, 3);
    }
    else
    {
        ConvertIntToDecimalStringN(sMonSummaryScreen->moveCurPpStrBufs[i],
                                   GetMonPpByMoveSlot(&sMonSummaryScreen->currentMon, i), STR_CONV_MODE_LEFT_ALIGN, 3);
        ConvertIntToDecimalStringN(sMonSummaryScreen->moveMaxPpStrBufs[i],
                                   CalculatePPWithBonus(sMonSummaryScreen->moveIds[i], GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_PP_BONUSES), i),
                                   STR_CONV_MODE_LEFT_ALIGN, 3);
    }

    sMonSummaryScreen->curPpXpos[i] = GetRightAlignXpos_NDigits(2, sMonSummaryScreen->moveCurPpStrBufs[i]);
    sMonSummaryScreen->maxPpXpos[i] = GetRightAlignXpos_NDigits(2, sMonSummaryScreen->moveMaxPpStrBufs[i]);

    if (gMovesInfo[sMonSummaryScreen->moveIds[i]].power <= 1)
        StringCopy(sMonSummaryScreen->movePowerStrBufs[i], gText_ThreeHyphens);
    else
        ConvertIntToDecimalStringN(sMonSummaryScreen->movePowerStrBufs[i], gMovesInfo[sMonSummaryScreen->moveIds[i]].power, STR_CONV_MODE_RIGHT_ALIGN, 3);

    if (gMovesInfo[sMonSummaryScreen->moveIds[i]].accuracy == 0)
        StringCopy(sMonSummaryScreen->moveAccuracyStrBufs[i], gText_ThreeHyphens);
    else
        ConvertIntToDecimalStringN(sMonSummaryScreen->moveAccuracyStrBufs[i], gMovesInfo[sMonSummaryScreen->moveIds[i]].accuracy, STR_CONV_MODE_RIGHT_ALIGN, 3);
}

static u8 PokeSum_HandleCreateSprites(void)
{
    switch (sMonSummaryScreen->spriteCreationStep)
    {
    case 0:
        CreateShinyIconSprite(TAG_PSS_SHINY_ICON, TAG_PSS_SHINY_ICON);
        break;
    case 1:
        CreatePokerusIconSprite(TAG_PSS_POKERUS_ICON, TAG_PSS_POKERUS_ICON);
        break;
    case 2:
        PokeSum_CreateMonMarkingsSprite();
        break;
    case 3:
        CreateMoveSelectionCursorSprites(TAG_PSS_MOVE_SELECTION_CURSOR_0, TAG_PSS_MOVE_SELECTION_CURSOR_0);
        break;
    case 4:
        CreateStatusIconSprite(TAG_PSS_STATUS_ICON, TAG_PSS_STATUS_ICON);
        break;
    case 5:
        CreateHpBarObjs(TAG_PSS_HP_BAR_0, TAG_PSS_HP_BAR_0);
        break;
    case 6:
        CreateExpBarObjs(TAG_PSS_EXP_BAR_0, TAG_PSS_EXP_BAR_0);
        break;
    case 7:
        CreateBallIconObj();
        break;
    case 8:
        PokeSum_CreateMonIconSprite();
        CreateTypeIconSprites();
        break;
    case 9:
        LoadCompressedSpriteSheet(&gSpriteSheet_CategoryIcons);
        LoadSpritePalette(&gSpritePal_CategoryIcons);
        break;
    default:
        PokeSum_CreateMonPicSprite();
        return TRUE;
    }

    sMonSummaryScreen->spriteCreationStep++;
    return FALSE;
}

static void PokeSum_Setup_SpritesReset(void)
{
    ResetSpriteData();
    ResetPaletteFade();
    FreeAllSpritePalettes();
    ScanlineEffect_Stop();
}

static void PokeSum_Setup_InitGpu(void)
{
    DmaClearLarge16(3, (void *)VRAM, VRAM_SIZE, 0x1000);
    DmaClear32(3, (void *)OAM, OAM_SIZE);
    DmaClear16(3, (void *)PLTT, PLTT_SIZE);

    SetGpuReg(REG_OFFSET_DISPCNT, 0);

    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, sBgTempaltes, ARRAY_COUNT(sBgTempaltes));

    ChangeBgX(0, 0, 0);
    ChangeBgY(0, 0, 0);
    ChangeBgX(1, 0, 0);
    ChangeBgY(1, 0, 0);
    ChangeBgX(2, 0, 0);
    ChangeBgY(2, 0, 0);
    ChangeBgX(3, 0, 0);
    ChangeBgY(3, 0, 0);

    DeactivateAllTextPrinters();

    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON | DISPCNT_WIN1_ON);

    PokeSum_UpdateWin1ActiveFlag(sMonSummaryScreen->curPageIndex);

    SetGpuReg(REG_OFFSET_WININ, (WININ_WIN0_OBJ | WININ_WIN0_BG0 | WININ_WIN0_BG1 | WININ_WIN0_BG2 | WININ_WIN0_BG3) << 8);
    SetGpuReg(REG_OFFSET_WINOUT, WINOUT_WIN01_BG0 | WINOUT_WIN01_BG1 | WINOUT_WIN01_BG2 | WINOUT_WIN01_BG3);
    SetGpuReg(REG_OFFSET_WIN1V, 32 << 8 | 135);
    SetGpuReg(REG_OFFSET_WIN1H, 2 << 8 | 240);

    SetBgTilemapBuffer(1, sMonSummaryScreen->bg1TilemapBuffer);
    SetBgTilemapBuffer(2, sMonSummaryScreen->bg2TilemapBuffer);
    SetBgTilemapBuffer(3, sMonSummaryScreen->bg3TilemapBuffer);

    ShowBg(0);
    ShowBg(1);
    ShowBg(2);
    ShowBg(3);
}

static void PokeSum_FinishSetup(void)
{
    if (sMonSummaryScreen->mode == PSS_MODE_SELECT_MOVE || sMonSummaryScreen->mode == PSS_MODE_FORGET_MOVE)
        sMonSummaryScreen->inputHandlerTaskId = CreateTask(Task_InputHandler_SelectOrForgetMove, 0);
    else
        sMonSummaryScreen->inputHandlerTaskId = CreateTask(Task_InputHandler_Info, 0);

    SetMainCallback2(MainCB2);
}

static void PokeSum_PrintPageName(const u8 *str)
{
    FillWindowPixelBuffer(sMonSummaryScreen->windowIds[PSS_WIN_PAGE_NAME], 0);
    AddTextPrinterParameterized3(sMonSummaryScreen->windowIds[PSS_WIN_PAGE_NAME], FONT_NORMAL, 4, 1, sLevelNickTextColors[1], 0, str);
    PutWindowTilemap(sMonSummaryScreen->windowIds[PSS_WIN_PAGE_NAME]);
}

static void PokeSum_PrintControlsString(const u8 *str)
{
    s32 width;
    u8 windowId;

    FillWindowPixelBuffer(sMonSummaryScreen->windowIds[PSS_WIN_CONTROLS], 0);
    width = GetStringWidth(FONT_SMALL, str, 0);
    windowId = sMonSummaryScreen->windowIds[PSS_WIN_CONTROLS];
    AddTextPrinterParameterized3(windowId, FONT_SMALL, 84 - width, 0, sLevelNickTextColors[1], 0, str);
    PutWindowTilemap(sMonSummaryScreen->windowIds[PSS_WIN_CONTROLS]);
}

static void PrintMonLevelNickOnWindow2(const u8 *str)
{
    FillWindowPixelBuffer(sMonSummaryScreen->windowIds[PSS_WIN_LVL_NICK], 0);

    if (!sMonSummaryScreen->isEgg)
    {
        u16 nicknameFont;

        if (sMonSummaryScreen->curPageIndex != PSS_PAGE_MOVES_INFO)
            AddTextPrinterParameterized3(sMonSummaryScreen->windowIds[PSS_WIN_LVL_NICK], 2, 4, 2, sLevelNickTextColors[1], TEXT_SKIP_DRAW, sMonSummaryScreen->levelStrBuf);

        nicknameFont = GetFontIdToFit(sMonSummaryScreen->nicknameStrBuf, FONT_NORMAL, 0, 60);
        AddTextPrinterParameterized3(sMonSummaryScreen->windowIds[PSS_WIN_LVL_NICK], nicknameFont, 40, 2, sLevelNickTextColors[1], TEXT_SKIP_DRAW, sMonSummaryScreen->nicknameStrBuf);

        if (GetMonGender(&sMonSummaryScreen->currentMon) == MON_FEMALE)
            AddTextPrinterParameterized3(sMonSummaryScreen->windowIds[PSS_WIN_LVL_NICK], FONT_NORMAL, 105, 2, sLevelNickTextColors[3], 0, sMonSummaryScreen->genderSymbolStrBuf);
        else
            AddTextPrinterParameterized3(sMonSummaryScreen->windowIds[PSS_WIN_LVL_NICK], FONT_NORMAL, 105, 2, sLevelNickTextColors[2], 0, sMonSummaryScreen->genderSymbolStrBuf);
    }

    PutWindowTilemap(sMonSummaryScreen->windowIds[PSS_WIN_LVL_NICK]);
}

static void PokeSum_PrintRightPaneText(void)
{
    FillWindowPixelBuffer(sMonSummaryScreen->windowIds[POKESUM_WIN_RIGHT_PANE], 0);

    switch (sMonSummaryScreen->curPageIndex)
    {
    case PSS_PAGE_INFO:
        PrintInfoPage();
        break;
    case PSS_PAGE_SKILLS:
        PrintSkillsPage();
        break;
    case PSS_PAGE_MOVES:
    case PSS_PAGE_MOVES_INFO:
        PrintMovesPage();
        break;
    case PSS_PAGE_MOVE_DELETER:
        break;
    }

    PutWindowTilemap(sMonSummaryScreen->windowIds[POKESUM_WIN_RIGHT_PANE]);
}

u32 GetInfoPageFontIdForString(u8 *str, u32 x)
{
    u8 letterSpacing = GetFontAttribute(FONT_NORMAL, FONTATTR_LETTER_SPACING);
    u32 maxTextWidth = WindowWidthPx(sMonSummaryScreen->windowIds[POKESUM_WIN_RIGHT_PANE]);

    if (x > maxTextWidth)
        maxTextWidth = 0;
    else
        maxTextWidth -= x;

    return GetFontIdToFit(str, FONT_NORMAL, letterSpacing, maxTextWidth);
}

static enum EggHatchTime GetEggHatchTime(void)
{
    u8 eggCycles;

    if (sMonSummaryScreen->isBadEgg)
        return EGG_HATCH_TIME_LONG;

    eggCycles = GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_FRIENDSHIP);

    if (eggCycles <= 5)
        return EGG_HATCH_TIME_ALMOST_READY;
    if (eggCycles <= 10)
        return EGG_HATCH_TIME_SOON;
    if (eggCycles <= 40)
        return EGG_HATCH_TIME_SOME;

    return EGG_HATCH_TIME_LONG;
}

static void PrintInfoPage(void)
{
    const u8 *colors = sLevelNickTextColors[0];
    AddTextPrinterParameterized3(sMonSummaryScreen->windowIds[POKESUM_WIN_RIGHT_PANE], FONT_NORMAL, 47, 19, colors, TEXT_SKIP_DRAW, sMonSummaryScreen->speciesNameStrBuf);

    if (!sMonSummaryScreen->isEgg)
    {
        u32 fontId;

        fontId = GetInfoPageFontIdForString(sMonSummaryScreen->dexNumStrBuf, 47);
        AddTextPrinterParameterized3(sMonSummaryScreen->windowIds[POKESUM_WIN_RIGHT_PANE], fontId, 47, 5, colors, TEXT_SKIP_DRAW, sMonSummaryScreen->dexNumStrBuf);
        fontId = GetInfoPageFontIdForString(sMonSummaryScreen->otNameStrBuf, 47);
        AddTextPrinterParameterized3(sMonSummaryScreen->windowIds[POKESUM_WIN_RIGHT_PANE], fontId, 47, 49, colors, TEXT_SKIP_DRAW, sMonSummaryScreen->otNameStrBuf);
        fontId = GetInfoPageFontIdForString(sMonSummaryScreen->otIdStrBuf, 47);
        AddTextPrinterParameterized3(sMonSummaryScreen->windowIds[POKESUM_WIN_RIGHT_PANE], fontId, 47, 64, colors, TEXT_SKIP_DRAW, sMonSummaryScreen->otIdStrBuf);
        fontId = GetInfoPageFontIdForString(sMonSummaryScreen->itemNameStrBuf, 47);
        AddTextPrinterParameterized3(sMonSummaryScreen->windowIds[POKESUM_WIN_RIGHT_PANE], fontId, 47, 79, colors, TEXT_SKIP_DRAW, sMonSummaryScreen->itemNameStrBuf);
    }
    else
    {
        enum EggHatchTime hatchMsgIndex = GetEggHatchTime();

        AddTextPrinterParameterized3(sMonSummaryScreen->windowIds[POKESUM_WIN_RIGHT_PANE], FONT_NORMAL, 7, 45, colors, TEXT_SKIP_DRAW, sEggHatchTimeTexts[hatchMsgIndex]);
    }
}

static void PrintSkillsPage(void)
{
    u8 statFontId, x, yDiff;
    switch (sMonSummaryScreen->skillsPageMode)
    {
#if P_SUMMARY_SCRREN_IV_DISPLAY == P_SUMMARY_SCREEN_IV_TEXT
        case PSS_SKILL_PAGE_IVS:
            x = 10;
            yDiff = 1;
            statFontId = FONT_SMALL;
            break;
#endif
        default:
            x = 13;
            yDiff = 0;
            statFontId = FONT_NORMAL;
            break;
    }
    AddTextPrinterParameterized3(sMonSummaryScreen->windowIds[POKESUM_WIN_RIGHT_PANE], statFontId, x + sMonSummaryScreen->curHpStrXpos, 4 - yDiff, sLevelNickTextColors[0], TEXT_SKIP_DRAW, sMonSummaryScreen->statValueStrBufs[PSS_STAT_HP]);
    AddTextPrinterParameterized3(sMonSummaryScreen->windowIds[POKESUM_WIN_RIGHT_PANE], statFontId, x + sMonSummaryScreen->atkStrXpos, 22 - yDiff, sLevelNickTextColors[0], TEXT_SKIP_DRAW, sMonSummaryScreen->statValueStrBufs[PSS_STAT_ATK]);
    AddTextPrinterParameterized3(sMonSummaryScreen->windowIds[POKESUM_WIN_RIGHT_PANE], statFontId, x + sMonSummaryScreen->defStrXpos, 35 - yDiff, sLevelNickTextColors[0], TEXT_SKIP_DRAW, sMonSummaryScreen->statValueStrBufs[PSS_STAT_DEF]);
    AddTextPrinterParameterized3(sMonSummaryScreen->windowIds[POKESUM_WIN_RIGHT_PANE], statFontId, x + sMonSummaryScreen->spAStrXpos, 48 - yDiff, sLevelNickTextColors[0], TEXT_SKIP_DRAW, sMonSummaryScreen->statValueStrBufs[PSS_STAT_SPA]);
    AddTextPrinterParameterized3(sMonSummaryScreen->windowIds[POKESUM_WIN_RIGHT_PANE], statFontId, x + sMonSummaryScreen->spDStrXpos, 61 - yDiff, sLevelNickTextColors[0], TEXT_SKIP_DRAW, sMonSummaryScreen->statValueStrBufs[PSS_STAT_SPD]);
    AddTextPrinterParameterized3(sMonSummaryScreen->windowIds[POKESUM_WIN_RIGHT_PANE], statFontId, x + sMonSummaryScreen->speStrXpos, 74 - yDiff, sLevelNickTextColors[0], TEXT_SKIP_DRAW, sMonSummaryScreen->statValueStrBufs[PSS_STAT_SPE]);
    AddTextPrinterParameterized3(sMonSummaryScreen->windowIds[POKESUM_WIN_RIGHT_PANE], FONT_NORMAL, 15 + sMonSummaryScreen->expStrXpos, 87, sLevelNickTextColors[0], TEXT_SKIP_DRAW, sMonSummaryScreen->expPointsStrBuf);
    AddTextPrinterParameterized3(sMonSummaryScreen->windowIds[POKESUM_WIN_RIGHT_PANE], FONT_NORMAL, 15 + sMonSummaryScreen->toNextLevelXpos, 100, sLevelNickTextColors[0], TEXT_SKIP_DRAW, sMonSummaryScreen->expToNextLevelStrBuf);
}

#define GetMoveNamePrinterYpos(x) ((x) * 28 + 5)
#define GetMovePpPrinterYpos(x) ((x) * 28 + 16)

static void PrintMovesPage(void)
{
    u8 i;

    for (i = 0; i < MAX_MON_MOVES; i++)
        PokeSum_PrintMoveName(i);

    if (sMonSummaryScreen->curPageIndex == PSS_PAGE_MOVES_INFO)
    {
        if (sMonSummaryScreen->mode == PSS_MODE_SELECT_MOVE)
            PokeSum_PrintMoveName(MAX_MON_MOVES);
        else
            AddTextPrinterParameterized3(sMonSummaryScreen->windowIds[POKESUM_WIN_RIGHT_PANE], FONT_NORMAL, 3, GetMoveNamePrinterYpos(4), sPrintMoveTextColors[MOVE_TEXT_COLOR_0], TEXT_SKIP_DRAW, gText_Cancel);
    }
}

static enum MoveTextColor GetMoveTextColor(u32 i, u8 curPP, u8 maxPP)
{
    if (sMonSummaryScreen->moveIds[i] == 0 || (curPP == maxPP))
        return MOVE_TEXT_COLOR_0;

    if (curPP == 0)
        return MOVE_TEXT_COLOR_3;

    if (maxPP == 3)
    {
        if (curPP == 2)
            return MOVE_TEXT_COLOR_2;
        else if (curPP == 1)
            return MOVE_TEXT_COLOR_1;

        return MOVE_TEXT_COLOR_0;
    }

    if (maxPP == 2)
    {
        if (curPP == 1)
            return MOVE_TEXT_COLOR_1;

        return MOVE_TEXT_COLOR_0;
    }

    if (curPP <= (maxPP / 4))
        return MOVE_TEXT_COLOR_2;
    else if (curPP <= (maxPP / 2))
        return MOVE_TEXT_COLOR_1;

    return MOVE_TEXT_COLOR_0;
}

static void PokeSum_PrintMoveName(u8 i)
{
    const u8 *color;
    enum MoveTextColor colorIdx;
    u8 curPP = GetMonPpByMoveSlot(&sMonSummaryScreen->currentMon, i);
    enum Move move = sMonSummaryScreen->moveIds[i];
    u8 ppBonuses = GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_PP_BONUSES);
    u8 maxPP = CalculatePPWithBonus(move, ppBonuses, i);
    u8 windowId = sMonSummaryScreen->windowIds[POKESUM_WIN_RIGHT_PANE];

    if (i == MAX_MON_MOVES)
        curPP = maxPP;

    AddTextPrinterParameterized3(windowId, FONT_NORMAL, 0, GetMoveNamePrinterYpos(i), sPrintMoveTextColors[MOVE_TEXT_COLOR_0], TEXT_SKIP_DRAW, sMonSummaryScreen->moveNameStrBufs[i]);

    colorIdx = GetMoveTextColor(i, curPP, maxPP);
    color = sPrintMoveTextColors[colorIdx];

    AddTextPrinterParameterized3(windowId, FONT_NORMAL, 36, GetMovePpPrinterYpos(i), color, TEXT_SKIP_DRAW, sText_PokeSum_PP);
    AddTextPrinterParameterized3(windowId, FONT_NORMAL, 46 + sMonSummaryScreen->curPpXpos[i], GetMovePpPrinterYpos(i), color, TEXT_SKIP_DRAW, sMonSummaryScreen->moveCurPpStrBufs[i]);

    if (sMonSummaryScreen->moveIds[i] != MOVE_NONE)
    {
        AddTextPrinterParameterized3(windowId, FONT_NORMAL, 58, GetMovePpPrinterYpos(i), color, TEXT_SKIP_DRAW, gText_Slash);
        AddTextPrinterParameterized3(windowId, FONT_NORMAL, 64 + sMonSummaryScreen->maxPpXpos[i], GetMovePpPrinterYpos(i), color, TEXT_SKIP_DRAW, sMonSummaryScreen->moveMaxPpStrBufs[i]);
    }
}

static void PokeSum_PrintBottomPaneText(void)
{
    FillWindowPixelBuffer(sMonSummaryScreen->windowIds[POKESUM_WIN_TRAINER_MEMO], 0);

    switch (sMonSummaryScreen->curPageIndex)
    {
    case PSS_PAGE_INFO:
        PokeSum_PrintTrainerMemo();
        break;
    case PSS_PAGE_SKILLS:
        PokeSum_PrintExpPoints_NextLv();
        break;
    case PSS_PAGE_MOVES_INFO:
        PokeSum_PrintSelectedMoveStats();
        break;
    case PSS_PAGE_MOVES:
    case PSS_PAGE_MOVE_DELETER:
        break;
    }

    PutWindowTilemap(sMonSummaryScreen->windowIds[POKESUM_WIN_TRAINER_MEMO]);
}

static void PokeSum_PrintTrainerMemo(void)
{
    if (!sMonSummaryScreen->isEgg)
        PokeSum_PrintTrainerMemo_Mon();
    else
        PokeSum_PrintTrainerMemo_Egg();
}

static void PokeSum_PrintTrainerMemo_Mon_HeldByOT(void)
{
    enum Nature nature;
    u8 level;
    u8 metLocation;
    u8 levelStr[5];
    u8 mapNameStr[32];
    u8 natureMetOrHatchedAtLevelStr[152];

    DynamicPlaceholderTextUtil_Reset();
    nature = GetNature(&sMonSummaryScreen->currentMon);
    DynamicPlaceholderTextUtil_SetPlaceholderPtr(0, gNaturesInfo[nature].name);
    level = GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_MET_LEVEL);

    if (level == 0)
        level = 5;

    ConvertIntToDecimalStringN(levelStr, level, STR_CONV_MODE_LEFT_ALIGN, 3);
    DynamicPlaceholderTextUtil_SetPlaceholderPtr(1, levelStr);

    metLocation = GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_MET_LOCATION);

    if (MapSecIsInKantoOrSevii(metLocation) == TRUE)
        GetMapNameGeneric_(mapNameStr, metLocation);
    else
    {
        if (sMonSummaryScreen->isEnemyParty == TRUE || IsMultiBattlePartner() == TRUE)
            StringCopy(mapNameStr, gText_Somewhere);
        else
            StringCopy(mapNameStr, sText_PokeSum_ATrade);
    }

    DynamicPlaceholderTextUtil_SetPlaceholderPtr(2, mapNameStr);

    // These pairs of strings are bytewise identical to each other in English,
    // but Japanese uses different grammar for Bold and Gentle natures.
    if (GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_MET_LEVEL) == 0) // Hatched
    {
        if (GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_MODERN_FATEFUL_ENCOUNTER) == TRUE)
            DynamicPlaceholderTextUtil_ExpandPlaceholders(natureMetOrHatchedAtLevelStr, sText_PokeSum_FatefulEncounterHatched);
        else
            DynamicPlaceholderTextUtil_ExpandPlaceholders(natureMetOrHatchedAtLevelStr, sText_PokeSum_Hatched);
    }
    else
    {
        if (metLocation == METLOC_FATEFUL_ENCOUNTER)
            DynamicPlaceholderTextUtil_ExpandPlaceholders(natureMetOrHatchedAtLevelStr, sText_PokeSum_FatefulEncounterMet);
        else
            DynamicPlaceholderTextUtil_ExpandPlaceholders(natureMetOrHatchedAtLevelStr, sText_PokeSum_Met);
    }

    AddTextPrinterParameterized4(sMonSummaryScreen->windowIds[POKESUM_WIN_TRAINER_MEMO], FONT_NORMAL, 0, 3, 0, 0, sLevelNickTextColors[0], TEXT_SKIP_DRAW, natureMetOrHatchedAtLevelStr);
}

static void PokeSum_PrintTrainerMemo_Mon_NotHeldByOT(void)
{
    enum Nature nature;
    u8 level;
    u8 metLocation;
    u8 levelStr[5];
    u8 mapNameStr[32];
    u8 natureMetOrHatchedAtLevelStr[152];

    DynamicPlaceholderTextUtil_Reset();
    nature = GetNature(&sMonSummaryScreen->currentMon);
    DynamicPlaceholderTextUtil_SetPlaceholderPtr(0, gNaturesInfo[nature].name);

    level = GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_MET_LEVEL);

    if (level == 0)
        level = 5;

    ConvertIntToDecimalStringN(levelStr, level, STR_CONV_MODE_LEFT_ALIGN, 3);
    DynamicPlaceholderTextUtil_SetPlaceholderPtr(1, levelStr);

    metLocation = GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_MET_LOCATION);

    if (!MapSecIsInKantoOrSevii(metLocation) || !CurrentMonIsFromGBA())
    {
        if (IsMultiBattlePartner() == TRUE)
        {
            PokeSum_PrintTrainerMemo_Mon_HeldByOT();
            return;
        }

        if (metLocation == METLOC_FATEFUL_ENCOUNTER)
            DynamicPlaceholderTextUtil_ExpandPlaceholders(natureMetOrHatchedAtLevelStr, sText_PokeSum_FatefulEncounterMet);
        else
            DynamicPlaceholderTextUtil_ExpandPlaceholders(natureMetOrHatchedAtLevelStr, sText_PokeSum_MetInATrade);

        AddTextPrinterParameterized4(sMonSummaryScreen->windowIds[POKESUM_WIN_TRAINER_MEMO], FONT_NORMAL, 0, 3, 0, 0, sLevelNickTextColors[0], TEXT_SKIP_DRAW, natureMetOrHatchedAtLevelStr);
        return;
    }

    if (MapSecIsInKantoOrSevii(metLocation) == TRUE)
        GetMapNameGeneric_(mapNameStr, metLocation);
    else
        StringCopy(mapNameStr, sText_PokeSum_ATrade);

    DynamicPlaceholderTextUtil_SetPlaceholderPtr(2, mapNameStr);

    if (GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_MET_LEVEL) == 0) // hatched from an EGG
    {
        if (GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_MODERN_FATEFUL_ENCOUNTER) == TRUE)
            DynamicPlaceholderTextUtil_ExpandPlaceholders(natureMetOrHatchedAtLevelStr, sText_PokeSum_ApparentlyFatefulEncounterHatched);
        else
            DynamicPlaceholderTextUtil_ExpandPlaceholders(natureMetOrHatchedAtLevelStr, sText_PokeSum_ApparentlyMet);
    }
    else
    {
        if (metLocation == METLOC_FATEFUL_ENCOUNTER)
            DynamicPlaceholderTextUtil_ExpandPlaceholders(natureMetOrHatchedAtLevelStr, sText_PokeSum_FatefulEncounterMet);
        else
            DynamicPlaceholderTextUtil_ExpandPlaceholders(natureMetOrHatchedAtLevelStr, sText_PokeSum_ApparentlyMet);
    }

    AddTextPrinterParameterized4(sMonSummaryScreen->windowIds[POKESUM_WIN_TRAINER_MEMO], FONT_NORMAL, 0, 3, 0, 0, sLevelNickTextColors[0], TEXT_SKIP_DRAW, natureMetOrHatchedAtLevelStr);
}

static void PokeSum_PrintTrainerMemo_Mon(void)
{
    if (PokeSum_BufferOtName_IsEqualToCurrentOwner(&sMonSummaryScreen->currentMon) == TRUE)
        PokeSum_PrintTrainerMemo_Mon_HeldByOT();
    else
        PokeSum_PrintTrainerMemo_Mon_NotHeldByOT();
}

static enum EggOrigin GetEggOrigin(void)
{
    u8 metLocation;
    enum GameVersion version;

    if (sMonSummaryScreen->isBadEgg)
        return EGG_ORIGIN_DAYCARE;

    metLocation = GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_MET_LOCATION);
    if (metLocation == METLOC_FATEFUL_ENCOUNTER || GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_MODERN_FATEFUL_ENCOUNTER) == TRUE)
        return EGG_ORIGIN_NICE_PLACE;

    if (PokeSum_BufferOtName_IsEqualToCurrentOwner(&sMonSummaryScreen->currentMon) == FALSE)
        return EGG_ORIGIN_TRADE;

    version = GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_MET_GAME);
    if (version != VERSION_LEAF_GREEN && version != VERSION_FIRE_RED)
    {
        if (sMonSummaryScreen->monList.mons != gEnemyParty)
            return EGG_ORIGIN_TRADE;

        if (metLocation == METLOC_SPECIAL_EGG)
            return EGG_ORIGIN_SPA;

        return EGG_ORIGIN_DAYCARE;
    }

    if (metLocation == METLOC_SPECIAL_EGG)
        return EGG_ORIGIN_TRAVELING_MAN;

    return EGG_ORIGIN_DAYCARE;
}

static void PokeSum_PrintTrainerMemo_Egg(void)
{
    enum EggOrigin eggOrigin = GetEggOrigin();

    AddTextPrinterParameterized4(sMonSummaryScreen->windowIds[POKESUM_WIN_TRAINER_MEMO], FONT_NORMAL, 0, 3, 0, 0, sLevelNickTextColors[0], TEXT_SKIP_DRAW, sEggOriginTexts[eggOrigin]);
}

static void PokeSum_PrintExpPoints_NextLv(void)
{
    u8 windowId = sMonSummaryScreen->windowIds[POKESUM_WIN_TRAINER_MEMO];

    AddTextPrinterParameterized3(windowId, FONT_NORMAL, 9,  7, sLevelNickTextColors[0], TEXT_SKIP_DRAW, sText_PokeSum_ExpPoints);
    AddTextPrinterParameterized3(windowId, FONT_NORMAL, 9, 20, sLevelNickTextColors[0], TEXT_SKIP_DRAW, sText_PokeSum_NextLv);
}

// code
static u8 ShowCategoryIcon(u32 category)
{
    if (sMonSummaryScreen->categoryIconSpriteId == 0xFF)
        sMonSummaryScreen->categoryIconSpriteId = CreateSprite(&gSpriteTemplate_CategoryIcons, 90, 63, 0);

    gSprites[sMonSummaryScreen->categoryIconSpriteId].invisible = FALSE;
    StartSpriteAnim(&gSprites[sMonSummaryScreen->categoryIconSpriteId], category);
    return sMonSummaryScreen->categoryIconSpriteId;
}

static void HideCategoryIcon()
{
    if (sMonSummaryScreen->categoryIconSpriteId != 0xFF)
        gSprites[sMonSummaryScreen->categoryIconSpriteId].invisible = TRUE;
}

static void DestroyCategoryIcon(void)
{
    if (sMonSummaryScreen->categoryIconSpriteId != 0xFF)
        DestroySprite(&gSprites[sMonSummaryScreen->categoryIconSpriteId]);
    sMonSummaryScreen->categoryIconSpriteId = 0xFF;
}

static void PokeSum_PrintSelectedMoveStats(void)
{
    const u8 *description;
    enum Move moveId;
    u8 windowId;

    if (sMoveSelectionCursorPos > MAX_MON_MOVES)
        return;

    if (sMonSummaryScreen->mode != PSS_MODE_SELECT_MOVE && sMoveSelectionCursorPos == MAX_MON_MOVES)
    {
        HideCategoryIcon();
        return;
    }

    windowId = sMonSummaryScreen->windowIds[POKESUM_WIN_TRAINER_MEMO];
    AddTextPrinterParameterized3(windowId, FONT_NORMAL, 57,  1, sLevelNickTextColors[0], TEXT_SKIP_DRAW, sMonSummaryScreen->movePowerStrBufs[sMoveSelectionCursorPos]);
    AddTextPrinterParameterized3(windowId, FONT_NORMAL, 57, 15, sLevelNickTextColors[0], TEXT_SKIP_DRAW, sMonSummaryScreen->moveAccuracyStrBufs[sMoveSelectionCursorPos]);

    moveId = sMonSummaryScreen->moveIds[sMoveSelectionCursorPos];
    if (gMovesInfo[moveId].effect == EFFECT_PLACEHOLDER)
        description = gNotDoneYetDescription;
    else
        description = gMovesInfo[moveId].description;

    AddTextPrinterParameterized4(windowId, FONT_NORMAL, 7, 42, 0, 0, sLevelNickTextColors[0], TEXT_SKIP_DRAW, description);

    if (B_SHOW_CATEGORY_ICON == TRUE)
        ShowCategoryIcon(GetBattleMoveCategory(sMonSummaryScreen->moveIds[sMoveSelectionCursorPos]));
}

static void PokeSum_PrintAbilityDataOrMoveTypes(void)
{
    switch (sMonSummaryScreen->curPageIndex)
    {
    case PSS_PAGE_INFO:
    case PSS_PAGE_MOVE_DELETER:
        break;
    case PSS_PAGE_SKILLS:
        PokeSum_PrintAbilityNameAndDesc();
        break;
    case PSS_PAGE_MOVES:
    case PSS_PAGE_MOVES_INFO:
        PokeSum_DrawMoveTypeIcons();
        break;
    }

    PutWindowTilemap(sMonSummaryScreen->windowIds[5]);
}

static void PokeSum_PrintAbilityNameAndDesc(void)
{
    FillWindowPixelBuffer(sMonSummaryScreen->windowIds[5], 0);

    AddTextPrinterParameterized3(sMonSummaryScreen->windowIds[5], FONT_NORMAL,
                                 49, 1, sLevelNickTextColors[0], TEXT_SKIP_DRAW, sMonSummaryScreen->abilityNameStrBuf);

    AddTextPrinterParameterized3(sMonSummaryScreen->windowIds[5], FONT_NORMAL,
                                 2, 15, sLevelNickTextColors[0], TEXT_SKIP_DRAW,
                                 sMonSummaryScreen->abilityDescStrBuf);

}

static void PokeSum_DrawMoveTypeIcons(void)
{
    u8 i;

    FillWindowPixelBuffer(sMonSummaryScreen->windowIds[5], 0);

    if (P_USE_TYPE_ICON_SPRITES)
    {
        UpdateMoveTypeIconSprites();
        return;
    }

    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        if (sMonSummaryScreen->moveIds[i] == MOVE_NONE)
            continue;

        BlitMenuTypeIcon(sMonSummaryScreen->windowIds[5], sMonSummaryScreen->moveTypes[i], 3, GetMoveNamePrinterYpos(i));
    }

    if (sMonSummaryScreen->mode == PSS_MODE_SELECT_MOVE)
        BlitMenuTypeIcon(sMonSummaryScreen->windowIds[5], sMonSummaryScreen->moveTypes[MAX_MON_MOVES], 3, GetMoveNamePrinterYpos(MAX_MON_MOVES));
}

static const u8 *const sText_PageRename = COMPOUND_STRING("{DPAD_RIGHT}PAGE {A_BUTTON}RENAME");

static void PrintControlsString(void)
{
    const u8 *controlsStr;

    switch (sMonSummaryScreen->curPageIndex)
    {
    case PSS_PAGE_INFO:
        if (!sMonSummaryScreen->isEgg)
        {
            if (CanRename())
                controlsStr = sText_PageRename;
            else
                controlsStr = sText_PokeSum_Controls_PageCancel;
        }
        else
        {
            controlsStr = sText_PokeSum_Controls_Cancel;
        }
        break;
    case PSS_PAGE_SKILLS:
        controlsStr = GetStatControlString();
        break;
    case PSS_PAGE_MOVES:
        controlsStr = sText_PokeSum_Controls_PageDetail;
        break;
    case PSS_PAGE_MOVES_INFO:
        if (!gMain.inBattle || gReceivedRemoteLinkPlayers)
            controlsStr = sText_PokeSum_Controls_PickSwitch;
        else
            controlsStr = sText_PokeSum_Controls_Pick;
        break;
    case PSS_PAGE_MOVE_DELETER:
        controlsStr = sText_PokeSum_Controls_PickDelete;
        break;
    default:
        return;
    }
    PokeSum_PrintControlsString(controlsStr);
}

static void PokeSum_PrintPageHeaderText(u8 curPageIndex)
{
    const u8 *pageNameStr;
    switch (curPageIndex)
    {
    case PSS_PAGE_INFO:
        pageNameStr = sText_PokeSum_PageName_PokemonInfo;
        break;
    case PSS_PAGE_SKILLS:
        pageNameStr = sText_PokeSum_PageName_PokemonSkills;
        break;
    case PSS_PAGE_MOVES:
    case PSS_PAGE_MOVES_INFO:
    case PSS_PAGE_MOVE_DELETER:
        pageNameStr = sText_PokeSum_PageName_KnownMoves;
        break;
    default:
        return;
    }
    PokeSum_PrintPageName(pageNameStr);
    PrintControlsString();
    PrintMonLevelNickOnWindow2(sText_PokeSum_NoData);
}

static void CommitStaticWindowTilemaps(void)
{
    PutWindowTilemap(sMonSummaryScreen->windowIds[PSS_WIN_PAGE_NAME]);
    PutWindowTilemap(sMonSummaryScreen->windowIds[PSS_WIN_CONTROLS]);
    PutWindowTilemap(sMonSummaryScreen->windowIds[PSS_WIN_LVL_NICK]);
}

static void Task_DestroyResourcesOnExit(u8 taskId)
{
    if (sMonSummaryScreen->savedCallback == gInitialSummaryScreenCallback)
        gInitialSummaryScreenCallback = NULL;
    PokeSum_DestroySprites();
    FreeAllSpritePalettes();

    if (IsCryPlayingOrClearCrySongs() == TRUE)
        StopCryAndClearCrySongs();

    PokeSum_RemoveWindows();
    FreeAllWindowBuffers();
    DestroyTask(taskId);
    SetMainCallback2(sMonSummaryScreen->savedCallback);

    gLastViewedMonIndex = GetLastViewedMonIndex();

    TRY_FREE_AND_SET_NULL(sMonSummaryScreen);
}

static void MainCB2(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

static void PokeSum_FlipPages_SlideHpExpBarsOut(void)
{
    u8 i;

    for (i = 0; i < 11; i++)
    {
        if (sExpBarObjs->xpos[i] < 240)
        {
            sExpBarObjs->xpos[i] += 60;
            sExpBarObjs->sprites[i]->x = sExpBarObjs->xpos[i] + 60;
        }

        if (i >= 9)
            continue;

        if (sHpBarObjs->xpos[i] < 240)
        {
            sHpBarObjs->xpos[i] += 60;
            sHpBarObjs->sprites[i]->x = sHpBarObjs->xpos[i] + 60;
        }
    }
}

static void PokeSum_FlipPages_SlideHpExpBarsIn(void)
{
    u8 i;

    for (i = 0; i < 11; i++)
    {
        if (sExpBarObjs->xpos[i] > 156 + (8 * i))
        {
            sExpBarObjs->xpos[i] -= 60;

            if (sExpBarObjs->xpos[i] < 156 + (8 * i))
                sExpBarObjs->xpos[i] = 156 + (8 * i);

            sExpBarObjs->sprites[i]->x = sExpBarObjs->xpos[i];
        }

        if (i >= 9)
            continue;

        if (sHpBarObjs->xpos[i] > 172 + (8 * i))
        {
            sHpBarObjs->xpos[i] -= 60;

            if (sHpBarObjs->xpos[i] < 172 + (8 * i))
                sHpBarObjs->xpos[i] = 172 + (8 * i);

            sHpBarObjs->sprites[i]->x = sHpBarObjs->xpos[i];
        }
    }
}

static void PokeSum_FlipPages_SlideLayerLeft(void)
{
    if (sMonSummaryScreen->flipPagesBgHofs < 240)
    {
        sMonSummaryScreen->flipPagesBgHofs += 60;
        if (sMonSummaryScreen->flipPagesBgHofs > 240)
            sMonSummaryScreen->flipPagesBgHofs = 240;

        if (sMonSummaryScreen->whichBgLayerToTranslate == 0)
            SetGpuReg(REG_OFFSET_BG2HOFS, -sMonSummaryScreen->flipPagesBgHofs);
        else
            SetGpuReg(REG_OFFSET_BG1HOFS, -sMonSummaryScreen->flipPagesBgHofs);
    }
}

static void PokeSum_FlipPages_SlideLayeRight(void)
{
    if (sMonSummaryScreen->flipPagesBgHofs >= 60)
    {
        sMonSummaryScreen->flipPagesBgHofs -= 60;
        if (sMonSummaryScreen->flipPagesBgHofs < 0)
            sMonSummaryScreen->flipPagesBgHofs = 0;

        if (sMonSummaryScreen->whichBgLayerToTranslate == 0)
            SetGpuReg(REG_OFFSET_BG1HOFS, -sMonSummaryScreen->flipPagesBgHofs);
        else
            SetGpuReg(REG_OFFSET_BG2HOFS, -sMonSummaryScreen->flipPagesBgHofs);

        if (sMonSummaryScreen->curPageIndex != PSS_PAGE_MOVES_INFO)
            SetGpuReg(REG_OFFSET_BG0HOFS, -sMonSummaryScreen->flipPagesBgHofs);
    }
}

static void PokeSum_FlipPages_HandleBgHofs(void)
{
    if (sMonSummaryScreen->pageFlipDirection == 1) // Right
    {
        if (sMonSummaryScreen->curPageIndex != PSS_PAGE_MOVES_INFO)
            PokeSum_FlipPages_SlideLayerLeft();
        else
            PokeSum_FlipPages_SlideLayeRight();
    }
    else
    {
        if (sMonSummaryScreen->curPageIndex != PSS_PAGE_MOVES)
            PokeSum_FlipPages_SlideLayeRight();
        else
            PokeSum_FlipPages_SlideLayerLeft();
    }
}

static void PokeSum_FlipPages_HandleHpExpBarSprites(void)
{
    if (sMonSummaryScreen->curPageIndex == PSS_PAGE_SKILLS
        && sMonSummaryScreen->pageFlipDirection == 0)
        PokeSum_FlipPages_SlideHpExpBarsIn();

    if (sMonSummaryScreen->curPageIndex == PSS_PAGE_MOVES
        && sMonSummaryScreen->pageFlipDirection == 1)
        PokeSum_FlipPages_SlideHpExpBarsOut();
}

static void VBlankCB_PokemonSummaryScreen(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();

    if (sMonSummaryScreen->flippingPages == FALSE)
        return;

    PokeSum_FlipPages_HandleBgHofs();
    PokeSum_FlipPages_HandleHpExpBarSprites();
}

static void PokeSum_Setup_SetVBlankCallback(void)
{
    SetVBlankCallback(VBlankCB_PokemonSummaryScreen);
}

static void PokeSum_CreateWindows(void)
{
    u8 i;

    InitWindows(sWindowTemplates_Dummy);

    for (i = 0; i < 3; i++)
        sMonSummaryScreen->windowIds[i] = AddWindow(&sWindowTemplates_Permanent_Bg1[i]);

    for (i = 0; i < 4; i++)
        switch (sMonSummaryScreen->curPageIndex)
        {
        case PSS_PAGE_INFO:
            sMonSummaryScreen->windowIds[i + 3] = AddWindow(&sWindowTemplates_Info[i]);
            break;
        case PSS_PAGE_SKILLS:
            sMonSummaryScreen->windowIds[i + 3] = AddWindow(&sWindowTemplates_Skills[i]);
            break;
        case PSS_PAGE_MOVES:
        case PSS_PAGE_MOVES_INFO:
            sMonSummaryScreen->windowIds[i + 3] = AddWindow(&sWindowTemplates_Moves[i]);
            break;
        default:
            break;
        }
}

static const struct WindowTemplate *GetPermanentWindowTemplate(void)
{
    u32 bgPriority1 = GetGpuReg(REG_OFFSET_BG1CNT) & 3;
    u32 bgPriority2 = GetGpuReg(REG_OFFSET_BG2CNT) & 3;
    if ((sMonSummaryScreen->pageFlipDirection == 1 && sMonSummaryScreen->curPageIndex != PSS_PAGE_MOVES_INFO)
        || (sMonSummaryScreen->pageFlipDirection == 0 && sMonSummaryScreen->curPageIndex == PSS_PAGE_MOVES))
    {
        if (bgPriority2 > bgPriority1)
            return sWindowTemplates_Permanent_Bg2;
        else
            return sWindowTemplates_Permanent_Bg1;
    }
    else
    {
        if (bgPriority2 > bgPriority1)
            return sWindowTemplates_Permanent_Bg1;
        else
            return sWindowTemplates_Permanent_Bg2;
    }
}

static const struct WindowTemplate *GetPageWindowTemplate(enum PokemonSummaryScreenPage pageIndex)
{
    switch (pageIndex)
    {
    case PSS_PAGE_INFO:
        return sWindowTemplates_Info;
    case PSS_PAGE_SKILLS:
    default:
        return sWindowTemplates_Skills;
    case PSS_PAGE_MOVES:
    case PSS_PAGE_MOVES_INFO:
        return sWindowTemplates_Moves;
    }
}

static void PokeSum_AddWindows(enum PokemonSummaryScreenPage curPageIndex)
{
    const struct WindowTemplate *permanentTemplate;
    const struct WindowTemplate *pageTemplate;

    for (u32 i = 0; i < SUMMARY_WINDOW_COUNT; i++)
        sMonSummaryScreen->windowIds[i] = WINDOW_NONE;

    permanentTemplate = GetPermanentWindowTemplate();
    pageTemplate = GetPageWindowTemplate(curPageIndex);

    for (u32 i = 0; i < SUMMARY_PRIMARY_WIN_COUNT; i++)
        sMonSummaryScreen->windowIds[i] = AddWindow(&permanentTemplate[i]);

    for (u32 i = 0; i < SUMMARY_SECONDARY_WIN_COUNT; i++)
        sMonSummaryScreen->windowIds[i + 3] = AddWindow(&pageTemplate[i]);
}

static void PokeSum_RemoveWindows(void)
{
    for (u32 i = 0; i < ARRAY_COUNT(sMonSummaryScreen->windowIds); i++)
        RemoveWindow(sMonSummaryScreen->windowIds[i]);
}

static void PokeSum_SetHelpContext(void)
{
    switch (sMonSummaryScreen->curPageIndex)
    {
    case PSS_PAGE_INFO:
        SetHelpContext(HELPCONTEXT_POKEMON_INFO);
        break;
    case PSS_PAGE_SKILLS:
        SetHelpContext(HELPCONTEXT_POKEMON_SKILLS);
        break;
    case PSS_PAGE_MOVES:
    case PSS_PAGE_MOVES_INFO:
        SetHelpContext(HELPCONTEXT_POKEMON_MOVES);
        break;
    case PSS_PAGE_MOVE_DELETER:
        break;
    }
}

static u8 PokeSum_BufferOtName_IsEqualToCurrentOwner(struct Pokemon *mon)
{
    u8 multiplayerId;
    u32 otId = 0;

    if (sMonSummaryScreen->monList.mons == gEnemyParty)
    {
        multiplayerId = GetMultiplayerId() ^ 1;
        otId = gLinkPlayers[multiplayerId].trainerId & 0xFFFF;
        StringCopy(sMonSummaryScreen->otNameStrBufs[0], gLinkPlayers[multiplayerId].name);
    }
    else
    {
        otId = GetPlayerTrainerId() & 0xFFFF;
        StringCopy(sMonSummaryScreen->otNameStrBufs[0], gSaveBlock2Ptr->playerName);
    }

    if (otId != (GetMonData(mon, MON_DATA_OT_ID) & 0xFFFF))
        return FALSE;

    GetMonData(mon, MON_DATA_OT_NAME, sMonSummaryScreen->otNameStrBufs[1]);

    if (!StringCompareWithoutExtCtrlCodes(sMonSummaryScreen->otNameStrBufs[0], sMonSummaryScreen->otNameStrBufs[1]))
        return TRUE;
    else
        return FALSE;

    return TRUE;
}

#define PAGE_PROGRESS_BASE_TILE_NUM (345)

static void PokeSum_DrawPageProgressTiles(void)
{
    switch (sMonSummaryScreen->curPageIndex)
    {
    case PSS_PAGE_INFO:
        if (!sMonSummaryScreen->isEgg)
        {
            FillBgTilemapBufferRect(3, 17 + PAGE_PROGRESS_BASE_TILE_NUM, 13, 0, 1, 1, 0);
            FillBgTilemapBufferRect(3, 33 + PAGE_PROGRESS_BASE_TILE_NUM, 13, 1, 1, 1, 0);
            FillBgTilemapBufferRect(3, 16 + PAGE_PROGRESS_BASE_TILE_NUM, 14, 0, 1, 1, 0);
            FillBgTilemapBufferRect(3, 32 + PAGE_PROGRESS_BASE_TILE_NUM, 14, 1, 1, 1, 0);
            FillBgTilemapBufferRect(3, 18 + PAGE_PROGRESS_BASE_TILE_NUM, 15, 0, 1, 1, 0);
            FillBgTilemapBufferRect(3, 34 + PAGE_PROGRESS_BASE_TILE_NUM, 15, 1, 1, 1, 0);
            FillBgTilemapBufferRect(3, 20 + PAGE_PROGRESS_BASE_TILE_NUM, 16, 0, 1, 1, 0);
            FillBgTilemapBufferRect(3, 36 + PAGE_PROGRESS_BASE_TILE_NUM, 16, 1, 1, 1, 0);
            FillBgTilemapBufferRect(3, 18 + PAGE_PROGRESS_BASE_TILE_NUM, 17, 0, 1, 1, 0);
            FillBgTilemapBufferRect(3, 34 + PAGE_PROGRESS_BASE_TILE_NUM, 17, 1, 1, 1, 0);
            FillBgTilemapBufferRect(3, 21 + PAGE_PROGRESS_BASE_TILE_NUM, 18, 0, 1, 1, 0);
            FillBgTilemapBufferRect(3, 37 + PAGE_PROGRESS_BASE_TILE_NUM, 18, 1, 1, 1, 0);
        }
        else
        {
            FillBgTilemapBufferRect(3, 17 + PAGE_PROGRESS_BASE_TILE_NUM, 13, 0, 1, 1, 0);
            FillBgTilemapBufferRect(3, 33 + PAGE_PROGRESS_BASE_TILE_NUM, 13, 1, 1, 1, 0);
            FillBgTilemapBufferRect(3, 48 + PAGE_PROGRESS_BASE_TILE_NUM, 14, 0, 1, 1, 0);
            FillBgTilemapBufferRect(3, 64 + PAGE_PROGRESS_BASE_TILE_NUM, 14, 1, 1, 1, 0);
            FillBgTilemapBufferRect(3,  2 + PAGE_PROGRESS_BASE_TILE_NUM, 15, 0, 4, 2, 0);
        }
        break;
    case PSS_PAGE_SKILLS:
        FillBgTilemapBufferRect(3, 49 + PAGE_PROGRESS_BASE_TILE_NUM, 13, 0, 1, 1, 0);
        FillBgTilemapBufferRect(3, 65 + PAGE_PROGRESS_BASE_TILE_NUM, 13, 1, 1, 1, 0);
        FillBgTilemapBufferRect(3,  1 + PAGE_PROGRESS_BASE_TILE_NUM, 14, 0, 1, 1, 0);
        FillBgTilemapBufferRect(3, 19 + PAGE_PROGRESS_BASE_TILE_NUM, 14, 1, 1, 1, 0);
        FillBgTilemapBufferRect(3, 17 + PAGE_PROGRESS_BASE_TILE_NUM, 15, 0, 1, 1, 0);
        FillBgTilemapBufferRect(3, 33 + PAGE_PROGRESS_BASE_TILE_NUM, 15, 1, 1, 1, 0);
        FillBgTilemapBufferRect(3, 16 + PAGE_PROGRESS_BASE_TILE_NUM, 16, 0, 1, 1, 0);
        FillBgTilemapBufferRect(3, 32 + PAGE_PROGRESS_BASE_TILE_NUM, 16, 1, 1, 1, 0);
        FillBgTilemapBufferRect(3, 18 + PAGE_PROGRESS_BASE_TILE_NUM, 17, 0, 1, 1, 0);
        FillBgTilemapBufferRect(3, 34 + PAGE_PROGRESS_BASE_TILE_NUM, 17, 1, 1, 1, 0);
        FillBgTilemapBufferRect(3, 21 + PAGE_PROGRESS_BASE_TILE_NUM, 18, 0, 1, 1, 0);
        FillBgTilemapBufferRect(3, 37 + PAGE_PROGRESS_BASE_TILE_NUM, 18, 1, 1, 1, 0);
        break;
    case PSS_PAGE_MOVES:
        FillBgTilemapBufferRect(3, 49 + PAGE_PROGRESS_BASE_TILE_NUM, 13, 0, 1, 1, 0);
        FillBgTilemapBufferRect(3, 65 + PAGE_PROGRESS_BASE_TILE_NUM, 13, 1, 1, 1, 0);
        FillBgTilemapBufferRect(3,  1 + PAGE_PROGRESS_BASE_TILE_NUM, 14, 0, 1, 1, 0);
        FillBgTilemapBufferRect(3, 19 + PAGE_PROGRESS_BASE_TILE_NUM, 14, 1, 1, 1, 0);
        FillBgTilemapBufferRect(3, 49 + PAGE_PROGRESS_BASE_TILE_NUM, 15, 0, 1, 1, 0);
        FillBgTilemapBufferRect(3, 65 + PAGE_PROGRESS_BASE_TILE_NUM, 15, 1, 1, 1, 0);
        FillBgTilemapBufferRect(3,  1 + PAGE_PROGRESS_BASE_TILE_NUM, 16, 0, 1, 1, 0);
        FillBgTilemapBufferRect(3, 19 + PAGE_PROGRESS_BASE_TILE_NUM, 16, 1, 1, 1, 0);
        FillBgTilemapBufferRect(3, 17 + PAGE_PROGRESS_BASE_TILE_NUM, 17, 0, 1, 1, 0);
        FillBgTilemapBufferRect(3, 33 + PAGE_PROGRESS_BASE_TILE_NUM, 17, 1, 1, 1, 0);
        FillBgTilemapBufferRect(3, 48 + PAGE_PROGRESS_BASE_TILE_NUM, 18, 0, 1, 1, 0);
        FillBgTilemapBufferRect(3, 64 + PAGE_PROGRESS_BASE_TILE_NUM, 18, 1, 1, 1, 0);
        break;
    case PSS_PAGE_MOVES_INFO:
        if (sMonSummaryScreen->mode == PSS_MODE_SELECT_MOVE)
        {
            FillBgTilemapBufferRect(3,  1 + PAGE_PROGRESS_BASE_TILE_NUM, 13, 0, 4, 1, 0);
            FillBgTilemapBufferRect(3, 19 + PAGE_PROGRESS_BASE_TILE_NUM, 13, 1, 4, 1, 0);
        }
        else
        {
            FillBgTilemapBufferRect(3, 49 + PAGE_PROGRESS_BASE_TILE_NUM, 13, 0, 1, 1, 0);
            FillBgTilemapBufferRect(3, 65 + PAGE_PROGRESS_BASE_TILE_NUM, 13, 1, 1, 1, 0);
            FillBgTilemapBufferRect(3,  1 + PAGE_PROGRESS_BASE_TILE_NUM, 14, 0, 1, 1, 0);
            FillBgTilemapBufferRect(3, 19 + PAGE_PROGRESS_BASE_TILE_NUM, 14, 1, 1, 1, 0);
            FillBgTilemapBufferRect(3, 49 + PAGE_PROGRESS_BASE_TILE_NUM, 15, 0, 1, 1, 0);
            FillBgTilemapBufferRect(3, 65 + PAGE_PROGRESS_BASE_TILE_NUM, 15, 1, 1, 1, 0);
            FillBgTilemapBufferRect(3,  1 + PAGE_PROGRESS_BASE_TILE_NUM, 16, 0, 1, 1, 0);
            FillBgTilemapBufferRect(3, 19 + PAGE_PROGRESS_BASE_TILE_NUM, 16, 1, 1, 1, 0);
        }
        FillBgTilemapBufferRect(3, 50 + PAGE_PROGRESS_BASE_TILE_NUM, 17, 0, 1, 1, 0);
        FillBgTilemapBufferRect(3, 66 + PAGE_PROGRESS_BASE_TILE_NUM, 17, 1, 1, 1, 0);
        FillBgTilemapBufferRect(3, 48 + PAGE_PROGRESS_BASE_TILE_NUM, 18, 0, 1, 1, 0);
        FillBgTilemapBufferRect(3, 64 + PAGE_PROGRESS_BASE_TILE_NUM, 18, 1, 1, 1, 0);
        break;
    case PSS_PAGE_MOVE_DELETER:
        break;
    }
}

static void PokeSum_PrintMonTypeIcons(void)
{
    switch (sMonSummaryScreen->curPageIndex)
    {
    case PSS_PAGE_INFO:
        if (!sMonSummaryScreen->isEgg)
        {
            if (P_USE_TYPE_ICON_SPRITES)
            {
                UpdateMonTypeIconSprites(TRUE);
                return;
            }
            BlitMenuTypeIcon(sMonSummaryScreen->windowIds[POKESUM_WIN_RIGHT_PANE], sMonSummaryScreen->monTypes[0], 47, 35);

            if (sMonSummaryScreen->monTypes[0] != sMonSummaryScreen->monTypes[1])
                BlitMenuTypeIcon(sMonSummaryScreen->windowIds[POKESUM_WIN_RIGHT_PANE], sMonSummaryScreen->monTypes[1], 83, 35);
            if (P_SHOW_TERA_TYPE == GEN_9 && sMonSummaryScreen->monTypes[2] != TYPE_NONE)
                BlitMenuTypeIcon(sMonSummaryScreen->windowIds[POKESUM_WIN_RIGHT_PANE], sMonSummaryScreen->monTypes[2], 83, 5);
        }
        break;
    case PSS_PAGE_SKILLS:
    case PSS_PAGE_MOVES:
    case PSS_PAGE_MOVE_DELETER:
        break;
    case PSS_PAGE_MOVES_INFO:
        FillWindowPixelBuffer(sMonSummaryScreen->windowIds[6], 0);
        if (P_USE_TYPE_ICON_SPRITES)
        {
            UpdateMonTypeIconSprites(FALSE);
            return;
        }
        BlitMenuTypeIcon(sMonSummaryScreen->windowIds[6], sMonSummaryScreen->monTypes[0], 0, 3);

        if (sMonSummaryScreen->monTypes[0] != sMonSummaryScreen->monTypes[1])
            BlitMenuTypeIcon(sMonSummaryScreen->windowIds[6], sMonSummaryScreen->monTypes[1], 36, 3);

        PutWindowTilemap(sMonSummaryScreen->windowIds[6]);
        break;
    }
}

u8 GetLastViewedMonIndex(void)
{
    return gLastViewedMonIndex;
}

u8 GetMoveSlotToReplace(void)
{
    return sMoveSwapCursorPos;
}

void SetPokemonSummaryScreenMode(u8 mode)
{
    sMonSummaryScreen->mode = mode;
}

static bool32 IsMultiBattlePartner(void)
{
    if (!IsOverworldLinkActive()
        && IsMultiBattle() == TRUE
        && gReceivedRemoteLinkPlayers == 1
        && (gLastViewedMonIndex >= 4 || gLastViewedMonIndex == 1))
        return TRUE;

    return FALSE;
}

static void BufferSelectedMonData(struct Pokemon *mon)
{
    if (!sMonSummaryScreen->isBoxMon)
    {
        struct Pokemon *partyMons = sMonSummaryScreen->monList.mons;
        *mon = partyMons[GetLastViewedMonIndex()];
    }
    else
    {
        struct BoxPokemon *boxMons = sMonSummaryScreen->monList.boxMons;
        BoxMonToMon(&boxMons[GetLastViewedMonIndex()], mon);
    }
}

static enum Move GetMonMoveBySlotId(struct Pokemon *mon, u8 moveSlot)
{
    enum Move move;

    switch (moveSlot)
    {
    case 0:
        move = GetMonData(mon, MON_DATA_MOVE1);
        break;
    case 1:
        move = GetMonData(mon, MON_DATA_MOVE2);
        break;
    case 2:
        move = GetMonData(mon, MON_DATA_MOVE3);
        break;
    default:
        move = GetMonData(mon, MON_DATA_MOVE4);
    }

    return move;
}

static u16 GetMonPpByMoveSlot(struct Pokemon *mon, u8 moveSlot)
{
    u16 pp;

    switch (moveSlot)
    {
    case 0:
        pp = GetMonData(mon, MON_DATA_PP1);
        break;
    case 1:
        pp = GetMonData(mon, MON_DATA_PP2);
        break;
    case 2:
        pp = GetMonData(mon, MON_DATA_PP3);
        break;
    default:
        pp = GetMonData(mon, MON_DATA_PP4);
    }
    return pp;
}

static u8 StatusToAilment(u32 status)
{
    if (GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_HP) == 0)
        return AILMENT_FNT;

    if ((status & STATUS1_PSN_ANY) != 0)
        return AILMENT_PSN;

    if ((status & STATUS1_PARALYSIS) != 0)
        return AILMENT_PRZ;

    if ((status & STATUS1_SLEEP) != 0)
        return AILMENT_SLP;

    if ((status & STATUS1_FREEZE) != 0)
        return AILMENT_FRZ;

    if ((status & STATUS1_BURN) != 0)
        return AILMENT_BRN;

    if ((status & STATUS1_FROSTBITE) != 0)
        return AILMENT_FRB;

    if (ShouldPokemonShowActivePokerus(&sMonSummaryScreen->currentMon))
        return AILMENT_PKRS;

    return AILMENT_NONE;
}

static void Task_HandleInput_SelectMove(u8 taskId)
{
    u8 i;

    switch (sMonSummaryScreen->selectMoveInputHandlerState)
    {
    case 0:
        if (IsActiveOverworldLinkBusy() == TRUE || IsLinkRecvQueueAtOverworldMax() == TRUE)
            return;

        if (JOY_NEW(DPAD_UP))
        {
            if (sMoveSelectionCursorPos > 0)
            {
                sMonSummaryScreen->selectMoveInputHandlerState = 2;
                PlaySE(SE_SELECT);

                for (i = sMoveSelectionCursorPos; i > 0; i--)
                    if (sMonSummaryScreen->moveIds[i - 1] != 0)
                    {
                        PlaySE(SE_SELECT);
                        sMoveSelectionCursorPos = i - 1;
                        return;
                    }
            }
            else
            {
                sMoveSelectionCursorPos = 4;
                sMonSummaryScreen->selectMoveInputHandlerState = 2;
                PlaySE(SE_SELECT);

                if (sMonSummaryScreen->isSwappingMoves == TRUE)
                    for (i = sMoveSelectionCursorPos; i > 0; i--)
                        if (sMonSummaryScreen->moveIds[i - 1] != 0)
                        {
                            PlaySE(SE_SELECT);
                            sMoveSelectionCursorPos = i - 1;
                            return;
                        }
            }
        }
        else if (JOY_NEW(DPAD_DOWN))
        {
            if (sMoveSelectionCursorPos < MAX_MON_MOVES)
            {
                u8 selectionLimit = MAX_MON_MOVES;

                sMonSummaryScreen->selectMoveInputHandlerState = 2;

                if (sMonSummaryScreen->isSwappingMoves == TRUE)
                {
                    if (sMoveSelectionCursorPos == MAX_MON_MOVES - 1)
                    {
                        sMoveSelectionCursorPos = 0;
                        sMonSummaryScreen->selectMoveInputHandlerState = 2;
                        PlaySE(SE_SELECT);
                        return;
                    }
                    selectionLimit--;
                }

                for (i = sMoveSelectionCursorPos; i < selectionLimit; i++)
                    if (sMonSummaryScreen->moveIds[i + 1] != MOVE_NONE)
                    {
                        PlaySE(SE_SELECT);
                        sMoveSelectionCursorPos = i + 1;
                        return;
                    }

                if (!sMonSummaryScreen->isSwappingMoves)
                {
                    PlaySE(SE_SELECT);
                    sMoveSelectionCursorPos = i;
                }
                else
                {
                    PlaySE(SE_SELECT);
                    sMoveSelectionCursorPos = 0;
                }

                return;
            }
            else if (sMoveSelectionCursorPos == MAX_MON_MOVES)
            {
                sMoveSelectionCursorPos = 0;
                sMonSummaryScreen->selectMoveInputHandlerState = 2;
                PlaySE(SE_SELECT);
                return;
            }
        }
        else if (JOY_NEW(A_BUTTON))
        {
            PlaySE(SE_SELECT);
            if (sMoveSelectionCursorPos == MAX_MON_MOVES)
            {
                sMoveSelectionCursorPos = 0;
                sMoveSwapCursorPos = 0;
                sMonSummaryScreen->isSwappingMoves = FALSE;
                HideMonTypeIcons();
                ShoworHideMoveSelectionCursor(TRUE);
                sMonSummaryScreen->pageFlipDirection = 0;
                PokeSum_RemoveWindows();
                sMonSummaryScreen->curPageIndex--;
                sMonSummaryScreen->selectMoveInputHandlerState = 1;
                return;
            }

            if (sMonSummaryScreen->isSwappingMoves != TRUE)
            {
                if (sMonSummaryScreen->isEnemyParty == FALSE
                    && gMain.inBattle == 0
                    && gReceivedRemoteLinkPlayers == 0)
                {
                    sMoveSwapCursorPos = sMoveSelectionCursorPos;
                    sMonSummaryScreen->isSwappingMoves = TRUE;
                }
                return;
            }
            else
            {
                sMonSummaryScreen->isSwappingMoves = FALSE;

                if (sMoveSelectionCursorPos == sMoveSwapCursorPos)
                    return;

                if (sMonSummaryScreen->isBoxMon == 0)
                    SwapMonMoveSlots();
                else
                    SwapBoxMonMoveSlots();

                UpdateCurrentMonBufferFromPartyOrBox(&sMonSummaryScreen->currentMon);
                BufferMonMoves();
                sMonSummaryScreen->selectMoveInputHandlerState = 2;
                return;
            }
        }
        else if (JOY_NEW(B_BUTTON))
        {
            if (sMonSummaryScreen->isSwappingMoves == TRUE)
            {
                sMoveSwapCursorPos = sMoveSelectionCursorPos;
                sMonSummaryScreen->isSwappingMoves = FALSE;
                return;
            }

            if (sMoveSelectionCursorPos == 4)
            {
                sMoveSelectionCursorPos = 0;
                sMoveSwapCursorPos = 0;
            }

            DestroyCategoryIcon();
            HideMonTypeIcons();
            ShoworHideMoveSelectionCursor(TRUE);
            sMonSummaryScreen->pageFlipDirection = 0;
            PokeSum_RemoveWindows();
            sMonSummaryScreen->curPageIndex--;
            sMonSummaryScreen->selectMoveInputHandlerState = 1;
        }
        break;
    case 1:
        gTasks[sMonSummaryScreen->inputHandlerTaskId].func = Task_BackOutOfSelectMove;
        sMonSummaryScreen->selectMoveInputHandlerState = 0;
        break;
    case 2:
        PokeSum_PrintRightPaneText();
        PokeSum_PrintBottomPaneText();
        PokeSum_PrintAbilityDataOrMoveTypes();
        sMonSummaryScreen->selectMoveInputHandlerState = 3;
        break;
    case 3:
        if (IsActiveOverworldLinkBusy() == TRUE || IsLinkRecvQueueAtOverworldMax() == TRUE)
            return;

        CopyWindowToVram(sMonSummaryScreen->windowIds[POKESUM_WIN_RIGHT_PANE], 2);
        CopyWindowToVram(sMonSummaryScreen->windowIds[POKESUM_WIN_TRAINER_MEMO], 2);
        CopyWindowToVram(sMonSummaryScreen->windowIds[5], 2);
        CopyWindowToVram(sMonSummaryScreen->windowIds[6], 2);
        CopyBgTilemapBufferToVram(0);
        CopyBgTilemapBufferToVram(3);
        sMonSummaryScreen->selectMoveInputHandlerState = 0;
        break;
    default:
        break;
    }
}

static void SwapMonMoveSlots(void)
{
    struct Pokemon *partyMons;
    struct Pokemon *mon;

    u16 move1, move2;
    u8 pp1, pp2;
    u8 allMovesPPBonuses;
    u8 move1ppBonus, move2ppBonus;

    partyMons = sMonSummaryScreen->monList.mons;
    mon = &partyMons[GetLastViewedMonIndex()];

    move1 = GetMonData(mon, MON_DATA_MOVE1 + sMoveSelectionCursorPos);
    move2 = GetMonData(mon, MON_DATA_MOVE1 + sMoveSwapCursorPos);

    pp1 = GetMonData(mon, MON_DATA_PP1 + sMoveSelectionCursorPos);
    pp2 = GetMonData(mon, MON_DATA_PP1 + sMoveSwapCursorPos);

    allMovesPPBonuses = GetMonData(mon, MON_DATA_PP_BONUSES);

    move1ppBonus = (allMovesPPBonuses & gPPUpGetMask[sMoveSelectionCursorPos]) >> (sMoveSelectionCursorPos * 2);
    move2ppBonus = (allMovesPPBonuses & gPPUpGetMask[sMoveSwapCursorPos]) >> (sMoveSwapCursorPos * 2);

    allMovesPPBonuses &= ~gPPUpGetMask[sMoveSelectionCursorPos];
    allMovesPPBonuses &= ~gPPUpGetMask[sMoveSwapCursorPos];
    allMovesPPBonuses |= (move1ppBonus << (sMoveSwapCursorPos * 2)) + (move2ppBonus << (sMoveSelectionCursorPos * 2));

    SetMonData(mon, MON_DATA_MOVE1 + sMoveSelectionCursorPos, (u8 *)&move2);
    SetMonData(mon, MON_DATA_MOVE1 + sMoveSwapCursorPos, (u8 *)&move1);
    SetMonData(mon, MON_DATA_PP1 + sMoveSelectionCursorPos, &pp2);
    SetMonData(mon, MON_DATA_PP1 + sMoveSwapCursorPos, &pp1);
    SetMonData(mon, MON_DATA_PP_BONUSES, &allMovesPPBonuses);
}

static void SwapBoxMonMoveSlots(void)
{
    struct BoxPokemon *boxMons;
    struct BoxPokemon *boxMon;

    u16 move1, move2;
    u8 pp1, pp2;
    u8 allMovesPPBonuses;
    u8 move1ppBonus, move2ppBonus;

    boxMons = sMonSummaryScreen->monList.boxMons;
    boxMon = &boxMons[GetLastViewedMonIndex()];

    move1 = GetBoxMonData(boxMon, MON_DATA_MOVE1 + sMoveSelectionCursorPos);
    move2 = GetBoxMonData(boxMon, MON_DATA_MOVE1 + sMoveSwapCursorPos);

    pp1 = GetBoxMonData(boxMon, MON_DATA_PP1 + sMoveSelectionCursorPos);
    pp2 = GetBoxMonData(boxMon, MON_DATA_PP1 + sMoveSwapCursorPos);

    allMovesPPBonuses = GetBoxMonData(boxMon, MON_DATA_PP_BONUSES);

    move1ppBonus = (allMovesPPBonuses & gPPUpGetMask[sMoveSelectionCursorPos]) >> (sMoveSelectionCursorPos * 2);
    move2ppBonus = (allMovesPPBonuses & gPPUpGetMask[sMoveSwapCursorPos]) >> (sMoveSwapCursorPos * 2);

    allMovesPPBonuses &= ~gPPUpGetMask[sMoveSelectionCursorPos];
    allMovesPPBonuses &= ~gPPUpGetMask[sMoveSwapCursorPos];
    allMovesPPBonuses |= (move1ppBonus << (sMoveSwapCursorPos * 2)) + (move2ppBonus << (sMoveSelectionCursorPos * 2));

    SetBoxMonData(boxMon, MON_DATA_MOVE1 + sMoveSelectionCursorPos, (u8 *)&move2);
    SetBoxMonData(boxMon, MON_DATA_MOVE1 + sMoveSwapCursorPos, (u8 *)&move1);
    SetBoxMonData(boxMon, MON_DATA_PP1 + sMoveSelectionCursorPos, &pp2);
    SetBoxMonData(boxMon, MON_DATA_PP1 + sMoveSwapCursorPos, &pp1);
    SetBoxMonData(boxMon, MON_DATA_PP_BONUSES, &allMovesPPBonuses);
}

static void UpdateCurrentMonBufferFromPartyOrBox(struct Pokemon *mon)
{
    if (!sMonSummaryScreen->isBoxMon)
    {
        struct Pokemon *partyMons;
        partyMons = sMonSummaryScreen->monList.mons;
        *mon = partyMons[GetLastViewedMonIndex()];
    }
    else
    {
        struct BoxPokemon *boxMons;
        boxMons = sMonSummaryScreen->monList.boxMons;
        BoxMonToMon(&boxMons[GetLastViewedMonIndex()], mon);
    }
}

static u8 PokeSum_CanForgetSelectedMove(void)
{
    enum Move move = GetMonMoveBySlotId(&sMonSummaryScreen->currentMon, sMoveSelectionCursorPos);

    if (CannotForgetMove(move) == TRUE && sMonSummaryScreen->mode != PSS_MODE_FORGET_MOVE)
        return FALSE;

    return TRUE;
}

static void Task_InputHandler_SelectOrForgetMove(u8 taskId)
{
    u8 i;

    switch (sMonSummaryScreen->selectMoveInputHandlerState)
    {
    case 0:
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, 0);
        sMonSummaryScreen->selectMoveInputHandlerState++;
        break;
    case 1:
        if (!gPaletteFade.active)
        {
            PlayMonCry();
            sMonSummaryScreen->selectMoveInputHandlerState++;
        }
        break;
    case 2:
        if (JOY_NEW(DPAD_UP))
        {
            if (sMoveSelectionCursorPos > 0)
            {
                sMonSummaryScreen->selectMoveInputHandlerState = 3;
                PlaySE(SE_SELECT);
                for (i = sMoveSelectionCursorPos; i > 0; i--)
                    if (sMonSummaryScreen->moveIds[i - 1] != 0)
                    {
                        PlaySE(SE_SELECT);
                        sMoveSelectionCursorPos = i - 1;
                        return;
                    }
            }
            else
            {
                sMoveSelectionCursorPos = 4;
                sMonSummaryScreen->selectMoveInputHandlerState = 3;
                PlaySE(SE_SELECT);
                return;
            }
        }
        else if (JOY_NEW(DPAD_DOWN))
        {
            if (sMoveSelectionCursorPos < MAX_MON_MOVES)
            {
                u8 selectionLimit = MAX_MON_MOVES;

                sMonSummaryScreen->selectMoveInputHandlerState = 3;

                if (sMonSummaryScreen->isSwappingMoves == TRUE)
                    selectionLimit--;

                for (i = sMoveSelectionCursorPos; i < selectionLimit; i++)
                    if (sMonSummaryScreen->moveIds[i + 1] != 0)
                    {
                        PlaySE(SE_SELECT);
                        sMoveSelectionCursorPos = i + 1;
                        return;
                    }

                if (!sMonSummaryScreen->isSwappingMoves)
                {
                    PlaySE(SE_SELECT);
                    sMoveSelectionCursorPos = i;
                }

                return;
            }
            else if (sMoveSelectionCursorPos == MAX_MON_MOVES)
            {
                sMoveSelectionCursorPos = 0;
                sMonSummaryScreen->selectMoveInputHandlerState = 3;
                PlaySE(SE_SELECT);
                return;
            }
        }
        else if (JOY_NEW(A_BUTTON))
        {
            if (PokeSum_CanForgetSelectedMove() == TRUE || sMoveSelectionCursorPos == MAX_MON_MOVES)
            {
                PlaySE(SE_SELECT);
                sMoveSwapCursorPos = sMoveSelectionCursorPos;
                gSpecialVar_0x8005 = sMoveSwapCursorPos;
                sMonSummaryScreen->selectMoveInputHandlerState = 6;
            }
            else
            {
                PlaySE(SE_FAILURE);
                sMonSummaryScreen->selectMoveInputHandlerState = 5;
            }
        }
        else if (JOY_NEW(B_BUTTON))
        {
            sMoveSwapCursorPos = 4;
            gSpecialVar_0x8005 = (u16)sMoveSwapCursorPos;
            sMonSummaryScreen->selectMoveInputHandlerState = 6;
        }
        break;
    case 3:
        PokeSum_PrintRightPaneText();
        PokeSum_PrintBottomPaneText();
        PokeSum_PrintAbilityDataOrMoveTypes();
        sMonSummaryScreen->selectMoveInputHandlerState = 4;
        break;
    case 4:
        if (IsActiveOverworldLinkBusy() == TRUE)
            return;
        if (IsLinkRecvQueueAtOverworldMax() == TRUE)
            return;

        CopyWindowToVram(sMonSummaryScreen->windowIds[POKESUM_WIN_RIGHT_PANE], 2);
        CopyWindowToVram(sMonSummaryScreen->windowIds[POKESUM_WIN_TRAINER_MEMO], 2);
        CopyWindowToVram(sMonSummaryScreen->windowIds[5], 2);
        CopyWindowToVram(sMonSummaryScreen->windowIds[6], 2);
        CopyBgTilemapBufferToVram(0);
        CopyBgTilemapBufferToVram(3);
        sMonSummaryScreen->selectMoveInputHandlerState = 2;
        break;
    case 5:
        FillWindowPixelBuffer(sMonSummaryScreen->windowIds[POKESUM_WIN_TRAINER_MEMO], 0);
        AddTextPrinterParameterized4(sMonSummaryScreen->windowIds[POKESUM_WIN_TRAINER_MEMO], FONT_NORMAL,
                                     7, 42,
                                     0, 0,
                                     sLevelNickTextColors[0], TEXT_SKIP_DRAW,
                                     sText_PokeSum_HmMovesCantBeForgotten);
        HideCategoryIcon();
        CopyWindowToVram(sMonSummaryScreen->windowIds[POKESUM_WIN_TRAINER_MEMO], 2);
        CopyBgTilemapBufferToVram(0);
        CopyBgTilemapBufferToVram(3);
        sMonSummaryScreen->selectMoveInputHandlerState = 2;
        break;
    case 6:
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, 0);
        sMonSummaryScreen->selectMoveInputHandlerState++;
        break;
    default:
        if (!gPaletteFade.active)
            Task_DestroyResourcesOnExit(taskId);
        break;
    }
}

static void SpriteCB_PokeSum_MonPicSprite(struct Sprite *sprite)
{
    if (sMonSummaryScreen->numMonPicBounces >= 2)
        return;

    if (sMonPicBounceState->initDelay++ >= 2)
    {
        u8 arrayLen;

        switch (sMonPicBounceState->vigor)
        {
        case 0:
            sprite->y += sMonPicBounceYDelta_Under60[sMonPicBounceState->animFrame++];
            arrayLen = ARRAY_COUNT(sMonPicBounceYDelta_Under60);
            break;
        case 1:
            sprite->y += sMonPicBounceYDelta_60to80[sMonPicBounceState->animFrame++];
            arrayLen = ARRAY_COUNT(sMonPicBounceYDelta_60to80);
            break;
        case 2:
            sprite->y += sMonPicBounceYDelta_80to99[sMonPicBounceState->animFrame++];
            arrayLen = ARRAY_COUNT(sMonPicBounceYDelta_80to99);
            break;
        case 3:
        default:
            sprite->y += sMonPicBounceYDelta_Full[sMonPicBounceState->animFrame++];
            arrayLen = ARRAY_COUNT(sMonPicBounceYDelta_Full);
            break;
        }

        if (sMonPicBounceState->animFrame >= arrayLen)
        {
            sMonPicBounceState->animFrame = 0;
            sMonSummaryScreen->numMonPicBounces++;
        }

        sMonPicBounceState->initDelay = 0;
    }
}

static void SpriteCB_PokeSum_EggPicShake(struct Sprite *sprite)
{
    if (sMonSummaryScreen->numMonPicBounces >= 2)
        return;

    switch (sMonPicBounceState->vigor)
    {
    case 0:
    default:
        if (sMonPicBounceState->initDelay++ >= 120)
        {
            sprite->x += sEggPicShakeXDelta_ItWillTakeSomeTime[sMonPicBounceState->animFrame];
            if (++sMonPicBounceState->animFrame >= ARRAY_COUNT(sEggPicShakeXDelta_ItWillTakeSomeTime))
            {
                sMonPicBounceState->animFrame = 0;
                sMonPicBounceState->initDelay = 0;
                sMonSummaryScreen->numMonPicBounces++;
            }
        }
        break;
    case 1:
        if (sMonPicBounceState->initDelay++ >= 90)
        {
            sprite->x += sEggPicShakeXDelta_OccasionallyMoves[sMonPicBounceState->animFrame];
            if (++sMonPicBounceState->animFrame >= ARRAY_COUNT(sEggPicShakeXDelta_OccasionallyMoves))
            {
                sMonPicBounceState->animFrame = 0;
                sMonPicBounceState->initDelay = 0;
                sMonSummaryScreen->numMonPicBounces++;
            }
        }
        break;
    case 2:
        if (sMonPicBounceState->initDelay++ >= 60)
        {
            sprite->x += sEggPicShakeXDelta_AlmostReadyToHatch[sMonPicBounceState->animFrame];
            if (++sMonPicBounceState->animFrame >= ARRAY_COUNT(sEggPicShakeXDelta_AlmostReadyToHatch))
            {
                sMonPicBounceState->animFrame = 0;
                sMonPicBounceState->initDelay = 0;
                sMonSummaryScreen->numMonPicBounces++;
            }
        }
        break;
    }
}

static void SpriteCB_MonPicDummy(struct Sprite *sprite)
{
}

static void PokeSum_CreateMonPicSprite(void)
{
    u16 spriteId;
    enum Species species;
    u32 personality;
    bool32 isShiny;
    u16 palSlot = IndexOfSpritePaletteTag(TAG_MON_SPRITE);

    if (palSlot == 0xFF)
        palSlot = AllocSpritePalette(TAG_MON_SPRITE);

    sMonPicBounceState = AllocZeroed(sizeof(struct MonPicBounceState));

    species = GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_SPECIES_OR_EGG);
    personality = GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_PERSONALITY);
    isShiny = GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_IS_SHINY, NULL);

    spriteId = CreateMonFrontPicSprite(species, isShiny, personality, 60, 65, palSlot, TAG_NONE);
    FreeSpriteOamMatrix(&gSprites[spriteId]);

    if (!IsMonSpriteNotFlipped(species))
        gSprites[spriteId].hFlip = TRUE;
    else
        gSprites[spriteId].hFlip = FALSE;

    sMonSummaryScreen->monPicSpriteId = spriteId;

    PokeSum_ShowOrHideMonPicSprite(TRUE);
    SetMonPicSpriteCallback(spriteId);
}

static void SetEggPicSpriteCallback(struct Sprite *sprite)
{
    u8 friendship = GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_FRIENDSHIP);

    if (friendship <= 5)
    {
        sMonPicBounceState->vigor = 2;
    }
    else
    {
        if (friendship <= 10)
            sMonPicBounceState->vigor = 1;
        else if (friendship <= 40)
            sMonPicBounceState->vigor = 0;
    }

    sprite->callback = SpriteCB_PokeSum_EggPicShake;
}

static void SetDefaultMonPicSpriteCallback(struct Sprite *sprite)
{
    u16 curHp = GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_HP);
    u16 maxHp = GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_MAX_HP);

    if (curHp == maxHp)
        sMonPicBounceState->vigor = 3;
    else if (maxHp * 0.8 <= curHp)
        sMonPicBounceState->vigor = 2;
    else if (maxHp * 0.6 <= curHp)
        sMonPicBounceState->vigor = 1;
    else
        sMonPicBounceState->vigor = 0;

    sprite->callback = SpriteCB_PokeSum_MonPicSprite;
}

static void SetStatusMonPicSpriteCallback(struct Sprite *sprite)
{
    if (sMonSummaryScreen->curMonStatusAilment == AILMENT_FNT)
        return;

    sprite->callback = SpriteCB_MonPicDummy;
}

static void SetMonPicSpriteCallback(u16 spriteId)
{
    sMonSummaryScreen->numMonPicBounces = 0;

    if (sMonSummaryScreen->isEgg == TRUE)
    {
        SetEggPicSpriteCallback(&gSprites[spriteId]);
        return;
    }

    if (sMonSummaryScreen->curMonStatusAilment != AILMENT_NONE && sMonSummaryScreen->curMonStatusAilment != AILMENT_PKRS)
    {
        SetStatusMonPicSpriteCallback(&gSprites[spriteId]);
        return;
    }

    SetDefaultMonPicSpriteCallback(&gSprites[spriteId]);
}

static void PokeSum_ShowOrHideMonPicSprite(u8 invisible)
{
    gSprites[sMonSummaryScreen->monPicSpriteId].invisible = invisible;
}

static void PokeSum_DestroyMonPicSprite(void)
{
    FreeAndDestroyMonPicSprite(sMonSummaryScreen->monPicSpriteId);
    FREE_AND_SET_NULL(sMonPicBounceState);
}

static void CreateBallIconObj(void)
{
    enum PokeBall ball = ItemIdToBallId(GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_POKEBALL));

    LoadBallGfx(ball);
    sMonSummaryScreen->ballIconSpriteId = CreateSprite(&gPokeBalls[ball].spriteTemplate, 106, 88, 0);
    gSprites[sMonSummaryScreen->ballIconSpriteId].callback = SpriteCallbackDummy;
    gSprites[sMonSummaryScreen->ballIconSpriteId].oam.priority = 0;

    ShowOrHideBallIconObj(TRUE);
}

static void ShowOrHideBallIconObj(u8 invisible)
{
    gSprites[sMonSummaryScreen->ballIconSpriteId].invisible = invisible;
}

static void DestroyBallIconObj(void)
{
    DestroySpriteAndFreeResources(&gSprites[sMonSummaryScreen->ballIconSpriteId]);
}

static void PokeSum_CreateMonIconSprite(void)
{
    enum Species species;
    u32 personality;

    species = GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_SPECIES_OR_EGG);
    personality = GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_PERSONALITY);

    SafeLoadMonIconPalette(species);

    sMonSummaryScreen->monIconSpriteId = CreateMonIcon(species, SpriteCallbackDummy, 24, 32, 0, personality);

    if (!IsMonSpriteNotFlipped(species))
        gSprites[sMonSummaryScreen->monIconSpriteId].hFlip = TRUE;
    else
        gSprites[sMonSummaryScreen->monIconSpriteId].hFlip = FALSE;

    PokeSum_ShowOrHideMonIconSprite(TRUE);
}

static void PokeSum_ShowOrHideMonIconSprite(bool8 invisible)
{
    gSprites[sMonSummaryScreen->monIconSpriteId].invisible = invisible;
}

static void PokeSum_DestroyMonIconSprite(void)
{
    enum Species species = GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_SPECIES_OR_EGG);
    SafeFreeMonIconPalette(species);
    FreeAndDestroyMonIconSprite(&gSprites[sMonSummaryScreen->monIconSpriteId]);
}

static struct Sprite *CreateIconSprite(const u32 *tiles, u32 size, const u16 *palData, const struct OamData *oam, const union AnimCmd *const *anims, u16 tileTag, u16 palTag, s16 x, s16 y)
{
    u16 spriteId;
    void *buffer = AllocZeroed(size);

    DecompressDataWithHeaderWram(tiles, buffer);

    struct SpriteSheet sheet = { .data = buffer, .size = size, .tag = tileTag };
    struct SpritePalette palette = { .data = palData, .tag = palTag };
    struct SpriteTemplate template =
    {
        .tileTag = tileTag,
        .paletteTag = palTag,
        .oam = oam,
        .anims = anims,
    };

    LoadSpriteSheet(&sheet);
    LoadSpritePalette(&palette);

    spriteId = CreateSprite(&template, 114, 92, 0);

    TRY_FREE_AND_SET_NULL(buffer);

    return &gSprites[spriteId];
}

static void CreateMoveSelectionCursorSprites(u16 tileTag, u16 palTag)
{
    u8 i;
    u8 spriteId;
    void *gfxBufferPtrs[2];
    gfxBufferPtrs[0] = AllocZeroed(0x20 * 64);
    gfxBufferPtrs[1] = AllocZeroed(0x20 * 64);

    for (u32 i = 0; i < ARRAY_COUNT(sMoveSelectionCursorObjs); i++)
        sMoveSelectionCursorObjs[i] = NULL;

    DecompressDataWithHeaderWram(sMoveSelectionCursorTiles_Left, gfxBufferPtrs[0]);
    DecompressDataWithHeaderWram(sMoveSelectionCursorTiles_Right, gfxBufferPtrs[1]);

    for (i = 0; i < 4; i++)
    {
        struct SpriteSheet sheet = {
            .data = gfxBufferPtrs[i % 2],
            .size = 0x20 * 64,
            .tag = tileTag + i
        };

        struct SpritePalette palette = {.data = sMoveSelectionCursorPals, .tag = palTag};
        struct SpriteTemplate template = {
            .tileTag = tileTag + i,
            .paletteTag = palTag,
            .oam = &sMoveSelectionCursorOamData,
            .anims = sMoveSelectionCursorOamAnimTable,
            .callback = SpriteCB_MoveSelectionCursor,
        };

        LoadSpriteSheet(&sheet);
        LoadSpritePalette(&palette);

        spriteId = CreateSprite(&template, 64 * (i % 2) + 152, sMoveSelectionCursorPos * 28 + 34, i % 2);
        sMoveSelectionCursorObjs[i] = &gSprites[spriteId];
        sMoveSelectionCursorObjs[i]->subpriority = i;

        if (i > 1)
            StartSpriteAnim(sMoveSelectionCursorObjs[i], 1);
    }

    ShoworHideMoveSelectionCursor(TRUE);

    TRY_FREE_AND_SET_NULL(gfxBufferPtrs[0]);
    TRY_FREE_AND_SET_NULL(gfxBufferPtrs[1]);
}

static void ShoworHideMoveSelectionCursor(bool8 invisible)
{
    u8 i;
    for (i = 0; i < 4; i++)
        sMoveSelectionCursorObjs[i]->invisible = invisible;
}

static void SpriteCB_MoveSelectionCursor(struct Sprite *sprite)
{
    u8 i;

    for (i = 0; i < 4; i++)
    {
        if (sMonSummaryScreen->isSwappingMoves == TRUE && i > 1)
            continue;

        sMoveSelectionCursorObjs[i]->y = sMoveSelectionCursorPos * 28 + 34;
    }

    if (sMonSummaryScreen->isSwappingMoves != TRUE)
    {
        if (sMonSummaryScreen->curPageIndex == PSS_PAGE_MOVES_INFO)
        {
            sMoveSelectionCursorObjs[0]->invisible = FALSE;
            sMoveSelectionCursorObjs[1]->invisible = FALSE;
        }
        return;
    }

    for (i = 0; i < 2; i++)
    {
        sprite = sMoveSelectionCursorObjs[i];
        sprite->data[0]++;

        if (sprite->invisible)
        {
            if (sprite->data[0] > 60)
            {
                sprite->invisible = FALSE;
                sprite->data[0] = 0;
            }
        }
        else if (sprite->data[0] > 60)
        {
            sprite->invisible = TRUE;
            sprite->data[0] = 0;
        }
    }
}

static void DestroyMoveSelectionCursorObjs(void)
{
    u8 i;

    for (i = 0; i < 4; i++)
    {
        if (sMoveSelectionCursorObjs[i] != NULL)
        {
            DestroySpriteAndFreeResources(sMoveSelectionCursorObjs[i]);
            sMoveSelectionCursorObjs[i] = NULL;
        }
    }
}

static void CreateStatusIconSprite(u16 tileTag, u16 palTag)
{
    sStatusIcon = CreateIconSprite(gSummaryScreen_StatusAilmentIcon_Gfx, 0x20 * 32, gSummaryScreen_StatusAilmentIcon_Pal, &sStatusAilmentIconOamData, sStatusAilmentIconAnimTable, tileTag, palTag, 0, 0);
    ShowOrHideStatusIcon(TRUE);
    UpdateMonStatusIconObj();
}

static void DestroyMonStatusIconObj(void)
{
    if (sStatusIcon == NULL)
        return;

    DestroySpriteAndFreeResources(sStatusIcon);
    sStatusIcon = NULL;
}

static void UpdateMonStatusIconObj(void)
{
    sMonSummaryScreen->curMonStatusAilment = StatusToAilment(GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_STATUS));

    if (sMonSummaryScreen->curMonStatusAilment == AILMENT_NONE)
    {
        ShowOrHideStatusIcon(TRUE);
        return;
    }

    StartSpriteAnim(sStatusIcon, sMonSummaryScreen->curMonStatusAilment - 1);
    ShowOrHideStatusIcon(FALSE);
}

static void ShowOrHideStatusIcon(u8 invisible)
{
    if (sMonSummaryScreen->curMonStatusAilment == AILMENT_NONE || sMonSummaryScreen->isEgg)
        sStatusIcon->invisible = TRUE;
    else
        sStatusIcon->invisible = invisible;

    if (sMonSummaryScreen->curPageIndex == PSS_PAGE_MOVES_INFO)
    {
        if (sStatusIcon->y != 45)
        {
            sStatusIcon->x = 16;
            sStatusIcon->y = 45;
            return;
        }
    }
    else if (sStatusIcon->y != 38)
    {
        sStatusIcon->x = 16;
        sStatusIcon->y = 38;
        return;
    }
}

static void CreateHpBarObjs(u16 tileTag, u16 palTag)
{
    u8 i;
    u8 spriteId;
    void *gfxBufferPtr;
    u32 curHp;
    u32 maxHp;
    u8 hpBarPalTagOffset = 0;

    sHpBarObjs = AllocZeroed(sizeof(struct HpBarObjs));
    gfxBufferPtr = AllocZeroed(0x20 * 12);
    DecompressDataWithHeaderWram(gSummaryScreen_HpBar_Gfx, gfxBufferPtr);

    curHp = GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_HP);
    maxHp = GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_MAX_HP);

    if (maxHp / 4 > curHp)
        hpBarPalTagOffset = 2;
    else if (maxHp / 2 > curHp)
        hpBarPalTagOffset = 1;

    if (gfxBufferPtr != NULL)
    {
        struct SpriteSheet sheet = {
            .data = gfxBufferPtr,
            .size = 0x20 * 12,
            .tag = tileTag
        };

        struct SpritePalette palette1 = {.data = sHpBarPals[0], .tag = palTag};
        struct SpritePalette palette2 = {.data = sHpBarPals[1], .tag = palTag + 1};
        struct SpritePalette palette3 = {.data = sHpBarPals[2], .tag = palTag + 2};

        LoadSpriteSheet(&sheet);
        LoadSpritePalette(&palette1);
        LoadSpritePalette(&palette2);
        LoadSpritePalette(&palette3);
    }

    for (i = 0; i < 9; i++)
    {
        struct SpriteTemplate template = {
            .tileTag = tileTag,
            .paletteTag = palTag + hpBarPalTagOffset,
            .oam = &sHpOrExpBarOamData,
            .anims = sHpOrExpBarAnimTable,
            .images = NULL,
            .affineAnims = gDummySpriteAffineAnimTable,
            .callback = SpriteCallbackDummy,
        };

        sHpBarObjs->xpos[i] = i * 8 + 172;
        spriteId = CreateSprite(&template, sHpBarObjs->xpos[i], 36, 0);
        sHpBarObjs->sprites[i] = &gSprites[spriteId];
        sHpBarObjs->sprites[i]->invisible = FALSE;
        sHpBarObjs->sprites[i]->oam.priority = 2;
        StartSpriteAnim(sHpBarObjs->sprites[i], 8);
    }

    UpdateHpBarObjs();
    ShowOrHideHpBarObjs(TRUE);

    TRY_FREE_AND_SET_NULL(gfxBufferPtr);
}

static void UpdateHpBarObjs(void)
{
    u8 numWholeHpBarTiles = 0;
    u8 i;
    u8 animNum;
    u8 two = 2;
    u8 hpBarPalOffset = 0;
    u32 curHp;
    u32 maxHp;
    s64 pointsPerTile;
    s64 totalPoints;

    if (sMonSummaryScreen->isEgg)
        return;

    curHp = GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_HP);
    maxHp = GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_MAX_HP);

    if (maxHp / 5 >= curHp)
        hpBarPalOffset = 2;
    else if (maxHp / 2 >= curHp)
        hpBarPalOffset = 1;

    switch (GetHPBarLevel(curHp, maxHp))
    {
    case 3:
    default:
        hpBarPalOffset = 0;
        break;
    case 2:
        hpBarPalOffset = 1;
        break;
    case 1:
        hpBarPalOffset = 2;
        break;
    }

    for (i = 0; i < 9; i++)
        sHpBarObjs->sprites[i]->oam.paletteNum = IndexOfSpritePaletteTag(TAG_PSS_HP_BAR_0) + hpBarPalOffset;

    if (curHp == maxHp)
        for (i = two; i < 8; i++)
            StartSpriteAnim(sHpBarObjs->sprites[i], 8);

    else
    {
        pointsPerTile = (maxHp << 2) / 6;
        totalPoints = (curHp << 2);

        while (TRUE)
        {
            if (totalPoints <= pointsPerTile)
                break;
            totalPoints -= pointsPerTile;
            numWholeHpBarTiles++;
        }

        numWholeHpBarTiles += two;

        for (i = two; i < numWholeHpBarTiles; i++)
            StartSpriteAnim(sHpBarObjs->sprites[i], 8);

        animNum = (totalPoints * 6) / pointsPerTile;
        StartSpriteAnim(sHpBarObjs->sprites[numWholeHpBarTiles], animNum);

        for (i = numWholeHpBarTiles + 1; i < 8; i++)
            StartSpriteAnim(sHpBarObjs->sprites[i], 0);
    }

    StartSpriteAnim(sHpBarObjs->sprites[0], 9);
    StartSpriteAnim(sHpBarObjs->sprites[1], 10);
    StartSpriteAnim(sHpBarObjs->sprites[8], 11);
}

static void DestroyHpBarObjs(void)
{
    u8 i;

    for (i = 0; i < 9; i++)
        if (sHpBarObjs->sprites[i] != NULL)
            DestroySpriteAndFreeResources(sHpBarObjs->sprites[i]);

    TRY_FREE_AND_SET_NULL(sHpBarObjs);
}

static void ShowOrHideHpBarObjs(u8 invisible)
{
    u8 i;

    for (i = 0; i < 9; i++)
        sHpBarObjs->sprites[i]->invisible = invisible;
}

static void CreateExpBarObjs(u16 tileTag, u16 palTag)
{
    u8 i;
    u8 spriteId;
    void *gfxBufferPtr;

    sExpBarObjs = AllocZeroed(sizeof(struct ExpBarObjs));
    gfxBufferPtr = AllocZeroed(0x20 * 12);

    DecompressDataWithHeaderWram(gSummaryScreen_ExpBar_Gfx, gfxBufferPtr);
    if (gfxBufferPtr != NULL)
    {
        struct SpriteSheet sheet = {
            .data = gfxBufferPtr,
            .size = 0x20 * 12,
            .tag = tileTag
        };

        struct SpritePalette palette = {.data = gSummaryScreen_HpExpBar_Pal, .tag = palTag};
        LoadSpriteSheet(&sheet);
        LoadSpritePalette(&palette);
    }

    for (i = 0; i < 11; i++)
    {
        struct SpriteTemplate template = {
            .tileTag = tileTag,
            .paletteTag = palTag,
            .oam = &sHpOrExpBarOamData,
            .anims = sHpOrExpBarAnimTable,
        };

        sExpBarObjs->xpos[i] = i * 8 + 156;
        spriteId = CreateSprite(&template, sExpBarObjs->xpos[i], 132, 0);
        sExpBarObjs->sprites[i] = &gSprites[spriteId];
        sExpBarObjs->sprites[i]->oam.priority = 2;
    }

    UpdateExpBarObjs();
    ShowOrHideExpBarObjs(TRUE);

    TRY_FREE_AND_SET_NULL(gfxBufferPtr);
}

static void UpdateExpBarObjs(void)
{
    u8 numWholeExpBarTiles = 0;
    u8 i;
    u8 level = 0;
    u32 exp;
    u32 totalExpToNextLevel;
    u32 curExpToNextLevel;
    enum Species species;
    s64 pointsPerTile;
    s64 totalPoints;
    u8 animNum;
    u8 two = 2;

    if (sMonSummaryScreen->isEgg)
        return;

    exp = GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_EXP);
    species = GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_SPECIES);
    while(level < MAX_LEVEL && exp >= gExperienceTables[gSpeciesInfo[species].growthRate][level + 1])
        level++;

    if (level < MAX_LEVEL)
    {
        totalExpToNextLevel = gExperienceTables[gSpeciesInfo[species].growthRate][level + 1] - gExperienceTables[gSpeciesInfo[species].growthRate][level];
        curExpToNextLevel = exp - gExperienceTables[gSpeciesInfo[species].growthRate][level];
        pointsPerTile = ((totalExpToNextLevel << 2) / 8);
        totalPoints = (curExpToNextLevel << 2);

        while (TRUE)
        {
            if (totalPoints <= pointsPerTile)
                break;
            totalPoints -= pointsPerTile;
            numWholeExpBarTiles++;
        }

        numWholeExpBarTiles += two;

        for (i = two; i < numWholeExpBarTiles; i++)
            StartSpriteAnim(sExpBarObjs->sprites[i], 8);

        if (numWholeExpBarTiles >= 10)
        {
            if (totalExpToNextLevel == curExpToNextLevel)
                return;
            else
                StartSpriteAnim(sExpBarObjs->sprites[9], 7);
        }

        animNum = (totalPoints * 8) / pointsPerTile;
        StartSpriteAnim(sExpBarObjs->sprites[numWholeExpBarTiles], animNum);

        for (i = numWholeExpBarTiles + 1; i < 10; i++)
            StartSpriteAnim(sExpBarObjs->sprites[i], 0);
    }
    else
        for (i = two; i < 10; i++)
            StartSpriteAnim(sExpBarObjs->sprites[i], 0);

    StartSpriteAnim(sExpBarObjs->sprites[0], 9);
    StartSpriteAnim(sExpBarObjs->sprites[1], 10);
    StartSpriteAnim(sExpBarObjs->sprites[10], 11);
}

static void DestroyExpBarObjs(void)
{
    u8 i;

    for (i = 0; i < 11; i++)
        if (sExpBarObjs->sprites[i] != NULL)
            DestroySpriteAndFreeResources(sExpBarObjs->sprites[i]);

    TRY_FREE_AND_SET_NULL(sExpBarObjs);
}

static void ShowOrHideExpBarObjs(u8 invisible)
{
    u8 i;

    for (i = 0; i < 11; i++)
        sExpBarObjs->sprites[i]->invisible = invisible;
}

static void CreatePokerusIconSprite(u16 tileTag, u16 palTag)
{
    sPokerusIconObj = CreateIconSprite(sPokerusIconObjTiles, 0x20 * 1, sPokerusIconObjPal, &sPokerusIconObjOamData, sPokerusIconObjAnimTable, tileTag, palTag, 114, 92);
    HideShowPokerusIcon(TRUE);
    ShowPokerusIconObjIfHasOrHadPokerus();
}

static void DestroyPokerusIconObj(void)
{
    if (sPokerusIconObj == NULL)
        return;

    DestroySpriteAndFreeResources(sPokerusIconObj);
    sPokerusIconObj = NULL;
}

static void ShowPokerusIconObjIfHasOrHadPokerus(void)
{
    if (!ShouldPokemonShowActivePokerus(&sMonSummaryScreen->currentMon)
        && CheckMonHasHadPokerus(&sMonSummaryScreen->currentMon))
        HideShowPokerusIcon(FALSE);
    else
        HideShowPokerusIcon(TRUE);
}

static void HideShowPokerusIcon(bool8 invisible)
{
    if (!ShouldPokemonShowActivePokerus(&sMonSummaryScreen->currentMon)
        && CheckMonHasHadPokerus(&sMonSummaryScreen->currentMon))
    {
        sPokerusIconObj->invisible = invisible;
        return;
    }
    else
        sPokerusIconObj->invisible = TRUE;

    if (sMonSummaryScreen->curPageIndex == PSS_PAGE_MOVES_INFO)
    {
        sPokerusIconObj->invisible = TRUE;
        sPokerusIconObj->x = 16;
        sPokerusIconObj->y = 44;
    }
    else
    {
        sPokerusIconObj->x = 114;
        sPokerusIconObj->y = 92;
    }
}

static void CreateShinyIconSprite(u16 tileTag, u16 palTag)
{
    sShinyStarObjData = CreateIconSprite(sStarObjTiles, 0x20 * 2, sStarObjPal, &sStarObjOamData, sStarObjAnimTable, tileTag, palTag, 106, 40);
    HideShowShinyStar(TRUE);
    ShowShinyStarObjIfMonShiny();
}

static void DestroyShinyStarObj(void)
{
    if (sShinyStarObjData == NULL)
        return;

    DestroySpriteAndFreeResources(sShinyStarObjData);
    sShinyStarObjData = NULL;
}

static void HideShowShinyStar(bool8 invisible)
{
    if (IsMonShiny(&sMonSummaryScreen->currentMon) == TRUE
        && !sMonSummaryScreen->isEgg)
        sShinyStarObjData->invisible = invisible;
    else
        sShinyStarObjData->invisible = TRUE;

    if (sMonSummaryScreen->curPageIndex == PSS_PAGE_MOVES_INFO)
    {
        sShinyStarObjData->x = 8;
        sShinyStarObjData->y = 24;
    }
    else
    {
        sShinyStarObjData->x = 106;
        sShinyStarObjData->y = 40;
    }
}

static void ShowShinyStarObjIfMonShiny(void)
{
    if (IsMonShiny(&sMonSummaryScreen->currentMon) == TRUE && !sMonSummaryScreen->isEgg)
        HideShowShinyStar(FALSE);
    else
        HideShowShinyStar(TRUE);
}

static void PokeSum_DestroySprites(void)
{
    DestroyMoveSelectionCursorObjs();
    DestroyHpBarObjs();
    DestroyExpBarObjs();
    PokeSum_DestroyMonPicSprite();
    PokeSum_DestroyMonIconSprite();
    DestroyBallIconObj();
    PokeSum_DestroyMonMarkingsSprite();
    DestroyMonStatusIconObj();
    DestroyPokerusIconObj();
    DestroyShinyStarObj();
    DestroyTypeIconSprites();
    ResetSpriteData();
}

static void PokeSum_CreateSprites(void)
{
    CreateBallIconObj();
    ShowOrHideBallIconObj(FALSE);
    PokeSum_CreateMonIconSprite();
    PokeSum_CreateMonPicSprite();
    PokeSum_ShowOrHideMonPicSprite(FALSE);
    UpdateHpBarObjs();
    UpdateExpBarObjs();
    PokeSum_UpdateMonMarkingsAnim();
    UpdateMonStatusIconObj();
    ShowPokerusIconObjIfHasOrHadPokerus();
    ShowShinyStarObjIfMonShiny();
}

static void PokeSum_CreateMonMarkingsSprite(void)
{
    u32 markings = GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_MARKINGS);

    DestroySpriteAndFreeResources(sMonSummaryScreen->markingSprite);
    sMonSummaryScreen->markingSprite = CreateMonMarkingAllCombosSprite(TAG_PSS_MON_MARKINGS, TAG_PSS_MON_MARKINGS, sMonMarkingSpritePalette);

    if (sMonSummaryScreen->markingSprite != NULL)
    {
        StartSpriteAnim(sMonSummaryScreen->markingSprite, markings);
        sMonSummaryScreen->markingSprite->x = 20;
        sMonSummaryScreen->markingSprite->y = 91;
    }

    PokeSum_ShowOrHideMonMarkingsSprite(TRUE);
}

static void PokeSum_DestroyMonMarkingsSprite(void)
{
    DestroySpriteAndFreeResources(sMonSummaryScreen->markingSprite);
}

static void PokeSum_ShowOrHideMonMarkingsSprite(u8 invisible)
{
    u32 markings = GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_MARKINGS);

    if (markings == 0)
        sMonSummaryScreen->markingSprite->invisible = TRUE;
    else
        sMonSummaryScreen->markingSprite->invisible = invisible;
}

static void PokeSum_UpdateMonMarkingsAnim(void)
{
    u32 markings = GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_MARKINGS);

    StartSpriteAnim(sMonSummaryScreen->markingSprite, markings);
    PokeSum_ShowOrHideMonMarkingsSprite(FALSE);
}

static void ChangeSummaryPokemon(u8 taskId, s8 direction)
{
    s8 scrollResult = -1;

    if (sMonSummaryScreen->isBoxMon == TRUE)
    {
        if (sMonSummaryScreen->curPageIndex != PSS_PAGE_INFO)
        {
            if (direction == 1)
                direction = 0;
            else
                direction = 2;
        }
        else
        {
            // Allow Eggs
            if (direction == 1)
                direction = 1;
            else
                direction = 3;
        }

        scrollResult = AdvanceStorageMonIndex(sMonSummaryScreen->monList.boxMons, GetLastViewedMonIndex(), sMonSummaryScreen->maxMonIndex, (u8)direction);
    }
    else if (IsMultiBattle() == TRUE)
    {
        scrollResult = AdvanceMultiBattleMonIndex(direction);
    }
    else
    {
        scrollResult = AdvanceMonIndex(direction);
    }

    if (scrollResult == -1)
        return;

    gLastViewedMonIndex = scrollResult;
    CreateTask(Task_PokeSum_SwitchDisplayedPokemon, 0);
    sMonSummaryScreen->switchMonTaskState = 0;
}

static s8 AdvanceMonIndex(s8 delta)
{
    u8 index = gLastViewedMonIndex;
    struct Pokemon *partyMons = sMonSummaryScreen->monList.mons;

    if (sMonSummaryScreen->curPageIndex == PSS_PAGE_INFO)
    {
        if (delta == -1 && index == 0)
            return -1;
        if (delta == 1 && index >= sMonSummaryScreen->maxMonIndex)
            return -1;

        return index + delta;
    }

    while (TRUE)
    {
        index += delta;

        if (0 > index || index > sMonSummaryScreen->maxMonIndex)
            return -1;

        if (GetMonData(&partyMons[index], MON_DATA_IS_EGG) == 0)
            return index;
    }

    return -1;
}

static u8 IsValidToViewInMulti(struct Pokemon *partyMons)
{
    if (GetMonData(partyMons, MON_DATA_SPECIES) == SPECIES_NONE)
        return FALSE;
    if (sMonSummaryScreen->curPageIndex == PSS_PAGE_INFO && GetMonData(partyMons, MON_DATA_IS_EGG))
        return FALSE;

    return TRUE;
}

static s8 AdvanceMultiBattleMonIndex(s8 direction)
{
    s8 index, arrID = 0;
    u8 i;

    for (i = 0; i < PARTY_SIZE; i++)
        if (sMultiBattleOrder[i] == GetLastViewedMonIndex())
        {
            arrID = i;
            break;
        }

    while (TRUE)
    {
        arrID += direction;

        if (arrID < 0 || arrID >= PARTY_SIZE)
            return -1;

        index = sMultiBattleOrder[arrID];
        if (IsValidToViewInMulti(&gPlayerParty[index]) == TRUE)
            return index;
    }
}

static void Task_PokeSum_SwitchDisplayedPokemon(u8 taskId)
{
    switch (sMonSummaryScreen->switchMonTaskState)
    {
    case 0:
        StopCryAndClearCrySongs();
        sMoveSelectionCursorPos = 0;
        sMoveSwapCursorPos = 0;
        sMonSummaryScreen->switchMonTaskState++;
        break;
    case 1:
        PokeSum_DestroyMonPicSprite();
        PokeSum_DestroyMonIconSprite();
        DestroyBallIconObj();
        sMonSummaryScreen->switchMonTaskState++;
        break;
    case 2:
        BufferSelectedMonData(&sMonSummaryScreen->currentMon);

        sMonSummaryScreen->isEgg = GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_IS_EGG);
        sMonSummaryScreen->isBadEgg = GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_SANITY_IS_BAD_EGG);

        if (sMonSummaryScreen->isBadEgg == TRUE)
            sMonSummaryScreen->isEgg = TRUE;

        sMonSummaryScreen->switchMonTaskState++;
        break;
    case 3:
        FillBgTilemapBufferRect_Palette0(0, 0, 0, 0, 30, 20);

        if (IsMonShiny(&sMonSummaryScreen->currentMon) == TRUE && !sMonSummaryScreen->isEgg)
        {
            LoadPalette(&gSummaryScreen_Bg_Pal[16 * 6], BG_PLTT_ID(0), PLTT_SIZE_4BPP);
            LoadPalette(&gSummaryScreen_Bg_Pal[16 * 5], BG_PLTT_ID(1), PLTT_SIZE_4BPP);
        }
        else
        {
            LoadPalette(&gSummaryScreen_Bg_Pal[16 * 0], BG_PLTT_ID(0), PLTT_SIZE_4BPP);
            LoadPalette(&gSummaryScreen_Bg_Pal[16 * 1], BG_PLTT_ID(1), PLTT_SIZE_4BPP);
        }

        sMonSummaryScreen->switchMonTaskState++;
        break;
    case 4:
        if (sMonSummaryScreen->curPageIndex == PSS_PAGE_INFO)
        {
            if (sMonSummaryScreen->isEgg)
            {
                CopyToBgTilemapBuffer(sMonSummaryScreen->skillsPageBgNum, gSummaryScreen_PageEgg_Tilemap, 0, 0);
                CopyToBgTilemapBuffer(sMonSummaryScreen->infoAndMovesPageBgNum, gSummaryScreen_PageSkills_Tilemap, 0, 0);
            }
            else
            {
                CopyToBgTilemapBuffer(sMonSummaryScreen->skillsPageBgNum, gSummaryScreen_PageInfo_Tilemap, 0, 0);
                CopyToBgTilemapBuffer(sMonSummaryScreen->infoAndMovesPageBgNum, gSummaryScreen_PageSkills_Tilemap, 0, 0);
            }
        }
        sMonSummaryScreen->switchMonTaskState++;
        break;
    case 5:
        BufferMonInfo();
        sMonSummaryScreen->switchMonTaskState++;
        break;
    case 6:
        if (!sMonSummaryScreen->isEgg)
            BufferMonSkills();

        sMonSummaryScreen->switchMonTaskState++;
        break;
    case 7:
        if (!sMonSummaryScreen->isEgg)
            BufferMonMoves();

        sMonSummaryScreen->switchMonTaskState++;
        break;
    case 8:
        PokeSum_PrintRightPaneText();
        PokeSum_PrintBottomPaneText();
        PokeSum_PrintAbilityDataOrMoveTypes();
        sMonSummaryScreen->switchMonTaskState++;
        break;
    case 9:
        PokeSum_PrintMonTypeIcons();
        PokeSum_DrawPageProgressTiles();
        PokeSum_PrintPageHeaderText(sMonSummaryScreen->curPageIndex);
        sMonSummaryScreen->switchMonTaskState++;
        break;
    case 10:
        CopyWindowToVram(sMonSummaryScreen->windowIds[PSS_WIN_PAGE_NAME], 2);
        CopyWindowToVram(sMonSummaryScreen->windowIds[PSS_WIN_CONTROLS], 2);
        CopyWindowToVram(sMonSummaryScreen->windowIds[PSS_WIN_LVL_NICK], 2);
        CopyWindowToVram(sMonSummaryScreen->windowIds[6], 2);
        CopyWindowToVram(sMonSummaryScreen->windowIds[POKESUM_WIN_RIGHT_PANE], 2);
        CopyWindowToVram(sMonSummaryScreen->windowIds[POKESUM_WIN_TRAINER_MEMO], 2);
        CopyWindowToVram(sMonSummaryScreen->windowIds[5], 2);
        CopyBgTilemapBufferToVram(0);
        CopyBgTilemapBufferToVram(2);
        CopyBgTilemapBufferToVram(3);
        sMonSummaryScreen->switchMonTaskState++;
        break;
    case 11:
        if (!Overworld_IsRecvQueueAtMax() && !IsLinkRecvQueueAtOverworldMax())
        {
            PokeSum_CreateSprites();
            PlayMonCry();
            sMonSummaryScreen->switchMonTaskState++;
        }
        break;
    default:
        sMonSummaryScreen->switchMonTaskState = 0;
        DestroyTask(taskId);
        break;
    }
}

static void PokeSum_UpdateWin1ActiveFlag(u8 curPageIndex)
{
    ClearGpuRegBits(REG_OFFSET_DISPCNT, DISPCNT_WIN1_ON);
    return;

    switch (curPageIndex)
    {
    case PSS_PAGE_INFO:
    case PSS_PAGE_SKILLS:
        SetGpuReg(REG_OFFSET_DISPCNT, GetGpuReg(REG_OFFSET_DISPCNT) | DISPCNT_WIN1_ON);
        break;
    case PSS_PAGE_MOVES:
        if (!P_USE_TYPE_ICON_SPRITES)
            SetGpuReg(REG_OFFSET_DISPCNT, GetGpuReg(REG_OFFSET_DISPCNT) | DISPCNT_WIN1_ON);
        break;
    default:
        break;
    }
}

static void PlayMonCry(void)
{
    if (!GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_IS_EGG))
    {
        if (ShouldPlayNormalMonCry(&sMonSummaryScreen->currentMon) == TRUE)
            PlayCry_ByMode(GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_SPECIES_OR_EGG), 0, CRY_MODE_NORMAL);
        else
            PlayCry_ByMode(GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_SPECIES_OR_EGG), 0, CRY_MODE_WEAK);
    }
}

static bool32 CurrentMonIsFromGBA(void)
{
    enum GameVersion version = GetMonData(&sMonSummaryScreen->currentMon, MON_DATA_MET_GAME);

    switch (version)
    {
        case VERSION_LEAF_GREEN:
        case VERSION_FIRE_RED:
        case VERSION_RUBY:
        case VERSION_SAPPHIRE:
        case VERSION_EMERALD:
            return TRUE;
        default:
            return FALSE;
    }
}

static bool32 MapSecIsInKantoOrSevii(u8 mapSec)
{
    return mapSec >= KANTO_MAPSEC_START && mapSec <= MAPSEC_SPECIAL_AREA;
}

static void CreateTypeIconSprites(void)
{
    if (!P_USE_TYPE_ICON_SPRITES)
        return;

    InitTypeIconGfx();

    for (u32 i = 0; i < MAX_MON_MOVES + 1; i++)
        sMonSummaryScreen->moveTypeIconSpriteIds[i] = CreateTypeIconSprite();

    for (u32 i = 0; i < sizeof(sMonSummaryScreen->monTypeIconSpriteIds); i++)
        sMonSummaryScreen->monTypeIconSpriteIds[i] = CreateTypeIconSprite();
}
static void UpdateMoveTypeIconSprites(void)
{
    if (!P_USE_TYPE_ICON_SPRITES)
        return;

    for (u32 i = 0; i < MAX_MON_MOVES + 1; i++)
    {
        struct Sprite *icon = &gSprites[sMonSummaryScreen->moveTypeIconSpriteIds[i]];
        enum Type type = sMonSummaryScreen->moveTypes[i];

        if (sMonSummaryScreen->moveIds[i] == MOVE_NONE)
            icon->invisible = TRUE;
        else
            ShowTypeIcon(icon, type, 139, 27 + (28 * i));
    }
}

static void UpdateMonTypeIconSprites(bool32 isInfoPage)
{
    struct Sprite *icon1, *icon2, *icon3;
    enum Type type1, type2, teraType;
    s32 x1, x2, y;
    bool32 showTera;

    if (!P_USE_TYPE_ICON_SPRITES)
        return;

    icon1 = &gSprites[sMonSummaryScreen->monTypeIconSpriteIds[0]];
    icon2 = &gSprites[sMonSummaryScreen->monTypeIconSpriteIds[1]];
    icon3 = &gSprites[sMonSummaryScreen->monTypeIconSpriteIds[2]];

    type1 = sMonSummaryScreen->monTypes[0];
    type2 = sMonSummaryScreen->monTypes[1];
    teraType = sMonSummaryScreen->monTypes[2];

    x1 = isInfoPage ? 183 : 64;
    x2 = isInfoPage ? 219 : 100;
    y = isInfoPage ? 57 : 41;

    ShowTypeIcon(icon1, type1, x1, y);

    if (type2 != type1)
        ShowTypeIcon(icon2, type2, x2, y);
    else
        icon2->invisible = TRUE;

    showTera = (P_SHOW_TERA_TYPE == GEN_9) && isInfoPage && teraType != TYPE_NONE;
    if (showTera)
        ShowTypeIcon(icon3, teraType, 219, 27);
    else
        icon3->invisible = TRUE;
}

static void HideMoveTypeIcons(void)
{
    if (!P_USE_TYPE_ICON_SPRITES)
        return;

    for (u32 i = 0; i < MAX_MON_MOVES + 1; i++)
        gSprites[sMonSummaryScreen->moveTypeIconSpriteIds[i]].invisible = TRUE;
}

static void HideMonTypeIcons(void)
{
    if (!P_USE_TYPE_ICON_SPRITES)
        return;

    for (u32 i = 0; i < sizeof(sMonSummaryScreen->monTypeIconSpriteIds); i++)
        gSprites[sMonSummaryScreen->monTypeIconSpriteIds[i]].invisible = TRUE;
}

static void DestroyTypeIconSprites(void)
{
    if (!P_USE_TYPE_ICON_SPRITES)
        return;

    for (u32 i = 0; i < MAX_MON_MOVES + 1; i++)
    {
        if (sMonSummaryScreen->moveTypeIconSpriteIds[i] != 0xFF)
        {
            DestroySprite(&gSprites[sMonSummaryScreen->moveTypeIconSpriteIds[i]]);
            sMonSummaryScreen->moveTypeIconSpriteIds[i] = 0xFF;
        }
    }

    for (u32 i = 0; i < sizeof(sMonSummaryScreen->monTypeIconSpriteIds); i++)
    {
        if (sMonSummaryScreen->monTypeIconSpriteIds[i] != 0xFF)
        {
            DestroySprite(&gSprites[sMonSummaryScreen->monTypeIconSpriteIds[i]]);
            sMonSummaryScreen->monTypeIconSpriteIds[i] = 0xFF;
        }
    }
}

static void CB2_ReturnToSummaryScreenFromNamingScreen(void)
{
    SetBoxMonData(GetSelectedBoxMonFromPcOrParty(), MON_DATA_NICKNAME, gStringVar2);
    ShowPokemonSummaryScreen(gPlayerParty, gSpecialVar_0x8004, gPlayerPartyCount - 1, gInitialSummaryScreenCallback, PSS_MODE_NORMAL);
}

static void CB2_PssChangePokemonNickname(void)
{
    ChangePokemonNicknameWithCallback(CB2_ReturnToSummaryScreenFromNamingScreen);
}
