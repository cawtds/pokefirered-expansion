static const u8 sText_PokeSum_MetInATrade[] = _("{DYNAMIC 0x00} nature.\nMet in a trade.");
static const u8 sText_PokeSum_FatefulEncounterMet[] = _("{DYNAMIC 0x00} nature.\nMet in a fateful encounter when\nat {LV_2} {DYNAMIC 0x01}.");
static const u8 sText_PokeSum_Met[] = _("{DYNAMIC 0x00} nature.\nMet in {DYNAMIC 0x02} at {LV_2} {DYNAMIC 0x01}.");
static const u8 sText_PokeSum_ApparentlyMet[] = _("{DYNAMIC 0x00} nature.\nApparently met in {DYNAMIC 0x02}\nat {LV_2} {DYNAMIC 0x01}.");
static const u8 sText_PokeSum_Hatched[] = _("{DYNAMIC 0x00} nature.\nHatched: {DYNAMIC 0x02}\nat {LV_2} {DYNAMIC 0x01}.");
static const u8 sText_PokeSum_FatefulEncounterHatched[] = _("{DYNAMIC 0x00} nature. Met in a fateful\nencounter (hatched: {DYNAMIC 0x02}\nat {LV_2} {DYNAMIC 0x01}).");
static const u8 sText_PokeSum_ApparentlyFatefulEncounterHatched[] = _("{DYNAMIC 0x00} nature. Apparently met in\na fateful encounter (hatched:\n{DYNAMIC 0x02} at {LV_2} {DYNAMIC 0x01}).");
static const u8 sText_PokeSum_ATrade[] = _("a trade");
static const u8 gText_Somewhere[] = _("Somewhere");
static const u8 sText_PokeSum_PageName_PokemonInfo[] = _("POKéMON INFO");
static const u8 sText_PokeSum_PageName_PokemonSkills[] = _("POKéMON SKILLS");
static const u8 sText_PokeSum_PageName_KnownMoves[] = _("KNOWN MOVES");
static const u8 sText_PokeSum_NoData[] = _("No data");
static const u8 sText_PokeSum_ExpPoints[] = _("EXP. POINTS");
static const u8 sText_PokeSum_NextLv[] = _("NEXT LV.");
static const u8 sText_PokeSum_Controls_PageCancel[] = _("{DPAD_RIGHT}PAGE {A_BUTTON}CANCEL");
static const u8 sText_PokeSum_Controls_Cancel[] = _("{A_BUTTON}CANCEL");
static const u8 sText_PokeSum_Controls_Page[] = _("{DPAD_LEFTRIGHT}PAGE");
static const u8 sText_PokeSum_Controls_PageStats[] = _("{DPAD_LEFTRIGHT}PAGE {A_BUTTON}STATS");
static const u8 sText_PokeSum_Controls_PageIVs[] = _("{DPAD_LEFTRIGHT}PAGE {A_BUTTON}IVs");
static const u8 sText_PokeSum_Controls_PageEVs[] = _("{DPAD_LEFTRIGHT}PAGE {A_BUTTON}EVs");
static const u8 sText_PokeSum_Controls_PageDetail[] = _("{DPAD_LEFT}PAGE {A_BUTTON}DETAIL");
static const u8 sText_PokeSum_Controls_PickSwitch[] = _("{DPAD_UPDOWN}PICK {A_BUTTON}SWITCH");
static const u8 sText_PokeSum_Controls_Pick[] = _("{DPAD_UPDOWN}PICK");
static const u8 sText_PokeSum_Controls_PickDelete[] = _("{DPAD_UPDOWN}PICK {A_BUTTON}DELETE");
static const u8 sText_PokeSum_HmMovesCantBeForgotten[] = _("HM moves can't be\nforgotten now.");
static const u8 sText_PokeSum_PP[] = _("{PP}");
static const u8 sText_PokeSum_OneHyphen[] = _("-");
static const u8 sText_PokeSum_TwoHyphens[] = _("--");
static const u8 sText_PokeSum_DexNoUnknown[] = _("???");
static const u8 sText_ColorBlue[] = _("{COLOR 7}");
static const u8 sText_ColorRed[] = _("{COLOR 1}");

