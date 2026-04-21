#include "global.h"
#include "bike.h"
#include "field_player_avatar.h"
#include "metatile_behavior.h"
#include "event_object_movement.h"
#include "fieldmap.h"
#include "field_camera.h"
#include "oras_dowse.h"
#include "overworld.h"
#include "constants/map_types.h"
#include "constants/songs.h"

static bool32 CanBikeFaceDirectionOnRail(enum Direction direction, u8 metatileBehavior);
static bool32 MetatileBehaviorForbidsBiking(u8 metatileBehavior);
static enum BikeTransitionId BikeInputHandler_Normal(enum Direction *, u16, u16);
static enum BikeTransitionId BikeInputHandler_Slope(enum Direction *, u16, u16);
static enum BikeTransitionId BikeInputHandler_Turning(enum Direction *, u16, u16);
static enum BikeTransitionId GetBikeTransitionId(enum Direction *, u16, u16);
static enum Collision GetBikeCollision(enum Direction direction);
static enum Collision GetBikeCollisionAt(struct ObjectEvent *playerObjEvent, s16 x, s16 y, enum Direction direction, u8 metatileBehavior);
static void Bike_SetBikeStill(void);
static void BikeTransition_FaceDirection(enum Direction direction);
static void BikeTransition_TurnDirection(enum Direction direction);
static void BikeTransition_MoveDirection(enum Direction direction);
static void BikeTransition_Downhill(enum Direction direction);
static void BikeTransition_Uphill(enum Direction direction);

static void (*const sBikeTransitions[])(enum Direction) =
{
    [BIKE_TRANS_FACE_DIRECTION] = BikeTransition_FaceDirection,
    [BIKE_TRANS_TURNING]        = BikeTransition_TurnDirection,
    [BIKE_TRANS_MOVE]           = BikeTransition_MoveDirection,
    [BIKE_TRANS_DOWNHILL]       = BikeTransition_Downhill,
    [BIKE_TRANS_UPHILL]         = BikeTransition_Uphill,
};

static enum BikeTransitionId (*const sBikeInputHandlers[])(enum Direction *, u16, u16) =
{
    [BIKE_STATE_NORMAL]  = BikeInputHandler_Normal,
    [BIKE_STATE_TURNING] = BikeInputHandler_Turning,
    [BIKE_STATE_SLOPE]   = BikeInputHandler_Slope,
};

void MovePlayerOnBike(enum Direction direction, u16 newKeys, u16 heldKeys)
{
    sBikeTransitions[GetBikeTransitionId(&direction, newKeys, heldKeys)](direction);
}

static enum BikeTransitionId GetBikeTransitionId(enum Direction *direction, u16 newKeys, u16 heldKeys)
{
    return sBikeInputHandlers[gPlayerAvatar.acroBikeState](direction, newKeys, heldKeys);
}

static enum BikeTransitionId BikeInputHandler_Normal(enum Direction *direction_p, u16 newKeys, u16 heldKeys)
{
    struct ObjectEvent *playerObjEvent = &gObjectEvents[gPlayerAvatar.objectEventId];
    enum Direction direction = GetPlayerMovementDirection();

    // fix direction when moving on sideways stairs
    switch (direction)
    {
    case DIR_SOUTHWEST:
    case DIR_NORTHWEST:
        direction = DIR_WEST;
        break;
    case DIR_SOUTHEAST:
    case DIR_NORTHEAST:
        direction = DIR_EAST;
        break;
    case DIR_NONE:
    case DIR_SOUTH:
    case DIR_NORTH:
    case DIR_WEST:
    case DIR_EAST:
        break;
    }

    gPlayerAvatar.bikeFrameCounter = 0;
    if (MetatileBehavior_IsCyclingRoadPullDownTile(playerObjEvent->currentMetatileBehavior) == TRUE)
    {
        if (!JOY_HELD(B_BUTTON))
        {
            gPlayerAvatar.acroBikeState = BIKE_STATE_SLOPE;
            gPlayerAvatar.runningState = MOVING;
            if (*direction_p < DIR_NORTH)
                return BIKE_TRANS_DOWNHILL;
            else
                return BIKE_TRANS_UPHILL;
        }
        else
        {
            if (*direction_p != DIR_NONE)
            {
                gPlayerAvatar.acroBikeState = BIKE_STATE_SLOPE;
                gPlayerAvatar.runningState = MOVING;
                return BIKE_TRANS_UPHILL;
            }
        }
    }
    if (*direction_p == DIR_NONE)
    {
        *direction_p = direction;
        gPlayerAvatar.runningState = NOT_MOVING;
        return BIKE_TRANS_FACE_DIRECTION;
    }
    else
    {
        if (*direction_p != direction && gPlayerAvatar.runningState != MOVING)
        {
            gPlayerAvatar.acroBikeState = BIKE_STATE_TURNING;
            gPlayerAvatar.newDirBackup = *direction_p;
            gPlayerAvatar.runningState = NOT_MOVING;
            return GetBikeTransitionId(direction_p, newKeys, heldKeys);
        }
        else
        {
            gPlayerAvatar.runningState = MOVING;
            return BIKE_TRANS_MOVE;
        }
    }
}

