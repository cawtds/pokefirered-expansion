#include "global.h"
#include "event_object_movement.h"
#include "field_effect_helpers.h"
#include "field_effect.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "metatile_behavior.h"
#include "script.h"
#include "sound.h"
#include "task.h"
#include "constants/songs.h"

static void Task_UseRockClimb(u8);
static bool8 RockClimb_Init(struct Task *, struct ObjectEvent *);
static bool8 RockClimb_FieldMovePose(struct Task *, struct ObjectEvent *);
static bool8 RockClimb_ShowMon(struct Task *, struct ObjectEvent *);
static bool8 RockClimb_JumpOnRockClimbBlob(struct Task *task, struct ObjectEvent *objectEvent);
static bool8 RockClimb_WaitJumpOnRockClimbBlob(struct Task *task, struct ObjectEvent *objectEvent);
static bool8 RockClimb_Ride(struct Task *task, struct ObjectEvent *objectEvent);
static bool8 RockClimb_ContinueRideOrEnd(struct Task *, struct ObjectEvent *);
static bool8 RockClimb_WaitStopRockClimb(struct Task *task, struct ObjectEvent *objectEvent);
static bool8 RockClimb_StopRockClimbInit(struct Task *task, struct ObjectEvent *objectEvent);

extern const struct SpriteTemplate gFieldEffectObjectTemplate_RockClimbBlob;
extern const struct SpriteTemplate gFieldEffectObjectTemplate_RockClimbDust;
extern const struct SpritePalette gSpritePalette_BigDust;

// ROCK CLIMB
enum RockClimbState
{
    STATE_ROCK_CLIMB_INIT,
    STATE_ROCK_CLIMB_POSE,
    STATE_ROCK_CLIMB_SHOW_MON,
    STATE_ROCK_CLIMB_JUMP_ON,
    STATE_ROCK_CLIMB_WAIT_JUMP,
    STATE_ROCK_CLIMB_RIDE,
    STATE_ROCK_CLIMB_CONTINUE_RIDE,
    STATE_ROCK_CLIMB_STOP_INIT,
    STATE_ROCK_CLIMB_WAIT_STOP
};

#define tState       data[0]
#define tDestX       data[1]
#define tDestY       data[2]
#define tMonId       data[15]

static u8 CreateRockClimbBlob(void)
{
    u8 spriteId;
    struct Sprite *sprite;

    SetSpritePosToOffsetMapCoords((s16 *)&gFieldEffectArguments[0], (s16 *)&gFieldEffectArguments[1], 8, 8);
    spriteId = CreateSpriteAtEnd(&gFieldEffectObjectTemplate_RockClimbBlob, gFieldEffectArguments[0], gFieldEffectArguments[1], 0x96);
    if (spriteId != MAX_SPRITES)
    {
        sprite = &gSprites[spriteId];
        sprite->coordOffsetEnabled = TRUE;
        sprite->oam.paletteNum = LoadPlayerObjectEventPalette(gSaveBlock2Ptr->playerGender);
        sprite->data[2] = gFieldEffectArguments[2];
        sprite->data[3] = -1;
        sprite->data[6] = -1;
        sprite->data[7] = -1;
    }

    return spriteId;
}

bool8 (*const sRockClimbFieldEffectFuncs[])(struct Task *, struct ObjectEvent *) =
{
    [STATE_ROCK_CLIMB_INIT]          = RockClimb_Init,
    [STATE_ROCK_CLIMB_POSE]          = RockClimb_FieldMovePose,
    [STATE_ROCK_CLIMB_SHOW_MON]      = RockClimb_ShowMon,
    [STATE_ROCK_CLIMB_JUMP_ON]       = RockClimb_JumpOnRockClimbBlob,
    [STATE_ROCK_CLIMB_WAIT_JUMP]     = RockClimb_WaitJumpOnRockClimbBlob,
    [STATE_ROCK_CLIMB_RIDE]          = RockClimb_Ride,
    [STATE_ROCK_CLIMB_CONTINUE_RIDE] = RockClimb_ContinueRideOrEnd,
    [STATE_ROCK_CLIMB_STOP_INIT]     = RockClimb_StopRockClimbInit,
    [STATE_ROCK_CLIMB_WAIT_STOP]     = RockClimb_WaitStopRockClimb
};

