#include "global.h"
#include "event_data.h"
#include "event_scripts.h"
#include "field_effect.h"
#include "field_specials.h"
#include "item_icon.h"
#include "list_menu.h"
#include "malloc.h"
#include "menu.h"
#include "palette.h"
#include "quest_log.h"
#include "script_menu.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "util.h"
#include "constants/menu.h"
#include "constants/seagallop.h"
#include "constants/songs.h"

#define GFXTAG_FOSSIL 7000

struct DynamicListMenuEventArgs
{
    struct ListMenuTemplate *list;
    u16 selectedItem;
    u8 windowId;
};

typedef void (*DynamicListCallback)(struct DynamicListMenuEventArgs *eventArgs);

struct DynamicListMenuEventCollection
{
    DynamicListCallback OnInit;
    DynamicListCallback OnSelectionChanged;
    DynamicListCallback OnDestroy;
};

static EWRAM_DATA u8 sProcessInputDelay = 0;
static EWRAM_DATA u8 sDynamicMenuEventId = 0;
static EWRAM_DATA struct DynamicMultichoiceStack *sDynamicMultiChoiceStack = NULL;
static EWRAM_DATA u16 *sDynamicMenuEventScratchPad = NULL;

static bool8 IsPicboxClosed(void);
static u16 GetStringTilesWide(const u8 *str);
static u32 GetMultiChoiceWindowHeight(u8 argc, u8 maxBeforeScroll);
static u8 CreateWindowFromRect(u8 left, u8 top, u8 width, u8 height);
static u8 GetMCWindowHeight(u8 count);
static u8 GetMenuWidthFromList(const struct MenuAction *items, u8 count);
static void ClearToTransparentAndRemoveWindow(u8 windowId);
static void CreatePCMultichoice(void);
static void DrawLinkServicesMultichoiceMenu(enum MultichoiceID mcId);
static void DrawMultichoiceMenu(u8 left, u8 top, enum MultichoiceID mcId, u8 ignoreBpress, u8 initPos);
static void DrawMultichoiceMenuDynamic(u8 left, u8 top, u8 argc, struct ListMenuItem *items, bool8 ignoreBPress, u32 initialRow, u8 maxBeforeScroll, u32 callbackSet);
static void FreeListMenuItems(struct ListMenuItem *items, u32 count);
static void InitMultichoiceCheckWrap(u8 ignoreBpress, u8 count, u8 windowId, enum MultichoiceID mcId);
static void MultichoiceDynamicEventDebug_OnDestroy(struct DynamicListMenuEventArgs *eventArgs);
static void MultichoiceDynamicEventDebug_OnInit(struct DynamicListMenuEventArgs *eventArgs);
static void MultichoiceDynamicEventDebug_OnSelectionChanged(struct DynamicListMenuEventArgs *eventArgs);
static void MultichoiceDynamicEventShowItem_OnDestroy(struct DynamicListMenuEventArgs *eventArgs);
static void MultichoiceDynamicEventShowItem_OnInit(struct DynamicListMenuEventArgs *eventArgs);
static void MultichoiceDynamicEventShowItem_OnSelectionChanged(struct DynamicListMenuEventArgs *eventArgs);
static void Task_HandleMultichoiceGridInput(u8 taskId);
static void Task_HandleMultichoiceInput(u8 taskId);
static void Task_HandleMultichoiceInput(u8 taskId);
static void Task_HandleScrollingMultichoiceInput(u8 taskId);
static void Task_HandleYesNoInput(u8 taskId);

static const u8 sText_MultiLink[] = _("MULTI-LINK");
static const u8 sText_Opponent[] = _("OPPONENT");
static const u8 sText_Tourney_Tree[] = _("TOURNEY TREE");
static const u8 sText_ReadyToStart[] = _("READY TO START");
static const u8 sText_Eggs[] = _("EGGS");
static const u8 sText_Victories[] = _("VICTORIES");
static const u8 sText_TradeCenter[] = _("TRADE CENTER");
static const u8 sText_Colosseum[] = _("COLOSSEUM");
static const u8 sText_GoOn[] = _("GO ON");
static const u8 sText_HelixFossil[] = _("HELIX FOSSIL");
static const u8 sText_DomeFossil[] = _("DOME FOSSIL");
static const u8 sText_OldAmber[] = _("OLD AMBER");
static const u8 sText_FreshWater[] = _("FRESH WATER");
static const u8 sText_SodaPop[] = _("SODA POP");
static const u8 sText_Lemonade[] = _("LEMONADE");
static const u8 sText_Vermilion[] = _("VERMILION");
static const u8 sText_OneIsland[] = _("ONE ISLAND");
static const u8 sText_TwoIsland[] = _("TWO ISLAND");
static const u8 sText_ThreeIsland[] = _("THREE ISLAND");
static const u8 sText_SeviiIslands[] = _("SEVII ISLANDS");
static const u8 sText_NavelRock[] = _("NAVEL ROCK");
static const u8 sText_BirthIsland[] = _("BIRTH ISLAND");
static const u8 sText_Retire[] = _("RETIRE");
static const u8 sText_Other[] = _("OTHER");
static const u8 sText_NoThanks[] = _("NO THANKS");
static const u8 sText_Quit[] = _("QUIT");

static const struct DynamicListMenuEventCollection sDynamicListMenuEventCollections[] =
{
    [DYN_MULTICHOICE_CB_DEBUG] =
    {
        .OnInit = MultichoiceDynamicEventDebug_OnInit,
        .OnSelectionChanged = MultichoiceDynamicEventDebug_OnSelectionChanged,
        .OnDestroy = MultichoiceDynamicEventDebug_OnDestroy
    },
    [DYN_MULTICHOICE_CB_SHOW_ITEM] =
    {
        .OnInit = MultichoiceDynamicEventShowItem_OnInit,
        .OnSelectionChanged = MultichoiceDynamicEventShowItem_OnSelectionChanged,
        .OnDestroy = MultichoiceDynamicEventShowItem_OnDestroy
    }
};

static const struct ListMenuTemplate sScriptableListMenuTemplate =
{
    .item_X = 8,
    .upText_Y = 1,
    .cursorPal = 2,
    .fillValue = 1,
    .cursorShadowPal = 3,
    .lettersSpacing = 1,
    .scrollMultiple = LIST_NO_MULTIPLE_SCROLL,
    .fontId = FONT_NORMAL,
};

static const struct MenuAction sMultichoiceList_YesNo[] = {
    { gText_Yes },
    { gText_No }
};

static const struct MenuAction sMultichoiceList_TrainerCardIconTint[] = {
    { COMPOUND_STRING("NORMAL") },
    { COMPOUND_STRING("BLACK") },
    { gText_Pink },
    { COMPOUND_STRING("SEPIA") }
};

static const struct MenuAction sMultichoiceList_HOF_Quit[] = {
    { gText_HallOfFame },
    { sText_Quit }
};

static const struct MenuAction sMultichoiceList_Eggs_Quit[] = {
    { sText_Eggs },
    { sText_Quit }
};

static const struct MenuAction sMultichoiceList_Victories_Quit[] = {
    { sText_Victories },
    { sText_Quit }
};

static const struct MenuAction sMultichoiceList_HOF_Eggs_Quit[] = {
    { gText_HallOfFame },
    { sText_Eggs },
    { sText_Quit }
};

static const struct MenuAction sMultichoiceList_HOF_Victories_Quit[] = {
    { gText_HallOfFame },
    { sText_Victories },
    { sText_Quit }
};

static const struct MenuAction sMultichoiceList_Eggs_Victories_Quit[] = {
    { sText_Eggs },
    { sText_Victories },
    { sText_Quit }
};

static const struct MenuAction sMultichoiceList_HOF_Eggs_Victories_Quit[] = {
    { gText_HallOfFame },
    { sText_Eggs },
    { sText_Victories },
    { sText_Quit }
};


static const struct MenuAction sMultichoiceList_TrainerSchoolWhiteboard[] = {
    { COMPOUND_STRING("SLP") },
    { COMPOUND_STRING("PSN") },
    { COMPOUND_STRING("PAR") },
    { COMPOUND_STRING("BRN") },
    { COMPOUND_STRING("FRZ") },
    { gOtherText_Exit }
};

static const struct MenuAction sMultichoiceList_YesNoInfo[] = {
    { gText_Yes },
    { gText_No },
    { gText_Info }
};

static const struct MenuAction sMultichoiceList_SingleDoubleMultiInfoExit[] = {
    { gText_SingleBattle },
    { gText_DoubleBattle },
    { gText_MultiBattle },
    { gText_Info },
    { gOtherText_Exit }
};

static const struct MenuAction sMultichoiceList_ChallengeInfoExit[] = {
    { COMPOUND_STRING("Make a challenge.") },
    { gText_Info },
    { gOtherText_Exit }
};

static const struct MenuAction sMultichoiceList_RooftopB1F[] = {
    { gText_Rooftop },
    { gText_B1F },
    { gOtherText_Exit }
};

static const struct MenuAction sMultichoiceList_Helix[] = {
    { sText_HelixFossil },
    { gOtherText_Exit }
};

static const struct MenuAction sMultichoiceList_Dome[] = {
    { sText_DomeFossil },
    { gOtherText_Exit }
};

static const struct MenuAction sMultichoiceList_Amber[] = {
    { sText_OldAmber },
    { gOtherText_Exit }
};

static const struct MenuAction sMultichoiceList_HelixAmber[] = {
    { sText_HelixFossil },
    { sText_OldAmber },
    { gOtherText_Exit }
};

static const struct MenuAction sMultichoiceList_DomeAmber[] = {
    { sText_DomeFossil },
    { sText_OldAmber },
    { gOtherText_Exit }
};

static const struct MenuAction sMultichoiceList_CeladonVendingMachine[] = {
    { COMPOUND_STRING("FRESH WATER{CLEAR_TO 0x57}{FONT_SMALL}¥200") },
    { COMPOUND_STRING("SODA POP{CLEAR_TO 0x57}{FONT_SMALL}¥300") },
    { COMPOUND_STRING("LEMONADE{CLEAR_TO 0x57}{FONT_SMALL}¥350") },
    { gOtherText_Exit }
};

