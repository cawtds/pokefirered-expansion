#include "global.h"
#include "metatile_behavior.h"
#include "constants/metatile_behaviors.h"

static const bool8 sBehaviorSurfable[NUM_METATILE_BEHAVIORS] = {
    [MB_POND_WATER]         = TRUE,
    [MB_FAST_WATER]         = TRUE,
    [MB_DEEP_WATER]         = TRUE,
    [MB_WATERFALL]          = TRUE,
    [MB_OCEAN_WATER]        = TRUE,
    [MB_UNUSED_WATER]       = TRUE,
    [MB_CYCLING_ROAD_WATER] = TRUE,
    [MB_EASTWARD_CURRENT]   = TRUE,
    [MB_WESTWARD_CURRENT]   = TRUE,
    [MB_NORTHWARD_CURRENT]  = TRUE,
    [MB_SOUTHWARD_CURRENT]  = TRUE
};

static const u8 sTileBitAttributes[32] = {
    [0] = 0,
    [1] = 1 << 0,
    [2] = 1 << 1,
    [3] = 1 << 2,
    [4] = 1 << 3,
};

bool32 MetatileBehavior_IsATile(enum MetatileBehavior metatileBehavior)
{
    return TRUE;
}

bool32 MetatileBehavior_IsJumpEast(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_JUMP_EAST)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsJumpWest(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_JUMP_WEST)
            return TRUE;
        else
            return FALSE;
}

bool32 MetatileBehavior_IsJumpNorth(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_JUMP_NORTH)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsJumpSouth(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_JUMP_SOUTH)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsPokeGrass(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_TALL_GRASS || metatileBehavior == MB_CYCLING_ROAD_PULL_DOWN_GRASS)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsSand(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_SAND || metatileBehavior == MB_SAND_CAVE)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsSnow(enum MetatileBehavior metatileBehavior)
{
    return metatileBehavior == MB_SNOW;
}

bool32 MetatileBehavior_IsSandOrShallowFlowingWater(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_SAND || metatileBehavior == MB_SHALLOW_WATER)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsDeepSand(enum MetatileBehavior metatileBehavior) { return FALSE; }

bool32 MetatileBehavior_IsReflective(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_POND_WATER
     || metatileBehavior == MB_PUDDLE
     || metatileBehavior == MB_UNUSED_WATER
     || metatileBehavior == MB_CYCLING_ROAD_WATER
     || metatileBehavior == MB_ICE)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsIce(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_ICE)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsWarpDoor(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_WARP_DOOR)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsWarpDoor_2(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_WARP_DOOR)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsEscalator(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior >= MB_UP_ESCALATOR && metatileBehavior <= MB_DOWN_ESCALATOR)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsDirectionalUpRightStairWarp(enum MetatileBehavior metatileBehavior)
{
    u8 result = FALSE;

    if (metatileBehavior == MB_UP_RIGHT_STAIR_WARP)
        result = TRUE;

    return result;
}

bool32 MetatileBehavior_IsDirectionalUpLeftStairWarp(enum MetatileBehavior metatileBehavior)
{
    u8 result = FALSE;

    if (metatileBehavior == MB_UP_LEFT_STAIR_WARP)
        result = TRUE;

    return result;
}

bool32 MetatileBehavior_IsDirectionalDownRightStairWarp(enum MetatileBehavior metatileBehavior)
{
    u8 result = FALSE;

    if (metatileBehavior == MB_DOWN_RIGHT_STAIR_WARP)
        result = TRUE;

    return result;
}

bool32 MetatileBehavior_IsDirectionalDownLeftStairWarp(enum MetatileBehavior metatileBehavior)
{
    u8 result = FALSE;

    if (metatileBehavior == MB_DOWN_LEFT_STAIR_WARP)
        result = TRUE;

    return result;
}

bool32 MetatileBehavior_IsDirectionalStairWarp(enum MetatileBehavior metatileBehavior)
{
    bool8 result = FALSE;

    if (metatileBehavior >= MB_UP_RIGHT_STAIR_WARP && metatileBehavior <= MB_DOWN_LEFT_STAIR_WARP)
        result = TRUE;
    else
        result = FALSE;

    return result;
}

