#include "global.h"
#include "bg.h"
#include "event_data.h"
#include "field_weather.h"
#include "gpu_regs.h"
#include "graphics.h"
#include "international_string_util.h"
#include "main.h"
#include "menu.h"
#include "map_name_popup_emerald.h"
#include "palette.h"
#include "region_map.h"
#include "rtc.h"
#include "start_menu.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "constants/layouts.h"
#include "constants/region_map_sections.h"
#include "constants/weather.h"
#include "config/general.h"
#include "config/overworld.h"

// enums
enum MapPopUp_Themes
{
    MAPPOPUP_THEME_WOOD,
    MAPPOPUP_THEME_MARBLE,
    MAPPOPUP_THEME_STONE,
    MAPPOPUP_THEME_BRICK,
    MAPPOPUP_THEME_UNDERWATER,
    MAPPOPUP_THEME_STONE2,
};

// static functions
static void Task_MapNamePopUpWindow(u8 taskId);
static void ShowMapNamePopUpWindow(void);
static void LoadMapNamePopUpWindowBg(void);

// EWRAM
EWRAM_DATA u8 gPopupTaskId = 0;

// .rodata
static const u8 sMapPopUp_Table[][960] =
{
    [MAPPOPUP_THEME_WOOD]       = INCBIN_U8("graphics/map_popup/wood.4bpp"),
    [MAPPOPUP_THEME_MARBLE]     = INCBIN_U8("graphics/map_popup/marble.4bpp"),
    [MAPPOPUP_THEME_STONE]      = INCBIN_U8("graphics/map_popup/stone.4bpp"),
    [MAPPOPUP_THEME_BRICK]      = INCBIN_U8("graphics/map_popup/brick.4bpp"),
    [MAPPOPUP_THEME_UNDERWATER] = INCBIN_U8("graphics/map_popup/underwater.4bpp"),
    [MAPPOPUP_THEME_STONE2]     = INCBIN_U8("graphics/map_popup/stone2.4bpp"),
};

static const u8 sMapPopUp_OutlineTable[][960] =
{
    [MAPPOPUP_THEME_WOOD]       = INCBIN_U8("graphics/map_popup/wood_outline.4bpp"),
    [MAPPOPUP_THEME_MARBLE]     = INCBIN_U8("graphics/map_popup/marble_outline.4bpp"),
    [MAPPOPUP_THEME_STONE]      = INCBIN_U8("graphics/map_popup/stone_outline.4bpp"),
    [MAPPOPUP_THEME_BRICK]      = INCBIN_U8("graphics/map_popup/brick_outline.4bpp"),
    [MAPPOPUP_THEME_UNDERWATER] = INCBIN_U8("graphics/map_popup/underwater_outline.4bpp"),
    [MAPPOPUP_THEME_STONE2]     = INCBIN_U8("graphics/map_popup/stone2_outline.4bpp"),
};

static const u16 sMapPopUp_PaletteTable[][16] =
{
    [MAPPOPUP_THEME_WOOD]       = INCBIN_U16("graphics/map_popup/wood.gbapal"),
    [MAPPOPUP_THEME_MARBLE]     = INCBIN_U16("graphics/map_popup/marble_outline.gbapal"),
    [MAPPOPUP_THEME_STONE]      = INCBIN_U16("graphics/map_popup/stone_outline.gbapal"),
    [MAPPOPUP_THEME_BRICK]      = INCBIN_U16("graphics/map_popup/brick_outline.gbapal"),
    [MAPPOPUP_THEME_UNDERWATER] = INCBIN_U16("graphics/map_popup/underwater_outline.gbapal"),
    [MAPPOPUP_THEME_STONE2]     = INCBIN_U16("graphics/map_popup/stone2_outline.gbapal"),
};

static const u16 sMapPopUp_Palette_Underwater[16] = INCBIN_U16("graphics/map_popup/underwater.gbapal");