static const struct MenuAction sMultichoiceList_GameCornerTMPrizes[] = {
    { COMPOUND_STRING("TM13{CLEAR_TO 0x48}{FONT_SMALL}4,000 COINS") },
    { COMPOUND_STRING("TM23{CLEAR_TO 0x48}{FONT_SMALL}3,500 COINS") },
    { COMPOUND_STRING("TM24{CLEAR_TO 0x48}{FONT_SMALL}4,000 COINS") },
    { COMPOUND_STRING("TM30{CLEAR_TO 0x48}{FONT_SMALL}4,500 COINS") },
    { COMPOUND_STRING("TM35{CLEAR_TO 0x48}{FONT_SMALL}4,000 COINS") },
    { sText_NoThanks }
};

static const struct MenuAction sMultichoiceList_GameCornerBattleItemPrizes[] = {
    { COMPOUND_STRING("SMOKE BALL{CLEAR_TO 0x5A}{FONT_SMALL}800 COINS") },
    { COMPOUND_STRING("MIRACLE SEED{CLEAR_TO 0x50}{FONT_SMALL}1,000 COINS") },
    { COMPOUND_STRING("CHARCOAL{CLEAR_TO 0x50}{FONT_SMALL}1,000 COINS") },
    { COMPOUND_STRING("MYSTIC WATER{CLEAR_TO 0x50}{FONT_SMALL}1,000 COINS") },
    { COMPOUND_STRING("YELLOW FLUTE{CLEAR_TO 0x50}{FONT_SMALL}1,600 COINS") },
    { sText_NoThanks }
};

static const struct MenuAction sMultichoiceList_GameCornerCoinPurchaseCounter[] = {
    { COMPOUND_STRING("{FONT_SMALL} 50 COINS{CLEAR_TO 0x45}¥1,000") },
    { COMPOUND_STRING("{FONT_SMALL}500 COINS{CLEAR_TO 0x40}¥10,000") },
    { gOtherText_Exit }
};

static const struct MenuAction sMultichoiceList_DeptStoreElevator[] = {
    { gText_5F },
    { gText_4F },
    { gText_3F },
    { gText_2F },
    { gText_1F },
    { gOtherText_Exit }
};

static const struct MenuAction sMultichoiceList_ThirstyGirlFreshWater[] = {
    { sText_FreshWater },
    { gOtherText_Exit }
};

static const struct MenuAction sMultichoiceList_ThirstyGirlSodaPop[] = {
    { sText_SodaPop },
    { gOtherText_Exit }
};

static const struct MenuAction sMultichoiceList_ThirstyGirlFreshWaterSodaPop[] = {
    { sText_FreshWater },
    { sText_SodaPop },
    { gOtherText_Exit }
};

static const struct MenuAction sMultichoiceList_ThirstyGirlLemonade[] = {
    { sText_Lemonade },
    { gOtherText_Exit }
};

static const struct MenuAction sMultichoiceList_ThirstyGirlFreshWaterLemonade[] = {
    { sText_FreshWater },
    { sText_Lemonade },
    { gOtherText_Exit }
};

static const struct MenuAction sMultichoiceList_ThirstyGirlSodaPopLemonade[] = {
    { sText_SodaPop },
    { sText_Lemonade },
    { gOtherText_Exit }
};

static const struct MenuAction sMultichoiceList_ThirstyGirlFreshWaterSodaPopLemonade[] = {
    { sText_FreshWater },
    { sText_SodaPop },
    { sText_Lemonade },
    { gOtherText_Exit }
};

static const struct MenuAction sMultichoiceList_Eeveelutions[] = {
    { COMPOUND_STRING("EEVEE") },
    { COMPOUND_STRING("FLAREON") },
    { COMPOUND_STRING("JOLTEON") },
    { COMPOUND_STRING("VAPOREON") },
    { COMPOUND_STRING("Quit looking.") }
};

static const struct MenuAction sMultichoiceList_BikeShop[] = {
    { COMPOUND_STRING("BICYCLE{CLEAR_TO 0x49}{FONT_SMALL}¥1,000,000") },
    { sText_NoThanks }
};

static const struct MenuAction sMultichoiceList_GameCornerPokemonPrizes[] = {
#if defined(FIRERED)
    { COMPOUND_STRING("ABRA{CLEAR_TO 0x55}{FONT_SMALL} 180 COINS") },
    { COMPOUND_STRING("CLEFAIRY{CLEAR_TO 0x55}{FONT_SMALL} 500 COINS") },
    { COMPOUND_STRING("DRATINI{CLEAR_TO 0x4B}{FONT_SMALL} 2,800 COINS") },
    { COMPOUND_STRING("SCYTHER{CLEAR_TO 0x4B}{FONT_SMALL} 5,500 COINS") },
    { COMPOUND_STRING("PORYGON{CLEAR_TO 0x4B}{FONT_SMALL} 9,999 COINS") },
#elif defined(LEAFGREEN)
    { COMPOUND_STRING("ABRA{CLEAR_TO 0x55}{FONT_SMALL} 120 COINS") },
    { COMPOUND_STRING("CLEFAIRY{CLEAR_TO 0x55}{FONT_SMALL} 750 COINS") },
    { COMPOUND_STRING("PINSIR{CLEAR_TO 0x4B}{FONT_SMALL} 2,500 COINS") },
    { COMPOUND_STRING("DRATINI{CLEAR_TO 0x4B}{FONT_SMALL} 4,600 COINS") },
    { COMPOUND_STRING("PORYGON{CLEAR_TO 0x4B}{FONT_SMALL} 6,500 COINS") },
#endif
    { sText_NoThanks }
};

static const struct MenuAction sMultichoiceList_TradeCenter_Colosseum[] = {
    { sText_TradeCenter },
    { sText_Colosseum },
    { gOtherText_Exit }
};

static const struct MenuAction sMultichoiceList_RocketHideoutElevator[] = {
    { gText_B1F },
    { gText_B2F },
    { gText_B4F },
    { gOtherText_Exit }
};

static const struct MenuAction sMultichoiceList_LinkedDirectUnion[] = {
    { COMPOUND_STRING("LINKED GAME PLAY") },
    { COMPOUND_STRING("DIRECT CORNER") },
    { COMPOUND_STRING("UNION ROOM") },
    { sText_Quit }
};

static const struct MenuAction sMultichoiceList_Island23[] = {
    { sText_TwoIsland },
    { sText_ThreeIsland },
    { gOtherText_Exit }
};

static const struct MenuAction sMultichoiceList_Island13[] = {
    { sText_OneIsland },
    { sText_ThreeIsland },
    { gOtherText_Exit }
};

static const struct MenuAction sMultichoiceList_Island12[] = {
    { sText_OneIsland },
    { sText_TwoIsland },
    { gOtherText_Exit }
};

static const struct MenuAction sMultichoiceList_TradeColosseumCrush[] = {
    { sText_TradeCenter },
    { sText_Colosseum },
    { gText_BerryCrush },
    { gOtherText_Exit }
};

static const struct MenuAction sMultichoiceList_TradeColosseum_2[] = {
    { sText_TradeCenter },
    { sText_Colosseum },
    { gOtherText_Exit }
};

static const struct MenuAction sMultichoiceList_PokejumpDodrio[] = {
    { gText_PokemonJump },
    { COMPOUND_STRING("DODRIO BERRY-PICKING") },
    { gOtherText_Exit }
};

static const struct MenuAction sMultichoiceList_Mushrooms[] = {
    {COMPOUND_STRING("2 TINYMUSHROOMS")},
    {COMPOUND_STRING("1 BIG MUSHROOM")},
};

static const struct MenuAction sMultichoiceList_SeviiNavel[] = {
    { sText_SeviiIslands },
    { sText_NavelRock },
    { gOtherText_Exit }
};

static const struct MenuAction sMultichoiceList_SeviiBirth[] = {
    { sText_SeviiIslands },
    { sText_BirthIsland },
    { gOtherText_Exit }
};

static const struct MenuAction sMultichoiceList_SeviiNavelBirth[] = {
    { sText_SeviiIslands },
    { sText_NavelRock },
    { sText_BirthIsland },
    { gOtherText_Exit }
};

static const struct MenuAction sMultichoiceList_Seagallop123[] = {
    { sText_OneIsland },
    { sText_TwoIsland },
    { sText_ThreeIsland },
    { gOtherText_Exit }
};

static const struct MenuAction sMultichoiceList_SeagallopV23[] = {
    { sText_Vermilion },
    { sText_TwoIsland },
    { sText_ThreeIsland },
    { gOtherText_Exit }
};

static const struct MenuAction sMultichoiceList_SeagallopV13[] = {
    { sText_Vermilion },
    { sText_OneIsland },
    { sText_ThreeIsland },
    { gOtherText_Exit }
};

static const struct MenuAction sMultichoiceList_SeagallopV12[] = {
    { sText_Vermilion },
    { sText_OneIsland },
    { sText_TwoIsland },
    { gOtherText_Exit }
};

static const struct MenuAction sMultichoiceList_SeagallopVermilion[] = {
    { sText_Vermilion },
    { gOtherText_Exit }
};

static const struct MenuAction sMultichoiceList_JoinOrLead[] = {
    {COMPOUND_STRING("JOIN GROUP")},
    {COMPOUND_STRING("BECOME LEADER")},
    {gOtherText_Exit}
};

static const struct MenuAction sMultichoiceList_TrainerTowerMode[] = {
    { gOtherText_Single },
    { gOtherText_Double },
    { gOtherText_Knockout },
    { gOtherText_Mixed },
    { gOtherText_Exit }
};

static const struct MenuAction sMultichoiceList_FrontierRules[] =
{
    {COMPOUND_STRING("TWO STYLES")},
    {COMPOUND_STRING("LV. 50")},
    {COMPOUND_STRING("OPEN LEVEL")},
    {COMPOUND_STRING("{PKMN} TYPE & NO.")},
    {COMPOUND_STRING("HOLD ITEMS")},
    {gText_Exit},
};

static const struct MenuAction sMultichoiceList_FrontierPassInfo[] =
{
    {COMPOUND_STRING("SYMBOLS")},
    {COMPOUND_STRING("RECORD")},
    {COMPOUND_STRING("BATTLE PTS")},
    {gText_Exit},
};

static const struct MenuAction sMultichoiceList_FrontierGamblerBet[] =
{
    {COMPOUND_STRING("  5BP")},
    {COMPOUND_STRING("10BP")},
    {COMPOUND_STRING("15BP")},
    {gText_Exit},
};