static const u16 gSummaryScreen_Bg_Pal[] = INCBIN_U16("graphics/summary_screen/bg.gbapal");
static const u32 gSummaryScreen_ExpBar_Gfx[] = INCBIN_U32("graphics/summary_screen/exp_bar.4bpp.smol");
static const u32 gSummaryScreen_HpBar_Gfx[] = INCBIN_U32("graphics/summary_screen/hp_bar.4bpp.smol");
static const u16 gSummaryScreen_HpExpBar_Pal[] = INCBIN_U16("graphics/summary_screen/exp_bar.gbapal"); // hp bar uses this too
static const u32 gSummaryScreen_PageInfo_Tilemap[] = INCBIN_U32("graphics/summary_screen/page_info.bin.smolTM");
#if P_SUMMARY_SCREEN_IV_EV_TILESET == TRUE
static const u32 gSummaryScreen_PageSkills_Tilemap[] = INCBIN_U32("graphics/summary_screen/page_skills_ev_iv.bin.smolTM");
static const u32 gSummaryScreen_Bg_Gfx[] = INCBIN_U32("graphics/summary_screen/bg_ev_iv.4bpp.smol");
#else
static const u32 gSummaryScreen_PageSkills_Tilemap[] = INCBIN_U32("graphics/summary_screen/page_skills.bin.smolTM");
static const u32 gSummaryScreen_Bg_Gfx[] = INCBIN_U32("graphics/summary_screen/bg.4bpp.smol");
#endif
static const u32 gSummaryScreen_PageMoves_Tilemap[] = INCBIN_U32("graphics/summary_screen/page_moves.bin.smolTM");
static const u32 gSummaryScreen_PageMovesInfo_Tilemap[] = INCBIN_U32("graphics/summary_screen/page_moves_info.bin.smolTM");
static const u32 gSummaryScreen_PageEgg_Tilemap[] = INCBIN_U32("graphics/summary_screen/page_egg.bin.smolTM");
static const u16 gSummaryScreen_StatusAilmentIcon_Pal[] = INCBIN_U16("graphics/summary_screen/status_ailment_icons.gbapal");
static const u32 gSummaryScreen_StatusAilmentIcon_Gfx[] = INCBIN_U32("graphics/summary_screen/status_ailment_icons.4bpp.smol");
static const u16 sPokeSummary_HpBarPalYellow[] = INCBIN_U16("graphics/summary_screen/hp_bar_yellow.gbapal");
static const u16 sPokeSummary_HpBarPalRed[] = INCBIN_U16("graphics/summary_screen/hp_bar_red.gbapal");
static const u32 sTextHeaderPalette[] = INCBIN_U32("graphics/summary_screen/text_header.gbapal");
static const u16 sMonMarkingSpritePalette[] = INCBIN_U16("graphics/summary_screen/marking.gbapal");
static const u32 sTextMovesPalette[] = INCBIN_U32("graphics/summary_screen/text_moves.gbapal");
static const u16 sMoveSelectionCursorPals[] = INCBIN_U16("graphics/summary_screen/move_selection_cursor.gbapal");
static const u32 sMoveSelectionCursorTiles_Left[] = INCBIN_U32("graphics/summary_screen/move_selection_cursor_left.4bpp.smol");
static const u32 sMoveSelectionCursorTiles_Right[] = INCBIN_U32("graphics/summary_screen/move_selection_cursor_right.4bpp.smol");
static const u16 sPokerusIconObjPal[] = INCBIN_U16("graphics/summary_screen/pokerus_cured.gbapal");
static const u32 sPokerusIconObjTiles[] = INCBIN_U32("graphics/summary_screen/pokerus_cured.4bpp.smol");
static const u16 sStarObjPal[] = INCBIN_U16( "graphics/summary_screen/shiny_star.gbapal");
static const u32 sStarObjTiles[] = INCBIN_U32( "graphics/summary_screen/shiny_star.4bpp.smol");
static const u32 sBgTilemap_MovesInfoPage[] = INCBIN_U32( "graphics/summary_screen/moves_info_page.bin.smolTM");
static const u32 sBgTilemap_MovesPage[] = INCBIN_U32( "graphics/summary_screen/moves_page.bin.smolTM");


static const struct OamData sMoveSelectionCursorOamData =
{
    .y = 0,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(64x32),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(64x32),
    .tileNum = 0,
    .priority = 0,
    .paletteNum = 0
};

