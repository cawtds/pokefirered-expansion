#include "global.h"
#include "battle_setup.h"
#include "battle.h"
#include "event_data.h"
#include "event_object_lock.h"
#include "event_object_movement.h"
#include "event_scripts.h"
#include "field_effect.h"
#include "field_player_avatar.h"
#include "field_specials.h"
#include "item_menu.h"
#include "item_use.h"
#include "item.h"
#include "malloc.h"
#include "menu.h"
#include "random.h"
#include "script_movement.h"
#include "script.h"
#include "sound.h"
#include "task.h"
#include "vs_seeker.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/items.h"
#include "constants/maps.h"
#include "constants/quest_log.h"
#include "constants/script_commands.h"
#include "constants/songs.h"
#include "constants/trainer_types.h"
#include "constants/vs_seeker.h"

// Each trainer can have up to 6 parties, including their original party.
// Each rematch is unavailable until the player has progressed to a certain point in the story (see TryGetRematchTrainerIdGivenGameState).
// A list of the trainer ids for each party is in sRematches. If a party doesn't update for a progression point it will have SKIP instead,
// and that trainer id will be ignored.
#define SKIP 0xFFFF

#define NO_REMATCH_LOCALID LOCALID_PLAYER

enum
{
   VSSEEKER_NOT_CHARGED,
   VSSEEKER_NO_ONE_IN_RANGE,
   VSSEEKER_CAN_USE,
};

typedef enum
{
    VSSEEKER_SINGLE_RESP_RAND,
    VSSEEKER_SINGLE_RESP_NO,
    VSSEEKER_SINGLE_RESP_YES
} VsSeekerSingleRespCode;

typedef enum
{
    VSSEEKER_RESPONSE_NO_RESPONSE,
    VSSEEKER_RESPONSE_UNFOUGHT_TRAINERS,
    VSSEEKER_RESPONSE_FOUND_REMATCHES
} VsSeekerResponseCode;

struct VsSeekerTrainerInfo
{
    const u8 *script;
    enum TrainerID trainerId;
    u8 localId;
    u8 objectEventId;
    s16 xCoord;
    s16 yCoord;
    u16 graphicsId;
};

struct VsSeekerStruct
{
    struct VsSeekerTrainerInfo trainerInfo[OBJECT_EVENTS_COUNT];
    enum TrainerID trainerIds[OBJECT_EVENTS_COUNT];
    u8 runningBehaviourEtcArray[OBJECT_EVENTS_COUNT];
    u8 numRematchableTrainers;
    u8 trainerHasNotYetBeenFought:1;
    u8 trainerDoesNotWantRematch:1;
    u8 trainerWantsRematch:1;
    u8 responseCode:5;
};

// static declarations
static EWRAM_DATA struct VsSeekerStruct *sVsSeeker = NULL;

static bool32 IsThisTrainerRematchable(u32 localId);
static bool8 CanUseVsSeeker(void);
static bool8 IsTrainerReadyForRematchInternal(const struct RematchData *array, enum TrainerID trainerId);
static bool8 ObjectEventIdIsSane(u8 objectEventId);
static enum TrainerID GetTrainerFlagFromScript(const u8 *script);
static int GetRematchIdx(const struct RematchData *vsSeekerData, enum TrainerID trainerId);
static int LookupVsSeekerOpponentInArray(const struct RematchData *array, enum TrainerID trainerId);
static u8 GetRandomFaceDirectionMovementType();
static u8 GetVsSeekerResponseInArea(const struct RematchData *vsSeekerData);
static u8 HasRematchTrainerAlreadyBeenFought(const struct RematchData *vsSeekerData, enum TrainerID trainerId);
static u8 ShouldTryRematchBattleInternal(const struct RematchData *vsSeekerData, enum TrainerID trainerId);
static void ClearAllTrainerRematchStates(void);
static void GatherNearbyTrainerInfo(void);
static void ResetMovementOfRematchableTrainers(void);
static void StartAllRespondantIdleMovements(void);
static void Task_ResetObjectsRematchWantedState(u8 taskId);
static void Task_VsSeeker_1(u8 taskId);
static void Task_VsSeeker_2(u8 taskId);
static void Task_VsSeeker_3(u8 taskId);
static void VsSeekerResetChargingStepCounter(void);
static void VsSeekerResetInBagStepCounter(void);
#if FREE_MATCH_CALL == FALSE
static bool8 IsTrainerVisibleOnScreen(struct VsSeekerTrainerInfo *trainerInfo);
static u8 GetCurVsSeekerResponse(s32 vsSeekerIdx, enum TrainerID trainerId);
static u8 GetNextAvailableRematchTrainer(const struct RematchData *vsSeekerData, enum TrainerID trainerId, u8 *idxPtr);
static u8 GetRematchableTrainerLocalId(void);
static u8 BackwardsSearchRematchTrainerIndex(const enum TrainerID *trainerIds, u8 rematchIndex);
static u8 GetRunningBehaviorFromGraphicsId(u16 graphicsId);
static void StartTrainerObjectMovementScript(struct VsSeekerTrainerInfo *trainerInfo, const u8 *script);
#endif //FREE_MATCH_CALL