u32 FldEff_UseRockClimb(void)
{
    u8 taskId;
    taskId = CreateTask(Task_UseRockClimb, 0xff);
    gTasks[taskId].tMonId = gFieldEffectArguments[0];
    Task_UseRockClimb(taskId);
    return FALSE;
}

static void Task_UseRockClimb(u8 taskId)
{
    while (sRockClimbFieldEffectFuncs[gTasks[taskId].tState](&gTasks[taskId], &gObjectEvents[gPlayerAvatar.objectEventId]));
}

static bool8 RockClimb_Init(struct Task *task, struct ObjectEvent *objectEvent)
{
    LockPlayerFieldControls();
    FreezeObjectEvents();
    // Put follower into pokeball before using Rock Climb
    HideFollowerForFieldEffect();
    gPlayerAvatar.preventStep = TRUE;
    SetPlayerAvatarStateMask(PLAYER_AVATAR_FLAG_SURFING);
    PlayerGetDestCoords(&task->tDestX, &task->tDestY);
    MoveCoords(gObjectEvents[gPlayerAvatar.objectEventId].movementDirection, &task->tDestX, &task->tDestY);
    task->tState++;
    return FALSE;
}

static bool8 RockClimb_FieldMovePose(struct Task *task, struct ObjectEvent *objectEvent)
{
    if (!ObjectEventIsMovementOverridden(objectEvent) || ObjectEventClearHeldMovementIfFinished(objectEvent))
    {
        StartPlayerAvatarSummonMonForFieldMoveAnim();
        ObjectEventSetHeldMovement(objectEvent, MOVEMENT_ACTION_START_ANIM_IN_DIRECTION);
        task->tState++;
    }
    return FALSE;
}

static bool8 RockClimb_ShowMon(struct Task *task, struct ObjectEvent *objectEvent)
{
    if (ObjectEventCheckHeldMovementStatus(objectEvent))
    {
        gFieldEffectArguments[0] = task->tMonId | SHOW_MON_CRY_NO_DUCKING;
        FieldEffectStart(FLDEFF_FIELD_MOVE_SHOW_MON_INIT);
        task->tState++;
        return TRUE;
    }
    return FALSE;
}

static bool8 RockClimb_JumpOnRockClimbBlob(struct Task *task, struct ObjectEvent *objectEvent)
{
    if (!FieldEffectActiveListContains(FLDEFF_FIELD_MOVE_SHOW_MON))
    {
        objectEvent->noShadow = TRUE; // hide shadow
        ObjectEventSetGraphicsId(objectEvent, GetPlayerAvatarGraphicsIdByStateId(PLAYER_AVATAR_STATE_SURFING));
        ObjectEventClearHeldMovementIfFinished(objectEvent);
        ObjectEventSetHeldMovement(objectEvent, GetJumpSpecialMovementAction(objectEvent->movementDirection));
        gFieldEffectArguments[0] = task->tDestX;
        gFieldEffectArguments[1] = task->tDestY;
        gFieldEffectArguments[2] = gPlayerAvatar.objectEventId;
        objectEvent->fieldEffectSpriteId = CreateRockClimbBlob();
        task->tState++;
    }

    return FALSE;
}

