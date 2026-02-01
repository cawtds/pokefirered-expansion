#include "global.h"
#include "battle_tower.h"
#include "random.h"
#include "text.h"
#include "event_data.h"
#include "string_util.h"
#include "battle.h"
#include "data.h"
#include "pokedex.h"
#include "overworld.h"
#include "battle_transition.h"
#include "easy_chat.h"
#include "battle_setup.h"
#include "battle_main.h"
#include "task.h"
#include "battle.h"
#include "party_menu.h"
#include "new_game.h"
#include "save.h"
#include "item.h"
#include "script_pokemon_util.h"
#include "constants/abilities.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/pokemon.h"
#include "constants/event_objects.h"
#include "constants/battle_special.h"

static const struct BattleTowerTrainer sBattleTowerTrainers[] =
{
// added entry to fix warnings
    {
        .trainerClass = FACILITY_CLASS_YOUNGSTER,
        .name = _("X"),
        .teamFlags = 0x01,
        .greeting = {
            EC_WORD_ARE,
            EC_WORD_YOU,
            EC_WORD_READY,
            EC_WORD_QUES,
            EC_WORD_HERE_I_COME,
            EC_WORD_EXCL,
        },
    },
};


#include "data/battle_tower/level_50_mons.h"
#include "data/battle_tower/level_100_mons.h"

u8 GetBattleTowerTrainerFrontSpriteId(u16 trainerId)
{
#if FREE_BATTLE_TOWER_E_READER == FALSE
    if (trainerId == BATTLE_TOWER_EREADER_TRAINER_ID)
    {
        return gFacilityClassToPicIndex[gSaveBlock2Ptr->battleTower.ereaderTrainer.trainerClass];
    }
    else if (trainerId < BATTLE_TOWER_RECORD_MIXING_TRAINER_BASE_ID)
#else
    if (trainerId < BATTLE_TOWER_RECORD_MIXING_TRAINER_BASE_ID)
#endif //FREE_BATTLE_TOWER_E_READER
    {
        return TRAINER_PIC_RED;
    }
    else
    {
        return gFacilityClassToPicIndex[gSaveBlock2Ptr->battleTower.records[trainerId - BATTLE_TOWER_RECORD_MIXING_TRAINER_BASE_ID].trainerClass];
    }
}

u8 GetBattleTowerTrainerClassNameId(void)
{
#if FREE_BATTLE_TOWER_E_READER == FALSE
    if (gSaveBlock2Ptr->battleTower.battleTowerTrainerId == BATTLE_TOWER_EREADER_TRAINER_ID)
    {
        return gFacilityClassToTrainerClass[gSaveBlock2Ptr->battleTower.ereaderTrainer.trainerClass];
    }
    else if (gSaveBlock2Ptr->battleTower.battleTowerTrainerId >= BATTLE_TOWER_RECORD_MIXING_TRAINER_BASE_ID)
#else
    if (gSaveBlock2Ptr->battleTower.battleTowerTrainerId >= BATTLE_TOWER_RECORD_MIXING_TRAINER_BASE_ID)
#endif //FREE_BATTLE_TOWER_E_READER
    {
        return gFacilityClassToTrainerClass[gSaveBlock2Ptr->battleTower.records[gSaveBlock2Ptr->battleTower.battleTowerTrainerId - BATTLE_TOWER_RECORD_MIXING_TRAINER_BASE_ID].trainerClass];
    }
    else
    {
        return TRAINER_CLASS_PLAYER;
    }
}

void GetBattleTowerTrainerName(u8 *dest)
{
    s32 i;
#if FREE_BATTLE_TOWER_E_READER == FALSE
    if (gSaveBlock2Ptr->battleTower.battleTowerTrainerId == BATTLE_TOWER_EREADER_TRAINER_ID)
    {
        for (i = 0; i < PLAYER_NAME_LENGTH; i++)
            dest[i] = gSaveBlock2Ptr->battleTower.ereaderTrainer.name[i];
    }
    else if (gSaveBlock2Ptr->battleTower.battleTowerTrainerId < BATTLE_TOWER_RECORD_MIXING_TRAINER_BASE_ID)
#else
    if (gSaveBlock2Ptr->battleTower.battleTowerTrainerId < BATTLE_TOWER_RECORD_MIXING_TRAINER_BASE_ID)
#endif //FREE_BATTLE_TOWER_E_READER
    {
        for (i = 0; i < 3; i++)
            dest[i] = sBattleTowerTrainers[gSaveBlock2Ptr->battleTower.battleTowerTrainerId].name[i];
    }
    else
    {
        for (i = 0; i < PLAYER_NAME_LENGTH; i++)
            dest[i] = gSaveBlock2Ptr->battleTower.records[gSaveBlock2Ptr->battleTower.battleTowerTrainerId - BATTLE_TOWER_RECORD_MIXING_TRAINER_BASE_ID].name[i];
    }
    dest[i] = EOS;
}