static enum BikeTransitionId BikeInputHandler_Turning(enum Direction *direction_p, u16 newKeys, u16 heldKeys)
{
    *direction_p = gPlayerAvatar.newDirBackup;
    gPlayerAvatar.runningState = TURN_DIRECTION;
    gPlayerAvatar.acroBikeState = BIKE_STATE_NORMAL;
    Bike_SetBikeStill();
    return BIKE_TRANS_TURNING;
}

static enum BikeTransitionId BikeInputHandler_Slope(enum Direction *direction_p, u16 newKeys, u16 heldKeys)
{
    enum Direction direction = GetPlayerMovementDirection();
    u8 playerObjEventId = gPlayerAvatar.objectEventId;
    if (MetatileBehavior_IsCyclingRoadPullDownTile(playerObjEventId[gObjectEvents].currentMetatileBehavior) == TRUE)
    {
        if (*direction_p != direction)
        {
            gPlayerAvatar.acroBikeState = BIKE_STATE_TURNING;
            gPlayerAvatar.newDirBackup = *direction_p;
            gPlayerAvatar.runningState = NOT_MOVING;
            return GetBikeTransitionId(direction_p, newKeys, heldKeys);
        }
        else
        {
            gPlayerAvatar.runningState = MOVING;
            gPlayerAvatar.acroBikeState = BIKE_STATE_SLOPE;
            if (*direction_p < DIR_NORTH)
                return BIKE_TRANS_DOWNHILL;
            else
                return BIKE_TRANS_UPHILL;
        }
    }
    gPlayerAvatar.acroBikeState = BIKE_STATE_NORMAL;
    if (*direction_p == DIR_NONE)
    {
        *direction_p = direction;
        gPlayerAvatar.runningState = NOT_MOVING;
        return BIKE_TRANS_FACE_DIRECTION;
    }
    else
    {
        gPlayerAvatar.runningState = MOVING;
        return BIKE_TRANS_MOVE;
    }
}

static void BikeTransition_FaceDirection(enum Direction direction)
{
    PlayerFaceDirection(direction);
}

static void BikeTransition_TurnDirection(enum Direction direction)
{
    struct ObjectEvent *playerObjEvent = &gObjectEvents[gPlayerAvatar.objectEventId];

    if (!CanBikeFaceDirectionOnRail(direction, playerObjEvent->currentMetatileBehavior))
        direction = playerObjEvent->movementDirection;
    PlayerFaceDirection(direction);
}

static void BikeTransition_MoveDirection(enum Direction direction)
{
    struct ObjectEvent *playerObjEvent;

    playerObjEvent = &gObjectEvents[gPlayerAvatar.objectEventId];
    if (!CanBikeFaceDirectionOnRail(direction, playerObjEvent->currentMetatileBehavior))
    {
        BikeTransition_FaceDirection(playerObjEvent->movementDirection);
    }
    else
    {
        enum Collision collision = GetBikeCollision(direction);

        if (collision > COLLISION_NONE && collision <= COLLISION_ISOLATED_HORIZONTAL_RAIL)
        {
            if (collision == COLLISION_LEDGE_JUMP)
                PlayerJumpLedge(direction);
            else if (collision != COLLISION_STOP_SURFING
                  && collision != COLLISION_LEDGE_JUMP
                  && collision != COLLISION_PUSHED_BOULDER
                  && collision != COLLISION_DIRECTIONAL_STAIR_WARP)
                PlayerOnBikeCollide(direction);
        }
        else
        {

            if (collision == COLLISION_COUNT)
                PlayerWalkFast(direction);
            else if (ObjectMovingOnRockStairs(playerObjEvent, direction))
                PlayerWalkFast(direction);
            else
                PlayerRideWaterCurrent(direction);
        }
    }
}

static void BikeTransition_Downhill(enum Direction direction)
{
    enum Collision collision = GetBikeCollision(DIR_SOUTH);

    if (collision == COLLISION_NONE)
        PlayerWalkFaster(DIR_SOUTH);
    else if (collision == COLLISION_LEDGE_JUMP)
        PlayerJumpLedge(DIR_SOUTH);
}

