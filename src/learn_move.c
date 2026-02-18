#include "global.h"
#include "gflib.h"
#include "data.h"
#include "event_data.h"
#include "field_fadetransition.h"
#include "graphics.h"
#include "item.h"
#include "list_menu.h"
#include "menu.h"
#include "move_relearner.h"
#include "move.h"
#include "overworld.h"
#include "party_menu.h"
#include "pokemon_storage_system.h"
#include "pokemon_summary_screen.h"
#include "script.h"
#include "strings.h"
#include "task.h"
#include "text_window.h"
#include "trig.h"
#include "constants/move_relearner.h"
#include "constants/moves.h"
#include "constants/songs.h"
#include "data/pokemon/egg_moves.h"
#include "data/tutor_moves.h"

/*
 * Move relearner state machine
 * ------------------------
 *
 * CB2_InitLearnMove
 *   - Creates listOffset to listen to right/left buttons.
 *   - Creates listRow to listen to up/down buttons.
 * DoMoveRelearnerMain: MENU_STATE_FADE_TO_BLACK
 * DoMoveRelearnerMain: MENU_STATE_WAIT_FOR_FADE
 *   - Go to MENU_STATE_IDLE_BATTLE_MODE
 *
 * DoMoveRelearnerMain: MENU_STATE_SETUP_BATTLE_MODE
 * DoMoveRelearnerMain: MENU_STATE_IDLE_BATTLE_MODE
 *   - If the player selected a move (pressed A), go to MENU_STATE_PRINT_TEACH_MOVE_PROMPT.
 *   - If the player cancelled (pressed B), go to MENU_STATE_PRINT_GIVE_UP_PROMPT.
 *
 * DoMoveRelearnerMain: MENU_STATE_PRINT_TEACH_MOVE_PROMPT
 * DoMoveRelearnerMain: MENU_STATE_TEACH_MOVE_CONFIRM
 *   - Wait for the player to confirm.
 *   - If cancelled, go to MENU_STATE_SETUP_BATTLE_MODE.
 *   - If confirmed and the pokemon had an empty move slot, set VAR_0x8004 to TRUE and go to
 *     MENU_STATE_PRINT_TEXT_THEN_FANFARE.
 *   - If confirmed and the pokemon doesn't have an empty move slot, go to
 *     MENU_STATE_PRINT_TRYING_TO_LEARN_PROMPT.
 *
 * DoMoveRelearnerMain: MENU_STATE_PRINT_TRYING_TO_LEARN_PROMPT
 * DoMoveRelearnerMain: MENU_STATE_WAIT_FOR_TRYING_TO_LEARN
 * DoMoveRelearnerMain: MENU_STATE_CONFIRM_DELETE_OLD_MOVE
 *   - If the player confirms, go to MENU_STATE_PRINT_WHICH_MOVE_PROMPT.
 *   - If the player cancels, go to MENU_STATE_PRINT_STOP_TEACHING
 *
 * DoMoveRelearnerMain: MENU_STATE_PRINT_STOP_TEACHING
 * DoMoveRelearnerMain: MENU_STATE_WAIT_FOR_STOP_TEACHING
 * DoMoveRelearnerMain: MENU_STATE_CONFIRM_STOP_TEACHING
 *   - If the player confirms, go to MENU_STATE_CHOOSE_SETUP_STATE.
 *   - If the player cancels, go back to MENU_STATE_PRINT_TRYING_TO_LEARN_PROMPT.
 *
 * DoMoveRelearnerMain: MENU_STATE_PRINT_WHICH_MOVE_PROMPT
 * DoMoveRelearnerMain: MENU_STATE_SHOW_MOVE_SUMMARY_SCREEN
 *   - Go to ShowSelectMovePokemonSummaryScreen. When done, control returns to
 *     CB2_InitLearnMoveReturnFromSelectMove.
 *
 * DoMoveRelearnerMain: MENU_STATE_DOUBLE_FANFARE_FORGOT_MOVE
 * DoMoveRelearnerMain: MENU_STATE_PRINT_TEXT_THEN_FANFARE
 * DoMoveRelearnerMain: MENU_STATE_WAIT_FOR_FANFARE
 * DoMoveRelearnerMain: MENU_STATE_WAIT_FOR_A_BUTTON
 * DoMoveRelearnerMain: MENU_STATE_FADE_AND_RETURN
 * DoMoveRelearnerMain: MENU_STATE_RETURN_TO_FIELD
 *   - Clean up and go to CB2_ReturnToField.
 *
 * DoMoveRelearnerMain: MENU_STATE_PRINT_GIVE_UP_PROMPT
 * DoMoveRelearnerMain: MENU_STATE_GIVE_UP_CONFIRM
 *   - If the player confirms, go to MENU_STATE_FADE_AND_RETURN, and set VAR_0x8004 to FALSE.
 *   - If the player cancels, go to MENU_STATE_SETUP_BATTLE_MODE.
 *
 * CB2_InitLearnMoveReturnFromSelectMove:
 *   - Do most of the same stuff as CB2_InitLearnMove.
 * DoMoveRelearnerMain: MENU_STATE_FADE_FROM_SUMMARY_SCREEN
 * DoMoveRelearnerMain: MENU_STATE_TRY_OVERWRITE_MOVE
 *   - If any of the pokemon's existing moves were chosen, overwrite the move and
 *     go to MENU_STATE_DOUBLE_FANFARE_FORGOT_MOVE and set VAR_0x8004 to TRUE.
 *   - If the chosen move is the one the player selected before the summary screen,
 *     go to MENU_STATE_PRINT_STOP_TEACHING.
 *
 */

#define MENU_STATE_FADE_TO_BLACK 0
#define MENU_STATE_WAIT_FOR_FADE 1
#define MENU_STATE_UNREACHABLE 2
#define MENU_STATE_SETUP_BATTLE_MODE 3
#define MENU_STATE_IDLE_BATTLE_MODE 4
// States 5, 6, and 7 are skipped.
#define MENU_STATE_PRINT_TEACH_MOVE_PROMPT 8
#define MENU_STATE_TEACH_MOVE_CONFIRM 9
// States 10 and 11 are skipped.
#define MENU_STATE_PRINT_GIVE_UP_PROMPT 12
#define MENU_STATE_GIVE_UP_CONFIRM 13
#define MENU_STATE_FADE_AND_RETURN 14
#define MENU_STATE_RETURN_TO_FIELD 15
#define MENU_STATE_PRINT_TRYING_TO_LEARN_PROMPT 16
#define MENU_STATE_WAIT_FOR_TRYING_TO_LEARN 17
#define MENU_STATE_CONFIRM_DELETE_OLD_MOVE 18
#define MENU_STATE_PRINT_WHICH_MOVE_PROMPT 19
#define MENU_STATE_SHOW_MOVE_SUMMARY_SCREEN 20
#define MENU_STATE_RETURN_TO_PARTY_MENU 21
// States 22 and 23 are skipped.
#define MENU_STATE_PRINT_STOP_TEACHING 24
#define MENU_STATE_WAIT_FOR_STOP_TEACHING 25
#define MENU_STATE_CONFIRM_STOP_TEACHING 26
#define MENU_STATE_CHOOSE_SETUP_STATE 27
#define MENU_STATE_FADE_FROM_SUMMARY_SCREEN 28
#define MENU_STATE_TRY_OVERWRITE_MOVE 29
#define MENU_STATE_DOUBLE_FANFARE_FORGOT_MOVE 30
#define MENU_STATE_PRINT_TEXT_THEN_FANFARE 31
#define MENU_STATE_WAIT_FOR_FANFARE 32
#define MENU_STATE_WAIT_FOR_A_BUTTON 33