static bool8 RockClimb_WaitJumpOnRockClimbBlob(struct Task *task, struct ObjectEvent *objectEvent)
{
    if (ObjectEventClearHeldMovementIfFinished(objectEvent))
    {
        SetSurfBlob_BobState(objectEvent->fieldEffectSpriteId, BOB_PLAYER_AND_MON);
        switch (objectEvent->facingDirection)
        {
        case DIR_EAST:
            //check southeast then northeast
            if (MetatileBehavior_IsRockClimbable(MapGridGetMetatileBehaviorAt(task->tDestX + 1, task->tDestY + 1)))
                objectEvent->movementDirection = DIR_SOUTHEAST;
            else if (MetatileBehavior_IsRockClimbable(MapGridGetMetatileBehaviorAt(task->tDestX + 1, task->tDestY - 1)))
                objectEvent->movementDirection = DIR_NORTHEAST;
            break;
        case DIR_WEST:
            //check northwest then southwest
            if (MetatileBehavior_IsRockClimbable(MapGridGetMetatileBehaviorAt(task->tDestX - 1, task->tDestY - 1)))
                objectEvent->movementDirection = DIR_NORTHWEST;
            else if (MetatileBehavior_IsRockClimbable(MapGridGetMetatileBehaviorAt(task->tDestX - 1, task->tDestY + 1)))
                objectEvent->movementDirection = DIR_SOUTHWEST;
            break;
        }

        task->tState = STATE_ROCK_CLIMB_CONTINUE_RIDE;
    }

    return FALSE;
}

struct RockClimbRide
{
    u8 action;
    s8 dx;
    s8 dy;
    u8 jumpDir;
};
static const struct RockClimbRide sRockClimbMovement[] =
{
    [DIR_NONE] = {MOVEMENT_ACTION_WALK_FAST_DOWN, 0, 0, DIR_NONE},
    [DIR_SOUTH] = {MOVEMENT_ACTION_WALK_FAST_DOWN, 0, -1, DIR_SOUTH},
    [DIR_NORTH] = {MOVEMENT_ACTION_WALK_FAST_UP, 0, 1, DIR_NORTH},
    [DIR_WEST] = {MOVEMENT_ACTION_WALK_FAST_LEFT, 1, 1, DIR_WEST},
    [DIR_EAST] = {MOVEMENT_ACTION_WALK_FAST_RIGHT, -1, -1, DIR_EAST},
    [DIR_SOUTHWEST] = {MOVEMENT_ACTION_WALK_FAST_DIAGONAL_DOWN_LEFT, 1, -1, DIR_WEST},
    [DIR_SOUTHEAST] = {MOVEMENT_ACTION_WALK_FAST_DIAGONAL_DOWN_RIGHT, -1, -1, DIR_EAST},
    [DIR_NORTHWEST] = {MOVEMENT_ACTION_WALK_FAST_DIAGONAL_UP_LEFT, 1, 1, DIR_WEST},
    [DIR_NORTHEAST] = {MOVEMENT_ACTION_WALK_FAST_DIAGONAL_UP_RIGHT, -1, 1, DIR_EAST},
};

static void RockClimbDust(struct ObjectEvent *objectEvent, enum Direction direction)
{
    s8 dx = sRockClimbMovement[direction].dx;
    s8 dy = sRockClimbMovement[direction].dy;

    gFieldEffectArguments[0] = objectEvent->currentCoords.x + dx;
    gFieldEffectArguments[1] = objectEvent->currentCoords.y + dy;
    gFieldEffectArguments[2] = objectEvent->previousElevation;
    gFieldEffectArguments[3] = gSprites[objectEvent->spriteId].oam.priority;
    FieldEffectStart(FLDEFF_ROCK_CLIMB_DUST);
}

static bool8 RockClimb_Ride(struct Task *task, struct ObjectEvent *objectEvent)
{
    ObjectEventSetHeldMovement(objectEvent, sRockClimbMovement[objectEvent->movementDirection].action);
    PlaySE(SE_M_ROCK_THROW);
    RockClimbDust(objectEvent, objectEvent->movementDirection);
    task->tState++;
    return FALSE;
}

