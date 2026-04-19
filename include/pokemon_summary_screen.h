#ifndef GUARD_POKEMON_SUMMARY_SCREEN_H
#define GUARD_POKEMON_SUMMARY_SCREEN_H

#include "main.h"

#define TAG_PSS_MOVE_SELECTION_CURSOR_0 100
#define TAG_PSS_MOVE_SELECTION_CURSOR_1 101
#define TAG_PSS_MOVE_SELECTION_CURSOR_2 102
#define TAG_PSS_MOVE_SELECTION_CURSOR_3 103
#define TAG_PSS_STATUS_ICON             110
#define TAG_PSS_HP_BAR_0                120
#define TAG_PSS_EXP_BAR_0               130
#define TAG_PSS_MON_MARKINGS            140
#define TAG_PSS_POKERUS_ICON            150
#define TAG_PSS_SHINY_ICON              160

// The Pokemon Summary Screen can operate in different modes. Certain features,
// such as move re-ordering, are available in the different modes.
enum PokemonSummaryScreenMode
{
    PSS_MODE_NORMAL,
    PSS_MODE_UNK1,
    PSS_MODE_SELECT_MOVE,
    PSS_MODE_FORGET_MOVE,
    PSS_MODE_TRADE,
    PSS_MODE_BOX,
    SUMMARY_MODE_RELEARNER_BATTLE, // returning from move relearner initiated from battle moves page
};

enum PokemonSummaryScreenPage
{
    PSS_PAGE_INFO,
    PSS_PAGE_SKILLS,
    PSS_PAGE_MOVES,
    PSS_PAGE_MOVES_INFO,
    PSS_PAGE_MOVE_DELETER,
};

enum PokemonSummaryScreenState
{
    PSS_STATE_FADEIN,
    PSS_STATE_PLAYCRY,
    PSS_STATE_HANDLEINPUT,
    PSS_STATE_FLIPPAGES,
    PSS_STATE_ATEXIT_FADEOUT,
    PSS_STATE_ATEXIT_WAITLINKDELAY,
    PSS_STATE_ATEXIT_WAITFADE
};

enum PokemonSummaryScreenStat
{
    PSS_STAT_HP,
    PSS_STAT_ATK,
    PSS_STAT_DEF,
    PSS_STAT_SPA,
    PSS_STAT_SPD,
    PSS_STAT_SPE
};

enum PokemonSummaryScreenSkillPageMode
{
    PSS_SKILL_PAGE_STATS,
    PSS_SKILL_PAGE_IVS,
    PSS_SKILL_PAGE_EVS,
    PSS_SKILL_PAGE_MODE_COUNT
};

struct StatData {
    u8 monDataStat;
    u8 monDataEv;
    u8 monDataIv;
    u8 monDataHyperTrained;
    u8 pssStat;
};

extern u8 gLastViewedMonIndex;
extern const u8 gNotDoneYetDescription[];
extern const struct CompressedSpriteSheet gSpriteSheet_CategoryIcons;
extern const struct SpritePalette gSpritePal_CategoryIcons;
extern const struct SpriteTemplate gSpriteTemplate_CategoryIcons;
extern MainCallback gInitialSummaryScreenCallback;

void ShowSelectMovePokemonSummaryScreen(struct Pokemon *mons, u8 monIndex, void (*callback)(void), enum Move newMove);
u8 GetMoveSlotToReplace(void);
void SummaryScreen_SetUnknownTaskId(u8 a0);
void SummaryScreen_DestroyUnknownTask(void);
u8 GetLastViewedMonIndex(void);
void ShowPokemonSummaryScreen(void *mons, u8 cursorPos, u8 lastIdx, void (*callback)(void), u8 a4);
void SetPokemonSummaryScreenMode(u8);
void SummaryScreen_SetAnimDelayTaskId(u8 taskId);

#endif // GUARD_POKEMON_SUMMARY_SCREEN_H
