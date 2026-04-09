#include "global.h"
#include "event_data.h"
#include "heal_location.h"
#include "constants/event_objects.h"
#include "constants/heal_locations.h"
#include "constants/map_event_ids.h"
#include "constants/maps.h"

// Arrays described here because mapjson will overrwrite the below data file

// sHealLocations
// This array defines the fly points for unlocked spawns.

// sWhiteoutRespawnHealCenterMapIdxs
// This array defines the map where you actually respawn when you white out,
// based on where you last checkpointed.
// This is either the player's house or a Pokémon Center.
// The data are u16 instead of u8 for reasons unknown.

// sWhiteoutRespawnHealerNpcIds
// When you respawn, your character scurries back to either their house
// or a Pokémon Center, and hands their fainted Pokémon to their mother
// or the Nurse for healing.
// This array defines the index of the NPC on the map defined above
// with whom your character interacts in this cutscene.

#include "data/heal_locations.h"

enum HealLocationID GetHealLocationIndexByWarpData(struct WarpData *warp)
{
    for (enum HealLocationID i = HEAL_LOCATION_NONE; i < ARRAY_COUNT(sHealLocations); i++)
    {
        if (sHealLocations[i].mapGroup == warp->mapGroup
        && sHealLocations[i].mapNum == warp->mapNum
        && sHealLocations[i].x == warp->x
        && sHealLocations[i].y == warp->y)
            return i + 1;
    }
    return HEAL_LOCATION_NONE;
}

const struct HealLocation *GetHealLocation(enum HealLocationID healLocationId)
{
    if (healLocationId == HEAL_LOCATION_NONE)
        return NULL;
    if (healLocationId > ARRAY_COUNT(sHealLocations))
        return NULL;
    return &sHealLocations[healLocationId - 1];
}

void SetWhiteoutRespawnWarpAndHealerNPC(struct WarpData *warp)
{
    enum HealLocationID healLocationId;
    u32 healNpcLocalId;

    if (VarGet(VAR_MAP_SCENE_TRAINER_TOWER) == 1)
    {
#if FREE_TRAINER_TOWER == FALSE
        if (!gSaveBlock1Ptr->trainerTower[gSaveBlock1Ptr->towerChallengeId].spokeToOwner)
#endif //FREE_TRAINER_TOWER
            VarSet(VAR_MAP_SCENE_TRAINER_TOWER, 0);

        healLocationId = HEAL_LOCATION_TRAINER_TOWER;
    }
    else
    {
        healLocationId = GetHealLocationIndexByWarpData(&gSaveBlock1Ptr->lastHealLocation);
    }

    healNpcLocalId = GetHealNpcLocalId(healLocationId);

    if (healNpcLocalId == LOCALID_NONE)
    {
        *(warp) = gSaveBlock1Ptr->lastHealLocation;
        return;
    }

    warp->mapGroup = sWhiteoutRespawnHealCenterMapIdxs[healLocationId - 1].mapGroup;
    warp->mapNum = sWhiteoutRespawnHealCenterMapIdxs[healLocationId - 1].mapNum;
    warp->warpId = WARP_ID_NONE;
    warp->x = sWhiteoutRespawnHealCenterMapIdxs[healLocationId - 1].x;
    warp->y = sWhiteoutRespawnHealCenterMapIdxs[healLocationId - 1].y;
    gSpecialVar_LastTalked = healNpcLocalId;
}

u32 GetHealNpcLocalId(enum HealLocationID healLocationId)
{
    if (healLocationId == HEAL_LOCATION_NONE || healLocationId >= NUM_HEAL_LOCATIONS)
        return LOCALID_NONE;

    return sWhiteoutRespawnHealerNpcIds[healLocationId - 1];
}