static const struct MenuAction sMultichoiceList_LevelMode[] =
{
    {gText_Lv50},
    {gText_OpenLevel},
    {gText_Exit},
};

static const struct MenuAction sMultichoiceList_BattleFactoryRules[] =
{
    {COMPOUND_STRING("BASIC RULES")},
    {COMPOUND_STRING("SWAP: PARTNER")},
    {COMPOUND_STRING("SWAP: NUMBER")},
    {COMPOUND_STRING("SWAP: NOTES")},
    {COMPOUND_STRING("OPEN LEVEL")},
    {gText_Exit},
};

static const struct MenuAction sMultichoiceList_GoOnRecordRestRetire[] =
{
    {sText_GoOn},
    {gText_Record},
    {gText_Rest},
    {sText_Retire},
};

static const struct MenuAction sMultichoiceList_GoOnRestRetire[] =
{
    {sText_GoOn},
    {gText_Rest},
    {sText_Retire},
};

static const struct MenuAction sMultichoiceList_GoOnRecordRetire[] =
{
    {sText_GoOn},
    {gText_Record},
    {sText_Retire},
};

static const struct MenuAction sMultichoiceList_GoOnRetire[] =
{
    {sText_GoOn},
    {sText_Retire},
};

static const struct MenuAction sMultichoiceList_BattleArenaRules[] =
{
    {COMPOUND_STRING("BATTLE RULES")},
    {COMPOUND_STRING("JUDGE: MIND")},
    {COMPOUND_STRING("JUDGE: SKILL")},
    {COMPOUND_STRING("JUDGE: BODY")},
    {gText_Exit},
};

static const struct MenuAction sMultichoiceList_BattleDomeRules[] =
{
    {COMPOUND_STRING("MATCHUP")},
    {COMPOUND_STRING("TOURNEY TREE")},
    {COMPOUND_STRING("DOUBLE KO")},
    {gText_Exit},
};

static const struct MenuAction sMultichoiceList_TourneyWithRecord[] =
{
    {sText_Opponent},
    {sText_Tourney_Tree},
    {sText_ReadyToStart},
    {gText_Record},
    {gText_Rest},
    {sText_Retire},
};

static const struct MenuAction sMultichoiceList_TourneyNoRecord[] =
{
    {sText_Opponent},
    {sText_Tourney_Tree},
    {sText_ReadyToStart},
    {gText_Rest},
    {sText_Retire},
};

static const struct MenuAction sMultichoiceList_BattlePalaceRules[] =
{
    {COMPOUND_STRING("BATTLE BASICS")},
    {COMPOUND_STRING("POKéMON NATURE")},
    {COMPOUND_STRING("POKéMON MOVES")},
    {COMPOUND_STRING("UNDERPOWERED")},
    {COMPOUND_STRING("WHEN IN DANGER")},
    {gText_Exit},
};

static const struct MenuAction sMultichoiceList_BattlePyramidRules[] =
{
    {COMPOUND_STRING("PYRAMID: POKéMON")},
    {COMPOUND_STRING("PYRAMID: TRAINERS")},
    {COMPOUND_STRING("PYRAMID: MAZE")},
    {COMPOUND_STRING("BATTLE BAG")},
    {gText_Exit},
};

static const struct MenuAction sMultichoiceList_BattlePikeRules[] =
{
    {COMPOUND_STRING("POKéNAV AND BAG")},
    {COMPOUND_STRING("HELD ITEMS")},
    {COMPOUND_STRING("POKéMON ORDER")},
    {gText_Exit},
};

static const struct MenuAction sMultichoiceList_FrontierItemChoose[] =
{
    {COMPOUND_STRING("BATTLE BAG")},
    {COMPOUND_STRING("HELD ITEM")},
    {gText_Exit},
};

static const struct MenuAction sMultichoiceList_BattleTowerRules[] =
{
    {COMPOUND_STRING("TOWER INFO")},
    {COMPOUND_STRING("BATTLE {PKMN}")},
    {COMPOUND_STRING("BATTLE SALON")},
    {sText_MultiLink},
    {gText_Exit},
};

static const struct MenuAction sMultichoiceList_BattleTowerFeelings[] =
{
    {COMPOUND_STRING("I'll battle now!")},
    {COMPOUND_STRING("I won!")},
    {COMPOUND_STRING("I lost!")},
    {COMPOUND_STRING("I won't tell.")},
};

static const struct MenuAction sMultichoiceList_LinkLeader[] =
{
    {COMPOUND_STRING("JOIN GROUP")},
    {COMPOUND_STRING("BECOME LEADER")},
    {gText_Exit},
};

static const struct MenuAction sMultichoiceList_Satisfaction[] =
{
    {COMPOUND_STRING("Satisfied")},
    {COMPOUND_STRING("Dissatisfied")},
};

struct MultichoiceListStruct
{
    const struct MenuAction *list;
    u8 count;
};

static const struct MultichoiceListStruct sMultichoiceLists[] =
{
    [MULTI_YESNO]                                      = MULTICHOICE(sMultichoiceList_YesNo),
    [MULTI_EEVEELUTIONS]                               = MULTICHOICE(sMultichoiceList_Eeveelutions),
    [MULTI_TRAINER_CARD_ICON_TINT]                     = MULTICHOICE(sMultichoiceList_TrainerCardIconTint),
    [MULTI_HOF_QUIT]                                   = MULTICHOICE(sMultichoiceList_HOF_Quit),
    [MULTI_EGGS_QUIT]                                  = MULTICHOICE(sMultichoiceList_Eggs_Quit),
    [MULTI_VICTORIES_QUIT]                             = MULTICHOICE(sMultichoiceList_Victories_Quit),
    [MULTI_HOF_EGGS_QUIT]                              = MULTICHOICE(sMultichoiceList_HOF_Eggs_Quit),
    [MULTI_HOF_VICTORIES_QUIT]                         = MULTICHOICE(sMultichoiceList_HOF_Victories_Quit),
    [MULTI_EGGS_VICTORIES_QUIT]                        = MULTICHOICE(sMultichoiceList_Eggs_Victories_Quit),
    [MULTI_HOF_EGGS_VICTORIES_QUIT]                    = MULTICHOICE(sMultichoiceList_HOF_Eggs_Victories_Quit),
    [MULTI_BIKE_SHOP]                                  = MULTICHOICE(sMultichoiceList_BikeShop),
    [MULTI_GAME_CORNER_POKEMON_PRIZES]                 = MULTICHOICE(sMultichoiceList_GameCornerPokemonPrizes),
    [MULTI_TRAINER_SCHOOL_WHITEBOARD]                  = MULTICHOICE(sMultichoiceList_TrainerSchoolWhiteboard),
    [MULTI_YES_NO_INFO]                                = MULTICHOICE(sMultichoiceList_YesNoInfo),
    [MULTI_SINGLE_DOUBLE_MULTI_INFO_EXIT]              = MULTICHOICE(sMultichoiceList_SingleDoubleMultiInfoExit),
    [MULTI_CHALLENGEINFO]                              = MULTICHOICE(sMultichoiceList_ChallengeInfoExit),
    [MULTI_ROOFTOP_B1F]                                = MULTICHOICE(sMultichoiceList_RooftopB1F),
    [MULTI_HELIX]                                      = MULTICHOICE(sMultichoiceList_Helix),
    [MULTI_DOME]                                       = MULTICHOICE(sMultichoiceList_Dome),
    [MULTI_AMBER]                                      = MULTICHOICE(sMultichoiceList_Amber),
    [MULTI_HELIX_AMBER]                                = MULTICHOICE(sMultichoiceList_HelixAmber),
    [MULTI_DOME_AMBER]                                 = MULTICHOICE(sMultichoiceList_DomeAmber),
    [MULTI_CELADON_VENDING_MACHINE]                    = MULTICHOICE(sMultichoiceList_CeladonVendingMachine),
    [MULTI_GAME_CORNER_COIN_PURCHASE_COUNTER]          = MULTICHOICE(sMultichoiceList_GameCornerCoinPurchaseCounter),
    [MULTI_GAME_CORNER_TMPRIZES]                       = MULTICHOICE(sMultichoiceList_GameCornerTMPrizes),
    [MULTI_DEPT_STORE_ELEVATOR]                        = MULTICHOICE(sMultichoiceList_DeptStoreElevator),
    [MULTI_THIRSTY_GIRL_FRESH_WATER]                   = MULTICHOICE(sMultichoiceList_ThirstyGirlFreshWater),
    [MULTI_THIRSTY_GIRL_SODA_POP]                      = MULTICHOICE(sMultichoiceList_ThirstyGirlSodaPop),
    [MULTI_THIRSTY_GIRL_FRESH_WATER_SODA_POP]          = MULTICHOICE(sMultichoiceList_ThirstyGirlFreshWaterSodaPop),
    [MULTI_THIRSTY_GIRL_LEMONADE]                      = MULTICHOICE(sMultichoiceList_ThirstyGirlLemonade),
    [MULTI_THIRSTY_GIRL_FRESH_WATER_LEMONADE]          = MULTICHOICE(sMultichoiceList_ThirstyGirlFreshWaterLemonade),
    [MULTI_THIRSTY_GIRL_SODA_POP_LEMONADE]             = MULTICHOICE(sMultichoiceList_ThirstyGirlSodaPopLemonade),
    [MULTI_THIRSTY_GIRL_FRESH_WATER_SODA_POP_LEMONADE] = MULTICHOICE(sMultichoiceList_ThirstyGirlFreshWaterSodaPopLemonade),
    [MULTI_TRADE_CENTER_COLOSSEUM]                     = MULTICHOICE(sMultichoiceList_TradeCenter_Colosseum),
    [MULTI_GAME_CORNER_BATTLE_ITEM_PRIZES]             = MULTICHOICE(sMultichoiceList_GameCornerBattleItemPrizes),
    [MULTI_ROCKET_HIDEOUT_ELEVATOR]                    = MULTICHOICE(sMultichoiceList_RocketHideoutElevator),
    [MULTI_LINKED_DIRECT_UNION]                        = MULTICHOICE(sMultichoiceList_LinkedDirectUnion),
    [MULTI_ISLAND_23]                                  = MULTICHOICE(sMultichoiceList_Island23),
    [MULTI_ISLAND_13]                                  = MULTICHOICE(sMultichoiceList_Island13),
    [MULTI_ISLAND_12]                                  = MULTICHOICE(sMultichoiceList_Island12),
    [MULTI_TRADE_COLOSSEUM_CRUSH]                      = MULTICHOICE(sMultichoiceList_TradeColosseumCrush),
    [MULTI_POKEJUMP_DODRIO]                            = MULTICHOICE(sMultichoiceList_PokejumpDodrio),
    [MULTI_TRADE_COLOSSEUM_2]                          = MULTICHOICE(sMultichoiceList_TradeColosseum_2),
    [MULTI_MUSHROOMS]                                  = MULTICHOICE(sMultichoiceList_Mushrooms),
    [MULTI_SEVII_NAVEL]                                = MULTICHOICE(sMultichoiceList_SeviiNavel),
    [MULTI_SEVII_BIRTH]                                = MULTICHOICE(sMultichoiceList_SeviiBirth),
    [MULTI_SEVII_NAVEL_BIRTH]                          = MULTICHOICE(sMultichoiceList_SeviiNavelBirth),
    [MULTI_SEAGALLOP_123]                              = MULTICHOICE(sMultichoiceList_Seagallop123),
    [MULTI_SEAGALLOP_V23]                              = MULTICHOICE(sMultichoiceList_SeagallopV23),
    [MULTI_SEAGALLOP_V13]                              = MULTICHOICE(sMultichoiceList_SeagallopV13),
    [MULTI_SEAGALLOP_V12]                              = MULTICHOICE(sMultichoiceList_SeagallopV12),
    [MULTI_SEAGALLOP_VERMILION]                        = MULTICHOICE(sMultichoiceList_SeagallopVermilion),
    [MULTI_JOIN_OR_LEAD]                               = MULTICHOICE(sMultichoiceList_JoinOrLead),
    [MULTI_TRAINER_TOWER_MODE]                         = MULTICHOICE(sMultichoiceList_TrainerTowerMode),
    [MULTI_FRONTIER_RULES]                             = MULTICHOICE(sMultichoiceList_FrontierRules),
    [MULTI_FRONTIER_PASS_INFO]                         = MULTICHOICE(sMultichoiceList_FrontierPassInfo),
    [MULTI_FRONTIER_GAMBLER_BET]                       = MULTICHOICE(sMultichoiceList_FrontierGamblerBet),
    [MULTI_LEVEL_MODE]                                 = MULTICHOICE(sMultichoiceList_LevelMode),
    [MULTI_BATTLE_FACTORY_RULES]                       = MULTICHOICE(sMultichoiceList_BattleFactoryRules),
    [MULTI_GO_ON_RECORD_REST_RETIRE]                   = MULTICHOICE(sMultichoiceList_GoOnRecordRestRetire),
    [MULTI_GO_ON_REST_RETIRE]                          = MULTICHOICE(sMultichoiceList_GoOnRestRetire),
    [MULTI_GO_ON_RECORD_RETIRE]                        = MULTICHOICE(sMultichoiceList_GoOnRecordRetire),
    [MULTI_GO_ON_RETIRE]                               = MULTICHOICE(sMultichoiceList_GoOnRetire),
    [MULTI_BATTLE_ARENA_RULES]                         = MULTICHOICE(sMultichoiceList_BattleArenaRules),
    [MULTI_BATTLE_DOME_RULES]                          = MULTICHOICE(sMultichoiceList_BattleDomeRules),
    [MULTI_TOURNEY_WITH_RECORD]                        = MULTICHOICE(sMultichoiceList_TourneyWithRecord),
    [MULTI_TOURNEY_NO_RECORD]                          = MULTICHOICE(sMultichoiceList_TourneyNoRecord),
    [MULTI_BATTLE_PALACE_RULES]                        = MULTICHOICE(sMultichoiceList_BattlePalaceRules),
    [MULTI_BATTLE_PYRAMID_RULES]                       = MULTICHOICE(sMultichoiceList_BattlePyramidRules),
    [MULTI_BATTLE_PIKE_RULES]                          = MULTICHOICE(sMultichoiceList_BattlePikeRules),
    [MULTI_FRONTIER_ITEM_CHOOSE]                       = MULTICHOICE(sMultichoiceList_FrontierItemChoose),
    [MULTI_BATTLE_TOWER_RULES]                         = MULTICHOICE(sMultichoiceList_BattleTowerRules),
    [MULTI_BATTLE_TOWER_FEELINGS]                      = MULTICHOICE(sMultichoiceList_BattleTowerFeelings),
    [MULTI_LINK_LEADER]                                = MULTICHOICE(sMultichoiceList_LinkLeader),
    [MULTI_SATISFACTION]                               = MULTICHOICE(sMultichoiceList_Satisfaction),
};

