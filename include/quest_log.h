#ifndef GUARD_QUEST_LOG_H
#define GUARD_QUEST_LOG_H

#include "global.h"
#include "quest_log_battle.h"
#include "field_control_avatar.h"
#include "constants/quest_log.h"
#include "constants/battle.h"

struct QuestLogAction
{
    union {
        struct {
            u8 localId;
            u8 mapNum;
            u8 mapGroup;
            u8 movementActionId;
        } a; // Data when type == 0
        struct {
            u8 localId;
            u8 mapNum;
            u8 mapGroup;
            u8 gfxState;
        } b; // Data when type == 1
        u8 fieldInput[4]; // Data when type == 2
        u8 raw[4];
    } data;
    u16 duration;
    u8 type;
};

struct QuestLogRepeatEventTracker
{
    u8 id;
    u8 numRepeats;
    u16 counter;
};

// Event data structs

// QL_EVENT_SWITCHED_PARTY_ORDER
struct QuestLogEvent_SwitchedPartyOrder
{
    u16 species1;
    u16 species2;
};

// QL_EVENT_USED_ITEM
// QL_EVENT_GAVE_HELD_ITEM
// QL_EVENT_GAVE_HELD_ITEM_BAG
// QL_EVENT_GAVE_HELD_ITEM_PC
// QL_EVENT_TOOK_HELD_ITEM
// QL_EVENT_DEPOSITED_ITEM_PC
// QL_EVENT_WITHDREW_ITEM_PC
struct QuestLogEvent_Item
{
    enum Item itemId;
    u16 unused;
    u16 species;
    u16 itemParam;
};

// QL_EVENT_SWAPPED_HELD_ITEM
// QL_EVENT_SWAPPED_HELD_ITEM_PC
struct QuestLogEvent_SwappedHeldItem
{
    u16 takenItemId;
    u16 givenItemId;
    u16 species;
};

// QL_EVENT_LINK_TRADED
// QL_EVENT_LINK_TRADED_UNION
struct QuestLogEvent_Traded
{
    u16 speciesSent;
    u16 speciesReceived;
    u8 partnerName[PLAYER_NAME_LENGTH];
};

// QL_EVENT_LINK_BATTLED_SINGLE
// QL_EVENT_LINK_BATTLED_DOUBLE
// QL_EVENT_LINK_BATTLED_MULTI
// QL_EVENT_LINK_BATTLED_UNION
struct QuestLogEvent_LinkBattle
{
    u8 outcome;
    u8 playerNames[MAX_BATTLERS_COUNT - 1][PLAYER_NAME_LENGTH];
};

// QL_EVENT_SWITCHED_MONS_BETWEEN_BOXES
// QL_EVENT_SWITCHED_MONS_WITHIN_BOX
// QL_EVENT_SWITCHED_PARTY_MON_FOR_PC_MON
// QL_EVENT_MOVED_MON_BETWEEN_BOXES
// QL_EVENT_MOVED_MON_WITHIN_BOX
// QL_EVENT_WITHDREW_MON_PC
// QL_EVENT_DEPOSITED_MON_PC
// QL_EVENT_SWITCHED_MULTIPLE_MONS
struct QuestLogEvent_MovedBoxMon
{
    u16 species1;
    u16 species2;
    u8 box1;
    u8 box2;
};

// QL_EVENT_DEFEATED_TRAINER
// QL_EVENT_DEFEATED_GYM_LEADER
// QL_EVENT_DEFEATED_E4_MEMBER
// QL_EVENT_DEFEATED_CHAMPION
struct QuestLogEvent_TrainerBattle
{
    u16 trainerId;
    u16 speciesOpponent;
    u16 speciesPlayer;
    u8 hpFractionId;
    u8 mapSec;
};

// QL_EVENT_DEFEATED_WILD_MON
struct QuestLogEvent_WildBattle
{
    u16 defeatedSpecies;
    u16 caughtSpecies;
    u8 mapSec;
};

// QL_EVENT_DEPARTED
struct QuestLogEvent_Departed
{
    u8 mapSec;
    u8 locationId;
};

// QL_EVENT_USED_FIELD_MOVE
struct QuestLogEvent_FieldMove
{
    u16 species;
    u8 fieldMove;
    u8 mapSec;
};

// QL_EVENT_BOUGHT_ITEM
// QL_EVENT_SOLD_ITEM
struct QuestLogEvent_Shop
{
    u32 totalMoney; // Total amount of money spent buying or earned selling
    enum Item lastItemId;
    u16 itemQuantity; // Total number of items bought or sold
    u8 mapSec;
    bool8 hasMultipleTransactions;
    u8 logEventId; // Either 0 (empty), 1 (bought) or 2 (sold)
};