#include "data/trainer_rematches.h"

static const u8 sMovementScript_Wait48[] = {
    MOVEMENT_ACTION_DELAY_16,
    MOVEMENT_ACTION_DELAY_16,
    MOVEMENT_ACTION_DELAY_16,
    MOVEMENT_ACTION_STEP_END
};

static const u8 sMovementScript_TrainerUnfought[] = {
    MOVEMENT_ACTION_EMOTE_EXCLAMATION_MARK,
    MOVEMENT_ACTION_STEP_END
};

static const u8 sMovementScript_TrainerNoRematch[] = {
    MOVEMENT_ACTION_EMOTE_X,
    MOVEMENT_ACTION_STEP_END
};

static const u8 sMovementScript_TrainerRematch[] = {
    MOVEMENT_ACTION_WALK_IN_PLACE_FASTER_DOWN,
    MOVEMENT_ACTION_EMOTE_DOUBLE_EXCL_MARK,
    MOVEMENT_ACTION_STEP_END
};

static const u8 sFaceDirectionMovementTypeByFacingDirection[] = {
    MOVEMENT_TYPE_FACE_DOWN,
    MOVEMENT_TYPE_FACE_DOWN,
    MOVEMENT_TYPE_FACE_UP,
    MOVEMENT_TYPE_FACE_LEFT,
    MOVEMENT_TYPE_FACE_RIGHT
};

// text

void VsSeekerFreezeObjectsAfterChargeComplete(void)
{
    CreateTask(Task_ResetObjectsRematchWantedState, 80);
}

static void Task_ResetObjectsRematchWantedState(u8 taskId)
{
    struct Task *task = &gTasks[taskId];
    u8 i;

    if (task->data[0] == 0 && IsPlayerStandingStill() == TRUE)
    {
        PlayerFreeze();
        task->data[0] = 1;
    }

    if (task->data[1] == 0)
    {
        for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
        {
            if (ObjectEventIdIsSane(i) == TRUE)
            {
                if (gObjectEvents[i].singleMovementActive)
                    return;
                FreezeObjectEvent(&gObjectEvents[i]);
            }
        }
    }

    task->data[1] = 1;
    if (task->data[0] != 0)
    {
        DestroyTask(taskId);
        StopPlayerAvatar();
        ScriptContext_Enable();
    }
}

void VsSeekerResetObjectMovementAfterChargeComplete(void)
{
    struct ObjectEventTemplate *templates = gSaveBlock1Ptr->objectEventTemplates;
    u8 i;
    u8 movementType;
    u8 objEventId;
    struct ObjectEvent *objectEvent;

    for (i = 0; i < gMapHeader.events->objectEventCount; i++)
    {
        if ((templates[i].trainerType == TRAINER_TYPE_NORMAL
          || templates[i].trainerType == TRAINER_TYPE_BURIED)
         && (templates[i].movementType == MOVEMENT_TYPE_RAISE_HAND_AND_STOP
          || templates[i].movementType == MOVEMENT_TYPE_RAISE_HAND_AND_JUMP
          || templates[i].movementType == MOVEMENT_TYPE_RAISE_HAND_AND_SWIM))
        {
            movementType = GetRandomFaceDirectionMovementType();
            TryGetObjectEventIdByLocalIdAndMap(templates[i].localId, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup, &objEventId);
            objectEvent = &gObjectEvents[objEventId];
            if (ObjectEventIdIsSane(objEventId) == TRUE)
            {
                SetTrainerMovementType(objectEvent, movementType);
            }
            templates[i].movementType = movementType;
        }
    }
}

bool8 UpdateVsSeekerStepCounter(void)
{
#if FREE_MATCH_CALL == FALSE
    u8 x = 0;

    if (CheckBagHasItem(ITEM_VS_SEEKER, 1) == TRUE)
    {
        if ((gSaveBlock1Ptr->trainerRematchStepCounter & 0xFF) < 100)
            gSaveBlock1Ptr->trainerRematchStepCounter++;
    }

    if (FlagGet(FLAG_SYS_VS_SEEKER_CHARGING) == TRUE)
    {
        if (((gSaveBlock1Ptr->trainerRematchStepCounter >> 8) & 0xFF) < 100)
        {
            x = (((gSaveBlock1Ptr->trainerRematchStepCounter >> 8) & 0xFF) + 1);
            gSaveBlock1Ptr->trainerRematchStepCounter = (gSaveBlock1Ptr->trainerRematchStepCounter & 0xFF) | (x << 8);
        }
        if (((gSaveBlock1Ptr->trainerRematchStepCounter >> 8) & 0xFF) == 100)
        {
            FlagClear(FLAG_SYS_VS_SEEKER_CHARGING);
            VsSeekerResetChargingStepCounter();
            ClearAllTrainerRematchStates();
            return TRUE;
        }
    }
#endif //FREE_MATCH_CALL

    return FALSE;
}

