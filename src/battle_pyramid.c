#include "global.h"
#include "battle_pyramid.h"
#include "constants/battle_pyramid.h"
#include "constants/layouts.h"

u8 CurrentBattlePyramidLocation(void)
{
    if (gMapHeader.mapLayoutId == LAYOUT_BATTLE_FRONTIER_BATTLE_PYRAMID_FLOOR)
        return PYRAMID_LOCATION_FLOOR;
    else if (gMapHeader.mapLayoutId == LAYOUT_BATTLE_FRONTIER_BATTLE_PYRAMID_TOP)
        return PYRAMID_LOCATION_TOP;
    else
        return PYRAMID_LOCATION_NONE;
}