static const u8 sRegionMapSectionId_To_PopUpThemeIdMapping[MAPSEC_COUNT] =
{
    [MAPSEC_LITTLEROOT_TOWN] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_OLDALE_TOWN] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_DEWFORD_TOWN] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_LAVARIDGE_TOWN] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_FALLARBOR_TOWN] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_VERDANTURF_TOWN] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_PACIFIDLOG_TOWN] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_PETALBURG_CITY] = MAPPOPUP_THEME_BRICK,
    [MAPSEC_SLATEPORT_CITY] = MAPPOPUP_THEME_MARBLE,
    [MAPSEC_MAUVILLE_CITY] = MAPPOPUP_THEME_MARBLE,
    [MAPSEC_RUSTBORO_CITY] = MAPPOPUP_THEME_MARBLE,
    [MAPSEC_FORTREE_CITY] = MAPPOPUP_THEME_BRICK,
    [MAPSEC_LILYCOVE_CITY] = MAPPOPUP_THEME_MARBLE,
    [MAPSEC_MOSSDEEP_CITY] = MAPPOPUP_THEME_BRICK,
    [MAPSEC_SOOTOPOLIS_CITY] = MAPPOPUP_THEME_MARBLE,
    [MAPSEC_EVER_GRANDE_CITY] = MAPPOPUP_THEME_BRICK,
    [MAPSEC_ROUTE_101] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_ROUTE_102] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_ROUTE_103] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_ROUTE_104] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_ROUTE_105] = MAPPOPUP_THEME_UNDERWATER,
    [MAPSEC_ROUTE_106] = MAPPOPUP_THEME_UNDERWATER,
    [MAPSEC_ROUTE_107] = MAPPOPUP_THEME_UNDERWATER,
    [MAPSEC_ROUTE_108] = MAPPOPUP_THEME_UNDERWATER,
    [MAPSEC_ROUTE_109] = MAPPOPUP_THEME_UNDERWATER,
    [MAPSEC_ROUTE_110] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_ROUTE_111] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_ROUTE_112] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_ROUTE_113] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_ROUTE_114] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_ROUTE_115] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_ROUTE_116] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_ROUTE_117] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_ROUTE_118] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_ROUTE_119] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_ROUTE_120] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_ROUTE_121] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_ROUTE_122] = MAPPOPUP_THEME_UNDERWATER,
    [MAPSEC_ROUTE_123] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_ROUTE_124] = MAPPOPUP_THEME_UNDERWATER,
    [MAPSEC_ROUTE_125] = MAPPOPUP_THEME_UNDERWATER,
    [MAPSEC_ROUTE_126] = MAPPOPUP_THEME_UNDERWATER,
    [MAPSEC_ROUTE_127] = MAPPOPUP_THEME_UNDERWATER,
    [MAPSEC_ROUTE_128] = MAPPOPUP_THEME_UNDERWATER,
    [MAPSEC_ROUTE_129] = MAPPOPUP_THEME_UNDERWATER,
    [MAPSEC_ROUTE_130] = MAPPOPUP_THEME_UNDERWATER,
    [MAPSEC_ROUTE_131] = MAPPOPUP_THEME_UNDERWATER,
    [MAPSEC_ROUTE_132] = MAPPOPUP_THEME_UNDERWATER,
    [MAPSEC_ROUTE_133] = MAPPOPUP_THEME_UNDERWATER,
    [MAPSEC_ROUTE_134] = MAPPOPUP_THEME_UNDERWATER,
    [MAPSEC_UNDERWATER_124] = MAPPOPUP_THEME_STONE2,
    [MAPSEC_UNDERWATER_125] = MAPPOPUP_THEME_STONE2,
    [MAPSEC_UNDERWATER_126] = MAPPOPUP_THEME_STONE2,
    [MAPSEC_UNDERWATER_127] = MAPPOPUP_THEME_STONE2,
    [MAPSEC_UNDERWATER_SOOTOPOLIS] = MAPPOPUP_THEME_STONE2,
    [MAPSEC_GRANITE_CAVE] = MAPPOPUP_THEME_STONE,
    [MAPSEC_MT_CHIMNEY] = MAPPOPUP_THEME_STONE,
    [MAPSEC_SAFARI_ZONE] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_BATTLE_FRONTIER] = MAPPOPUP_THEME_MARBLE,
    [MAPSEC_PETALBURG_WOODS] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_RUSTURF_TUNNEL] = MAPPOPUP_THEME_STONE,
    [MAPSEC_ABANDONED_SHIP] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_NEW_MAUVILLE] = MAPPOPUP_THEME_MARBLE,
    [MAPSEC_METEOR_FALLS] = MAPPOPUP_THEME_STONE,
    [MAPSEC_METEOR_FALLS2] = MAPPOPUP_THEME_STONE,
    [MAPSEC_MT_PYRE] = MAPPOPUP_THEME_STONE,
    [MAPSEC_AQUA_HIDEOUT_OLD] = MAPPOPUP_THEME_STONE,
    [MAPSEC_SHOAL_CAVE] = MAPPOPUP_THEME_STONE,
    [MAPSEC_SEAFLOOR_CAVERN] = MAPPOPUP_THEME_STONE,
    [MAPSEC_UNDERWATER_128] = MAPPOPUP_THEME_STONE2,
    [MAPSEC_VICTORY_ROAD] = MAPPOPUP_THEME_STONE,
    [MAPSEC_MIRAGE_ISLAND] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_CAVE_OF_ORIGIN] = MAPPOPUP_THEME_STONE,
    [MAPSEC_SOUTHERN_ISLAND] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_FIERY_PATH] = MAPPOPUP_THEME_STONE,
    [MAPSEC_FIERY_PATH2] = MAPPOPUP_THEME_STONE,
    [MAPSEC_JAGGED_PASS] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_JAGGED_PASS2] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_SEALED_CHAMBER] = MAPPOPUP_THEME_STONE,
    [MAPSEC_UNDERWATER_SEALED_CHAMBER] = MAPPOPUP_THEME_STONE2,
    [MAPSEC_SCORCHED_SLAB] = MAPPOPUP_THEME_STONE,
    [MAPSEC_ISLAND_CAVE] = MAPPOPUP_THEME_STONE,
    [MAPSEC_DESERT_RUINS] = MAPPOPUP_THEME_STONE,
    [MAPSEC_ANCIENT_TOMB] = MAPPOPUP_THEME_STONE,
    [MAPSEC_INSIDE_OF_TRUCK] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_SKY_PILLAR] = MAPPOPUP_THEME_STONE,
    [MAPSEC_SECRET_BASE] = MAPPOPUP_THEME_STONE,
    [MAPSEC_DYNAMIC] = MAPPOPUP_THEME_MARBLE,
    // FRLG
    [MAPSEC_PALLET_TOWN] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_VIRIDIAN_CITY] = MAPPOPUP_THEME_BRICK,
    [MAPSEC_PEWTER_CITY] = MAPPOPUP_THEME_BRICK,
    [MAPSEC_CERULEAN_CITY] = MAPPOPUP_THEME_BRICK,
    [MAPSEC_LAVENDER_TOWN] = MAPPOPUP_THEME_BRICK,
    [MAPSEC_VERMILION_CITY] = MAPPOPUP_THEME_BRICK,
    [MAPSEC_CELADON_CITY] = MAPPOPUP_THEME_MARBLE,
    [MAPSEC_FUCHSIA_CITY] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_CINNABAR_ISLAND] = MAPPOPUP_THEME_STONE2,
    [MAPSEC_INDIGO_PLATEAU] = MAPPOPUP_THEME_BRICK,
    [MAPSEC_SAFFRON_CITY] = MAPPOPUP_THEME_MARBLE,
    [MAPSEC_ROUTE_4_POKECENTER] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_ROUTE_10_POKECENTER] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_ROUTE_1] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_ROUTE_2] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_ROUTE_3] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_ROUTE_4] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_ROUTE_5] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_ROUTE_6] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_ROUTE_7] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_ROUTE_8] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_ROUTE_9] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_ROUTE_10] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_ROUTE_11] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_ROUTE_12] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_ROUTE_13] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_ROUTE_14] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_ROUTE_15] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_ROUTE_16] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_ROUTE_17] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_ROUTE_18] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_ROUTE_19] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_ROUTE_20] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_ROUTE_21] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_ROUTE_22] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_ROUTE_23] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_ROUTE_24] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_ROUTE_25] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_VIRIDIAN_FOREST] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_MT_MOON] = MAPPOPUP_THEME_STONE,
    [MAPSEC_S_S_ANNE] = MAPPOPUP_THEME_MARBLE,
    [MAPSEC_UNDERGROUND_PATH] = MAPPOPUP_THEME_STONE,
    [MAPSEC_UNDERGROUND_PATH_2] = MAPPOPUP_THEME_STONE,
    [MAPSEC_DIGLETTS_CAVE] = MAPPOPUP_THEME_STONE,
    [MAPSEC_KANTO_VICTORY_ROAD] = MAPPOPUP_THEME_STONE,
    [MAPSEC_ROCKET_HIDEOUT] = MAPPOPUP_THEME_STONE,
    [MAPSEC_SILPH_CO] = MAPPOPUP_THEME_STONE,
    [MAPSEC_POKEMON_MANSION] = MAPPOPUP_THEME_STONE,
    [MAPSEC_KANTO_SAFARI_ZONE] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_POKEMON_LEAGUE] = MAPPOPUP_THEME_BRICK,
    [MAPSEC_ROCK_TUNNEL] = MAPPOPUP_THEME_STONE,
    [MAPSEC_SEAFOAM_ISLANDS] = MAPPOPUP_THEME_STONE,
    [MAPSEC_POKEMON_TOWER] = MAPPOPUP_THEME_STONE,
    [MAPSEC_CERULEAN_CAVE] = MAPPOPUP_THEME_STONE,
    [MAPSEC_POWER_PLANT] = MAPPOPUP_THEME_STONE,
    [MAPSEC_ONE_ISLAND] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_TWO_ISLAND] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_THREE_ISLAND] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_FOUR_ISLAND] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_FIVE_ISLAND] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_SEVEN_ISLAND] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_SIX_ISLAND] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_KINDLE_ROAD] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_TREASURE_BEACH] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_CAPE_BRINK] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_BOND_BRIDGE] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_THREE_ISLE_PORT] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_RESORT_GORGEOUS] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_WATER_LABYRINTH] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_FIVE_ISLE_MEADOW] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_MEMORIAL_PILLAR] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_OUTCAST_ISLAND] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_GREEN_PATH] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_WATER_PATH] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_RUIN_VALLEY] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_TRAINER_TOWER] = MAPPOPUP_THEME_STONE,
    [MAPSEC_CANYON_ENTRANCE] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_SEVAULT_CANYON] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_TANOBY_RUINS] = MAPPOPUP_THEME_STONE,
    [MAPSEC_NAVEL_ROCK_FRLG] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_MT_EMBER] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_BERRY_FOREST] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_ICEFALL_CAVE] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_ROCKET_WAREHOUSE] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_TRAINER_TOWER_2] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_DOTTED_HOLE] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_LOST_CAVE] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_PATTERN_BUSH] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_ALTERING_CAVE_FRLG] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_TANOBY_CHAMBERS] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_THREE_ISLE_PATH] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_TANOBY_KEY] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_BIRTH_ISLAND_FRLG] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_MONEAN_CHAMBER] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_LIPTOO_CHAMBER] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_WEEPTH_CHAMBER] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_DILFORD_CHAMBER] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_SCUFIB_CHAMBER] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_RIXY_CHAMBER] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_VIAPOIS_CHAMBER] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_EMBER_SPA] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_SPECIAL_AREA] = MAPPOPUP_THEME_WOOD,
    [MAPSEC_ARTISAN_CAVE] = MAPPOPUP_THEME_WOOD,
};