void MapResetTrainerRematches(u16 mapGroup, u16 mapNum)
{
    FlagClear(FLAG_SYS_VS_SEEKER_CHARGING);
    VsSeekerResetChargingStepCounter();
    ClearAllTrainerRematchStates();
    ResetMovementOfRematchableTrainers();
}

static void ResetMovementOfRematchableTrainers(void)
{
    u8 i;

    for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
    {
        struct ObjectEvent *objectEvent = &gObjectEvents[i];
        if (objectEvent->movementType == MOVEMENT_TYPE_RAISE_HAND_AND_STOP
                || objectEvent->movementType == MOVEMENT_TYPE_RAISE_HAND_AND_JUMP
                || objectEvent->movementType == MOVEMENT_TYPE_RAISE_HAND_AND_SWIM)
        {
            u8 movementType = GetRandomFaceDirectionMovementType();
            if (objectEvent->active && gSprites[objectEvent->spriteId].data[0] == i)
            {
                gSprites[objectEvent->spriteId].x2 = 0;
                gSprites[objectEvent->spriteId].y2 = 0;
                SetTrainerMovementType(objectEvent, movementType);
            }
        }
    }
}

static void VsSeekerResetInBagStepCounter(void)
{
#if FREE_MATCH_CALL == FALSE
   gSaveBlock1Ptr->trainerRematchStepCounter &= 0xFF00;
#endif //FREE_MATCH_CALL
}

static void VsSeekerResetChargingStepCounter(void)
{
#if FREE_MATCH_CALL == FALSE
   gSaveBlock1Ptr->trainerRematchStepCounter &= 0x00FF;
#endif //FREE_MATCH_CALL
}

void Task_VsSeeker_0(u8 taskId)
{
    u8 i;
    u8 respval;

    for (i = 0; i < 16; i++)
        gTasks[taskId].data[i] = 0;

    sVsSeeker = AllocZeroed(sizeof(struct VsSeekerStruct));
    GatherNearbyTrainerInfo();
    respval = CanUseVsSeeker();
    if (respval == VSSEEKER_NOT_CHARGED)
    {
        Free(sVsSeeker);
        DisplayItemMessageOnField(taskId, FONT_NORMAL, VSSeeker_Text_BatteryNotChargedNeedXSteps, Task_ItemUse_CloseMessageBoxAndReturnToField_VsSeeker);
    }
    else if (respval == VSSEEKER_NO_ONE_IN_RANGE)
    {
        Free(sVsSeeker);
        DisplayItemMessageOnField(taskId, FONT_NORMAL, VSSeeker_Text_NoTrainersWithinRange, Task_ItemUse_CloseMessageBoxAndReturnToField_VsSeeker);
    }
    else if (respval == VSSEEKER_CAN_USE)
    {
        ItemUse_SetQuestLogEvent(QL_EVENT_USED_ITEM, 0, gSpecialVar_ItemId, 0xFFFF);
        FieldEffectStart(FLDEFF_USE_VS_SEEKER);
        gTasks[taskId].func = Task_VsSeeker_1;
        gTasks[taskId].data[0] = 15;
    }
}

static void Task_VsSeeker_1(u8 taskId)
{
    if (--gTasks[taskId].data[0] == 0)
    {
        gTasks[taskId].func = Task_VsSeeker_2;
        gTasks[taskId].data[1] = 16;
    }
}

static void Task_VsSeeker_2(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    if (data[2] != 2 && --data[1] == 0)
    {
        PlaySE(SE_CONTEST_MONS_TURN);
        data[1] = 11;
        data[2]++;
    }

    if (!FieldEffectActiveListContains(FLDEFF_USE_VS_SEEKER))
    {
        data[1] = 0;
        data[2] = 0;
        VsSeekerResetInBagStepCounter();
        sVsSeeker->responseCode = GetVsSeekerResponseInArea(sRematches);
        ScriptMovement_StartObjectMovementScript(0xFF, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup, sMovementScript_Wait48);
        gTasks[taskId].func = Task_VsSeeker_3;
    }
}