static void BikeTransition_Uphill(enum Direction direction)
{
    if (GetBikeCollision(direction) == COLLISION_NONE)
        PlayerWalkNormal(direction);
}

static enum Collision GetBikeCollision(enum Direction direction)
{
    struct ObjectEvent *playerObjEvent = &gObjectEvents[gPlayerAvatar.objectEventId];
    s16 x, y;
    u8 metatileBehavior;

    x = playerObjEvent->currentCoords.x;
    y = playerObjEvent->currentCoords.y;
    MoveCoords(direction, &x, &y);
    metatileBehavior = MapGridGetMetatileBehaviorAt(x, y);
    return GetBikeCollisionAt(playerObjEvent, x, y, direction, metatileBehavior);
}

static enum Collision GetBikeCollisionAt(struct ObjectEvent *playerObjEvent, s16 x, s16 y, enum Direction direction, u8 metatileBehavior)
{
    enum Collision collision = CheckForObjectEventCollision(playerObjEvent, x, y, direction, metatileBehavior);

    if (collision <= COLLISION_OBJECT_EVENT)
    {
        bool8 isCrackedIce = MetatileBehavior_IsCrackedIce(metatileBehavior);
        if (isCrackedIce == TRUE)
            return COLLISION_COUNT;
        if (collision == COLLISION_NONE && MetatileBehaviorForbidsBiking(metatileBehavior))
            collision = COLLISION_IMPASSABLE;
    }
    return collision;
}

bool32 IsRunningDisallowed(u8 metatileBehavior)
{
    if ((OW_RUNNING_INDOORS == GEN_3 && !gMapHeader.allowRunning) || MetatileBehaviorForbidsBiking(metatileBehavior) == TRUE)
        return TRUE;

    return FALSE;
}

static bool32 MetatileBehaviorForbidsBiking(u8 metatileBehavior)
{
    if (MetatileBehavior_IsRunningDisallowed(metatileBehavior))
        return TRUE;
    if (!MetatileBehavior_IsFortreeBridge(metatileBehavior))
        return FALSE;
    if (PlayerGetElevation() & 1)
        return FALSE;
    return TRUE;
}

static bool32 CanBikeFaceDirectionOnRail(enum Direction direction, u8 metatileBehavior)
{
    if (direction == DIR_EAST || direction == DIR_WEST)
    {
        if (MetatileBehavior_IsIsolatedVerticalRail(metatileBehavior) || MetatileBehavior_IsVerticalRail(metatileBehavior))
            return FALSE;
    }
    else
    {
        if (MetatileBehavior_IsIsolatedHorizontalRail(metatileBehavior) || MetatileBehavior_IsHorizontalRail(metatileBehavior))
            return FALSE;
    }
    return TRUE;
}

bool32 IsBikingDisallowedByPlayer(void)
{
    s16 x, y;
    u8 metatileBehavior;

    if (gPlayerAvatar.playerState != PLAYER_AVATAR_STATE_SURFING && gPlayerAvatar.playerState != PLAYER_AVATAR_STATE_UNDERWATER)
    {
        PlayerGetDestCoords(&x, &y);
        metatileBehavior = MapGridGetMetatileBehaviorAt(x, y);
        if (!MetatileBehaviorForbidsBiking(metatileBehavior))
            return FALSE;
    }
    return TRUE;
}

bool32 IsPlayerNotUsingAcroBikeOnBumpySlope(void)
{
    if (TestPlayerAvatarState(PLAYER_AVATAR_STATE_ACRO_BIKE))
    {
        if (MetatileBehavior_IsBumpySlope(gObjectEvents[gPlayerAvatar.objectEventId].currentMetatileBehavior))
            return FALSE;
    }
    return TRUE;
}

void GetOnOffBike(enum AvatarState transitionState)
{
    if (IsPlayerBiking())
    {
        SetPlayerAvatarTransitionState(PLAYER_AVATAR_STATE_NORMAL);
        Overworld_ClearSavedMusic();
        Overworld_PlaySpecialMapMusic();
    }
    else
    {
        EndORASDowsing();
        SetPlayerAvatarTransitionState(transitionState);
        if (Overworld_MusicCanOverrideMapMusic(MUS_CYCLING))
        {
            Overworld_SetSavedMusic(MUS_CYCLING);
            Overworld_ChangeMusicTo(MUS_CYCLING);
        }
    }
}