static bool8 UNUSED StartMenu_ShowMapNamePopup(void)
{
    HideStartMenu();
    ShowMapNamePopupEmerald();
    return TRUE;
}

// States and data defines for Task_MapNamePopUpWindow
enum {
    STATE_SLIDE_IN,
    STATE_WAIT,
    STATE_SLIDE_OUT,
    STATE_UNUSED,
    STATE_ERASE,
    STATE_END,
    STATE_PRINT, // For some reason the first state is numerically last.
};

#define POPUP_OFFSCREEN_Y  40
#define POPUP_SLIDE_SPEED  2

#define tState         data[0]
#define tOnscreenTimer data[1]
#define tYOffset       data[2]
#define tIncomingPopUp data[3]
#define tPrintTimer    data[4]

void ShowMapNamePopupEmerald(void)
{
    if (FlagGet(FLAG_DONT_SHOW_MAP_NAME_POPUP))
        return;

    if (!FuncIsActiveTask(Task_MapNamePopUpWindow))
    {
        gPopupTaskId = CreateTask(Task_MapNamePopUpWindow, 90);
        SetGpuReg(REG_OFFSET_BG0VOFS, POPUP_OFFSCREEN_Y);
        gTasks[gPopupTaskId].tState = STATE_PRINT;
        gTasks[gPopupTaskId].tYOffset = POPUP_OFFSCREEN_Y;
    }
    else
    {
        // There's already a pop up window running.
        // Hurry the old pop up offscreen so the new one can appear.
        if (gTasks[gPopupTaskId].tState != STATE_SLIDE_OUT)
            gTasks[gPopupTaskId].tState = STATE_SLIDE_OUT;
        gTasks[gPopupTaskId].tIncomingPopUp = TRUE;
    }
}