static void GatherNearbyTrainerInfo(void)
{
    struct ObjectEventTemplate *templates = gSaveBlock1Ptr->objectEventTemplates;
    u8 objectEventId = 0;
    u8 vsSeekerObjectIdx = 0;
    s32 objectEventIdx;

    for (objectEventIdx = 0; objectEventIdx < gMapHeader.events->objectEventCount; objectEventIdx++)
    {
        if (templates[objectEventIdx].trainerType == TRAINER_TYPE_NORMAL || templates[objectEventIdx].trainerType == TRAINER_TYPE_BURIED)
        {
            sVsSeeker->trainerInfo[vsSeekerObjectIdx].script = templates[objectEventIdx].script;
            sVsSeeker->trainerInfo[vsSeekerObjectIdx].trainerId = GetTrainerFlagFromScript(templates[objectEventIdx].script);
            sVsSeeker->trainerInfo[vsSeekerObjectIdx].localId = templates[objectEventIdx].localId;
            TryGetObjectEventIdByLocalIdAndMap(templates[objectEventIdx].localId, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup, &objectEventId);
            sVsSeeker->trainerInfo[vsSeekerObjectIdx].objectEventId = objectEventId;
            sVsSeeker->trainerInfo[vsSeekerObjectIdx].xCoord = gObjectEvents[objectEventId].currentCoords.x - 7;
            sVsSeeker->trainerInfo[vsSeekerObjectIdx].yCoord = gObjectEvents[objectEventId].currentCoords.y - 7;
            sVsSeeker->trainerInfo[vsSeekerObjectIdx].graphicsId = templates[objectEventIdx].graphicsId;
            vsSeekerObjectIdx++;
        }
    }
    sVsSeeker->trainerInfo[vsSeekerObjectIdx].localId = NO_REMATCH_LOCALID;
}

static void Task_VsSeeker_3(u8 taskId)
{
    if (ScriptMovement_IsObjectMovementFinished(NO_REMATCH_LOCALID, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup))
    {
        if (sVsSeeker->responseCode == VSSEEKER_RESPONSE_NO_RESPONSE)
        {
            DisplayItemMessageOnField(taskId, FONT_NORMAL, VSSeeker_Text_TrainersNotReady, Task_ItemUse_CloseMessageBoxAndReturnToField_VsSeeker);
        }
        else
        {
            if (sVsSeeker->responseCode == VSSEEKER_RESPONSE_FOUND_REMATCHES)
                StartAllRespondantIdleMovements();
            ClearDialogWindowAndFrame(0, TRUE);
            ScriptUnfreezeObjectEvents();
            UnlockPlayerFieldControls();
            DestroyTask(taskId);
        }
        Free(sVsSeeker);
    }
}

static u8 CanUseVsSeeker(void)
{
#if FREE_MATCH_CALL == FALSE
    u8 vsSeekerChargeSteps = gSaveBlock1Ptr->trainerRematchStepCounter;
    if (vsSeekerChargeSteps == 100)
    {
        if (GetRematchableTrainerLocalId() == NO_REMATCH_LOCALID)
            return VSSEEKER_NO_ONE_IN_RANGE;
        else
            return VSSEEKER_CAN_USE;
    }
    else
    {
        TV_PrintIntToStringVar(0, 100 - vsSeekerChargeSteps);
        return VSSEEKER_NOT_CHARGED;
    }
#else
   return VSSEEKER_NO_ONE_IN_RANGE;
#endif //FREE_MATCH_CALL
}

