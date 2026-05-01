#ifndef GUARD_WILD_ENCOUNTER_H
#define GUARD_WILD_ENCOUNTER_H

#include "global.h"
#include "rtc.h"
#include "constants/metatile_behaviors.h"

enum WildPokemonArea
{
    WILD_AREA_LAND,
    WILD_AREA_WATER,
    WILD_AREA_ROCKS,
    WILD_AREA_FISHING,
    WILD_AREA_HIDDEN
};

#define LAND_WILD_COUNT     12
#define WATER_WILD_COUNT    5
#define ROCK_WILD_COUNT     5
#define FISH_WILD_COUNT     10
#define HIDDEN_WILD_COUNT   3

#define NUM_ALTERING_CAVE_TABLES 9

#define FISHING_CHAIN_LENGTH_MAX 999
#define FISHING_CHAIN_SHINY_STREAK_MAX 20

struct WildPokemon
{
    u8 minLevel;
    u8 maxLevel;
    enum Species species;
};

struct WildPokemonInfo
{
    u8 encounterRate;
    const struct WildPokemon *wildPokemon;
};

struct WildEncounterTypes
{
    const struct WildPokemonInfo *landMonsInfo;
    const struct WildPokemonInfo *waterMonsInfo;
    const struct WildPokemonInfo *rockSmashMonsInfo;
    const struct WildPokemonInfo *fishingMonsInfo;
    const struct WildPokemonInfo *hiddenMonsInfo;
};

struct WildPokemonHeader
{
    u8 mapGroup;
    u8 mapNum;
    const struct WildEncounterTypes encounterTypes[(OW_SEASON_ENCOUNTERS ? SEASON_COUNT : 1)][(OW_TIME_OF_DAY_ENCOUNTERS ? TIMES_OF_DAY_COUNT : 1)];
};

extern const struct WildPokemonHeader gWildMonHeaders[];
extern bool8 gIsFishingEncounter;
extern bool8 gIsSurfingEncounter;
extern u8 gChainFishingDexNavStreak;

bool32 MapHasNoEncounterData(void);
bool32 StandardWildEncounter(u32 currMetatileAttrs, enum MetatileBehavior previousMetaTileBehavior);
bool8 DoesCurrentMapHaveFishingMons(void);
bool8 SweetScentWildEncounter(void);
bool8 SweetScentWildEncounter(void);
bool8 TryDoDoubleWildBattle(void);
bool8 TryStandardWildEncounter(u32 currMetatileAttrs);
bool8 UpdateRepelCounter(void);
u16 GetCurrentMapWildMonHeaderId(void);
u16 GetLocalWaterMon(void);
u16 GetLocalWildMon(bool8 *isWaterMon);
u32 ChooseWildMonIndex_Rocks(void);
u32 ChooseWildMonIndex_Water(void);
u8 ChooseHiddenMonIndex(void);
u8 ChooseWildMonIndex_Land(void);
void CreateWildMon(enum Species species, u8 level, u8 unownSlot);
void DisableWildEncounters(bool8 disabled);
void DisableWildEncounters(bool8 state);
void FishingWildEncounter(u8 rod);
void GetSeasonAndTimeOfDayForEncounters(u32 headerId, enum WildPokemonArea area, enum Season *season, enum TimeOfDay *timeOfDay);
void ResetEncounterRateModifiers(void);
void SeedWildEncounterRng(u16 randVal);

#endif // GUARD_WILD_ENCOUNTER_H