const u8 *const gStdStrings[] = {
    [STDSTRING_BOULDER_BADGE]    = gText_BoulderBadge,
    [STDSTRING_CASCADE_BADGE]    = gText_CascadeBadge,
    [STDSTRING_THUNDER_BADGE]    = gText_ThunderBadge,
    [STDSTRING_RAINBOW_BADGE]    = gText_RainbowBadge,
    [STDSTRING_SOUL_BADGE]       = gText_SoulBadge,
    [STDSTRING_MARSH_BADGE]      = gText_MarshBadge,
    [STDSTRING_VOLCANO_BADGE]    = gText_VolcanoBadge,
    [STDSTRING_EARTH_BADGE]      = gText_EarthBadge,
    [STDSTRING_COINS]            = gText_Coins,
    [STDSTRING_ITEMS_POCKET]     = COMPOUND_STRING("ITEMS POCKET"),
    [STDSTRING_KEY_ITEMS_POCKET] = COMPOUND_STRING("KEY ITEMS POCKET"),
    [STDSTRING_POKEBALLS_POCKET] = COMPOUND_STRING("POKé BALLS POCKET"),
    [STDSTRING_TM_CASE]          = gText_TMCase,
    [STDSTRING_BERRY_POUCH]      = gText_BerryPouch,
    [STDSTRING_SINGLE]           = gOtherText_Single,
    [STDSTRING_DOUBLE]           = gOtherText_Double,
    [STDSTRING_MULTI]            = COMPOUND_STRING("MULTI"),
    [STDSTRING_MULTI_LINK]       = sText_MultiLink,
    [STDSTRING_BATTLE_DOME]      = gText_BattleDome,
    [STDSTRING_BATTLE_FACTORY]   = gText_BattleFactory,
    [STDSTRING_BATTLE_PALACE]    = gText_BattlePalace,
    [STDSTRING_BATTLE_ARENA]     = gText_BattleArena,
    [STDSTRING_BATTLE_PIKE]      = gText_BattlePike,
    [STDSTRING_BATTLE_PYRAMID]   = gText_BattlePyramid,
};

static const u8 *const sDescriptionPtrs_CableClub_TradeBattleCancel[] = {
    CableClub_Text_TradeMonsUsingLinkCable,
    CableClub_Text_BattleUsingLinkCable,
    CableClub_Text_CancelSelectedItem
};

static const u8 *const sDescriptionPtrs_WirelessCenter_TradeBattleCrushCancel[] = {
    CableClub_Text_YouMayTradeHere,
    CableClub_Text_YouMayBattleHere,
    CableClub_Text_CanMakeBerryPowder,
    CableClub_Text_CancelSelectedItem
};

static const u8 *const sDescriptionPtrs_WirelessCenter_TradeBattleCancel[] = {
    CableClub_Text_YouMayTradeHere,
    CableClub_Text_YouMayBattleHere,
    CableClub_Text_CancelSelectedItem
};

static const union AnimCmd sMuseumFossilAnim0[] = {
    ANIMCMD_FRAME(0, 10),
    ANIMCMD_END
};

static const union AnimCmd *const sMuseumFossilAnimCmdTable[] = {
    sMuseumFossilAnim0
};

static const struct OamData sMuseumFossilOamData = {
    .shape = SPRITE_SHAPE(64x64),
    .size = SPRITE_SIZE(64x64)
};

static const struct SpriteTemplate sMuseumFossilSprTemplate = {
    .tileTag = GFXTAG_FOSSIL,
    .paletteTag = TAG_NONE,
    .oam = &sMuseumFossilOamData,
    .anims = sMuseumFossilAnimCmdTable,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy
};

static const u16 sMuseumAerodactylSprTiles[] = INCBIN_U16("graphics/script_menu/aerodactyl_fossil.4bpp");
static const u16 sMuseumAerodactylSprPalette[] = INCBIN_U16("graphics/script_menu/aerodactyl_fossil.gbapal");
static const u16 sMuseumKabutopsSprTiles[] = INCBIN_U16("graphics/script_menu/kabutops_fossil.4bpp");
static const u16 sMuseumKabutopsSprPalette[] = INCBIN_U16("graphics/script_menu/kabutops_fossil.gbapal");

static const struct SpriteSheet sMuseumKabutopsSprSheets[] = {
    {sMuseumKabutopsSprTiles, sizeof(sMuseumKabutopsSprTiles), GFXTAG_FOSSIL},
    {}
};

static const struct SpriteSheet sMuseumAerodactylSprSheets[] = {
    {sMuseumAerodactylSprTiles, sizeof(sMuseumAerodactylSprTiles), GFXTAG_FOSSIL},
    {}
};

static const u8 *const sSeagallopDestStrings[] = {
    [SEAGALLOP_VERMILION_CITY] = sText_Vermilion,
    [SEAGALLOP_ONE_ISLAND]     = sText_OneIsland,
    [SEAGALLOP_TWO_ISLAND]     = sText_TwoIsland,
    [SEAGALLOP_THREE_ISLAND]   = sText_ThreeIsland,
    [SEAGALLOP_FOUR_ISLAND]    = COMPOUND_STRING("FOUR ISLAND"),
    [SEAGALLOP_FIVE_ISLAND]    = COMPOUND_STRING("FIVE ISLAND"),
    [SEAGALLOP_SIX_ISLAND]     = COMPOUND_STRING("SIX ISLAND"),
    [SEAGALLOP_SEVEN_ISLAND]   = COMPOUND_STRING("SEVEN ISLAND"),
};

bool8 ScriptMenu_MultichoiceDynamic(u8 left, u8 top, u8 argc, struct ListMenuItem *items, bool8 ignoreBPress, u8 maxBeforeScroll, u32 initialRow, u32 callbackSet)
{
    if (FuncIsActiveTask(Task_HandleMultichoiceInput) == TRUE)
    {
        FreeListMenuItems(items, argc);
        return FALSE;
    }
    else
    {
        gSpecialVar_Result = SCR_MENU_UNSET;
        DrawMultichoiceMenuDynamic(left, top, argc, items, ignoreBPress, initialRow, maxBeforeScroll, callbackSet);
        return TRUE;
    }
}