static u8 GetVsSeekerResponseInArea(const struct RematchData *vsSeekerData)
{
#if FREE_MATCH_CALL == FALSE
    enum TrainerID trainerId = TRAINER_NONE;
    u16 rval = 0;
    u8 rematchTrainerIdx;
    u8 unusedIdx = 0;
    u8 response = 0;
    s32 vsSeekerIdx = 0;

    while (sVsSeeker->trainerInfo[vsSeekerIdx].localId != NO_REMATCH_LOCALID)
    {
        if (IsTrainerVisibleOnScreen(&sVsSeeker->trainerInfo[vsSeekerIdx]) == TRUE)
        {
            trainerId = sVsSeeker->trainerInfo[vsSeekerIdx].trainerId;
            if (!HasTrainerBeenFought(trainerId))
            {
                StartTrainerObjectMovementScript(&sVsSeeker->trainerInfo[vsSeekerIdx], sMovementScript_TrainerUnfought);
                sVsSeeker->trainerHasNotYetBeenFought = 1;
                vsSeekerIdx++;
                continue;
            }
            rematchTrainerIdx = GetNextAvailableRematchTrainer(vsSeekerData, trainerId, &unusedIdx);
            if (rematchTrainerIdx == 0)
            {
                StartTrainerObjectMovementScript(&sVsSeeker->trainerInfo[vsSeekerIdx], sMovementScript_TrainerNoRematch);
                sVsSeeker->trainerDoesNotWantRematch = 1;
            }
            else
            {
                rval = Random() % 100; // Even if it's overwritten below, it progresses the RNG.
                response = GetCurVsSeekerResponse(vsSeekerIdx, trainerId);
                if (response == VSSEEKER_SINGLE_RESP_YES)
                    rval = 100; // Definitely yes
                else if (response == VSSEEKER_SINGLE_RESP_NO)
                    rval = 0; // Definitely no
                // Otherwise it's a 70% chance to want a rematch
                if (rval < 30)
                {
                    StartTrainerObjectMovementScript(&sVsSeeker->trainerInfo[vsSeekerIdx], sMovementScript_TrainerNoRematch);
                    sVsSeeker->trainerDoesNotWantRematch = 1;
                }
                else
                {
                    gSaveBlock1Ptr->trainerRematches[sVsSeeker->trainerInfo[vsSeekerIdx].localId] = rematchTrainerIdx;
                    ShiftStillObjectEventCoords(&gObjectEvents[sVsSeeker->trainerInfo[vsSeekerIdx].objectEventId]);
                    StartTrainerObjectMovementScript(&sVsSeeker->trainerInfo[vsSeekerIdx], sMovementScript_TrainerRematch);
                    sVsSeeker->trainerIds[sVsSeeker->numRematchableTrainers] = trainerId;
                    sVsSeeker->runningBehaviourEtcArray[sVsSeeker->numRematchableTrainers] = GetRunningBehaviorFromGraphicsId(sVsSeeker->trainerInfo[vsSeekerIdx].graphicsId);
                    sVsSeeker->numRematchableTrainers++;
                    sVsSeeker->trainerWantsRematch = 1;
                }
            }
        }
        vsSeekerIdx++;
    }

    if (sVsSeeker->trainerWantsRematch)
    {
        PlaySE(SE_PIN);
        FlagSet(FLAG_SYS_VS_SEEKER_CHARGING);
        VsSeekerResetChargingStepCounter();
        return VSSEEKER_RESPONSE_FOUND_REMATCHES;
    }
    if (sVsSeeker->trainerHasNotYetBeenFought)
        return VSSEEKER_RESPONSE_UNFOUGHT_TRAINERS;
#endif //FREE_MATCH_CALL
    return VSSEEKER_RESPONSE_NO_RESPONSE;
}

void ClearRematchStateByTrainerId(void)
{
   u8 objEventId = 0;
   struct ObjectEventTemplate *objectEventTemplates = gSaveBlock1Ptr->objectEventTemplates;
   int vsSeekerDataIdx = LookupVsSeekerOpponentInArray(sRematches, TRAINER_BATTLE_PARAM.opponentA);

   if (vsSeekerDataIdx == -1)
      return;

   for (int i = 0; i < gMapHeader.events->objectEventCount; i++)
   {
      if ((objectEventTemplates[i].trainerType == TRAINER_TYPE_NORMAL
         || objectEventTemplates[i].trainerType == TRAINER_TYPE_BURIED)
         && vsSeekerDataIdx == LookupVsSeekerOpponentInArray(sRematches, GetTrainerFlagFromScript(objectEventTemplates[i].script)))
      {
            struct ObjectEvent *objectEvent;

            TryGetObjectEventIdByLocalIdAndMap(objectEventTemplates[i].localId, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup, &objEventId);
            objectEvent = &gObjectEvents[objEventId];
            #if __STDC_VERSION__ < 202311L
            GetRandomFaceDirectionMovementType(&objectEventTemplates[i]); // You are using this function incorrectly.  Please consult the manual.
            #else
            GetRandomFaceDirectionMovementType();
            #endif
            OverrideMovementTypeForObjectEvent(objectEvent, sFaceDirectionMovementTypeByFacingDirection[objectEvent->facingDirection]);
#if FREE_MATCH_CALL == FALSE
            gSaveBlock1Ptr->trainerRematches[objectEventTemplates[i].localId] = 0;
#endif //FREE_MATCH_CALL
            if (gSelectedObjectEvent == objEventId)
               objectEvent->movementType = sFaceDirectionMovementTypeByFacingDirection[objectEvent->facingDirection];
            else
               objectEvent->movementType = MOVEMENT_TYPE_FACE_DOWN;
      }
   }
}

#if FREE_MATCH_CALL == FALSE
static u8 TryGetRematchTrainerIdGivenGameState(const enum TrainerID *trainerIds, u8 rematchIndex)
{
    bool32 isFlagSet = TRUE;

    switch (rematchIndex)
    {
     case 1:
        isFlagSet = FlagGet(FLAG_GOT_VS_SEEKER);
         break;
     case 2:
        isFlagSet = FlagGet(FLAG_WORLD_MAP_CELADON_CITY);
         break;
     case 3:
        isFlagSet = FlagGet(FLAG_WORLD_MAP_FUCHSIA_CITY);
         break;
     case 4:
        isFlagSet = FlagGet(FLAG_SYS_GAME_CLEAR);
         break;
     case 5:
        isFlagSet = FlagGet(FLAG_SYS_CAN_LINK_WITH_RS);
         break;
    }

    if (isFlagSet)
        return rematchIndex;

    return BackwardsSearchRematchTrainerIndex(trainerIds, rematchIndex);
}