static bool8 RockClimb_ContinueRideOrEnd(struct Task *task, struct ObjectEvent *objectEvent)
{
    if (!ObjectEventClearHeldMovementIfFinished(objectEvent))
        return FALSE;

    PlayerGetDestCoords(&task->tDestX, &task->tDestY);
    MoveCoords(objectEvent->movementDirection, &task->tDestX, &task->tDestY);
    if (MetatileBehavior_IsRockClimbable(MapGridGetMetatileBehaviorAt(task->tDestX, task->tDestY)))
    {
        task->tState = STATE_ROCK_CLIMB_RIDE;
        return TRUE;
    }

    LockPlayerFieldControls();
    gPlayerAvatar.flags &= ~PLAYER_AVATAR_FLAG_SURFING;
    gPlayerAvatar.flags |= PLAYER_AVATAR_FLAG_ON_FOOT;
    task->tState++;
    return FALSE;
}

static bool8 RockClimb_StopRockClimbInit(struct Task *task, struct ObjectEvent *objectEvent)
{
    if (ObjectEventIsMovementOverridden(objectEvent))
    {
        if (!ObjectEventClearHeldMovementIfFinished(objectEvent))
            return FALSE;
    }

    RockClimbDust(objectEvent, DIR_NONE);   //dust on final spot
    ObjectEventSetHeldMovement(objectEvent, GetJumpSpecialMovementAction(sRockClimbMovement[objectEvent->movementDirection].jumpDir));
    SetSurfBlob_BobState(objectEvent->fieldEffectSpriteId, BOB_NONE);
    task->tState++;
    return TRUE;
}

static bool8 RockClimb_WaitStopRockClimb(struct Task *task, struct ObjectEvent *objectEvent)
{
    struct ObjectEvent *followerObject = GetFollowerObject();
    if (ObjectEventClearHeldMovementIfFinished(objectEvent))
    {
        ObjectEventSetGraphicsId(objectEvent, GetPlayerAvatarGraphicsIdByStateId(PLAYER_AVATAR_STATE_NORMAL));
        ObjectEventSetHeldMovement(objectEvent, GetFaceDirectionMovementAction(objectEvent->facingDirection));
        gPlayerAvatar.preventStep = FALSE;
        if (followerObject)
            ObjectEventClearHeldMovementIfFinished(followerObject); // restore follower to normal
        objectEvent->noShadow = FALSE; // restore shadow
        UnfreezeObjectEvents();
        UnlockPlayerFieldControls();
        DestroySprite(&gSprites[objectEvent->fieldEffectSpriteId]);
        FieldEffectActiveListRemove(FLDEFF_USE_ROCK_CLIMB);
        objectEvent->triggerGroundEffectsOnMove = TRUE; // e.g. if dismount on grass
        DestroyTask(FindTaskIdByFunc(Task_UseRockClimb));
    }

    return FALSE;
}

bool8 IsRockClimbActive(void)
{
    if (FieldEffectActiveListContains(FLDEFF_USE_ROCK_CLIMB))
        return TRUE;
    else
        return FALSE;
}

#undef tState
#undef tSpriteId
#undef tTargetX
#undef tTargetY
#undef tCurX
#undef tCurY
#undef tVelocityX
#undef tVelocityY
#undef tMoveSteps
#undef tObjEventId

u32 FldEff_RockClimbDust(void)
{
    u8 spriteId;
    struct Sprite *sprite;

    FieldEffectScript_LoadFadedPal(&gSpritePalette_BigDust);
    SetSpritePosToOffsetMapCoords((s16 *)&gFieldEffectArguments[0], (s16 *)&gFieldEffectArguments[1], 8, 12);
    spriteId = CreateSpriteAtEnd(&gFieldEffectObjectTemplate_RockClimbDust, gFieldEffectArguments[0], gFieldEffectArguments[1], 0);
    if (spriteId != MAX_SPRITES)
    {
        sprite = &gSprites[spriteId];
        sprite->coordOffsetEnabled = TRUE;
        sprite->oam.priority = gFieldEffectArguments[3];
        sprite->data[0] = gFieldEffectArguments[2];
        sprite->data[1] = FLDEFF_ROCK_CLIMB_DUST;
    }

    return 0;
}