bool8 ScriptMenu_Multichoice(u8 left, u8 top, enum MultichoiceID mcId, u8 ignoreBpress)
{
    if (FuncIsActiveTask(Task_HandleMultichoiceInput) == TRUE)
        return FALSE;

    gSpecialVar_Result = SCR_MENU_UNSET;
    DrawMultichoiceMenu(left, top, mcId, ignoreBpress, 0);
    return TRUE;
}

bool32 ScriptMenu_MultichoiceWithDefault(u8 left, u8 top, enum MultichoiceID multichoiceId, bool32 ignoreBPress, u8 cursorPos)
{
    if (FuncIsActiveTask(Task_HandleMultichoiceInput) == TRUE)
        return FALSE;

    gSpecialVar_Result = SCR_MENU_UNSET;
    DrawMultichoiceMenu(left, top, multichoiceId, ignoreBPress, cursorPos);
    return TRUE;
}

static void MultichoiceDynamicEventDebug_OnInit(struct DynamicListMenuEventArgs *eventArgs)
{
    DebugPrintf("OnInit: %d", eventArgs->windowId);
}

static void MultichoiceDynamicEventDebug_OnSelectionChanged(struct DynamicListMenuEventArgs *eventArgs)
{
    DebugPrintf("OnSelectionChanged: %d", eventArgs->selectedItem);
}

static void MultichoiceDynamicEventDebug_OnDestroy(struct DynamicListMenuEventArgs *eventArgs)
{
    DebugPrintf("OnDestroy: %d", eventArgs->windowId);
}

#define sAuxWindowId sDynamicMenuEventScratchPad[0]
#define sItemSpriteId sDynamicMenuEventScratchPad[1]
#define TAG_CB_ITEM_ICON 3000

static void MultichoiceDynamicEventShowItem_OnInit(struct DynamicListMenuEventArgs *eventArgs)
{
    struct WindowTemplate *template = &gWindows[eventArgs->windowId].window;
    u32 baseBlock = template->baseBlock + template->width * template->height;
    struct WindowTemplate auxTemplate = CreateWindowTemplate(0, template->tilemapLeft + template->width + 2, template->tilemapTop, 4, 4, 15, baseBlock);
    u32 auxWindowId = AddWindow(&auxTemplate);
    SetStandardWindowBorderStyle(auxWindowId, FALSE);
    FillWindowPixelBuffer(auxWindowId, 0x11);
    CopyWindowToVram(auxWindowId, COPYWIN_FULL);
    sAuxWindowId = auxWindowId;
    sItemSpriteId = MAX_SPRITES;
}

static void MultichoiceDynamicEventShowItem_OnSelectionChanged(struct DynamicListMenuEventArgs *eventArgs)
{
    struct WindowTemplate *template = &gWindows[eventArgs->windowId].window;
    u32 x = template->tilemapLeft * 8 + template->width * 8 + 36;
    u32 y = template->tilemapTop * 8 + 20;

    if (sItemSpriteId != MAX_SPRITES)
    {
        FreeSpriteTilesByTag(TAG_CB_ITEM_ICON);
        FreeSpritePaletteByTag(TAG_CB_ITEM_ICON);
        DestroySprite(&gSprites[sItemSpriteId]);
    }

    sItemSpriteId = AddItemIconSprite(TAG_CB_ITEM_ICON, TAG_CB_ITEM_ICON, eventArgs->selectedItem);
    gSprites[sItemSpriteId].oam.priority = 0;
    gSprites[sItemSpriteId].x = x;
    gSprites[sItemSpriteId].y = y;
}

static void MultichoiceDynamicEventShowItem_OnDestroy(struct DynamicListMenuEventArgs *eventArgs)
{
    ClearStdWindowAndFrame(sAuxWindowId, TRUE);
    RemoveWindow(sAuxWindowId);

    if (sItemSpriteId != MAX_SPRITES)
    {
        FreeSpriteTilesByTag(TAG_CB_ITEM_ICON);
        FreeSpritePaletteByTag(TAG_CB_ITEM_ICON);
        DestroySprite(&gSprites[sItemSpriteId]);
    }
}

#undef sAuxWindowId
#undef sItemSpriteId
#undef TAG_CB_ITEM_ICON

static void FreeListMenuItems(struct ListMenuItem *items, u32 count)
{
    u32 i;
    for (i = 0; i < count; ++i)
    {
        // All items were dynamically allocated, so items[i].name is not actually constant.
        Free((void *)items[i].name);
    }
    Free(items);
}

void MultichoiceDynamic_InitStack(u32 capacity)
{
    AGB_ASSERT(sDynamicMultiChoiceStack == NULL);
    sDynamicMultiChoiceStack = AllocZeroed(sizeof(*sDynamicMultiChoiceStack));
    AGB_ASSERT(sDynamicMultiChoiceStack != NULL);
    sDynamicMultiChoiceStack->capacity = capacity;
    sDynamicMultiChoiceStack->top = -1;
    sDynamicMultiChoiceStack->elements = AllocZeroed(capacity * sizeof(struct ListMenuItem));
}

void MultichoiceDynamic_ReallocStack(u32 newCapacity)
{
    struct ListMenuItem *newElements;
    AGB_ASSERT(sDynamicMultiChoiceStack != NULL);
    AGB_ASSERT(sDynamicMultiChoiceStack->capacity < newCapacity);
    newElements = AllocZeroed(newCapacity * sizeof(struct ListMenuItem));
    AGB_ASSERT(newElements != NULL);
    memcpy(newElements, sDynamicMultiChoiceStack->elements, sDynamicMultiChoiceStack->capacity * sizeof(struct ListMenuItem));
    Free(sDynamicMultiChoiceStack->elements);
    sDynamicMultiChoiceStack->elements = newElements;
    sDynamicMultiChoiceStack->capacity = newCapacity;
}

bool32 MultichoiceDynamic_StackFull(void)
{
    AGB_ASSERT(sDynamicMultiChoiceStack != NULL);
    return sDynamicMultiChoiceStack->top == sDynamicMultiChoiceStack->capacity - 1;
}

bool32 MultichoiceDynamic_StackEmpty(void)
{
    AGB_ASSERT(sDynamicMultiChoiceStack != NULL);
    return sDynamicMultiChoiceStack->top == -1;
}

u32 MultichoiceDynamic_StackSize(void)
{
    AGB_ASSERT(sDynamicMultiChoiceStack != NULL);
    return sDynamicMultiChoiceStack->top + 1;
}

void MultichoiceDynamic_PushElement(struct ListMenuItem item)
{
    if (sDynamicMultiChoiceStack == NULL)
        MultichoiceDynamic_InitStack(MULTI_DYNAMIC_STACK_SIZE);
    if (MultichoiceDynamic_StackFull())
        MultichoiceDynamic_ReallocStack(sDynamicMultiChoiceStack->capacity + MULTI_DYNAMIC_STACK_INC);
    sDynamicMultiChoiceStack->elements[++sDynamicMultiChoiceStack->top] = item;
}

struct ListMenuItem *MultichoiceDynamic_PopElement(void)
{
    if (sDynamicMultiChoiceStack == NULL)
        return NULL;
    if (MultichoiceDynamic_StackEmpty())
        return NULL;
    return &sDynamicMultiChoiceStack->elements[sDynamicMultiChoiceStack->top--];
}

struct ListMenuItem *MultichoiceDynamic_PeekElement(void)
{
    if (sDynamicMultiChoiceStack == NULL)
        return NULL;
    if (MultichoiceDynamic_StackEmpty())
        return NULL;
    return &sDynamicMultiChoiceStack->elements[sDynamicMultiChoiceStack->top];
}

struct ListMenuItem *MultichoiceDynamic_PeekElementAt(u32 index)
{
    if (sDynamicMultiChoiceStack == NULL)
        return NULL;
    if (sDynamicMultiChoiceStack->top < index)
        return NULL;
    return &sDynamicMultiChoiceStack->elements[index];
}

void MultichoiceDynamic_DestroyStack(void)
{
    TRY_FREE_AND_SET_NULL(sDynamicMultiChoiceStack->elements);
    TRY_FREE_AND_SET_NULL(sDynamicMultiChoiceStack);
}

static void MultichoiceDynamic_MoveCursor(s32 itemIndex, bool8 onInit, struct ListMenu *list)
{
    u8 taskId;
    if (!onInit)
        PlaySE(SE_SELECT);

    taskId = FindTaskIdByFunc(Task_HandleScrollingMultichoiceInput);
    if (taskId == TASK_NONE)
        return;

    ListMenuGetScrollAndRow(gTasks[taskId].data[0], &gScrollableMultichoice_ScrollOffset, NULL);
    if (sDynamicMenuEventId != DYN_MULTICHOICE_CB_NONE && sDynamicListMenuEventCollections[sDynamicMenuEventId].OnSelectionChanged && !onInit)
    {
        struct DynamicListMenuEventArgs eventArgs = {.selectedItem = itemIndex, .windowId = list->template.windowId, .list = &list->template};
        sDynamicListMenuEventCollections[sDynamicMenuEventId].OnSelectionChanged(&eventArgs);
    }
}