// QL_EVENT_OBTAINED_STORY_ITEM
struct QuestLogEvent_StoryItem
{
    enum Item itemId;
    u8 mapSec;
};

extern u8 gQuestLogState;
extern u8 gQuestLogPlaybackState;
extern struct FieldInput gQuestLogFieldInput;
extern struct QuestLogRepeatEventTracker gQuestLogRepeatEventTracker;
extern u16 *gQuestLogDefeatedWildMonRecord;
extern u16 *gQuestLogRecordingPointer;
extern u16 gQuestLogCurActionIdx;

bool8 QL_AvoidDisplay(void (*func)(void));
bool8 QL_IsRoomToSaveAction(const void *cursor, size_t size);
bool8 QL_IsRoomToSaveEvent(const void *cursor, size_t size);
bool8 QL_IsTrainerSightDisabled(void);
bool8 QL_LoadEvent(const u16 *);
bool8 QL_TryRepeatEvent(const u16 *);
bool8 QuestLog_ShouldEndSceneOnMapChange(void);
bool8 QuestLogScenePlaybackIsEnding(void);
enum QLPlaybackState QL_GetPlaybackState(void);
u16 *QL_LoadAction_Input(u16 *, struct QuestLogAction *);
u16 *QL_LoadAction_MovementOrGfxChange(u16 *, struct QuestLogAction *);
u16 *QL_LoadAction_SceneEnd(u16 *, struct QuestLogAction *);
u16 *QL_LoadAction_Wait(u16 *, struct QuestLogAction *);
u16 *QL_RecordAction_Input(u16 *, struct QuestLogAction *);
u16 *QL_RecordAction_MovementOrGfxChange(u16 *, struct QuestLogAction *);
u16 *QL_RecordAction_SceneEnd(u16 *);
u16 *QL_SkipCommand(u16 *, u16 **);
enum QLStartType GetQuestLogStartType(void);
void *QuestLogGetFlagOrVarPtr(bool8 isFlag, u16 idx);
void CommitQuestLogWindow1(void);
void QL_AddASLROffset(void *oldSaveBlockPtr);
void QL_AfterRecordFishActionSuccessful(void);
void QL_CopySaveState(void);
void QL_EnableRecordingSteps(void);
void QL_FinishRecordingScene(void);
void QL_HandleInput(void);
void QL_InitSceneObjectsAndActions(void);
void QL_RecordFieldInput(struct FieldInput *fieldInput);
void QL_RecordWait(u16);
void QL_ResetDefeatedWildMonRecord(void);
void QL_ResetEventStates(void);
void QL_ResetPartyAndPC(void);
void QL_ResetRepeatEventTracker(void);
void QL_RestoreMapLayoutId(void);
void QL_StartRecordingAction(enum QLEvent eventId);
void QL_TryRunActions(void);
void QL_UpdateLastDepartedLocation(const u16 *);
void QL_UpdateObject(struct Sprite *sprite);
void QuestLog_AdvancePlayhead_(void);
void QuestLog_BackUpPalette(u16 offset, u16 size);
void QuestLog_CutRecording(void);
void QuestLog_DrawPreviouslyOnQuestHeaderIfInPlaybackMode(void);
void QuestLog_InitPalettesBackup(void);
void QuestLog_OnEscalatorWarp(enum QLEscalatorType direction);
void QuestLog_RecordEnteredMap(u16);
void QuestLogEvents_HandleEndTrainerBattle(void);
void QuestLogRecordNPCStep(u8 a0, u8 a1, u8 a2, u8 a3);
void QuestLogRecordNPCStepWithDuration(u8 localId, u8 mapNum, u8 mapGroup, u8 movementActionId, u8 duration);
void QuestLogRecordPlayerAvatarGfxTransition(u8);
void QuestLogRecordPlayerAvatarGfxTransitionWithDuration(u8 movementActionId, u8 duration);
void QuestLogRecordPlayerStep(u8 movementActionId);
void QuestLogRecordPlayerStepWithDuration(u8 movementActionId, u8 duration);
void QuestLogSetFlagOrVar(bool8 isFlag, u16 idx, u16 value);
void ResetDeferredLinkEvent(void);
void ResetQuestLog(void);
void ResetTrainerFanClub(void);
void RunQuestLogCB(void);
void SaveQuestLogData(void);
void SetQLPlayedTheSlots(void);
void SetQuestLogEvent_Arrived(void);
void SetQuestLogEvent(enum QLEvent, const u16 *);
void Special_UpdateTrainerFansAfterLinkBattle(void);
void TryStartQuestLogPlayback(u8 taskId);

#endif //GUARD_QUEST_LOG_H