#define GFXTAG_UI       5525
#define PALTAG_UI       5526

struct RelearnType
{
    bool32 (*hasMoveToRelearn)(struct BoxPokemon*);
};

struct LearnMoveGfxResources
{
    u8 state;
    u8 spriteIds[2];
    u8 scrollPositionMaybe;
    u8 numLearnableMoves;
    struct ListMenuItem listMenuItems[MAX_RELEARNER_MOVES + 1];
    u16 learnableMoves[MAX_RELEARNER_MOVES];
    bool8 scheduleMoveInfoUpdate;
    u8 selectedPartyMember;
    u8 selectedMoveSlot;
    u8 listMenuTaskId;
    u8 bg1TilemapBuffer[BG_SCREEN_SIZE]; // 264
    u8 textColor[3]; // A64
    s32 selectedItemId;
    u16 listOffset;
    u16 listRow;
    u8 numToShowAtOnce;
};

static EWRAM_DATA struct LearnMoveGfxResources *sMoveRelearnerStruct = NULL;
EWRAM_DATA enum MoveRelearnerStates gMoveRelearnerState = MOVE_RELEARNER_LEVEL_UP_MOVES;
EWRAM_DATA enum RelearnMode gRelearnMode = RELEARN_MODE_NONE;

static void Task_WaitForFadeOut(u8 taskId);
static void CB2_MoveRelearnerMain(void);
static void DoMoveRelearnerMain(void);
static void DrawTextBorderOnWindows6and7(void);
static void ShowTeachMoveText(void);
static void SpriteCB_ListMenuScrollIndicators(struct Sprite *sprite);
static void SpawnListMenuScrollIndicatorSprites(void);
static void CreateLearnableMovesList(void);
static void HandleInput(void);
static void MoveLearnerInitListMenu(void);
static void LoadMoveInfoUI(void);
static void PrintMoveInfoHandleCancel_CopyToVram(void);
static void MoveRelearnerMenu_MoveCursorFunc(s32 itemIndex, bool8 onInit, struct ListMenu *list);
static s8 YesNoMenuProcessInput(void);
static void PrintTextOnWindow(u8 windowId, const u8 *str, u8 x, u8 y, s32 speed, s32 colorIdx);
static s32 GetCurrentSelectedMove(void);
static bool32 HasRelearnerLevelUpMoves(struct BoxPokemon *boxMon);
static bool32 HasRelearnerEggMoves(struct BoxPokemon *boxMon);
static bool32 HasRelearnerTMMoves(struct BoxPokemon *boxMon);
static bool32 HasRelearnerTutorMoves(struct BoxPokemon *boxMon);
static u32 GetRelearnerLevelUpMoves(struct BoxPokemon *mon, u16 *moves);
static u32 GetRelearnerEggMoves(struct BoxPokemon *mon, u16 *moves);
static u32 GetRelearnerTMMoves(struct BoxPokemon *mon, u16 *moves);
static u32 GetRelearnerTutorMoves(struct BoxPokemon *mon, u16 *moves);

static const struct RelearnType sRelearnTypes[MOVE_RELEARNER_COUNT] =
{
    [MOVE_RELEARNER_LEVEL_UP_MOVES] = {HasRelearnerLevelUpMoves},
    [MOVE_RELEARNER_EGG_MOVES] = {HasRelearnerEggMoves},
    [MOVE_RELEARNER_TM_MOVES] = {HasRelearnerTMMoves},
    [MOVE_RELEARNER_TUTOR_MOVES] = {HasRelearnerTutorMoves},
};

static const u16 sUI_Pal[] = INCBIN_U16("graphics/learn_move/interface_sprites.gbapal");
static const u16 sUI_Tiles[] = INCBIN_U16("graphics/learn_move/interface_sprites.4bpp");

static const struct SpriteSheet sMoveRelearnerSpriteSheet = {
    .data = sUI_Tiles,
    .size = sizeof(sUI_Tiles),
    .tag = GFXTAG_UI,
};

static const struct SpritePalette sMoveRelearnerPalette = {
    .data = sUI_Pal,
    .tag = PALTAG_UI,
};

static const struct OamData sOamdata_MoveRelearnerListMenuScrollIndicators = {
    .shape = SPRITE_SHAPE(16x8),
    .size = SPRITE_SIZE(16x8)
};

static const union AnimCmd sAnimCmd_ScrollIndicatorDown[] = {
    ANIMCMD_FRAME(4, 5),
    ANIMCMD_END
};

static const union AnimCmd sAnimCmd_ScrollIndicatorUp[] = {
    ANIMCMD_FRAME(6, 5),
    ANIMCMD_END
};

static const union AnimCmd *const sSpriteAnimTable_MoveRelearnerListMenuScrollIndicators[] = {
    sAnimCmd_ScrollIndicatorDown,
    sAnimCmd_ScrollIndicatorUp
};

static const struct SpriteTemplate sSpriteTemplate_MoveRelearnerListMenuScrollIndicators = {
        .tileTag = GFXTAG_UI,
        .paletteTag = PALTAG_UI,
        .oam = &sOamdata_MoveRelearnerListMenuScrollIndicators,
        .anims = sSpriteAnimTable_MoveRelearnerListMenuScrollIndicators,
        .images = NULL,
        .affineAnims = gDummySpriteAffineAnimTable,
        .callback = SpriteCB_ListMenuScrollIndicators,
};

static const struct BgTemplate sMoveRelearnerMenuBackgroundTemplates[2] = {
    {
        .bg = 0,
        .charBaseIndex = 2,
        .mapBaseIndex = 31,
        .priority = 0
    },
    {
        .bg = 1,
        .charBaseIndex = 0,
        .mapBaseIndex = 8,
        .priority = 1
    }
};

enum RelearnerWindowId
{
    RELEARNER_WIN_MOVE_GFX1,
    RELEARNER_WIN_MOVE_GFX2,
    RELEARNER_WIN_MOVE_TYPE,
    RELEARNER_WIN_MOVE_POW_ACC,
    RELEARNER_WIN_MOVE_PP,
    RELEARNER_WIN_MOVE_DESC,
    RELEARNER_WIN_MOVE_LIST,
    RELEARNER_WIN_MESSAGE_BOX,
};