static void DrawMultichoiceMenuDynamic(u8 left, u8 top, u8 argc, struct ListMenuItem *items, bool8 ignoreBPress, u32 initialRow, u8 maxBeforeScroll, u32 callbackSet)
{
    u32 i;
    u8 windowId;
    s32 width = 0;
    u8 newWidth;
    u8 taskId;
    u32 windowHeight;
    struct ListMenu *list;

    for (i = 0; i < argc; ++i)
    {
        width = DisplayTextAndGetWidth(items[i].name, width);
    }
    LoadMessageBoxAndBorderGfx();
    windowHeight = GetMultiChoiceWindowHeight(argc, maxBeforeScroll);

    newWidth = ConvertPixelWidthToTileWidth(width);
    left = ScriptMenu_AdjustLeftCoordFromWidth(left, newWidth);
    windowId = CreateWindowFromRect(left, top, newWidth, windowHeight);
    SetStandardWindowBorderStyle(windowId, FALSE);
    CopyWindowToVram(windowId, COPYWIN_FULL);

    // I don't like this being global either, but I could not come up with another solution that
    // does not invade the whole ListMenu infrastructure.
    sDynamicMenuEventId = callbackSet;
    sDynamicMenuEventScratchPad = AllocZeroed(100 * sizeof(u16));
    if (sDynamicMenuEventId != DYN_MULTICHOICE_CB_NONE && sDynamicListMenuEventCollections[sDynamicMenuEventId].OnInit)
    {
        struct DynamicListMenuEventArgs eventArgs = {.selectedItem = initialRow, .windowId = windowId, .list = NULL};
        sDynamicListMenuEventCollections[sDynamicMenuEventId].OnInit(&eventArgs);
    }

    gMultiuseListMenuTemplate = sScriptableListMenuTemplate;
    gMultiuseListMenuTemplate.windowId = windowId;
    gMultiuseListMenuTemplate.items = items;
    gMultiuseListMenuTemplate.totalItems = argc;
    gMultiuseListMenuTemplate.maxShowed = maxBeforeScroll;
    gMultiuseListMenuTemplate.moveCursorFunc = MultichoiceDynamic_MoveCursor;

    taskId = CreateTask(Task_HandleScrollingMultichoiceInput, 80);
    gTasks[taskId].data[0] = ListMenuInit(&gMultiuseListMenuTemplate, 0, 0);
    gTasks[taskId].data[1] = ignoreBPress;
    gTasks[taskId].data[2] = windowId;
    gTasks[taskId].data[5] = argc;
    gTasks[taskId].data[7] = maxBeforeScroll;
    StoreWordInTwoHalfwords((u16*) &gTasks[taskId].data[3], (u32) items);
    list = (void *) gTasks[gTasks[taskId].data[0]].data;
    ListMenuChangeSelectionFull(list, TRUE, FALSE, initialRow, TRUE);

    if (sDynamicMenuEventId != DYN_MULTICHOICE_CB_NONE && sDynamicListMenuEventCollections[sDynamicMenuEventId].OnSelectionChanged)
    {
        struct DynamicListMenuEventArgs eventArgs = {.selectedItem = items[initialRow].id, .windowId = windowId, .list = &gMultiuseListMenuTemplate};
        sDynamicListMenuEventCollections[sDynamicMenuEventId].OnSelectionChanged(&eventArgs);
    }
    ListMenuGetScrollAndRow(gTasks[taskId].data[0], &gScrollableMultichoice_ScrollOffset, NULL);
    if (argc > maxBeforeScroll)
    {
        // Create Scrolling Arrows
        struct ScrollArrowsTemplate template;
        template.firstX = (newWidth / 2) * 8 + 12 + (left) * 8;
        template.firstY = top * 8 + 5;
        template.secondX = template.firstX;
        template.secondY = top * 8 + windowHeight * 8 + 12;
        template.fullyUpThreshold = 0;
        template.fullyDownThreshold = argc - maxBeforeScroll;
        template.firstArrowType = SCROLL_ARROW_UP;
        template.secondArrowType = SCROLL_ARROW_DOWN;
        template.tileTag = 2000;
        template.palTag = 100,
        template.palNum = 0;

        gTasks[taskId].data[6] = AddScrollIndicatorArrowPair(&template, &gScrollableMultichoice_ScrollOffset);
    }
}

void DrawMultichoiceMenuInternal(u8 left, u8 top, enum MultichoiceID multichoiceId, bool8 ignoreBPress, u8 cursorPos, const struct MenuAction *actions, int count)
{
    s32 i;
    s32 strWidth;
    s32 tmp;
    u8 width;
    u8 height;
    u8 windowId;

    if ((ignoreBPress & 2) || QL_AvoidDisplay(QL_DestroyAbortedDisplay) != TRUE)
    {
        ignoreBPress &= 1;
        strWidth = 0;
        for (i = 0; i < count; i++)
        {
            tmp = GetStringWidth(FONT_NORMAL, actions[i].text, 0);
            if (tmp > strWidth)
                strWidth = tmp;
        }
        width = (strWidth + 9) / 8 + 1;
        if (left + width > 28)
            left = 28 - width;
        height = GetMCWindowHeight(count);
        windowId = CreateWindowFromRect(left, top, width, height);
        SetStandardWindowBorderStyle(windowId, FALSE);
        PrintMenuActionTextsWithSpacing(windowId, FONT_NORMAL, 8, 2, 14, count, actions, 0, 2);
        InitMenuNormal(windowId, FONT_NORMAL, 0, 2, 14, count, cursorPos);
        InitMultichoiceCheckWrap(ignoreBPress, count, windowId, multichoiceId);
        ScheduleBgCopyTilemapToVram(0);
    }
}

static void DrawMultichoiceMenu(u8 left, u8 top, enum MultichoiceID mcId, u8 ignoreBpress, u8 initPos)
{
    DrawMultichoiceMenuInternal(left, top, mcId, ignoreBpress, initPos, sMultichoiceLists[mcId].list, sMultichoiceLists[mcId].count);
}

#define tTimer         data[2]
#define tIgnoreBPress  data[4]
#define tDoWrap    data[5]
#define tWindowId      data[6]
#define tMultichoiceId data[7]

static void InitMultichoiceCheckWrap(bool8 ignoreBPress, u8 count, u8 windowId, enum MultichoiceID multichoiceId)
{
    u8 taskId;
    sProcessInputDelay = 0;

    switch (multichoiceId)
    {
    case MULTI_TRADE_CENTER_COLOSSEUM:
    case MULTI_TRADE_COLOSSEUM_CRUSH:
    case MULTI_TRADE_COLOSSEUM_2:
        sProcessInputDelay = 12;
        break;
    default:
        sProcessInputDelay = 0;
        break;
    }

    taskId = CreateTask(Task_HandleMultichoiceInput, 80);
    gTasks[taskId].tIgnoreBPress = ignoreBPress;

    if (count > 3)
        gTasks[taskId].tDoWrap = TRUE;
    else
        gTasks[taskId].tDoWrap = FALSE;

    gTasks[taskId].tWindowId = windowId;
    gTasks[taskId].tMultichoiceId = multichoiceId;

    DrawLinkServicesMultichoiceMenu(multichoiceId);
}

static void Task_HandleScrollingMultichoiceInput(u8 taskId)
{
    bool32 done = FALSE;
    s32 input = ListMenu_ProcessInput(gTasks[taskId].data[0]);

    switch (input)
    {
    case LIST_HEADER:
    case LIST_NOTHING_CHOSEN:
        break;
    case LIST_CANCEL:
        if (!gTasks[taskId].data[1])
        {
            gSpecialVar_Result = MULTI_B_PRESSED;
            done = TRUE;
        }
        break;
    default:
        gSpecialVar_Result = input;
        done = TRUE;
        break;
    }

    if (done)
    {
        struct ListMenuItem *items;

        PlaySE(SE_SELECT);

        if (sDynamicMenuEventId != DYN_MULTICHOICE_CB_NONE && sDynamicListMenuEventCollections[sDynamicMenuEventId].OnDestroy)
        {
            struct DynamicListMenuEventArgs eventArgs = {.selectedItem = input, .windowId = gTasks[taskId].data[2], .list = NULL};
            sDynamicListMenuEventCollections[sDynamicMenuEventId].OnDestroy(&eventArgs);
        }

        sDynamicMenuEventId = DYN_MULTICHOICE_CB_NONE;

        if (gTasks[taskId].data[5] > gTasks[taskId].data[7])
        {
            RemoveScrollIndicatorArrowPair(gTasks[taskId].data[6]);
        }

        LoadWordFromTwoHalfwords((u16*) &gTasks[taskId].data[3], (u32* )(&items));
        FreeListMenuItems(items, gTasks[taskId].data[5]);
        TRY_FREE_AND_SET_NULL(sDynamicMenuEventScratchPad);
        DestroyListMenuTask(gTasks[taskId].data[0], NULL, NULL);
        ClearStdWindowAndFrame(gTasks[taskId].data[2], TRUE);
        RemoveWindow(gTasks[taskId].data[2]);
        ScriptContext_Enable();
        DestroyTask(taskId);
    }
}

static void Task_HandleMultichoiceInput(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    s8 input;

    if (gPaletteFade.active)
        return;

    if (sProcessInputDelay != 0)
    {
        sProcessInputDelay--;
        return;
    }

    if (tDoWrap == FALSE)
        input = Menu_ProcessInputNoWrap();
    else
        input = Menu_ProcessInput();

    if (JOY_NEW(DPAD_UP | DPAD_DOWN))
        DrawLinkServicesMultichoiceMenu(tMultichoiceId);

    switch (input)
    {
    case MENU_NOTHING_CHOSEN:
        return;
    case MENU_B_PRESSED:
        if (tIgnoreBPress)
            return;
        PlaySE(SE_SELECT);
        gSpecialVar_Result = MULTI_B_PRESSED;
        break;
    default:
        gSpecialVar_Result = input;
        break;
    }
    ClearToTransparentAndRemoveWindow(tWindowId);
    DestroyTask(taskId);
    ScriptContext_Enable();
}

bool8 ScriptMenu_YesNo(u8 unused, u8 stuff)
{
    if (FuncIsActiveTask(Task_HandleYesNoInput) == TRUE)
        return FALSE;

    gSpecialVar_Result = SCR_MENU_UNSET;

    if (QL_AvoidDisplay(QL_DestroyAbortedDisplay))
        return TRUE;

    DisplayYesNoMenuDefaultYes();
    CreateTask(Task_HandleYesNoInput, 80);
    return TRUE;
}

static void Task_HandleYesNoInput(u8 taskId)
{
    if (gTasks[taskId].tTimer < 5)
    {
        gTasks[taskId].tTimer++;
        return;
    }

    switch (Menu_ProcessInputNoWrapClearOnChoose())
    {
    case MENU_NOTHING_CHOSEN:
        return;
    case MENU_B_PRESSED:
    case 1: // NO
        PlaySE(SE_SELECT);
        gSpecialVar_Result = FALSE;
        break;
    case 0: // YES
        gSpecialVar_Result = TRUE;
        break;
    }
    DestroyTask(taskId);
    ScriptContext_Enable();
}