static void Task_MapNamePopUpWindow(u8 taskId)
{
    struct Task *task = &gTasks[taskId];

    switch (task->tState)
    {
    case STATE_PRINT:
        // Wait, then create and print the pop up window
        if (++task->tPrintTimer > 30)
        {
            task->tState = STATE_SLIDE_IN;
            task->tPrintTimer = 0;
            ShowMapNamePopUpWindow();
        }
        break;
    case STATE_SLIDE_IN:
        // Slide the window onscreen.
        task->tYOffset -= POPUP_SLIDE_SPEED;
        if (task->tYOffset <= 0 )
        {
            task->tYOffset = 0;
            task->tState = STATE_WAIT;
            gTasks[gPopupTaskId].tOnscreenTimer = 0;
        }
        break;
    case STATE_WAIT:
        // Wait while the window is fully onscreen.
        if (++task->tOnscreenTimer > 120)
        {
            task->tOnscreenTimer = 0;
            task->tState = STATE_SLIDE_OUT;
        }
        break;
    case STATE_SLIDE_OUT:
        // Slide the window offscreen.
        task->tYOffset += POPUP_SLIDE_SPEED;
        if (task->tYOffset >= POPUP_OFFSCREEN_Y)
        {
            task->tYOffset = POPUP_OFFSCREEN_Y;
            if (task->tIncomingPopUp)
            {
                // A new pop up window is incoming,
                // return to the first state to show it.
                task->tState = STATE_PRINT;
                task->tPrintTimer = 0;
                task->tIncomingPopUp = FALSE;
            }
            else
            {
                task->tState = STATE_ERASE;
                return;
            }
        }
        break;
    case STATE_ERASE:
        ClearStdWindowAndFrame(GetMapNamePopUpWindowId(), TRUE);
        task->tState = STATE_END;
        break;
    case STATE_END:
        HideMapNamePopUpEmeraldWindow();
        return;
    }
    SetGpuReg(REG_OFFSET_BG0VOFS, task->tYOffset);
}