static const struct WindowTemplate sMoveRelearnerWindowTemplates[] = {
    [RELEARNER_WIN_MOVE_GFX1] =
    {
        .bg = 0,
        .tilemapLeft = 0,
        .tilemapTop = 0,
        .width = 6,
        .height = 7,
        .paletteNum = 13,
        .baseBlock = 0x014
    },
    [RELEARNER_WIN_MOVE_GFX2] =
    {
        .bg = 0,
        .tilemapLeft = 10,
        .tilemapTop = 0,
        .width = 5,
        .height = 5,
        .paletteNum = 13,
        .baseBlock = 0x03e
    },
    [RELEARNER_WIN_MOVE_TYPE] =
    {
        .bg = 0,
        .tilemapLeft = 5,
        .tilemapTop = 0,
        .width = 5,
        .height = 2,
        .paletteNum = 13,
        .baseBlock = 0x057
    },
    [RELEARNER_WIN_MOVE_POW_ACC] =
    {
        .bg = 0,
        .tilemapLeft = 15,
        .tilemapTop = 0,
        .width = 3,
        .height = 5,
        .paletteNum = 15,
        .baseBlock = 0x061
    },
    [RELEARNER_WIN_MOVE_PP] =
    {
        .bg = 0,
        .tilemapLeft = 5,
        .tilemapTop = 2,
        .width = 3,
        .height = 3,
        .paletteNum = 15,
        .baseBlock = 0x070
    },
    [RELEARNER_WIN_MOVE_DESC] =
    {
        .bg = 0,
        .tilemapLeft = 2,
        .tilemapTop = 6,
        .width = 15,
        .height = 8,
        .paletteNum = 15,
        .baseBlock = 0x079
    },
    [RELEARNER_WIN_MOVE_LIST] =
    {
        .bg = 0,
        .tilemapLeft = 19,
        .tilemapTop = 1,
        .width = 10,
        .height = 12,
        .paletteNum = 15,
        .baseBlock = 0x0f1
    },
    [RELEARNER_WIN_MESSAGE_BOX] =
    {
        .bg = 0,
        .tilemapLeft = 2,
        .tilemapTop = 15,
        .width = 26,
        .height = 4,
        .paletteNum = 15,
        .baseBlock = 0x169
    }, DUMMY_WIN_TEMPLATE
};

static const struct WindowTemplate sMoveRelearnerYesNoMenuTemplate = {
    .bg = 0,
    .tilemapLeft = 21,
    .tilemapTop = 8,
    .width = 6,
    .height = 4,
    .paletteNum = 15,
    .baseBlock = 0x1d1
};

static const struct ListMenuTemplate sMoveRelearnerMovesListTemplate = {
    .items = NULL,
    .moveCursorFunc = MoveRelearnerMenu_MoveCursorFunc,
    .itemPrintFunc = NULL,
    .totalItems = 0,
    .maxShowed = 7,
    .windowId = RELEARNER_WIN_MOVE_LIST,
    .header_X = 0,
    .item_X = 8,
    .cursor_X = 0,
    .upText_Y = 0,
    .cursorPal = 2,
    .fillValue = 1,
    .cursorShadowPal = 3,
    .lettersSpacing = 1,
    .itemVerticalPadding = 0,
    .scrollMultiple = 0,
    .fontId = FONT_NORMAL,
    .cursorKind = 0,
};

static void VBlankCB_MoveRelearner(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

// Script arguments: The Pokémon to teach is in VAR_0x8004
void TeachMoveRelearnerMove(void)
{
    LockPlayerFieldControls();
    CreateTask(Task_WaitForFadeOut, 10);
    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
}

static void Task_WaitForFadeOut(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        SetMainCallback2(CB2_InitLearnMove);
        gFieldCallback = FieldCB_ContinueScriptHandleMusic;
        DestroyTask(taskId);
    }
}

static void InitMoveRelearnerBackgroundLayers(void)
{
    ResetBgsAndClearDma3BusyFlags(FALSE);
    InitBgsFromTemplates(0, sMoveRelearnerMenuBackgroundTemplates, ARRAY_COUNT(sMoveRelearnerMenuBackgroundTemplates));
    ResetTempTileDataBuffers();
}

static void InitMoveRelearnerWindows(void)
{
    u32 i;

    InitWindows(sMoveRelearnerWindowTemplates);
    DeactivateAllTextPrinters();
    LoadUserWindowBorderGfx(RELEARNER_WIN_MOVE_GFX1, 1, BG_PLTT_ID(14));
    ListMenuLoadStdPalAt(BG_PLTT_ID(13), 1);

    for (i = 0; i < ARRAY_COUNT(sMoveRelearnerWindowTemplates); i++)
    {
        ClearWindowTilemap(i);
        FillWindowPixelBuffer(i, PIXEL_FILL(0));
    }
    FillWindowPixelBuffer(RELEARNER_WIN_MESSAGE_BOX, PIXEL_FILL(1));
    FillBgTilemapBufferRect(0, 0x000, 0, 0, 30, 20, 15);
    SetBgTilemapBuffer(1, sMoveRelearnerStruct->bg1TilemapBuffer);
    LoadPalette(gMoveRelearner_Pal, BG_PLTT_ID(0), PLTT_SIZE_4BPP);
    DecompressAndLoadBgGfxUsingHeap(1, gMoveRelearner_Gfx, 0, 0, 0);
    CopyToBgTilemapBuffer(1, gMoveRelearner_Tilemap, 0, 0);
    CopyBgTilemapBufferToVram(1);
    SetGpuReg(REG_OFFSET_BG0VOFS, 0);
    SetGpuReg(REG_OFFSET_BG0HOFS, 0);
    SetGpuReg(REG_OFFSET_BG1VOFS, 0);
    SetGpuReg(REG_OFFSET_BG1HOFS, 0);
}

void CB2_InitLearnMove(void)
{
    SetGpuReg(REG_OFFSET_DISPCNT, 0);

    ResetSpriteData();
    FreeAllSpritePalettes();
    ResetTasks();
    sMoveRelearnerStruct = AllocZeroed(sizeof(*sMoveRelearnerStruct));
    sMoveRelearnerStruct->selectedPartyMember = gSpecialVar_0x8004;
    SetVBlankCallback(VBlankCB_MoveRelearner);

    InitMoveRelearnerBackgroundLayers();
    InitMoveRelearnerWindows();

    sMoveRelearnerStruct->listOffset = 0;
    sMoveRelearnerStruct->listRow = 0;

    // if ((!P_ENABLE_MOVE_RELEARNERS
    // && !P_TM_MOVES_RELEARNER
    // && !FlagGet(P_FLAG_EGG_MOVES)
    // && !FlagGet(P_FLAG_TUTOR_MOVES)))
    // {
    //     StringCopy(gStringVar3, MoveRelearner_Text_MoveLWR);
    // }
    // else
    // {
    //     switch (gMoveRelearnerState)
    //     {
    //     case MOVE_RELEARNER_EGG_MOVES:
    //         StringCopy(gStringVar3, MoveRelearner_Text_EggMoveLWR);
    //         break;
    //     case MOVE_RELEARNER_TM_MOVES:
    //         StringCopy(gStringVar3, MoveRelearner_Text_TMMoveLWR);
    //         break;
    //     case MOVE_RELEARNER_TUTOR_MOVES:
    //         StringCopy(gStringVar3, MoveRelearner_Text_TutorMoveLWR);
    //         break;
    //     case MOVE_RELEARNER_LEVEL_UP_MOVES:
    //     default:
    //         StringCopy(gStringVar3, MoveRelearner_Text_LevelUpMoveLWR);
    //         break;
    //     }
    // }

    CreateLearnableMovesList();

    LoadSpriteSheet(&sMoveRelearnerSpriteSheet);
    LoadSpritePalette(&sMoveRelearnerPalette);
    SpawnListMenuScrollIndicatorSprites();

    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
    SetMainCallback2(CB2_MoveRelearnerMain);
}