static u8 BackwardsSearchRematchTrainerIndex(const enum TrainerID *trainerIds, u8 rematchIndex)
{
    while (--rematchIndex != 0)
    {
        if (trainerIds[rematchIndex] != SKIP)
            return rematchIndex;
    }

    return 0;
}
#endif //FREE_MATCH_CALL

bool8 ShouldTryRematchBattle(void)
{
   return ShouldTryRematchBattleForTrainerId(TRAINER_BATTLE_PARAM.opponentA);
}

bool8 ShouldTryRematchBattleForTrainerId(enum TrainerID trainerId)
{
    if (ShouldTryRematchBattleInternal(sRematches, trainerId))
    {
        return TRUE;
    }
    return HasRematchTrainerAlreadyBeenFought(sRematches, trainerId);
}

static bool8 ShouldTryRematchBattleInternal(const struct RematchData *vsSeekerData, enum TrainerID trainerId)
{
    s32 rematchIdx = GetRematchIdx(vsSeekerData, trainerId);

    if (rematchIdx == -1)
        return FALSE;
    if (rematchIdx >= 0 && rematchIdx < ARRAY_COUNT(sRematches))
    {
        if (IsThisTrainerRematchable(gSpecialVar_LastTalked))
            return TRUE;
    }
    return FALSE;
}

static bool8 HasRematchTrainerAlreadyBeenFought(const struct RematchData *vsSeekerData, enum TrainerID trainerId)
{
    s32 rematchIdx = GetRematchIdx(vsSeekerData, trainerId);

    if (rematchIdx == -1)
        return FALSE;
    if (!HasTrainerBeenFought(vsSeekerData[rematchIdx].trainerIDs[0]))
        return FALSE;
    return TRUE;
}

void ClearRematchStateOfLastTalked(void)
{
#if FREE_MATCH_CALL == FALSE
   gSaveBlock1Ptr->trainerRematches[gSpecialVar_LastTalked] = 0;
#endif //FREE_MATCH_CALL
   SetBattledTrainerFlag();
}

static int LookupVsSeekerOpponentInArray(const struct RematchData *array, enum TrainerID trainerId)
{
    int i, j;

    for (i = 0; i < ARRAY_COUNT(sRematches); i++)
    {
        for (j = 0; j < MAX_REMATCH_PARTIES; j++)
        {
            enum TrainerID testTrainerId;
            if (array[i].trainerIDs[j] == TRAINER_NONE)
                break;
            testTrainerId = array[i].trainerIDs[j];
            if (testTrainerId == SKIP)
                continue;
            if (testTrainerId == trainerId)
                return i;
        }
    }

    return -1;
}

#if FREE_MATCH_CALL == FALSE
enum TrainerID GetRematchTrainerId(enum TrainerID trainerId)
{
    u8 i;
    u8 j;
    j = GetNextAvailableRematchTrainer(sRematches, trainerId, &i);
    if (!j)
        return TRAINER_NONE;

    j = TryGetRematchTrainerIdGivenGameState(sRematches[i].trainerIDs, j);
    return sRematches[i].trainerIDs[j];
}
#endif //FREE_MATCH_CALL

u8 IsTrainerReadyForRematch(void)
{
    return IsTrainerReadyForRematchInternal(sRematches, TRAINER_BATTLE_PARAM.opponentA);
}

static bool8 IsTrainerReadyForRematchInternal(const struct RematchData *array, enum TrainerID trainerId)
{
    int rematchTrainerIdx = LookupVsSeekerOpponentInArray(array, trainerId);

    if (rematchTrainerIdx == -1)
        return FALSE;
    if (rematchTrainerIdx >= ARRAY_COUNT(sRematches))
        return FALSE;
    if (!IsThisTrainerRematchable(gSpecialVar_LastTalked))
        return FALSE;
    return TRUE;
}

static bool8 ObjectEventIdIsSane(u8 objectEventId)
{
    struct ObjectEvent *objectEvent = &gObjectEvents[objectEventId];

    if (objectEvent->active && gMapHeader.events->objectEventCount >= objectEvent->localId && gSprites[objectEvent->spriteId].data[0] == objectEventId)
        return TRUE;
    return FALSE;
}