bool8 ScriptMenu_MultichoiceGrid(u8 left, u8 top, enum MultichoiceID multichoiceId, bool8 ignoreBpress, u8 columnCount)
{
    const struct MenuAction *list;
    u8 count;
    u8 width;
    u8 rowCount;
    u8 taskId;

    if (FuncIsActiveTask(Task_HandleMultichoiceGridInput) == TRUE)
        return FALSE;

    gSpecialVar_Result = SCR_MENU_UNSET;

    if (QL_AvoidDisplay(QL_DestroyAbortedDisplay) == TRUE)
        return TRUE;

    list = sMultichoiceLists[multichoiceId].list;
    count = sMultichoiceLists[multichoiceId].count;
    width = GetMenuWidthFromList(list, count) + 1;
    rowCount = count / columnCount;
    taskId = CreateTask(Task_HandleMultichoiceGridInput, 80);
    gTasks[taskId].tIgnoreBPress = ignoreBpress;
    gTasks[taskId].tWindowId = CreateWindowFromRect(left, top, width * columnCount, rowCount * 2);
    SetStandardWindowBorderStyle(gTasks[taskId].tWindowId, FALSE);
    MultichoiceGrid_PrintItems(gTasks[taskId].tWindowId, FONT_NORMAL_COPY_1, width * 8, 16, columnCount, rowCount, list);
    MultichoiceGrid_InitCursor(gTasks[taskId].tWindowId, FONT_NORMAL_COPY_1, 0, 1, width * 8, columnCount, rowCount, 0);
    ScheduleBgCopyTilemapToVram(0);

    return TRUE;
}

static void Task_HandleMultichoiceGridInput(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    s8 selection = Menu_ProcessGridInput();

    switch (selection)
    {
    case MENU_NOTHING_CHOSEN:
        return;
    case MENU_B_PRESSED:
        if (tIgnoreBPress)
            return;
        PlaySE(SE_SELECT);
        gSpecialVar_Result = MULTI_B_PRESSED;
        break;
    default:
        gSpecialVar_Result = selection;
        break;
    }

    ClearToTransparentAndRemoveWindow(tWindowId);
    DestroyTask(taskId);
    ScriptContext_Enable();
}

#undef tIgnoreBPress
#undef tDoWrap
#undef tWindowId
#undef tMultichoiceId


bool32 ScriptMenu_CreatePCMultichoice(void)
{
    if (FuncIsActiveTask(Task_HandleMultichoiceInput) == TRUE)
        return FALSE;

    gSpecialVar_Result = SCR_MENU_UNSET;
    CreatePCMultichoice();

    return TRUE;
}

static void CreatePCMultichoice(void)
{
    u8 cursorWidth = GetMenuCursorDimensionByFont(FONT_NORMAL, 0);
    u8 windowWidth;
    u8 numItems;
    u8 windowId;

    switch (GetStringTilesWide(gText_SPc))
    {
    default:
        if (FlagGet(FLAG_SYS_POKEDEX_GET))
            windowWidth = 14;
        else
            windowWidth = 13;
        break;
    case 9:
    case 10:
        windowWidth = 14;
        break;
    }

    if (FlagGet(FLAG_SYS_GAME_CLEAR))
    {
        numItems = 5;
        windowId = CreateWindowFromRect(0, 0, windowWidth, 10);
        SetStandardWindowBorderStyle(windowId, FALSE);
        AddTextPrinterParameterized(windowId, FONT_NORMAL, gText_ProfOakSPc, cursorWidth, 34, TEXT_SKIP_DRAW, NULL);
        AddTextPrinterParameterized(windowId, FONT_NORMAL, gText_HallOfFame, cursorWidth, 50, TEXT_SKIP_DRAW, NULL);
        AddTextPrinterParameterized(windowId, FONT_NORMAL, gText_LogOff, cursorWidth, 66, TEXT_SKIP_DRAW, NULL);
    }
    else
    {
        if (FlagGet(FLAG_SYS_POKEDEX_GET))
            numItems = 4;
        else
            numItems = 3;

        windowId = CreateWindowFromRect(0, 0, windowWidth, numItems * 2);
        SetStandardWindowBorderStyle(windowId, FALSE);
        if (FlagGet(FLAG_SYS_POKEDEX_GET))
            AddTextPrinterParameterized(windowId, FONT_NORMAL, gText_ProfOakSPc, cursorWidth, 34, TEXT_SKIP_DRAW, NULL);

        AddTextPrinterParameterized(windowId, FONT_NORMAL, gText_LogOff, cursorWidth, 2 + 16 * (numItems - 1), TEXT_SKIP_DRAW, NULL);
    }

    if (FlagGet(FLAG_SYS_NOT_SOMEONES_PC))
        AddTextPrinterParameterized(windowId, FONT_NORMAL, gText_BillSPc, cursorWidth, 2 , TEXT_SKIP_DRAW, NULL);
    else
        AddTextPrinterParameterized(windowId, FONT_NORMAL, gText_SomeoneSPc, cursorWidth, 2 , TEXT_SKIP_DRAW, NULL);

    StringExpandPlaceholders(gStringVar4, gText_SPc);
    PrintPlayerNameOnWindow(windowId, gStringVar4, cursorWidth, 18);
    InitMenuNormal(windowId, FONT_NORMAL, 0, 2, 16, numItems, 0);
    InitMultichoiceCheckWrap(FALSE, numItems, windowId, MULTI_NONE);
    ScheduleBgCopyTilemapToVram(0);
}

void ScriptMenu_DisplayPCStartupPrompt(void)
{
    LoadMessageBoxAndFrameGfx(0, TRUE);
    AddTextPrinterParameterized2(0, FONT_NORMAL, gText_WhichPCShouldBeAccessed, 0, NULL, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_WHITE, TEXT_COLOR_LIGHT_GRAY);
}

#define tState        data[0]
#define tSpecies      data[1]
#define tMonSpriteId  data[2]
#define tWindowId     data[5]

static void Task_PokemonPicWindow(u8 taskId)
{
    struct Task *task = &gTasks[taskId];
    switch (task->tState)
    {
    case 0:
        task->tState++;
        break;
    case 1:
        break;
    case 2:
        FreeResourcesAndDestroySprite(&gSprites[task->tMonSpriteId], task->tMonSpriteId);
        task->tState++;
        break;
    case 3:
        ClearToTransparentAndRemoveWindow(task->tWindowId);
        DestroyTask(taskId);
        break;
    }
}

bool8 ScriptMenu_ShowPokemonPic(u16 species, u8 x, u8 y)
{
    u8 spriteId;
    u8 taskId;

    if (QL_AvoidDisplay(QL_DestroyAbortedDisplay) == TRUE)
        return TRUE;
    if (FindTaskIdByFunc(Task_PokemonPicWindow) != TASK_NONE)
        return FALSE;

    spriteId = CreateMonSprite_PicBox(species, 8 * x + 40, 8 * y + 40, FALSE);
    taskId = CreateTask(Task_PokemonPicWindow, 80);
    gTasks[taskId].tWindowId = CreateWindowFromRect(x, y, 8, 8);
    gTasks[taskId].tState = 0;
    gTasks[taskId].tSpecies = species;
    gTasks[taskId].tMonSpriteId = spriteId;
    gSprites[spriteId].callback = SpriteCallbackDummy;
    gSprites[spriteId].oam.priority = 0;
    SetStandardWindowBorderStyle(gTasks[taskId].tWindowId, TRUE);
    ScheduleBgCopyTilemapToVram(0);

    return TRUE;
}

bool8 (*ScriptMenu_HidePokemonPic(void))(void)
{
    u8 taskId = FindTaskIdByFunc(Task_PokemonPicWindow);
    if (taskId == TASK_NONE)
        return NULL;

    gTasks[taskId].tState++;
    return IsPicboxClosed;
}

static bool8 IsPicboxClosed(void)
{
    if (FindTaskIdByFunc(Task_PokemonPicWindow) == TASK_NONE)
        return TRUE;
    else
        return FALSE;
}

static u8 CreateWindowFromRect(u8 left, u8 top, u8 width, u8 height)
{
    struct WindowTemplate template = CreateWindowTemplate(0, left + 1, top + 1, width, height, 15, 0x038);
    u8 windowId = AddWindow(&template);
    PutWindowTilemap(windowId);

    return windowId;
}

static void ClearToTransparentAndRemoveWindow(u8 windowId)
{
    ClearWindowTilemap(windowId);
    ClearStdWindowAndFrameToTransparent(windowId, TRUE);
    RemoveWindow(windowId);
}

static void DrawLinkServicesMultichoiceMenu(enum MultichoiceID mcId)
{
    switch (mcId)
    {
    case MULTI_TRADE_CENTER_COLOSSEUM:
        FillWindowPixelBuffer(0, PIXEL_FILL(1));
        AddTextPrinterParameterized2(0, FONT_NORMAL, sDescriptionPtrs_CableClub_TradeBattleCancel[Menu_GetCursorPos()], 0, NULL, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_WHITE, TEXT_COLOR_LIGHT_GRAY);
        break;
    case MULTI_TRADE_COLOSSEUM_CRUSH:
        FillWindowPixelBuffer(0, PIXEL_FILL(1));
        AddTextPrinterParameterized2(0, FONT_NORMAL, sDescriptionPtrs_WirelessCenter_TradeBattleCrushCancel[Menu_GetCursorPos()], 0, NULL, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_WHITE, TEXT_COLOR_LIGHT_GRAY);
        break;
    case MULTI_TRADE_COLOSSEUM_2:
        FillWindowPixelBuffer(0, PIXEL_FILL(1));
        AddTextPrinterParameterized2(0, FONT_NORMAL, sDescriptionPtrs_WirelessCenter_TradeBattleCancel[Menu_GetCursorPos()], 0, NULL, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_WHITE, TEXT_COLOR_LIGHT_GRAY);
        break;
    default:
        return;
    }
}

void PicboxCancel(void)
{
    struct Task *task;
    u8 taskId = FindTaskIdByFunc(Task_PokemonPicWindow);

    if (taskId == TASK_NONE)
        return;

    task = &gTasks[taskId];
    switch (task->tState)
    {
    case 0:
    case 1:
    case 2:
        FreeResourcesAndDestroySprite(&gSprites[task->tMonSpriteId], task->tMonSpriteId);
        ClearToTransparentAndRemoveWindow(task->tWindowId);
        DestroyTask(taskId);
        break;
    case 3:
        ClearToTransparentAndRemoveWindow(task->tWindowId);
        DestroyTask(taskId);
        break;
    }
}