bool32 MetatileBehavior_IsLadder(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_LADDER)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsNonAnimDoor(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_CAVE_DOOR)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsDeepSouthWarp(enum MetatileBehavior metatileBehavior) { return FALSE; }

bool32 MetatileBehavior_IsSurfable(enum MetatileBehavior metatileBehavior)
{
    if (sBehaviorSurfable[metatileBehavior] & 1)
        return TRUE;
    else
        return FALSE;
}

// Water that's too fast to surf on
bool32 MetatileBehavior_IsFastWater(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_FAST_WATER)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsEastArrowWarp(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_EAST_ARROW_WARP)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsWestArrowWarp(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_WEST_ARROW_WARP)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsNorthArrowWarp(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_NORTH_ARROW_WARP)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsSouthArrowWarp(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_SOUTH_ARROW_WARP)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsArrowWarp(enum MetatileBehavior metatileBehavior)
{
    u8 result = FALSE;

    if (MetatileBehavior_IsEastArrowWarp(metatileBehavior)
     || MetatileBehavior_IsWestArrowWarp(metatileBehavior)
     || MetatileBehavior_IsNorthArrowWarp(metatileBehavior)
     || MetatileBehavior_IsSouthArrowWarp(metatileBehavior))
        result = TRUE;

    return result;
}