static u8 GetRandomFaceDirectionMovementType()
{
    u16 r1 = Random() % 4;

    switch (r1)
    {
        case 0:
            return MOVEMENT_TYPE_FACE_UP;
        case 1:
            return MOVEMENT_TYPE_FACE_DOWN;
        case 2:
            return MOVEMENT_TYPE_FACE_LEFT;
        case 3:
            return MOVEMENT_TYPE_FACE_RIGHT;
        default:
            return MOVEMENT_TYPE_FACE_DOWN;
    }
}

#if FREE_MATCH_CALL == FALSE
static u8 GetRunningBehaviorFromGraphicsId(u16 graphicsId)
{
    switch (graphicsId)
    {
        case OBJ_EVENT_GFX_LITTLE_GIRL:
        case OBJ_EVENT_GFX_YOUNGSTER:
        case OBJ_EVENT_GFX_BOY:
        case OBJ_EVENT_GFX_BUG_CATCHER:
        case OBJ_EVENT_GFX_LASS:
        case OBJ_EVENT_GFX_WOMAN_1:
        case OBJ_EVENT_GFX_CRUSH_GIRL:
        case OBJ_EVENT_GFX_MAN:
        case OBJ_EVENT_GFX_ROCKER:
        case OBJ_EVENT_GFX_WOMAN_2:
        case OBJ_EVENT_GFX_BEAUTY:
        case OBJ_EVENT_GFX_BALDING_MAN:
        case OBJ_EVENT_GFX_TUBER_F:
        case OBJ_EVENT_GFX_CAMPER:
        case OBJ_EVENT_GFX_PICNICKER:
        case OBJ_EVENT_GFX_COOLTRAINER_M:
        case OBJ_EVENT_GFX_COOLTRAINER_F:
        case OBJ_EVENT_GFX_SWIMMER_M_LAND:
        case OBJ_EVENT_GFX_SWIMMER_F_LAND:
        case OBJ_EVENT_GFX_BLACK_BELT:
        case OBJ_EVENT_GFX_HIKER:
        case OBJ_EVENT_GFX_SAILOR:
            return MOVEMENT_TYPE_RAISE_HAND_AND_JUMP;
        case OBJ_EVENT_GFX_TUBER_M_WATER:
        case OBJ_EVENT_GFX_SWIMMER_M_WATER:
        case OBJ_EVENT_GFX_SWIMMER_F_WATER:
            return MOVEMENT_TYPE_RAISE_HAND_AND_SWIM;
        default:
            return MOVEMENT_TYPE_RAISE_HAND_AND_STOP;
    }
}
#endif //FREE_MATCH_CALL

void NativeVsSeekerRematchId(struct ScriptContext *ctx)
{
    enum TrainerID trainerId = ScriptReadHalfword(ctx);
    if (ctx->breakOnTrainerBattle && HasTrainerBeenFought(trainerId) && !ShouldTryRematchBattleForTrainerId(trainerId))
        StopScript(ctx);
}

static enum TrainerID GetTrainerFlagFromScript(const u8 *script)
{
    // The trainer flag is located 3 bytes (command + flags + localIdA) from the script pointer, assuming the trainerbattle command is first in the script.
    // Because scripts are unaligned, and because the ARM processor requires shorts to be 16-bit aligned, this function needs to perform explicit bitwise operations to get the correct flag.
    enum TrainerID trainerFlag;
    switch (script[0])
    {
        case SCR_OP_TRAINERBATTLE:
            script += 3;
            trainerFlag = script[0];
            trainerFlag |= script[1] << 8;
            break;
        case SCR_OP_CALLNATIVE:
        {
            u32 callnativeFunc = (((((script[4] << 8) + script[3]) << 8) + script[2]) << 8) + script[1];
            if (callnativeFunc == ((u32)NativeVsSeekerRematchId | 0xA000000)) // | 0xA000000 corresponds to the request_effects=1 version of the function
            {
                script += 5;
                trainerFlag = script[0];
                trainerFlag |= script[1] << 8;
            }
            else
            {
                trainerFlag = TRAINER_NONE;
            }
            break;
        }
        default:
            trainerFlag = TRAINER_NONE;
        break;
    }
    return trainerFlag;
}

static int GetRematchIdx(const struct RematchData *vsSeekerData, enum TrainerID trainerId)
{
    int i;

    for (i = 0; i < ARRAY_COUNT(sRematches); i++)
    {
        if (vsSeekerData[i].trainerIDs[0] == trainerId)
            return i;
    }

    return -1;
}

static bool32 IsThisTrainerRematchable(u32 localId)
{
#if FREE_MATCH_CALL == FALSE
   if (!gSaveBlock1Ptr->trainerRematches[localId])
      return FALSE;
#endif //FREE_MATCH_CALL
   return TRUE;
}

