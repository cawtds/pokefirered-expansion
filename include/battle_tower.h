#ifndef GUARD_BATTLE_TOWER_H
#define GUARD_BATTLE_TOWER_H

#include "global.h"
#include "constants/battle_tower.h"

#define BATTLE_TOWER_RECORD_MIXING_TRAINER_BASE_ID 100

struct BattleTowerTrainer
{
    u8 trainerClass;
    u8 name[3];
    u8 teamFlags;
    u16 greeting[6];
};

struct BattleTowerPokemonTemplate
{
    u16 species;
    u8 heldItem;
    u8 teamFlags;
    u16 moves[4];
    u8 evSpread;
    u8 nature;
};


u8 GetBattleTowerTrainerFrontSpriteId(u16 trainerId);
u8 GetBattleTowerTrainerClassNameId(void);
void GetBattleTowerTrainerName(u8 *text);

#endif //GUARD_BATTLE_TOWER_H