bool32 MetatileBehavior_IsForcedMovementTile(enum MetatileBehavior metatileBehavior)
{
    if ((metatileBehavior >= MB_WALK_EAST && metatileBehavior <= MB_TRICK_HOUSE_PUZZLE_8_FLOOR)
      || (metatileBehavior >= MB_EASTWARD_CURRENT && metatileBehavior <= MB_SOUTHWARD_CURRENT)
      ||  metatileBehavior == MB_WATERFALL
      ||  metatileBehavior == MB_ICE
      || (metatileBehavior >= MB_SPIN_RIGHT && metatileBehavior <= MB_SPIN_DOWN))
            return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsIce_2(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_ICE)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsTrickHouseSlipperyFloor(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_TRICK_HOUSE_PUZZLE_8_FLOOR)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsWalkNorth(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_WALK_NORTH)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsWalkSouth(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_WALK_SOUTH)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsWalkWest(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_WALK_WEST)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsWalkEast(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_WALK_EAST)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsNorthwardCurrent(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_NORTHWARD_CURRENT)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsSouthwardCurrent(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_SOUTHWARD_CURRENT)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsWestwardCurrent(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_WESTWARD_CURRENT)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsEastwardCurrent(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_EASTWARD_CURRENT)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsSlideNorth(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_SLIDE_NORTH)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsSlideSouth(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_SLIDE_SOUTH)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsSlideWest(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_SLIDE_WEST)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsSlideEast(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_SLIDE_EAST)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsCounter(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_COUNTER)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsPlayerFacingTVScreen(enum MetatileBehavior metatileBehavior, u8 playerDirection)
{
    if (playerDirection != DIR_NORTH)
        return FALSE;
    else if (metatileBehavior == MB_TELEVISION)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsPC(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_PC)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_HasRipples(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_POND_WATER || metatileBehavior == MB_PUDDLE)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsPuddle(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_PUDDLE)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsTallGrass(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_TALL_GRASS || metatileBehavior == MB_CYCLING_ROAD_PULL_DOWN_GRASS)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsLongGrass(enum MetatileBehavior metatileBehavior) { return FALSE; }
bool32 MetatileBehavior_IsAshGrass(enum MetatileBehavior metatileBehavior) { return FALSE; }
bool32 MetatileBehavior_IsFootprints(enum MetatileBehavior metatileBehavior) { return FALSE; }
bool32 MetatileBehavior_IsBridge(enum MetatileBehavior metatileBehavior) { return FALSE; }
bool32 MetatileBehavior_GetBridgeType(enum MetatileBehavior metatileBehavior) { return FALSE; }

bool32 MetatileBehavior_IsUnused01(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_UNUSED_01)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_UnusedIsTallGrass(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_TALL_GRASS)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsIndoorEncounter(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_INDOOR_ENCOUNTER)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsMountain(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_MOUNTAIN_TOP)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsDiveable(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior >= MB_FAST_WATER && metatileBehavior <= MB_DEEP_WATER)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsUnableToEmerge(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_UNDERWATER_BLOCKED_ABOVE)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsShallowFlowingWater(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_SHALLOW_WATER)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsThinIce(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_THIN_ICE)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsCrackedIce(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_CRACKED_ICE)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsDeepWaterTerrain(enum MetatileBehavior metatileBehavior)
{
    if ((metatileBehavior >= MB_FAST_WATER && metatileBehavior <= MB_DEEP_WATER)
      || metatileBehavior == MB_OCEAN_WATER)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsUnusedWater(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_UNUSED_WATER)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsSurfableAndNotWaterfall(enum MetatileBehavior metatileBehavior)
{
    if (MetatileBehavior_IsSurfable(metatileBehavior)
        && !MetatileBehavior_IsWaterfall(metatileBehavior))
            return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsEastBlocked(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_IMPASSABLE_EAST
     || metatileBehavior == MB_IMPASSABLE_NORTHEAST
     || metatileBehavior == MB_IMPASSABLE_SOUTHEAST)
            return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsWestBlocked(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_IMPASSABLE_WEST
     || metatileBehavior == MB_IMPASSABLE_NORTHWEST
     || metatileBehavior == MB_IMPASSABLE_SOUTHWEST)
            return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsNorthBlocked(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_IMPASSABLE_NORTH
     || metatileBehavior == MB_IMPASSABLE_NORTHEAST
     || metatileBehavior == MB_IMPASSABLE_NORTHWEST)
            return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsSouthBlocked(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_IMPASSABLE_SOUTH
     || metatileBehavior == MB_IMPASSABLE_SOUTHEAST
     || metatileBehavior == MB_IMPASSABLE_SOUTHWEST)
            return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsShortGrass(enum MetatileBehavior metatileBehavior) { return FALSE; }

bool32 MetatileBehavior_IsHotSprings(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_HOT_SPRINGS)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsWaterfall(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_WATERFALL)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsFortreeBridge(enum MetatileBehavior metatileBehavior) { return FALSE; }
bool32 MetatileBehavior_IsPacifidlogVerticalLogTop(enum MetatileBehavior metatileBehavior){ return FALSE; }
bool32 MetatileBehavior_IsPacifidlogVerticalLogBottom(enum MetatileBehavior metatileBehavior) { return FALSE; }
bool32 MetatileBehavior_IsPacifidlogHorizontalLogLeft(enum MetatileBehavior metatileBehavior) { return FALSE; }
bool32 MetatileBehavior_IsPacifidlogHorizontalLogRight(enum MetatileBehavior metatileBehavior) { return FALSE; }
bool32 MetatileBehavior_IsPacifidlogLog(enum MetatileBehavior metatileBehavior) { return FALSE; }
bool32 MetatileBehavior_IsTrickHousePuzzleDoor(enum MetatileBehavior metatileBehavior) { return FALSE; }

bool32 MetatileBehavior_IsRegionMap(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_REGION_MAP)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsRoulette(enum MetatileBehavior metatileBehavior) { return FALSE; }
bool32 MetatileBehavior_IsPokeblockFeeder(enum MetatileBehavior metatileBehavior) { return FALSE; }
bool32 MetatileBehavior_IsSecretBaseJumpMat(enum MetatileBehavior metatileBehavior) { return FALSE; }
bool32 MetatileBehavior_IsSecretBaseSpinMat(enum MetatileBehavior metatileBehavior) { return FALSE; }
bool32 MetatileBehavior_IsLavaridgeB1FWarp(enum MetatileBehavior metatileBehavior) { return FALSE; }

bool32 MetatileBehavior_IsLavaridge1FWarp(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_LAVARIDGE_1F_WARP)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsWarpPad(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_REGULAR_WARP)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsUnionRoomWarp(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_UNION_ROOM_WARP)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsWater(enum MetatileBehavior metatileBehavior)
{
    if ((metatileBehavior >= MB_POND_WATER && metatileBehavior <= MB_DEEP_WATER)
     ||  metatileBehavior == MB_OCEAN_WATER
     || (metatileBehavior >= MB_EASTWARD_CURRENT && metatileBehavior <= MB_SOUTHWARD_CURRENT))
            return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsFallWarp(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_FALL_WARP)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsCrackedFloor(enum MetatileBehavior metatileBehavior){ return FALSE; }

bool32 MetatileBehavior_IsCyclingRoadPullDownTile(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior >= MB_CYCLING_ROAD_PULL_DOWN && metatileBehavior <= MB_CYCLING_ROAD_PULL_DOWN_GRASS)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsCyclingRoadPullDownTileGrass(enum MetatileBehavior metatileBehavior)
{
    return metatileBehavior == MB_CYCLING_ROAD_PULL_DOWN_GRASS;
}

bool32 MetatileBehavior_IsBumpySlope(enum MetatileBehavior metatileBehavior) { return FALSE; }
bool32 MetatileBehavior_IsIsolatedVerticalRail(enum MetatileBehavior metatileBehavior) { return FALSE; }
bool32 MetatileBehavior_IsIsolatedHorizontalRail(enum MetatileBehavior metatileBehavior) { return FALSE; }
bool32 MetatileBehavior_IsVerticalRail(enum MetatileBehavior metatileBehavior) { return FALSE; }
bool32 MetatileBehavior_IsHorizontalRail(enum MetatileBehavior metatileBehavior) { return FALSE; }

bool32 MetatileBehavior_IsSeaweed(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_SEAWEED)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsRunningDisallowed(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_RUNNING_DISALLOWED)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsPictureBookShelf(enum MetatileBehavior metatileBehavior) { return FALSE; }

bool32 MetatileBehavior_IsBookshelf(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_BOOKSHELF)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsPokeMartShelf(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_POKEMART_SHELF)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsPlayerFacingPokemonCenterSign(enum MetatileBehavior metatileBehavior, enum Direction direction)
{
    if (direction != DIR_NORTH)
        return FALSE;
    else if (metatileBehavior == MB_POKEMON_CENTER_SIGN)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsPlayerFacingPokeMartSign(enum MetatileBehavior metatileBehavior, enum Direction direction)
{
    if (direction != DIR_NORTH)
        return FALSE;
    else if (metatileBehavior == MB_POKEMART_SIGN)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_UnknownDummy1(enum MetatileBehavior metatileBehavior) { return FALSE; }
bool32 MetatileBehavior_UnknownDummy2(enum MetatileBehavior metatileBehavior) { return FALSE; }
bool32 MetatileBehavior_UnknownDummy3(enum MetatileBehavior metatileBehavior) { return FALSE; }
bool32 MetatileBehavior_UnknownDummy4(enum MetatileBehavior metatileBehavior) { return FALSE; }

bool32 TestMetatileAttributeBit(u8 arg1, u8 arg2)
{
    if (sTileBitAttributes[arg1] & arg2)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsSpinRight(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_SPIN_RIGHT)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsSpinLeft(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_SPIN_LEFT)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsSpinUp(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_SPIN_UP)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsSpinDown(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_SPIN_DOWN)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsStopSpinning(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_STOP_SPINNING)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsSpinTile(enum MetatileBehavior metatileBehavior)
{
    bool8 result = FALSE;

    if (metatileBehavior >= MB_SPIN_RIGHT && metatileBehavior <= MB_SPIN_DOWN)
        result = TRUE;
    else
        result = FALSE;

    return result;
}

bool32 MetatileBehavior_IsSignpost(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_SIGNPOST)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsCabinet(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_CABINET)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsKitchen(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_KITCHEN)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsDresser(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_DRESSER)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsSnacks(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_SNACKS)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsStrengthButton(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_STRENGTH_BUTTON)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsPlayerFacingCableClubWirelessMonitor(enum MetatileBehavior metatileBehavior, u8 playerDirection)
{
    if (playerDirection != DIR_NORTH)
        return FALSE;
    else if (metatileBehavior == MB_CABLE_CLUB_WIRELESS_MONITOR)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsPlayerFacingBattleRecords(enum MetatileBehavior metatileBehavior, u8 playerDirection)
{
    if (playerDirection != DIR_NORTH)
        return FALSE;
    else if (metatileBehavior == MB_BATTLE_RECORDS)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsQuestionnaire(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_QUESTIONNAIRE)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsIndigoPlateauSign1(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_INDIGO_PLATEAU_SIGN_1)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsIndigoPlateauSign2(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_INDIGO_PLATEAU_SIGN_2)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsFood(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_FOOD)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsRockStairs(enum MetatileBehavior metatileBehavior)
{
    bool8 result = FALSE;

    if (metatileBehavior == MB_ROCK_STAIRS)
        result = TRUE;
    else
        result = FALSE;

    return result;
}

bool32 MetatileBehavior_IsBlueprints(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_BLUEPRINTS)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsPainting(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_PAINTING)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsPowerPlantMachine(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_POWER_PLANT_MACHINE)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsTelephone(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_TELEPHONE)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsComputer(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_COMPUTER)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsAdvertisingPoster(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_ADVERTISING_POSTER)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsTastyFood(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_FOOD_SMELLS_TASTY)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsTrashBin(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_TRASH_BIN)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsCup(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_CUP)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsPolishedWindow(enum MetatileBehavior metatileBehavior) { return FALSE; }
bool32 MetatileBehavior_IsBeautifulSkyWindow(enum MetatileBehavior metatileBehavior) { return FALSE; }

bool32 MetatileBehavior_IsBlinkingLights(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_BLINKING_LIGHTS)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsNeatlyLinedUpTools(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_NEATLY_LINED_UP_TOOLS)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsImpressiveMachine(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_IMPRESSIVE_MACHINE)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsVideoGame(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_VIDEO_GAME)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsBurglary(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_BURGLARY)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsTrainerTowerMonitor(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_TRAINER_TOWER_MONITOR)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsSidewaysStairsRightSide(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_SIDEWAYS_STAIRS_RIGHT_SIDE || metatileBehavior == MB_SIDEWAYS_STAIRS_RIGHT_SIDE_BOTTOM)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsSidewaysStairsLeftSide(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_SIDEWAYS_STAIRS_LEFT_SIDE || metatileBehavior == MB_SIDEWAYS_STAIRS_LEFT_SIDE_BOTTOM)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsSidewaysStairsRightSideTop(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_SIDEWAYS_STAIRS_RIGHT_SIDE_TOP)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsSidewaysStairsLeftSideTop(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_SIDEWAYS_STAIRS_LEFT_SIDE_TOP)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsSidewaysStairsRightSideBottom(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_SIDEWAYS_STAIRS_RIGHT_SIDE_BOTTOM)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsSidewaysStairsLeftSideBottom(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_SIDEWAYS_STAIRS_LEFT_SIDE_BOTTOM)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsSidewaysStairsRightSideAny(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_SIDEWAYS_STAIRS_RIGHT_SIDE
     || metatileBehavior == MB_SIDEWAYS_STAIRS_RIGHT_SIDE_BOTTOM
     || metatileBehavior == MB_SIDEWAYS_STAIRS_RIGHT_SIDE_TOP)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsSidewaysStairsLeftSideAny(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_SIDEWAYS_STAIRS_LEFT_SIDE
     || metatileBehavior == MB_SIDEWAYS_STAIRS_LEFT_SIDE_BOTTOM
     || metatileBehavior == MB_SIDEWAYS_STAIRS_LEFT_SIDE_TOP)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsBattlePyramidWarp(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_BATTLE_PYRAMID_WARP)
        return TRUE;
    else
        return FALSE;
}

bool32 MetatileBehavior_IsRockClimbable(enum MetatileBehavior metatileBehavior)
{
    if (metatileBehavior == MB_ROCK_CLIMB)
        return TRUE;
    else
        return FALSE;
}
