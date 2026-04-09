#ifndef GUARD_HEAL_LOCATION_H
#define GUARD_HEAL_LOCATION_H

#include "global.h"
#include "constants/heal_locations.h"

struct HealLocation
{
    s8 mapGroup;
    s8 mapNum;
    s16 x;
    s16 y;
};

const struct HealLocation *GetHealLocation(enum HealLocationID healLocationId);
void SetWhiteoutRespawnWarpAndHealerNPC(struct WarpData *warp);
u32 GetHealNpcLocalId(enum HealLocationID healLocationId);
enum HealLocationID GetHealLocationIndexByWarpData(struct WarpData *warp);

#endif // GUARD_HEAL_LOCATION_H