static void ClearAllTrainerRematchStates(void)
{
#if FREE_MATCH_CALL == FALSE
   for (u32 i = 0; i < ARRAY_COUNT(gSaveBlock1Ptr->trainerRematches); i++)
      gSaveBlock1Ptr->trainerRematches[i] = 0;
#endif //FREE_MATCH_CALL
}

#if FREE_MATCH_CALL == FALSE
static bool8 IsTrainerVisibleOnScreen(struct VsSeekerTrainerInfo *trainerInfo)
{
    s16 x;
    s16 y;

    PlayerGetDestCoords(&x, &y);
    x -= 7;
    y -= 7;

    if (   x - 7 <= trainerInfo->xCoord
        && x + 7 >= trainerInfo->xCoord
        && y - 5 <= trainerInfo->yCoord
        && y + 5 >= trainerInfo->yCoord
        && ObjectEventIdIsSane(trainerInfo->objectEventId))
        return TRUE;
    return FALSE;
}

static u8 GetNextAvailableRematchTrainer(const struct RematchData *vsSeekerData, enum TrainerID trainerId, u8 *idxPtr)
{
    for (u32 i = 0; i < ARRAY_COUNT(sRematches); i++)
    {
        if (vsSeekerData[i].trainerIDs[0] == trainerId)
        {
            u32 j;

            *idxPtr = i;
            for (j = 1; j < MAX_REMATCH_PARTIES; j++)
            {
                if (vsSeekerData[i].trainerIDs[j] == TRAINER_NONE)
                    return j - 1;
                if (vsSeekerData[i].trainerIDs[j] == SKIP)
                    continue;
                if (HasTrainerBeenFought(vsSeekerData[i].trainerIDs[j]))
                    continue;
                return j;
            }
            return j - 1;
        }
    }

    *idxPtr = 0;
    return 0;
}

static u8 GetRematchableTrainerLocalId(void)
{
    u8 idx;

    for (u32 i = 0; sVsSeeker->trainerInfo[i].localId != NO_REMATCH_LOCALID; i++)
    {
        if (IsTrainerVisibleOnScreen(&sVsSeeker->trainerInfo[i]))
        {
            if (!HasTrainerBeenFought(sVsSeeker->trainerInfo[i].trainerId) || GetNextAvailableRematchTrainer(sRematches, sVsSeeker->trainerInfo[i].trainerId, &idx))
                return sVsSeeker->trainerInfo[i].localId;
        }
    }

    return NO_REMATCH_LOCALID;
}

static void StartTrainerObjectMovementScript(struct VsSeekerTrainerInfo *trainerInfo, const u8 *script)
{
    UnfreezeObjectEvent(&gObjectEvents[trainerInfo->objectEventId]);
    ScriptMovement_StartObjectMovementScript(trainerInfo->localId, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup, script);
}

static u8 GetCurVsSeekerResponse(s32 vsSeekerIdx, enum TrainerID trainerId)
{
    for (u32 i = 0; i < vsSeekerIdx; i++)
    {
        if (IsTrainerVisibleOnScreen(&sVsSeeker->trainerInfo[i]) && sVsSeeker->trainerInfo[i].trainerId == trainerId)
        {
            for (u32 j = 0; j < sVsSeeker->numRematchableTrainers; j++)
            {
                if (sVsSeeker->trainerIds[j] == sVsSeeker->trainerInfo[i].trainerId)
                    return VSSEEKER_SINGLE_RESP_YES;
            }
            return VSSEEKER_SINGLE_RESP_NO;
        }
    }
    return VSSEEKER_SINGLE_RESP_RAND;
}
#endif //FREE_MATCH_CALL

static void StartAllRespondantIdleMovements(void)
{
#if FREE_MATCH_CALL == FALSE
    u8 dummy = 0;

    for (u32 i = 0; i < sVsSeeker->numRematchableTrainers; i++)
    {
        for (u32 j = 0; sVsSeeker->trainerInfo[j].localId != NO_REMATCH_LOCALID; j++)
        {
            if (sVsSeeker->trainerInfo[j].trainerId == sVsSeeker->trainerIds[i])
            {
                struct ObjectEvent *objectEvent = &gObjectEvents[sVsSeeker->trainerInfo[j].objectEventId];

                if (ObjectEventIdIsSane(sVsSeeker->trainerInfo[j].objectEventId))
                    SetTrainerMovementType(objectEvent, sVsSeeker->runningBehaviourEtcArray[i]);

                OverrideMovementTypeForObjectEvent(objectEvent, sVsSeeker->runningBehaviourEtcArray[i]);
                gSaveBlock1Ptr->trainerRematches[sVsSeeker->trainerInfo[j].localId] = GetNextAvailableRematchTrainer(sRematches, sVsSeeker->trainerInfo[j].trainerId, &dummy);
            }
        }
    }
#endif //FREE_MATCH_CALL
}