static void CB2_InitLearnMoveReturnFromSelectMove(void)
{
    SetGpuReg(REG_OFFSET_DISPCNT, 0);
    ResetSpriteData();
    FreeAllSpritePalettes();
    ResetTasks();
    CreateLearnableMovesList();
    sMoveRelearnerStruct->selectedPartyMember = gSpecialVar_0x8004;
    sMoveRelearnerStruct->selectedMoveSlot = gSpecialVar_0x8005;
    SetVBlankCallback(VBlankCB_MoveRelearner);

    InitMoveRelearnerBackgroundLayers();
    InitMoveRelearnerWindows();

    LoadSpriteSheet(&sMoveRelearnerSpriteSheet);
    LoadSpritePalette(&sMoveRelearnerPalette);
    SpawnListMenuScrollIndicatorSprites();

    SetBackdropFromColor(RGB_BLACK);

    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
    SetMainCallback2(CB2_MoveRelearnerMain);
}

static void CB2_MoveRelearnerMain(void)
{
    if (!IsTextPrinterActiveOnWindow(RELEARNER_WIN_MESSAGE_BOX))
        DoMoveRelearnerMain();
    if (sMoveRelearnerStruct->scheduleMoveInfoUpdate)
    {
        PrintMoveInfoHandleCancel_CopyToVram();
        sMoveRelearnerStruct->scheduleMoveInfoUpdate = FALSE;
    }
    RunTasks();
    RunTextPrinters();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

static void PrintMessageWithPlaceholders(const u8 *str)
{
    StringExpandPlaceholders(gStringVar4, str);
    PrintTextOnWindow(RELEARNER_WIN_MESSAGE_BOX, gStringVar4, 0, 2, GetPlayerTextSpeedDelay(), 2);
}

static void DoMoveRelearnerMain(void)
{
    switch (sMoveRelearnerStruct->state)
    {
    case MENU_STATE_FADE_TO_BLACK:
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        ShowBg(0);
        ShowBg(1);
        LoadMoveInfoUI();
        sMoveRelearnerStruct->state++;
        DrawTextBorderOnWindows6and7();
        ShowTeachMoveText();
        MoveLearnerInitListMenu();
        sMoveRelearnerStruct->scheduleMoveInfoUpdate = TRUE;
        break;
    case MENU_STATE_WAIT_FOR_FADE:
        if (!gPaletteFade.active)
            sMoveRelearnerStruct->state = 4;
        break;
    case MENU_STATE_UNREACHABLE:
        sMoveRelearnerStruct->state++;
        break;
    case MENU_STATE_SETUP_BATTLE_MODE:
        ShowTeachMoveText();
        sMoveRelearnerStruct->scheduleMoveInfoUpdate = TRUE;
        sMoveRelearnerStruct->state++;
        break;
    case MENU_STATE_IDLE_BATTLE_MODE:
        HandleInput();
        break;
    case MENU_STATE_PRINT_TEACH_MOVE_PROMPT:
        CreateYesNoMenuAtPos(&sMoveRelearnerYesNoMenuTemplate, FONT_NORMAL_COPY_2, 0, 2, 0x001, 14, 0);
        sMoveRelearnerStruct->state++;
        break;
    case MENU_STATE_TEACH_MOVE_CONFIRM :
        switch (YesNoMenuProcessInput())
        {
        case 0:
            {
                struct BoxPokemon *boxmon;
                if (sMoveRelearnerStruct->selectedPartyMember == PC_MON_CHOSEN)
                    boxmon = GetBoxedMonPtr(gSpecialVar_MonBoxId, gSpecialVar_MonBoxPos);
                else
                    boxmon = &(gPlayerParty[sMoveRelearnerStruct->selectedPartyMember].box);

                if (GiveMoveToBoxMon(boxmon, GetCurrentSelectedMove()) != MON_HAS_MAX_MOVES)
                {
                    PrintMessageWithPlaceholders(gText_MonLearnedMove);
                    gSpecialVar_0x8004 = TRUE;
                    sMoveRelearnerStruct->state = 31;
                }
                else
                {
                    sMoveRelearnerStruct->state = 16;
                }
            }
            break;
        case 1:
        case -1:
            sMoveRelearnerStruct->state = 3;
            break;
        }
        break;
    case MENU_STATE_PRINT_GIVE_UP_PROMPT:
        CreateYesNoMenuAtPos(&sMoveRelearnerYesNoMenuTemplate, FONT_NORMAL_COPY_2, 0, 2, 0x001, 14, 0);
        sMoveRelearnerStruct->state++;
        break;
    case MENU_STATE_GIVE_UP_CONFIRM:
        switch (YesNoMenuProcessInput())
        {
        case 0:
            gSpecialVar_0x8004 = FALSE;
            sMoveRelearnerStruct->state = MENU_STATE_FADE_AND_RETURN;
            break;
        case 1:
        case -1:
            sMoveRelearnerStruct->state = 3;
            break;
        }
        break;
    case MENU_STATE_PRINT_TRYING_TO_LEARN_PROMPT:
        PrintMessageWithPlaceholders(gText_MonIsTryingToLearnMove);
        sMoveRelearnerStruct->state++;
        break;
    case MENU_STATE_WAIT_FOR_TRYING_TO_LEARN:
        CreateYesNoMenuAtPos(&sMoveRelearnerYesNoMenuTemplate, FONT_NORMAL_COPY_2, 0, 2, 0x001, 14, 0);
        sMoveRelearnerStruct->state = 18;
        break;
    case MENU_STATE_CONFIRM_DELETE_OLD_MOVE:
        switch (YesNoMenuProcessInput())
        {
        case 0:
            PrintMessageWithPlaceholders(gText_WhichMoveShouldBeForgotten);
            sMoveRelearnerStruct->state = 19;
            break;
        case 1:
        case -1:
            sMoveRelearnerStruct->state = 24;
            break;
        }
        break;
    case MENU_STATE_PRINT_STOP_TEACHING:
        PrintMessageWithPlaceholders(gText_StopLearningMove);
        sMoveRelearnerStruct->state++;
        break;
    case MENU_STATE_WAIT_FOR_STOP_TEACHING:
        CreateYesNoMenuAtPos(&sMoveRelearnerYesNoMenuTemplate, FONT_NORMAL_COPY_2, 0, 2, 0x001, 14, 0);
        sMoveRelearnerStruct->state = 26;
        break;
    case MENU_STATE_CONFIRM_STOP_TEACHING:
        switch (YesNoMenuProcessInput())
        {
        case 0:
            sMoveRelearnerStruct->state = 27;
            break;
        case 1:
        case -1:
            sMoveRelearnerStruct->state = 16;
            break;
        }
        break;
    case MENU_STATE_CHOOSE_SETUP_STATE :
        sMoveRelearnerStruct->state = 3;
        break;
    case MENU_STATE_PRINT_WHICH_MOVE_PROMPT:
        sMoveRelearnerStruct->state = 20;
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        break;
    case MENU_STATE_SHOW_MOVE_SUMMARY_SCREEN:
        if (!gPaletteFade.active)
        {
            ListMenuGetScrollAndRow(sMoveRelearnerStruct->listMenuTaskId, &sMoveRelearnerStruct->listOffset, &sMoveRelearnerStruct->listRow);
            FreeAllWindowBuffers();
            ShowSelectMovePokemonSummaryScreen(gPlayerParty, gSpecialVar_0x8004, CB2_InitLearnMoveReturnFromSelectMove, GetCurrentSelectedMove());
            sMoveRelearnerStruct->state = 28;
        }
        break;
    case 21:
        sMoveRelearnerStruct->state = MENU_STATE_FADE_AND_RETURN;
        break;
    case 22:
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        break;
    case MENU_STATE_FADE_AND_RETURN:
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        sMoveRelearnerStruct->state++;
        break;
    case MENU_STATE_RETURN_TO_FIELD:
        if (!gPaletteFade.active)
        {
            if (gInitialSummaryScreenCallback != NULL)
            {
                switch (gRelearnMode)
                {
                case RELEARN_MODE_PSS_PAGE_BATTLE_MOVES:
                    ShowPokemonSummaryScreen(gPlayerParty, sMoveRelearnerStruct->selectedPartyMember, gPlayerPartyCount - 1, gInitialSummaryScreenCallback, SUMMARY_MODE_RELEARNER_BATTLE);
                    break;
                case RELEARN_MODE_BOX_PSS_PAGE_BATTLE_MOVES:
                    ShowPokemonSummaryScreen(GetBoxedMonPtr(gSpecialVar_MonBoxId, 0), gSpecialVar_MonBoxPos, IN_BOX_COUNT - 1, gInitialSummaryScreenCallback, SUMMARY_MODE_RELEARNER_BATTLE);
                    break;
                default:
                    ShowPokemonSummaryScreen(gPlayerParty, sMoveRelearnerStruct->selectedPartyMember, gPlayerPartyCount - 1, gInitialSummaryScreenCallback, PSS_MODE_NORMAL);
                    break;
                }
            }
            else
            {
                SetMainCallback2(CB2_ReturnToField);
            }
            FreeAllWindowBuffers();
            Free(sMoveRelearnerStruct);
            gRelearnMode = RELEARN_MODE_NONE;
        }
        break;
    case MENU_STATE_FADE_FROM_SUMMARY_SCREEN:
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        ShowBg(0);
        ShowBg(1);
        sMoveRelearnerStruct->state++;
        LoadMoveInfoUI();
        DrawTextBorderOnWindows6and7();
        MoveLearnerInitListMenu();
        PrintMoveInfoHandleCancel_CopyToVram();
        break;
    case MENU_STATE_TRY_OVERWRITE_MOVE:
        if (!gPaletteFade.active)
        {
            if (sMoveRelearnerStruct->selectedMoveSlot == MAX_MON_MOVES)
            {
                sMoveRelearnerStruct->state = MENU_STATE_PRINT_STOP_TEACHING;
            }
            else
            {
                enum Move move;
                struct BoxPokemon *boxmon;
                if (sMoveRelearnerStruct->selectedPartyMember == PC_MON_CHOSEN)
                    boxmon = GetBoxedMonPtr(gSpecialVar_MonBoxId, gSpecialVar_MonBoxPos);
                else
                    boxmon = &(gPlayerParty[sMoveRelearnerStruct->selectedPartyMember].box);
                move = GetBoxMonData(boxmon, MON_DATA_MOVE1 + sMoveRelearnerStruct->selectedMoveSlot);
                u8 originalPP = GetBoxMonData(boxmon, MON_DATA_PP1 + sMoveRelearnerStruct->selectedMoveSlot);

                RemoveBoxMonPPBonus(boxmon, sMoveRelearnerStruct->selectedMoveSlot);
                SetBoxMonMoveSlot(boxmon, GetCurrentSelectedMove(), sMoveRelearnerStruct->selectedMoveSlot);
                u8 newPP = GetBoxMonData(boxmon, MON_DATA_PP1 + sMoveRelearnerStruct->selectedMoveSlot);
                if (!P_SUMMARY_MOVE_RELEARNER_FULL_PP
                 && (gRelearnMode == RELEARN_MODE_PSS_PAGE_BATTLE_MOVES || gRelearnMode == RELEARN_MODE_PSS_PAGE_CONTEST_MOVES) && originalPP < newPP)
                    SetBoxMonData(boxmon, MON_DATA_PP1 + sMoveRelearnerStruct->selectedMoveSlot, &originalPP);

                StringCopy(gStringVar3, GetMoveName(move));
                StringCopy(gStringVar2, GetMoveName(GetCurrentSelectedMove()));
                PrintMessageWithPlaceholders(gText_1_2_and_Poof);
                sMoveRelearnerStruct->state = MENU_STATE_DOUBLE_FANFARE_FORGOT_MOVE;
                gSpecialVar_0x8004 = TRUE;
            }
        }
        break;
    case MENU_STATE_DOUBLE_FANFARE_FORGOT_MOVE:
        PrintMessageWithPlaceholders(gText_MonForgotOldMoveAndMonLearnedNewMove);
        sMoveRelearnerStruct->state = 31;
        PlayFanfare(MUS_LEVEL_UP);
        break;
    case MENU_STATE_PRINT_TEXT_THEN_FANFARE:
        PlayFanfare(MUS_LEVEL_UP);
        sMoveRelearnerStruct->state = 32;
        break;
    case MENU_STATE_WAIT_FOR_FANFARE:
        if (IsFanfareTaskInactive())
            sMoveRelearnerStruct->state = 33;
        break;
    case MENU_STATE_WAIT_FOR_A_BUTTON:
        if (JOY_NEW(A_BUTTON))
        {
            PlaySE(SE_SELECT);
            sMoveRelearnerStruct->state = MENU_STATE_FADE_AND_RETURN;
        }
        break;
    }
}

static void DrawTextBorderOnWindows6and7(void)
{
    DrawTextBorderOuter(RELEARNER_WIN_MOVE_LIST, 0x001, 14);
    DrawTextBorderOuter(RELEARNER_WIN_MESSAGE_BOX, 0x001, 14);
}

static void ShowTeachMoveText(void)
{
    StringExpandPlaceholders(gStringVar4, gText_TeachWhichMoveToMon);
    PrintTextOnWindow(RELEARNER_WIN_MESSAGE_BOX, gStringVar4, 0, 2, 0, 2);
    PutWindowTilemap(RELEARNER_WIN_MESSAGE_BOX);
    CopyWindowToVram(RELEARNER_WIN_MESSAGE_BOX, COPYWIN_FULL);
}

static void SpriteCB_ListMenuScrollIndicators(struct Sprite *sprite)
{
    s16 abcissa = (sprite->data[1] * 10) & 0xFF;
    switch (sprite->data[0])
    {
    case 0:
        break;
    case 1:
        sprite->x2 = Sin(abcissa, 3) * sprite->data[2];
        break;
    case 2:
        sprite->y2 = Sin(abcissa, 1) * sprite->data[2];
        break;
    }
    sprite->data[1]++;
}

static void SpawnListMenuScrollIndicatorSprites(void)
{
    int i;
    sMoveRelearnerStruct->spriteIds[0] = CreateSprite(&sSpriteTemplate_MoveRelearnerListMenuScrollIndicators, 200, 4, 0);
    StartSpriteAnim(&gSprites[sMoveRelearnerStruct->spriteIds[0]], 1);
    gSprites[sMoveRelearnerStruct->spriteIds[0]].data[0] = 2;
    gSprites[sMoveRelearnerStruct->spriteIds[0]].data[2] = -1;

    // Bug: This should be using the second element of spriteIds.
    sMoveRelearnerStruct->spriteIds[0] = CreateSprite(&sSpriteTemplate_MoveRelearnerListMenuScrollIndicators, 200, 108, 0);
    gSprites[sMoveRelearnerStruct->spriteIds[0]].data[0] = 2;
    gSprites[sMoveRelearnerStruct->spriteIds[0]].data[2] = 1;
    for (i = 0; i < 2; i++)
        gSprites[sMoveRelearnerStruct->spriteIds[i]].invisible = TRUE;
}

static u8 LoadMoveRelearnerMovesList(const struct ListMenuItem *items, u16 numChoices)
{
    gMultiuseListMenuTemplate = sMoveRelearnerMovesListTemplate;
    gMultiuseListMenuTemplate.totalItems = numChoices;
    gMultiuseListMenuTemplate.items = items;

    if (numChoices < 7)
        gMultiuseListMenuTemplate.maxShowed = numChoices;
    else
        gMultiuseListMenuTemplate.maxShowed = 7;

    return gMultiuseListMenuTemplate.maxShowed;
}

static void CreateLearnableMovesList(void)
{
    int i;
    struct BoxPokemon *boxmon = GetSelectedBoxMonFromPcOrParty();
    switch (gMoveRelearnerState)
    {
    case MOVE_RELEARNER_EGG_MOVES:
        sMoveRelearnerStruct->numLearnableMoves = GetRelearnerEggMoves(boxmon, sMoveRelearnerStruct->learnableMoves);
        break;
    case MOVE_RELEARNER_TM_MOVES:
        sMoveRelearnerStruct->numLearnableMoves = GetRelearnerTMMoves(boxmon, sMoveRelearnerStruct->learnableMoves);
        break;
    case MOVE_RELEARNER_TUTOR_MOVES:
        sMoveRelearnerStruct->numLearnableMoves = GetRelearnerTutorMoves(boxmon, sMoveRelearnerStruct->learnableMoves);
        break;
    case MOVE_RELEARNER_LEVEL_UP_MOVES:
    default:
        sMoveRelearnerStruct->numLearnableMoves = GetRelearnerLevelUpMoves(boxmon, sMoveRelearnerStruct->learnableMoves);
        break;
    }

    GetBoxMonData(boxmon, MON_DATA_NICKNAME, gStringVar1);

    for (i = 0; i < sMoveRelearnerStruct->numLearnableMoves; i++)
    {
        sMoveRelearnerStruct->listMenuItems[i].name = GetMoveName(sMoveRelearnerStruct->learnableMoves[i]);
        sMoveRelearnerStruct->listMenuItems[i].id = sMoveRelearnerStruct->learnableMoves[i];
    }
    sMoveRelearnerStruct->listMenuItems[i].name = gFameCheckerText_Cancel;
    sMoveRelearnerStruct->listMenuItems[i].id = LIST_CANCEL;
    sMoveRelearnerStruct->numLearnableMoves++;
    sMoveRelearnerStruct->numToShowAtOnce = LoadMoveRelearnerMovesList(sMoveRelearnerStruct->listMenuItems, sMoveRelearnerStruct->numLearnableMoves);
}

static void HandleInput(void)
{
    s32 itemId = ListMenu_ProcessInput(sMoveRelearnerStruct->listMenuTaskId);
    ListMenuGetScrollAndRow(sMoveRelearnerStruct->listMenuTaskId, &sMoveRelearnerStruct->listOffset, &sMoveRelearnerStruct->listRow);

    switch (itemId)
    {
        case LIST_NOTHING_CHOSEN:
            break;
        case LIST_CANCEL:
            PlaySE(SE_SELECT);
            sMoveRelearnerStruct->state = 12;
            PrintMessageWithPlaceholders(gText_GiveUpTryingToTeachNewMove);
            break;
        default:
            PlaySE(SE_SELECT);
            if (sMoveRelearnerStruct->selectedItemId != LIST_CANCEL)
            {
                sMoveRelearnerStruct->state = 8;
                StringCopy(gStringVar2, GetMoveName(GetCurrentSelectedMove()));
                PrintMessageWithPlaceholders(gText_TeachMoveQues);
            }
            else
            {
                PrintMessageWithPlaceholders(gText_GiveUpTryingToTeachNewMove);
                sMoveRelearnerStruct->state = 12;
            }
            break;
    }

    if (sMoveRelearnerStruct->numLearnableMoves > 6)
    {
        gSprites[0].invisible = FALSE;
        gSprites[1].invisible = FALSE;
        if (sMoveRelearnerStruct->scrollPositionMaybe == 0)
            gSprites[0].invisible = TRUE;
        else if (sMoveRelearnerStruct->scrollPositionMaybe == sMoveRelearnerStruct->numLearnableMoves - 6)
            gSprites[1].invisible = TRUE;
    }
}

static s32 GetCurrentSelectedMove(void)
{
    return sMoveRelearnerStruct->listMenuItems[sMoveRelearnerStruct->listRow + sMoveRelearnerStruct->listOffset].id;
}

static void MoveLearnerInitListMenu(void)
{
    sMoveRelearnerStruct->listMenuTaskId = ListMenuInit(&gMultiuseListMenuTemplate, sMoveRelearnerStruct->listOffset, sMoveRelearnerStruct->listRow);
    CopyWindowToVram(RELEARNER_WIN_MOVE_LIST, COPYWIN_MAP);
}

static void PrintMoveInfo(u16 move)
{
    u8 buffer[50];
    u16 power = GetMovePower(move);
    u16 accuracy = GetMoveAccuracy(move);

    BlitMenuInfoIcon(RELEARNER_WIN_MOVE_TYPE, GetMoveType(move) + 1, 1, 4);

    if (power < 2)
    {
        PrintTextOnWindow(RELEARNER_WIN_MOVE_POW_ACC, gText_ThreeHyphens, 1, 4, 0, 0);
    }
    else
    {
        ConvertIntToDecimalStringN(buffer, power, STR_CONV_MODE_RIGHT_ALIGN, 3);
        PrintTextOnWindow(RELEARNER_WIN_MOVE_POW_ACC, buffer, 1, 4, 0, 0);
    }

    if (accuracy == 0)
    {
        PrintTextOnWindow(RELEARNER_WIN_MOVE_POW_ACC, gText_ThreeHyphens, 1, 18, 0, 1);
    }
    else
    {
        ConvertIntToDecimalStringN(buffer, accuracy, STR_CONV_MODE_RIGHT_ALIGN, 3);
        PrintTextOnWindow(RELEARNER_WIN_MOVE_POW_ACC, buffer, 1, 18, 0, 1);
    }
    ConvertIntToDecimalStringN(buffer, GetMovePP(move), STR_CONV_MODE_LEFT_ALIGN, 2);
    PrintTextOnWindow(RELEARNER_WIN_MOVE_PP, buffer, 2, 2, 0, 0);
    PrintTextOnWindow(RELEARNER_WIN_MOVE_DESC, GetMoveDescription(move), 1, 0, 0, 0);
}

static void LoadMoveInfoUI(void)
{
    BlitMenuInfoIcon(RELEARNER_WIN_MOVE_GFX1, MENU_INFO_ICON_TYPE, 1, 4);
    BlitMenuInfoIcon(RELEARNER_WIN_MOVE_GFX1, MENU_INFO_ICON_PP, 1, 19);
    BlitMenuInfoIcon(RELEARNER_WIN_MOVE_GFX1, MENU_INFO_ICON_EFFECT, 1, 34);

    BlitMenuInfoIcon(RELEARNER_WIN_MOVE_GFX2, MENU_INFO_ICON_POWER, 0, 4);
    BlitMenuInfoIcon(RELEARNER_WIN_MOVE_GFX2, MENU_INFO_ICON_ACCURACY, 0, 19);

    PutWindowTilemap(RELEARNER_WIN_MOVE_GFX1);
    PutWindowTilemap(RELEARNER_WIN_MOVE_GFX2);
    PutWindowTilemap(RELEARNER_WIN_MOVE_TYPE);
    PutWindowTilemap(RELEARNER_WIN_MOVE_POW_ACC);
    PutWindowTilemap(RELEARNER_WIN_MOVE_PP);
    PutWindowTilemap(RELEARNER_WIN_MOVE_DESC);
    PutWindowTilemap(RELEARNER_WIN_MESSAGE_BOX);

    CopyWindowToVram(RELEARNER_WIN_MOVE_GFX1, COPYWIN_GFX);
    CopyWindowToVram(RELEARNER_WIN_MOVE_GFX2, COPYWIN_GFX);
}

static void PrintMoveInfoHandleCancel_CopyToVram(void)
{
    if (sMoveRelearnerStruct->selectedItemId != LIST_CANCEL)
    {
        PrintMoveInfo(sMoveRelearnerStruct->selectedItemId);
    }
    else
    {
        FillWindowPixelBuffer(RELEARNER_WIN_MOVE_TYPE, PIXEL_FILL(0));
        FillWindowPixelBuffer(RELEARNER_WIN_MOVE_POW_ACC, PIXEL_FILL(0));
        FillWindowPixelBuffer(RELEARNER_WIN_MOVE_PP, PIXEL_FILL(0));
        FillWindowPixelBuffer(RELEARNER_WIN_MOVE_DESC, PIXEL_FILL(0));
    }

    CopyWindowToVram(RELEARNER_WIN_MOVE_TYPE, COPYWIN_GFX);
    CopyWindowToVram(RELEARNER_WIN_MOVE_POW_ACC, COPYWIN_GFX);
    CopyWindowToVram(RELEARNER_WIN_MOVE_PP, COPYWIN_GFX);
    CopyWindowToVram(RELEARNER_WIN_MOVE_DESC, COPYWIN_GFX);
    CopyWindowToVram(RELEARNER_WIN_MESSAGE_BOX, COPYWIN_FULL);
}

static void MoveRelearnerMenu_MoveCursorFunc(s32 itemIndex, bool8 onInit, struct ListMenu *list)
{
    sMoveRelearnerStruct->selectedItemId = itemIndex;
    if (!onInit)
    {
        PlaySE(SE_SELECT);
        sMoveRelearnerStruct->scheduleMoveInfoUpdate = TRUE;
    }
}

static s8 YesNoMenuProcessInput(void)
{
    s8 input = Menu_ProcessInputNoWrapClearOnChoose();
    if (input != MENU_NOTHING_CHOSEN)
    {
        PutWindowTilemap(RELEARNER_WIN_MOVE_LIST);
        CopyWindowToVram(RELEARNER_WIN_MOVE_LIST, COPYWIN_MAP);
    }
    return input;
}

static void PrintTextOnWindow(u8 windowId, const u8 *str, u8 x, u8 y, s32 speed, s32 colorIdx)
{
    s32 letterSpacing = 1;
    s32 lineSpacing = 1;
    if (colorIdx == 0 || colorIdx == 1)
    {
        letterSpacing = 0;
        lineSpacing = 0;
    }
    switch (colorIdx)
    {
    case 0:
    case 1:
        sMoveRelearnerStruct->textColor[0] = 0;
        sMoveRelearnerStruct->textColor[1] = 2;
        sMoveRelearnerStruct->textColor[2] = 3;
        break;
    case 2:
        sMoveRelearnerStruct->textColor[0] = 1;
        sMoveRelearnerStruct->textColor[1] = 2;
        sMoveRelearnerStruct->textColor[2] = 3;
    }
    if (colorIdx != 1)
        FillWindowPixelBuffer(windowId, PIXEL_FILL(sMoveRelearnerStruct->textColor[0]));
    AddTextPrinterParameterized4(windowId, FONT_NORMAL_COPY_2, x, y, letterSpacing, lineSpacing, sMoveRelearnerStruct->textColor, speed, str);
}

static void QuickSortMoves(u16 *moves, s32 left, s32 right)
{
    if (left >= right)
        return;

    u16 pivot = moves[(left + right) / 2];
    s32 i = left, j = right;

    while (i <= j)
    {
        while (moves[i] != MOVE_NONE && StringCompare(GetMoveName(moves[i]), GetMoveName(pivot)) < 0)
            i++;
        while (moves[j] != MOVE_NONE && StringCompare(GetMoveName(moves[j]), GetMoveName(pivot)) > 0)
            j--;

        if (i <= j)
        {
            u16 temp = moves[i];
            moves[i] = moves[j];
            moves[j] = temp;
            i++;
            j--;
        }
    }

    QuickSortMoves(moves, left, j);
    QuickSortMoves(moves, i, right);
}

static void SortMovesAlphabetically(u16 *moves, u32 numMoves)
{
    if (numMoves > 1)
        QuickSortMoves(moves, 0, numMoves - 1);
}

static u32 GetRelearnerLevelUpMoves(struct BoxPokemon *mon, u16 *moves)
{
    u32 numMoves = 0;
    u32 species = GetBoxMonData(mon, MON_DATA_SPECIES_OR_EGG);

    if (species == SPECIES_EGG)
        return 0;

    u32 level = (P_ENABLE_ALL_LEVEL_UP_MOVES ? MAX_LEVEL : GetLevelFromBoxMonExp(mon));
    do
    {
        const struct LevelUpMove *learnset = GetSpeciesLevelUpLearnset(species);

        for (u32 i = 0; i < MAX_LEVEL_UP_MOVES && learnset[i].move != LEVEL_UP_MOVE_END; i++)
        {
            if (learnset[i].level > level)
                break;

            if (!BoxMonKnowsMove(mon, learnset[i].move))
                moves[numMoves++] = learnset[i].move;
        }

        species = (P_PRE_EVO_MOVES ? GetSpeciesPreEvolution(species) : SPECIES_NONE);
    } while (species != SPECIES_NONE);

    if (P_SORT_MOVES)
        SortMovesAlphabetically(moves, numMoves);

    return numMoves;
}

static u32 GetRelearnerEggMoves(struct BoxPokemon *mon, u16 *moves)
{
    if (!FlagGet(P_FLAG_EGG_MOVES) && !P_ENABLE_MOVE_RELEARNERS)
        return 0;

    u32 species = GetBoxMonData(mon, MON_DATA_SPECIES_OR_EGG);

    if (species == SPECIES_EGG)
        return 0;

    u32 numMoves = 0;
    while (GetSpeciesPreEvolution(species) != SPECIES_NONE)
        species = GetSpeciesPreEvolution(species);

    const u16 *eggMoves = GetSpeciesEggMoves(species);

    if (eggMoves == sNoneEggMoveLearnset)
        return 0;

    for (u32 i = 0; eggMoves[i] != MOVE_UNAVAILABLE; i++)
    {
        if (!BoxMonKnowsMove(mon, eggMoves[i]))
            moves[numMoves++] = eggMoves[i];
    }

    if (P_SORT_MOVES)
        SortMovesAlphabetically(moves, numMoves);

    return numMoves;
}

static u32 GetRelearnerTMMoves(struct BoxPokemon *mon, u16 *moves)
{
    if (!P_TM_MOVES_RELEARNER && !P_ENABLE_MOVE_RELEARNERS)
        return 0;

    u32 species = GetBoxMonData(mon, MON_DATA_SPECIES_OR_EGG);

    if (species == SPECIES_EGG)
        return 0;

    u32 numMoves = 0;

    for (u32 i = 0; i < NUM_ALL_MACHINES; i++)
    {
        enum Item item = GetTMHMItemId(i + 1);
        enum Move move = GetTMHMMoveId(i + 1);

        if (move == MOVE_NONE)
            continue;

        if (!P_ENABLE_ALL_TM_MOVES && !CheckBagHasItem(item, 1))
            continue;

        if (!CanLearnTeachableMove(species, move))
            continue;

        if (!BoxMonKnowsMove(mon, move))
            moves[numMoves++] = move;
    }

    if (P_SORT_MOVES)
        SortMovesAlphabetically(moves, numMoves);

    return numMoves;
}

static u32 GetRelearnerTutorMoves(struct BoxPokemon *mon, u16 *moves)
{
    if (!FlagGet(P_FLAG_TUTOR_MOVES) && !P_ENABLE_MOVE_RELEARNERS)
        return 0;

    u32 species = GetBoxMonData(mon, MON_DATA_SPECIES_OR_EGG);

    if (species == SPECIES_EGG)
        return 0;

    u32 numMoves = 0;

    for (u32 i = 0; gTutorMoves[i] != MOVE_UNAVAILABLE; i++)
    {
        enum Move move = gTutorMoves[i];

        if (!CanLearnTeachableMove(species, move))
            continue;

        if (!BoxMonKnowsMove(mon, move))
            moves[numMoves++] = move;
    }

    if (P_SORT_MOVES)
        SortMovesAlphabetically(moves, numMoves);

    return numMoves;
}

void HasMovesToRelearn(void)
{
    struct BoxPokemon *boxmon = GetSelectedBoxMonFromPcOrParty();
    if (CanBoxMonRelearnMoves(boxmon, gMoveRelearnerState))
        gSpecialVar_Result = TRUE;
    else
        gSpecialVar_Result = FALSE;
}

bool32 CanBoxMonRelearnMoves(struct BoxPokemon *boxMon, enum MoveRelearnerStates state)
{
    return sRelearnTypes[state].hasMoveToRelearn(boxMon);
}

static bool32 HasRelearnerLevelUpMoves(struct BoxPokemon *boxMon)
{
    u32 species = GetBoxMonData(boxMon, MON_DATA_SPECIES_OR_EGG);

    if (species == SPECIES_EGG)
        return FALSE;

    u32 level = (P_ENABLE_ALL_LEVEL_UP_MOVES == TRUE) ? MAX_LEVEL : GetLevelFromBoxMonExp(boxMon);

    do
    {
        const struct LevelUpMove *learnset = GetSpeciesLevelUpLearnset(species);

        for (u32 i = 0; i < MAX_LEVEL_UP_MOVES && learnset[i].move != LEVEL_UP_MOVE_END; i++)
        {
            if (learnset[i].level > level)
                break;

            if (!BoxMonKnowsMove(boxMon, learnset[i].move))
                return TRUE;
        }

        species = (P_PRE_EVO_MOVES ? GetSpeciesPreEvolution(species) : SPECIES_NONE);

    } while (species != SPECIES_NONE);

    return FALSE;
}

static bool32 HasRelearnerEggMoves(struct BoxPokemon *boxMon)
{
    if (!FlagGet(P_FLAG_EGG_MOVES) && !P_ENABLE_MOVE_RELEARNERS)
        return FALSE;

    u32 species = GetBoxMonData(boxMon, MON_DATA_SPECIES_OR_EGG);

    if (species == SPECIES_EGG)
        return FALSE;

    while (GetSpeciesPreEvolution(species) != SPECIES_NONE)
        species = GetSpeciesPreEvolution(species);

    const u16 *eggMoves = GetSpeciesEggMoves(species);

    if (eggMoves == sNoneEggMoveLearnset)
        return FALSE;

    for (u32 i = 0; eggMoves[i] != MOVE_UNAVAILABLE; i++)
    {
        if (!BoxMonKnowsMove(boxMon, eggMoves[i]))
            return TRUE;
    }

    return FALSE;
}

static bool32 HasRelearnerTMMoves(struct BoxPokemon *boxMon)
{
    if (!P_TM_MOVES_RELEARNER && !P_ENABLE_MOVE_RELEARNERS)
        return FALSE;

    u32 species = GetBoxMonData(boxMon, MON_DATA_SPECIES_OR_EGG);

    if (species == SPECIES_EGG)
        return FALSE;

    for (u32 i = 0; i < NUM_ALL_MACHINES; i++)
    {
        enum Item item = GetTMHMItemId(i + 1);
        enum Move move = GetTMHMMoveId(i + 1);

        if (move == MOVE_NONE)
            continue;

        if (!P_ENABLE_ALL_TM_MOVES && !CheckBagHasItem(item, 1))
            continue;

        if (!CanLearnTeachableMove(species, move))
            continue;

        if (!BoxMonKnowsMove(boxMon, move))
            return TRUE;
    }

    return FALSE;
}

static bool32 HasRelearnerTutorMoves(struct BoxPokemon *boxMon)
{
    if (!FlagGet(P_FLAG_TUTOR_MOVES) && !P_ENABLE_MOVE_RELEARNERS)
        return FALSE;

    u32 species = GetBoxMonData(boxMon, MON_DATA_SPECIES_OR_EGG);

    if (species == SPECIES_EGG)
        return FALSE;

    for (u32 i = 0; gTutorMoves[i] != MOVE_UNAVAILABLE; i++)
    {
        enum Move move = gTutorMoves[i];

        if (!CanLearnTeachableMove(species, move))
            continue;

        if (!BoxMonKnowsMove(boxMon, move))
            return TRUE;
    }

    return FALSE;
}