static const union AnimCmd sMoveSelectionCursorOamAnim_Red[] =
{
    ANIMCMD_FRAME(0, 20),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd sMoveSelectionCursorOamAnim_Blue[] =
{
    ANIMCMD_FRAME(0x20, 20),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd *const sMoveSelectionCursorOamAnimTable[] =
{
    sMoveSelectionCursorOamAnim_Red,
    sMoveSelectionCursorOamAnim_Blue
};

static const struct OamData sStatusAilmentIconOamData = {
    .y = 0,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(32x8),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(32x8),
    .tileNum = 0,
    .priority = 0,
    .paletteNum = 0
};

static const union AnimCmd sStatusAilmentIconAnim_PSN[] =
{
    ANIMCMD_FRAME(0, 20),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd sStatusAilmentIconAnim_PRZ[] =
{
    ANIMCMD_FRAME(4, 20),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd sStatusAilmentIconAnim_SLP[] =
{
    ANIMCMD_FRAME(8, 20),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd sStatusAilmentIconAnim_FRZ[] =
{
    ANIMCMD_FRAME(12, 20),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd sStatusAilmentIconAnim_BRN[] =
{
    ANIMCMD_FRAME(16, 20),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd sStatusAilmentIconAnim_PKRS[] =
{
    ANIMCMD_FRAME(20, 20),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd sStatusAilmentIconAnim_FNT[] =
{
    ANIMCMD_FRAME(24, 20),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd sStatusAilmentIconAnim_Blank[] =
{
    ANIMCMD_FRAME(28, 20),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd *const sStatusAilmentIconAnimTable[] =
{
    sStatusAilmentIconAnim_PSN,
    sStatusAilmentIconAnim_PRZ,
    sStatusAilmentIconAnim_SLP,
    sStatusAilmentIconAnim_FRZ,
    sStatusAilmentIconAnim_BRN,
    sStatusAilmentIconAnim_PKRS,
    sStatusAilmentIconAnim_FNT,
    sStatusAilmentIconAnim_Blank
};

static const struct OamData sHpOrExpBarOamData = {
    .y = 0,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(8x8),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(8x8),
    .tileNum = 0,
    .priority = 0,
    .paletteNum = 0
};

static const union AnimCmd sHpOrExpAnim_0[] =
{
    ANIMCMD_FRAME(0, 20),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd sHpOrExpAnim_1[] =
{
    ANIMCMD_FRAME(1, 20),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd sHpOrExpAnim_2[] =
{
    ANIMCMD_FRAME(2, 20),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd sHpOrExpAnim_3[] =
{
    ANIMCMD_FRAME(3, 20),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd sHpOrExpAnim_4[] =
{
    ANIMCMD_FRAME(4, 20),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd sHpOrExpAnim_5[] =
{
    ANIMCMD_FRAME(5, 20),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd sHpOrExpAnim_6[] =
{
    ANIMCMD_FRAME(6, 20),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd sHpOrExpAnim_7[] =
{
    ANIMCMD_FRAME(7, 20),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd sHpOrExpAnim_8[] =
{
    ANIMCMD_FRAME(8, 20),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd sHpOrExpAnim_9[] =
{
    ANIMCMD_FRAME(9, 20),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd sHpOrExpAnim_10[] =
{
    ANIMCMD_FRAME(10, 20),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd sHpOrExpAnim_11[] =
{
    ANIMCMD_FRAME(11, 20),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd *const sHpOrExpBarAnimTable[] =
{
    sHpOrExpAnim_0,
    sHpOrExpAnim_1,
    sHpOrExpAnim_2,
    sHpOrExpAnim_3,
    sHpOrExpAnim_4,
    sHpOrExpAnim_5,
    sHpOrExpAnim_6,
    sHpOrExpAnim_7,
    sHpOrExpAnim_8,
    sHpOrExpAnim_9,
    sHpOrExpAnim_10,
    sHpOrExpAnim_11
};

static const struct OamData sPokerusIconObjOamData = {
    .y = 0,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(8x8),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(8x8),
    .tileNum = 0,
    .priority = 0,
    .paletteNum = 0
};

static const union AnimCmd sPokerusIconObjAnim0[] =
{
    ANIMCMD_FRAME(0, 20),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd *const sPokerusIconObjAnimTable[] =
{
    sPokerusIconObjAnim0
};

static const struct OamData sStarObjOamData =
{
    .y = 0,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(8x8),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(8x8),
    .tileNum = 0,
    .priority = 0,
    .paletteNum = 0
};

static const union AnimCmd sStarObjAnim0[] =
{
    ANIMCMD_FRAME(1, 20),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd *const sStarObjAnimTable[] =
{
    sStarObjAnim0
};

static const u8 *const sEggHatchTimeTexts[] =
{
    [EGG_HATCH_TIME_LONG]         = COMPOUND_STRING("It looks like this\nEGG will take a\nlong time to hatch."),
    [EGG_HATCH_TIME_SOME]         = COMPOUND_STRING("What will hatch\nfrom this? It will\ntake some time."),
    [EGG_HATCH_TIME_SOON]         = COMPOUND_STRING("It occasionally\nmoves. It should\nhatch soon."),
    [EGG_HATCH_TIME_ALMOST_READY] = COMPOUND_STRING("It's making sounds.\nIt's almost ready\nto hatch!"),
};

static const u8 *const sEggOriginTexts[] =
{
    [EGG_ORIGIN_DAYCARE]       = COMPOUND_STRING("An odd POKéMON EGG found by the\nDAY-CARE couple."),
    [EGG_ORIGIN_TRADE]         = COMPOUND_STRING("A peculiar POKéMON EGG obtained\nin a trade."),
    [EGG_ORIGIN_TRAVELING_MAN] = COMPOUND_STRING("An odd POKéMON EGG obtained from\nthe traveling man."),
    [EGG_ORIGIN_NICE_PLACE]    = COMPOUND_STRING("A wondrously peculiar POKéMON EGG\nobtained at a nice place."),
    [EGG_ORIGIN_SPA]           = COMPOUND_STRING("An odd POKéMON EGG obtained from\nan old woman at the SPA."),
};

static const u8 sPrintMoveTextColors[][3] = {
    {0, 7, 8},
    {0, 1, 2},
    {0, 3, 4},
    {0, 5, 6}
};

static const struct BgTemplate sBgTempaltes[] =
{
	 {
	 	.bg = 0,
	 	.charBaseIndex = 0,
	 	.mapBaseIndex = 14,
	 	.screenSize = 1,
	 	.paletteMode = 0,
	 	.priority = 0,
	 	.baseTile = 0x0000
	 },
	 {
	 	.bg = 2,
	 	.charBaseIndex = 2,
	 	.mapBaseIndex = 10,
	 	.screenSize = 1,
	 	.paletteMode = 0,
	 	.priority = 1,
	 	.baseTile = 0x0000
	 },
	 {
	 	.bg = 3,
	 	.charBaseIndex = 2,
	 	.mapBaseIndex = 9,
	 	.screenSize = 0,
	 	.paletteMode = 0,
	 	.priority = 3,
	 	.baseTile = 0x0000
	 },
	 {
	 	.bg = 1,
	 	.charBaseIndex = 2,
	 	.mapBaseIndex = 12,
	 	.screenSize = 1,
	 	.paletteMode = 0,
	 	.priority = 2,
	 	.baseTile = 0x0000
	 }
};

static const struct WindowTemplate sWindowTemplates_Permanent_Bg1[] =
{
    [PSS_WIN_PAGE_NAME] =
    {
        .bg = 1,
        .tilemapLeft = 0,
        .tilemapTop = 0,
        .width = 13,
        .height = 2,
        .paletteNum = 7,
        .baseBlock = 0x0258
    },
    [PSS_WIN_CONTROLS] =
    {
        .bg = 1,
        .tilemapLeft = 19,
        .tilemapTop = 0,
        .width = 11,
        .height = 2,
        .paletteNum = 7,
        .baseBlock = 0x0272
    },
    [PSS_WIN_LVL_NICK] =
    {
        .bg = 1,
        .tilemapLeft = 0,
        .tilemapTop = 2,
        .width = 15,
        .height = 2,
        .paletteNum = 7,
        .baseBlock = 0x0288
    }
};

static const struct WindowTemplate sWindowTemplates_Permanent_Bg2[] =
{
    [PSS_WIN_PAGE_NAME] =
    {
        .bg = 2,
        .tilemapLeft = 0,
        .tilemapTop = 0,
        .width = 13,
        .height = 2,
        .paletteNum = 7,
        .baseBlock = 0x0258
    },
    [PSS_WIN_CONTROLS] =
    {
        .bg = 2,
        .tilemapLeft = 19,
        .tilemapTop = 0,
        .width = 11,
        .height = 2,
        .paletteNum = 7,
        .baseBlock = 0x0272
    },
    [PSS_WIN_LVL_NICK] =
    {
        .bg = 2,
        .tilemapLeft = 0,
        .tilemapTop = 2,
        .width = 15,
        .height = 2,
        .paletteNum = 7,
        .baseBlock = 0x0288
    },
};

static const struct WindowTemplate sWindowTemplates_Info[SUMMARY_SECONDARY_WIN_COUNT] =
{
    [PSS_SECONDARY_WIN_1 - SUMMARY_PRIMARY_WIN_COUNT] =
    {
        .bg = 0,
        .tilemapLeft = 15,
        .tilemapTop = 2,
        .width = 15,
        .height = 12,
        .paletteNum = 6,
        .baseBlock = 0x0001
    },
    [PSS_SECONDARY_WIN_2 - SUMMARY_PRIMARY_WIN_COUNT] =
    {
        .bg = 0,
        .tilemapLeft = 1,
        .tilemapTop = 14,
        .width = 28,
        .height = 6,
        .paletteNum = 6,
        .baseBlock = 0x00b5
    },
    [PSS_SECONDARY_WIN_3 - SUMMARY_PRIMARY_WIN_COUNT] =
    {
        .bg = 0,
        .tilemapLeft = 0,
        .tilemapTop = 0,
        .width = 0,
        .height = 0,
        .paletteNum = 0,
        .baseBlock = 0x0000
    },
    [PSS_SECONDARY_WIN_4 - SUMMARY_PRIMARY_WIN_COUNT] =
    {
        .bg = 0,
        .tilemapLeft = 0,
        .tilemapTop = 0,
        .width = 0,
        .height = 0,
        .paletteNum = 0,
        .baseBlock = 0x0000
    },
};

static const struct WindowTemplate sWindowTemplates_Skills[SUMMARY_SECONDARY_WIN_COUNT] =
{
    [PSS_SECONDARY_WIN_1 - SUMMARY_PRIMARY_WIN_COUNT] =
    {
        .bg = 0,
        .tilemapLeft = 20,
        .tilemapTop = 2,
        .width = 10,
        .height = 14,
        .paletteNum = 6,
        .baseBlock = 0x0001
    },
    [PSS_SECONDARY_WIN_2 - SUMMARY_PRIMARY_WIN_COUNT] =
    {
        .bg = 0,
        .tilemapLeft = 6,
        .tilemapTop = 12,
        .width = 14,
        .height = 4,
        .paletteNum = 6,
        .baseBlock = 0x008d
    },
    [PSS_SECONDARY_WIN_3 - SUMMARY_PRIMARY_WIN_COUNT] =
    {
        .bg = 0,
        .tilemapLeft = 1,
        .tilemapTop = 16,
        .width = 29,
        .height = 4,
        .paletteNum = 6,
        .baseBlock = 0x00c5
    },
    [PSS_SECONDARY_WIN_4 - SUMMARY_PRIMARY_WIN_COUNT] =
    {
        .bg = 0,
        .tilemapLeft = 0,
        .tilemapTop = 0,
        .width = 0,
        .height = 0,
        .paletteNum = 0,
        .baseBlock = 0x0000
    },
};

static const struct WindowTemplate sWindowTemplates_Moves[SUMMARY_SECONDARY_WIN_COUNT] =
{
    [PSS_SECONDARY_WIN_1 - SUMMARY_PRIMARY_WIN_COUNT] =
    {
        .bg = 0,
        .tilemapLeft = 20,
        .tilemapTop = 2,
        .width = 10,
        .height = 18,
        .paletteNum = 8,
        .baseBlock = 0x0001
    },
    [PSS_SECONDARY_WIN_2 - SUMMARY_PRIMARY_WIN_COUNT] =
    {
        .bg = 0,
        .tilemapLeft = 0,
        .tilemapTop = 7,
        .width = 15,
        .height = 13,
        .paletteNum = 6,
        .baseBlock = 0x00b5
    },
    [PSS_SECONDARY_WIN_3 - SUMMARY_PRIMARY_WIN_COUNT] =
    {
        .bg = 0,
        .tilemapLeft = 15,
        .tilemapTop = 2,
        .width = 5,
        .height = 18,
        .paletteNum = 6,
        .baseBlock = 0x0178
    },
    [PSS_SECONDARY_WIN_4 - SUMMARY_PRIMARY_WIN_COUNT] =
    {
        .bg = 0,
        .tilemapLeft = 6,
        .tilemapTop = 4,
        .width = 9,
        .height = 2,
        .paletteNum = 6,
        .baseBlock = 0x01d2
    },
};

static const struct WindowTemplate sWindowTemplates_Dummy[] =
{
    {
        .bg = 255,
        .tilemapLeft = 0,
        .tilemapTop = 0,
        .width = 0,
        .height = 0,
        .paletteNum = 0,
        .baseBlock = 0x0000
    },
};


static const u8 sLevelNickTextColors[][3] =
{
    {0, 14, 10},
    {0, 1, 2},
    {0, 9, 8},
    {0, 5, 4},
    {0, 2, 3},
    {0, 11, 10},
};

static const u8 ALIGNED(4) sMultiBattlePartyOrder[] =
{
    0, 2, 3, 1, 4, 5
};

static const s8 sMonPicBounceYDelta_Under60[] =
{
    -1, 0, 1
};

static const s8 sMonPicBounceYDelta_60to80[] =
{
    -2, -1, 0, 1, 2
};

static const s8 sMonPicBounceYDelta_80to99[] =
{
    -3, -2, -1, 0, 1, 2, 3
};

static const s8 sMonPicBounceYDelta_Full[] =
{
    -5, -3, -1, 0, 1, 3, 5
};

static const s8 sEggPicShakeXDelta_ItWillTakeSomeTime[] =
{
    1, 1, 0, -1, -1, 0, -1, -1, 0, 1, 1
};

static const s8 sEggPicShakeXDelta_OccasionallyMoves[] =
{
    2, 1, 0, -1, -2, 0, -2, -1, 0, 1, 2
};

static const s8 sEggPicShakeXDelta_AlmostReadyToHatch[] =
{
    2, 1, 1, 0, -1, -1, -2, 0, -2, -1, -1, 0, 1, 1, 2
};

static const u16 *const sHpBarPals[] =
{
    gSummaryScreen_HpExpBar_Pal,
    sPokeSummary_HpBarPalYellow,
    sPokeSummary_HpBarPalRed,
};

static const struct OamData sOamData_CategoryIcons =
{
    .size = SPRITE_SIZE(16x16),
    .shape = SPRITE_SHAPE(16x16),
    .priority = 0,
};

const struct CompressedSpriteSheet gSpriteSheet_CategoryIcons =
{
    .data = gCategoryIcons_Gfx,
    .size = 16*16*3/2,
    .tag = TAG_CATEGORY_ICONS,
};

const struct SpritePalette gSpritePal_CategoryIcons =
{
    .data = gCategoryIcons_Pal,
    .tag = TAG_CATEGORY_ICONS
};

static const union AnimCmd sSpriteAnim_CategoryIcon0[] =
{
    ANIMCMD_FRAME(0, 0),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_CategoryIcon1[] =
{
    ANIMCMD_FRAME(4, 0),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_CategoryIcon2[] =
{
    ANIMCMD_FRAME(8, 0),
    ANIMCMD_END
};

static const union AnimCmd *const sSpriteAnimTable_CategoryIcons[] =
{
    sSpriteAnim_CategoryIcon0,
    sSpriteAnim_CategoryIcon1,
    sSpriteAnim_CategoryIcon2,
};

const struct SpriteTemplate gSpriteTemplate_CategoryIcons =
{
    .tileTag = TAG_CATEGORY_ICONS,
    .paletteTag = TAG_CATEGORY_ICONS,
    .oam = &sOamData_CategoryIcons,
    .anims = sSpriteAnimTable_CategoryIcons,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy
};