void BikeClearState(void)
{
    gPlayerAvatar.acroBikeState = BIKE_STATE_NORMAL;
    gPlayerAvatar.newDirBackup = 0;
    gPlayerAvatar.bikeFrameCounter = 0;
    gPlayerAvatar.bikeSpeed = PLAYER_SPEED_STANDING;
    gPlayerAvatar.directionHistory = 0;
    gPlayerAvatar.abStartSelectHistory = 0;
    gPlayerAvatar.lastSpinTile = 0;
    for (u32 i = 0; i < ARRAY_COUNT(gPlayerAvatar.dirTimerHistory); i++)
        gPlayerAvatar.dirTimerHistory[i] = 0;
}

void Bike_UpdateBikeCounterSpeed(u8 counter)
{
    gPlayerAvatar.bikeFrameCounter = counter;
    gPlayerAvatar.bikeSpeed = counter + (gPlayerAvatar.bikeFrameCounter >> 1);
}

static void Bike_SetBikeStill(void)
{
    gPlayerAvatar.bikeFrameCounter = 0;
    gPlayerAvatar.bikeSpeed = PLAYER_SPEED_STANDING;
}

s16 GetPlayerSpeed(void)
{
    s16 machBikeSpeeds[] = { PLAYER_SPEED_NORMAL, PLAYER_SPEED_FAST, PLAYER_SPEED_FASTEST };

    if (gPlayerAvatar.playerState == PLAYER_AVATAR_STATE_MACH_BIKE)
        return machBikeSpeeds[gPlayerAvatar.bikeFrameCounter];
    if (gPlayerAvatar.playerState == PLAYER_AVATAR_STATE_ACRO_BIKE)
        return PLAYER_SPEED_FASTER;
    if (gPlayerAvatar.playerState == PLAYER_AVATAR_STATE_SURFING || gPlayerAvatar.dashing)
        return PLAYER_SPEED_FAST;

    return PLAYER_SPEED_NORMAL;
}

void Bike_HandleBumpySlopeJump(void)
{
    s16 x, y;
    u8 tileBehavior;

    if (gPlayerAvatar.playerState != PLAYER_AVATAR_STATE_ACRO_BIKE)
        return;

    PlayerGetDestCoords(&x, &y);
    tileBehavior = MapGridGetMetatileBehaviorAt(x, y);
    if (MetatileBehavior_IsBumpySlope(tileBehavior))
    {
        gPlayerAvatar.acroBikeState = BIKE_STATE_SLOPE;
        PlayerUseAcroBikeOnBumpySlope(GetPlayerMovementDirection());
    }
}

// R/S leftover

// the struct below is used for checking button combinations of the last input so that the acro can potentially perform a side/turn jump.
// its possible that at some point Game Freak intended for the acro bike to have more complex tricks: but only the acro jump combinations can be seen in the final ROM.
struct BikeHistoryInputInfo
{
    enum Direction dirHistoryMatch; // the direction you need to press
    u32 abStartSelectHistoryMatch; // the button you need to press
    u32 dirHistoryMask; // mask applied so that way only the recent nybble (the recent input) is checked
    u32 abStartSelectHistoryMask; // mask applied so that way only the recent nybble (the recent input) is checked
    const u8 *dirTimerHistoryList; // list of timers to check for direction before the button+dir combination can be verified.
    const u8 *abStartSelectHistoryList; // list of timers to check for buttons before the button+dir combination can be verified.
    enum Direction direction; // direction to jump
};

// this is a list of timers to compare against later, terminated with 0. the only timer being compared against is 4 frames in this list.
static const u8 sAcroBikeJumpTimerList[] = {4, 0};

// this is a list of history inputs to do in order to do the check to retrieve a jump direction for acro bike. it seems to be an extensible list, so its possible that Game Freak may have intended for the Acro Bike to have more complex tricks at some point. The final list only has the acro jump.
static const struct BikeHistoryInputInfo sAcroBikeTricksList[] =
{
    // the 0xF is a mask performed with each byte of the array in order to perform the check on only the last entry of the history list, otherwise the check wouldn't work as there can be 0xF0 as opposed to 0x0F.
    {DIR_SOUTH, B_BUTTON, 0xF, 0xF, sAcroBikeJumpTimerList, sAcroBikeJumpTimerList, DIR_SOUTH},
    {DIR_NORTH, B_BUTTON, 0xF, 0xF, sAcroBikeJumpTimerList, sAcroBikeJumpTimerList, DIR_NORTH},
    {DIR_WEST, B_BUTTON, 0xF, 0xF, sAcroBikeJumpTimerList, sAcroBikeJumpTimerList, DIR_WEST},
    {DIR_EAST, B_BUTTON, 0xF, 0xF, sAcroBikeJumpTimerList, sAcroBikeJumpTimerList, DIR_EAST},
};