void HideMapNamePopUpEmeraldWindow(void)
{
    if (FuncIsActiveTask(Task_MapNamePopUpWindow))
    {
    #ifdef UBFIX
        if (GetMapNamePopUpWindowId() != WINDOW_NONE)
    #endif // UBFIX
        {
            ClearStdWindowAndFrame(GetMapNamePopUpWindowId(), TRUE);
            RemoveMapNamePopUpWindow();
        }

        SetGpuReg_ForcedBlank(REG_OFFSET_BG0VOFS, 0);
        DestroyTask(gPopupTaskId);
    }
}

static void ShowMapNamePopUpWindow(void)
{
    u8 mapDisplayHeader[24];
    u8 *withoutPrefixPtr;
    u8 x;

    withoutPrefixPtr = &(mapDisplayHeader[3]);
    GetMapName(withoutPrefixPtr, gMapHeader.regionMapSectionId, 0);
    AddMapNamePopUpWindow();
    LoadMapNamePopUpWindowBg();

    mapDisplayHeader[0] = EXT_CTRL_CODE_BEGIN;
    mapDisplayHeader[1] = EXT_CTRL_CODE_HIGHLIGHT;
    mapDisplayHeader[2] = TEXT_COLOR_TRANSPARENT;

    x = GetStringCenterAlignXOffset(FONT_NARROW, withoutPrefixPtr, 80);
    AddTextPrinterParameterized(GetMapNamePopUpWindowId(), FONT_NARROW, mapDisplayHeader, x, 3, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(GetMapNamePopUpWindowId(), COPYWIN_FULL);
}

#define TILE_TOP_EDGE_START 0x21D
#define TILE_TOP_EDGE_END   0x228
#define TILE_LEFT_EDGE_TOP  0x229
#define TILE_RIGHT_EDGE_TOP 0x22A
#define TILE_LEFT_EDGE_MID  0x22B
#define TILE_RIGHT_EDGE_MID 0x22C
#define TILE_LEFT_EDGE_BOT  0x22D
#define TILE_RIGHT_EDGE_BOT 0x22E
#define TILE_BOT_EDGE_START 0x22F
#define TILE_BOT_EDGE_END   0x23A

static void DrawMapNamePopUpFrame(u8 bg, u8 x, u8 y, u8 deltaX, u8 deltaY, u8 unused)
{
    s32 i;

    // Draw top edge
    for (i = 0; i < 1 + TILE_TOP_EDGE_END - TILE_TOP_EDGE_START; i++)
        FillBgTilemapBufferRect(bg, TILE_TOP_EDGE_START + i, i - 1 + x, y - 1, 1, 1, 14);

    // Draw sides
    FillBgTilemapBufferRect(bg, TILE_LEFT_EDGE_TOP,       x - 1,     y, 1, 1, 14);
    FillBgTilemapBufferRect(bg, TILE_RIGHT_EDGE_TOP, deltaX + x,     y, 1, 1, 14);
    FillBgTilemapBufferRect(bg, TILE_LEFT_EDGE_MID,       x - 1, y + 1, 1, 1, 14);
    FillBgTilemapBufferRect(bg, TILE_RIGHT_EDGE_MID, deltaX + x, y + 1, 1, 1, 14);
    FillBgTilemapBufferRect(bg, TILE_LEFT_EDGE_BOT,       x - 1, y + 2, 1, 1, 14);
    FillBgTilemapBufferRect(bg, TILE_RIGHT_EDGE_BOT, deltaX + x, y + 2, 1, 1, 14);

    // Draw bottom edge
    for (i = 0; i < 1 + TILE_BOT_EDGE_END - TILE_BOT_EDGE_START; i++)
        FillBgTilemapBufferRect(bg, TILE_BOT_EDGE_START + i, i - 1 + x, y + deltaY, 1, 1, 14);
}

static void LoadMapNamePopUpWindowBg(void)
{
    u8 popUpThemeId;
    u8 popupWindowId = GetMapNamePopUpWindowId();
    u16 regionMapSectionId = gMapHeader.regionMapSectionId;

    popUpThemeId = sRegionMapSectionId_To_PopUpThemeIdMapping[regionMapSectionId];
    LoadBgTiles(GetWindowAttribute(popupWindowId, WINDOW_BG), sMapPopUp_OutlineTable[popUpThemeId], 0x400, 0x21D);
    CallWindowFunction(popupWindowId, DrawMapNamePopUpFrame);
    PutWindowTilemap(popupWindowId);
    if (gMapHeader.weather == WEATHER_UNDERWATER_BUBBLES)
        LoadPalette(&sMapPopUp_Palette_Underwater, BG_PLTT_ID(14), sizeof(sMapPopUp_Palette_Underwater));
    else
        LoadPalette(sMapPopUp_PaletteTable[popUpThemeId], BG_PLTT_ID(14), sizeof(sMapPopUp_PaletteTable[0]));
    BlitBitmapToWindow(popupWindowId, sMapPopUp_Table[popUpThemeId], 0, 0, 80, 24);
}