void Task_WaitMuseumFossilPic(u8 taskId)
{
    struct Task *task = &gTasks[taskId];
    switch (task->tState)
    {
    case 0:
        task->tState++;
        break;
    case 1:
        break;
    case 2:
        DestroySprite(&gSprites[task->tMonSpriteId]);
        FreeSpriteTilesByTag(GFXTAG_FOSSIL);
        task->tState++;
        break;
    case 3:
        ClearToTransparentAndRemoveWindow(task->tWindowId);
        DestroyTask(taskId);
        break;
    }
}

#define FOSSIL_PIC_PAL_NUM  13

bool8 OpenMuseumFossilPic(void)
{
    u8 spriteId;
    u8 taskId;
    if (QL_AvoidDisplay(QL_DestroyAbortedDisplay) == TRUE)
        return TRUE;
    if (FindTaskIdByFunc(Task_WaitMuseumFossilPic) != TASK_NONE)
        return FALSE;
    if (gSpecialVar_0x8004 == SPECIES_KABUTOPS)
    {
        LoadSpriteSheets(sMuseumKabutopsSprSheets);
        LoadPalette(sMuseumKabutopsSprPalette, OBJ_PLTT_ID(FOSSIL_PIC_PAL_NUM), sizeof(sMuseumKabutopsSprPalette));
    }
    else if (gSpecialVar_0x8004 == SPECIES_AERODACTYL)
    {
        LoadSpriteSheets(sMuseumAerodactylSprSheets);
        LoadPalette(sMuseumAerodactylSprPalette, OBJ_PLTT_ID(FOSSIL_PIC_PAL_NUM), sizeof(sMuseumAerodactylSprPalette));
    }
    else
    {
        return FALSE;
    }
    spriteId = CreateSprite(&sMuseumFossilSprTemplate, gSpecialVar_0x8005 * 8 + 40, gSpecialVar_0x8006 * 8 + 40, 0);
    gSprites[spriteId].oam.paletteNum = FOSSIL_PIC_PAL_NUM;
    taskId = CreateTask(Task_WaitMuseumFossilPic, 80);
    gTasks[taskId].tWindowId = CreateWindowFromRect(gSpecialVar_0x8005, gSpecialVar_0x8006, 8, 8);
    gTasks[taskId].tState = 0;
    gTasks[taskId].tMonSpriteId = spriteId;
    SetStandardWindowBorderStyle(gTasks[taskId].tWindowId, TRUE);
    ScheduleBgCopyTilemapToVram(0);
    return TRUE;
}

bool8 CloseMuseumFossilPic(void)
{
    u8 taskId = FindTaskIdByFunc(Task_WaitMuseumFossilPic);
    if (taskId == TASK_NONE)
        return FALSE;
    gTasks[taskId].tState++;
    return TRUE;
}

void QL_DestroyAbortedDisplay(void)
{
    u8 taskId;
    s16 *data;
    ScriptContext_SetupScript(EventScript_ReleaseEnd);

    taskId = FindTaskIdByFunc(Task_PokemonPicWindow);
    if (taskId != TASK_NONE)
    {
        data = gTasks[taskId].data;
        if (tState < 2)
            FreeResourcesAndDestroySprite(&gSprites[tMonSpriteId], tMonSpriteId);
    }

    taskId = FindTaskIdByFunc(Task_WaitMuseumFossilPic);
    if (taskId != TASK_NONE)
    {
        data = gTasks[taskId].data;
        if (tState < 2)
        {
            DestroySprite(&gSprites[tMonSpriteId]);
            FreeSpriteTilesByTag(GFXTAG_FOSSIL);
        }
    }
}

#undef tState
#undef tSpecies
#undef tMonSpriteId
#undef tWindowId

static int DisplayTextAndGetWidthInternal(const u8 *str)
{
    u8 temp[64];
    StringExpandPlaceholders(temp, str);
    return GetStringWidth(FONT_NORMAL, temp, 0);
}

int DisplayTextAndGetWidth(const u8 *str, int prevWidth)
{
    int width = DisplayTextAndGetWidthInternal(str);
    if (width < prevWidth)
    {
        width = prevWidth;
    }
    return width;
}

int ConvertPixelWidthToTileWidth(int width)
{
    return (((width + 9) / 8) + 1) > MAX_MULTICHOICE_WIDTH ? MAX_MULTICHOICE_WIDTH : (((width + 9) / 8) + 1);
}

int ScriptMenu_AdjustLeftCoordFromWidth(int left, int width)
{
    int adjustedLeft = left;

    if (left + width > MAX_MULTICHOICE_WIDTH)
    {
        if (MAX_MULTICHOICE_WIDTH - width < 0)
        {
            adjustedLeft = 0;
        }
        else
        {
            adjustedLeft = MAX_MULTICHOICE_WIDTH - width;
        }
    }

    return adjustedLeft;
}

void DrawSeagallopDestinationMenu(void)
{
    // 8004 = Starting location
    // 8005 = Page (0: Verm, One, Two, Three, Four, Other, Exit; 1: Four, Five, Six, Seven, Other, Exit)
    u8 destinationId;
    u8 top;
    u8 numItems;
    u8 cursorWidth;
    u8 windowId;
    u8 i;
    gSpecialVar_Result = SCR_MENU_UNSET;

    if (QL_AvoidDisplay(QL_DestroyAbortedDisplay) == TRUE)
        return;

    if (gSpecialVar_0x8005 == 1)
    {
        if (gSpecialVar_0x8004 < SEAGALLOP_FIVE_ISLAND)
            destinationId = SEAGALLOP_FIVE_ISLAND;
        else
            destinationId = SEAGALLOP_FOUR_ISLAND;
        numItems = 5;
        top = 2;
    }
    else
    {
        destinationId = SEAGALLOP_VERMILION_CITY;
        numItems = 6;
        top = 0;
    }
    cursorWidth = GetMenuCursorDimensionByFont(FONT_NORMAL, 0);
    windowId = CreateWindowFromRect(17, top, 11, numItems * 2);
    SetStandardWindowBorderStyle(windowId, FALSE);

    // -2 excludes "Other" and "Exit", appended after the loop
    for (i = 0; i < numItems - 2; i++)
    {
        if (destinationId != gSpecialVar_0x8004)
            AddTextPrinterParameterized(windowId, FONT_NORMAL, sSeagallopDestStrings[destinationId], cursorWidth, i * 16 + 2, TEXT_SKIP_DRAW, NULL);
        else
            i--;
        destinationId++;

        // Wrap around
        if (destinationId == SEAGALLOP_SEVEN_ISLAND + 1)
            destinationId = SEAGALLOP_VERMILION_CITY;
    }
    AddTextPrinterParameterized(windowId, FONT_NORMAL, sText_Other, cursorWidth, i * 16 + 2, TEXT_SKIP_DRAW, NULL);
    i++;
    AddTextPrinterParameterized(windowId, FONT_NORMAL, gOtherText_Exit, cursorWidth, i * 16 + 2, TEXT_SKIP_DRAW, NULL);
    InitMenuNormal(windowId, FONT_NORMAL, 0, 2, 16, numItems, 0);
    InitMultichoiceCheckWrap(FALSE, numItems, windowId, MULTI_NONE);
    ScheduleBgCopyTilemapToVram(0);
}

u16 GetSelectedSeagallopDestination(void)
{
    // 8004 = Starting location
    // 8005 = Page (0: Verm, One, Two, Three, Four, Other, Exit; 1: Four, Five, Six, Seven, Other, Exit)
    if (gSpecialVar_Result == MULTI_B_PRESSED)
        return MULTI_B_PRESSED;
    if (gSpecialVar_0x8005 == 1)
    {
        if (gSpecialVar_Result == 3)
        {
            return SEAGALLOP_MORE;
        }
        else if (gSpecialVar_Result == 4)
        {
            return MULTI_B_PRESSED;
        }
        else if (gSpecialVar_Result == 0)
        {
            if (gSpecialVar_0x8004 > SEAGALLOP_FOUR_ISLAND)
                return SEAGALLOP_FOUR_ISLAND;
            else
                return SEAGALLOP_FIVE_ISLAND;
        }
        else if (gSpecialVar_Result == 1)
        {
            if (gSpecialVar_0x8004 > SEAGALLOP_FIVE_ISLAND)
                return SEAGALLOP_FIVE_ISLAND;
            else
                return SEAGALLOP_SIX_ISLAND;
        }
        else if (gSpecialVar_Result == 2)
        {
            if (gSpecialVar_0x8004 > SEAGALLOP_SIX_ISLAND)
                return SEAGALLOP_SIX_ISLAND;
            else
                return SEAGALLOP_SEVEN_ISLAND;
        }
    }
    else
    {
        if (gSpecialVar_Result == 4)
            return SEAGALLOP_MORE;
        else if (gSpecialVar_Result == 5)
            return MULTI_B_PRESSED;
        else if (gSpecialVar_Result >= gSpecialVar_0x8004)
            return gSpecialVar_Result + 1;
        else
            return gSpecialVar_Result;
    }
    return SEAGALLOP_VERMILION_CITY;
}

static u16 GetStringTilesWide(const u8 *str)
{
    return (GetStringWidth(FONT_NORMAL_COPY_1, str, 0) + 7) / 8;
}

static u8 GetMenuWidthFromList(const struct MenuAction *items, u8 count)
{
    u16 i;
    u8 width = GetStringTilesWide(items[0].text);
    u8 tmp;

    for (i = 1; i < count; i++)
    {
        tmp = GetStringTilesWide(items[i].text);
        if (width < tmp)
            width = tmp;
    }
    return width;
}

static u8 GetMCWindowHeight(u8 count)
{
    switch (count)
    {
    case 0:
        return 1;
    case 1:
        return 2;
    case 2:
        return 4;
    case 3:
        return 6;
    case 4:
        return 7;
    case 5:
        return 9;
    case 6:
        return 11;
    case 7:
        return 13;
    case 8:
        return 14;
    default:
        return 1;
    }
}

static u32 GetMultiChoiceWindowHeight(u8 argc, u8 maxBeforeScroll)
{
    u32 windowHeight;
    u8 numItems = argc < maxBeforeScroll ? argc : maxBeforeScroll;

    windowHeight = numItems * GetFontAttribute(sScriptableListMenuTemplate.fontId, FONTATTR_MAX_LETTER_HEIGHT);

    return (windowHeight + 7) / 8;
}
