#ifndef GUARD_OVERWORLD_H
#define GUARD_OVERWORLD_H

#include "global.h"
#include "main.h"

#define LINK_KEY_CODE_NULL 0x00
#define LINK_KEY_CODE_EMPTY 0x11
#define LINK_KEY_CODE_DPAD_DOWN 0x12
#define LINK_KEY_CODE_DPAD_UP 0x13
#define LINK_KEY_CODE_DPAD_LEFT 0x14
#define LINK_KEY_CODE_DPAD_RIGHT 0x15
#define LINK_KEY_CODE_READY 0x16
#define LINK_KEY_CODE_EXIT_ROOM 0x17
#define LINK_KEY_CODE_START_BUTTON 0x18
#define LINK_KEY_CODE_A_BUTTON 0x19
#define LINK_KEY_CODE_IDLE 0x1A

// These two are a hack to stop user input until link stuff can be
// resolved.
#define LINK_KEY_CODE_HANDLE_RECV_QUEUE 0x1B
#define LINK_KEY_CODE_HANDLE_SEND_QUEUE 0x1C

#define LINK_KEY_CODE_EXIT_SEAT 0x1D

#define MOVEMENT_MODE_FREE 0
#define MOVEMENT_MODE_FROZEN 1
#define MOVEMENT_MODE_SCRIPTED 2

#define TIME_OF_DAY_NIGHT       0
#define TIME_OF_DAY_TWILIGHT    1
#define TIME_OF_DAY_DAY         2
#define TIME_OF_DAY_MAX         TIME_OF_DAY_DAY

struct LinkPlayerObjectEvent
{
    u8 active;
    u8 linkPlayerId;
    u8 objEventId;
    u8 movementMode;
};

struct CreditsOverworldCmd
{
    s16 unk_0;
    s16 unk_2;
    s16 unk_4;
};

/* gDisableMapMusicChangeOnMapLoad */
enum {
    MUSIC_DISABLE_OFF,
    MUSIC_DISABLE_STOP,
    MUSIC_DISABLE_KEEP,
};

// Item Description Headers
enum ItemObtainFlags
{
    FLAG_GET_ITEM_OBTAINED,
    FLAG_SET_ITEM_OBTAINED,
};

extern struct WarpData gLastUsedWarp;
extern struct LinkPlayerObjectEvent gLinkPlayerObjectEvents[4];
extern u16 *gOverworldTilemapBuffer_Bg2;
extern u16 *gOverworldTilemapBuffer_Bg1;
extern u16 *gOverworldTilemapBuffer_Bg3;
extern u16 gHeldKeyCodeToSend;
extern MainCallback gFieldCallback;
extern bool8 (* gFieldCallback2)(void);
extern u8 gLocalLinkPlayerId;
extern u8 gFieldLinkPlayerCount;
extern u8 gExitStairsMovementDisabled;
extern u8 gDisableMapMusicChangeOnMapLoad;
extern u8 gGlobalFieldTintMode;
extern u8 gTimeOfDay;
extern s16 gTimeUpdateCounter;
extern struct TimeBlendSettings gTimeBlend;
extern const struct Coords32 gDirectionToVectors[];

bool32 CurrentMapHasShadows(void);
bool32 IsOverworldLinkActive(void);
bool32 IsSendingKeysOverCable(void);
bool32 MapHasNaturalLight(u8 mapType);
bool32 Overworld_DoScrollSceneForCredits(u8 *, const struct CreditsOverworldCmd *, u8);
bool32 Overworld_IsBikingAllowed(void);
bool32 Overworld_IsRecvQueueAtMax(void);
bool32 Overworld_MusicCanOverrideMapMusic(u16 song);
bool32 Overworld_RecvKeysFromLinkIsRunning(void);
bool32 Overworld_SendKeysToLinkIsRunning(void);
bool8 BGMusicStopped(void);
bool8 GetSetItemObtained(enum Item item, enum ItemObtainFlags caseId);
bool8 IsMapTypeIndoors(u8 mapType);
bool8 IsMapTypeOutdoors(u8 mapType);
bool8 MetatileBehavior_IsSurfableInSeafoamIslands(u16 metatileBehavior);
bool8 Overworld_MapTypeAllowsTeleportAndFly(u8 mapType);
bool8 SetDiveWarpDive(u16 x, u16 y);
bool8 SetDiveWarpEmerge(u16 x, u16 y);
const struct MapHeader *const GetDestinationWarpMapHeader(void);
const struct MapHeader *const Overworld_GetMapHeaderByGroupAndId(u16, u16);
u16 QueueExitLinkRoomKey(void);
u16 SetInCableClubSeat(void);
u16 SetLinkWaitingForScript(void);
u16 SetStartedCableClubActivity(void);
u32 ComputeWhiteOutMoneyLoss(void);
u32 GetCableClubPartnersReady(void);
u32 GetGameStat(u8 statId);
u8 GetCurrentMapBattleScene(void);
u8 GetCurrentMapType(void);
u8 GetCurrentRegionMapSectionId(void);
u8 GetFlashLevel(void);
u8 GetLastUsedWarpMapSectionId(void);
u8 GetLastUsedWarpMapType(void);
u8 GetMapTypeByGroupAndId(s8 mapGroup, s8 mapNum);
u8 IsMapTypeOutdoors(u8 mapType);
u8 UpdateSpritePaletteWithTime(u8 paletteNum);
void CB1_Overworld(void);
void CB2_ContinueSavedGame(void);
void CB2_EnterFieldFromQuestLog(void);
void CB2_LoadMap(void);
void CB2_NewGame(void);
void CB2_Overworld(void);
void CB2_OverworldBasic(void);
void CB2_ReturnToField(void);
void CB2_ReturnToFieldCableClub(void);
void CB2_ReturnToFieldContinueScript(void);
void CB2_ReturnToFieldContinueScriptPlayMapMusic(void);
void CB2_ReturnToFieldFromDiploma(void);
void CB2_ReturnToFieldFromMultiplayer(void);
void CB2_ReturnToFieldWithOpenMenu(void);
void CB2_SetUpOverworldForQLPlayback(void);
void CB2_SetUpOverworldForQLPlaybackWithWarpExit(void);
void CB2_WhiteOut(void);
void CleanupOverworldWindowsAndTilemaps(void);
void ClearLinkPlayerObjectEvents(void);
void IncrementGameStat(u8 index);
void LoadMapFromCameraTransition(u8 mapGroup, u8 mapNum);
void ObjectEventMoveDestCoords(struct ObjectEvent *objectEvent, enum Direction direction, s16 *x, s16 *y);
void Overworld_ChangeMusicTo(u16);
void Overworld_ChangeMusicToDefault(void);
void Overworld_ClearSavedMusic(void);
void Overworld_CreditsMainCB(void);
void Overworld_FadeOutMapMusic(void);
void Overworld_PlaySpecialMapMusic(void);
void Overworld_ResetMapMusic(void);
void Overworld_ResetStateAfterDigEscRope(void);
void Overworld_ResetStateAfterFly(void);
void Overworld_ResetStateAfterTeleport(void);
void Overworld_SetHealLocationWarp(u8);
void Overworld_SetSavedMusic(u16);
void Overworld_SetWarpDestinationFromWarp(struct WarpData *);
void OverworldWhiteOutGetMoneyLoss(void);
void ResetGameStats(void);
void ResetInitialPlayerAvatarState(void);
void SetContinueGameWarpToDynamicWarp(int);
void SetContinueGameWarpToHealLocation(u8 loc);
void SetCurrentMapLayout(u16 mapLayoutId);
void SetDynamicWarp(s32 unused, s8 mapGroup, s8 mapNum, s8 warpId);
void SetDynamicWarpWithCoords(s32 unused, s8 mapGroup, s8 mapNum, s8 warpId, s8 x, s8 y);
void SetEscapeWarp(s8 mapGroup, s8 mapNum, s8 warpId, s8 x, s8 y);
void SetFixedDiveWarp(s8 mapGroup, s8 mapNum, s8 warpId, s8 x, s8 y);
void SetFixedHoleWarp(s8 mapGroup, s8 mapNum, s8 warpId, s8 x, s8 y);
void SetFlashLevel(s32 a1);
void SetGameStat(u8 statId, u32 value);
void SetLastHealLocationWarp(u8 healLocaionId);
void SetMainCallback1(MainCallback cb);
void SetObjEventTemplateCoords(u8, s16, s16);
void SetObjEventTemplateMovementType(u8, u8);
void SetWarpDestination(s8 mapGroup, s8 mapNum, s8 warpId, s8 x, s8 y);
void SetWarpDestinationToDynamicWarp(u8 unused);
void SetWarpDestinationToEscapeWarp(void);
void SetWarpDestinationToFixedHoleWarp(s16 x, s16 y);
void SetWarpDestinationToHealLocation(u8 a0);
void SetWarpDestinationToLastHealLocation(void);
void SetWarpDestinationToMapWarp(s8 mapGroup, s8 mapNum, s8 warpNum);
void StoreInitialPlayerAvatarState(void);
void TryFadeOutOldMapMusic(void);
void UpdateAltBgPalettes(u16 palettes);
void UpdateAmbientCry(s16 *state, u16 *delayCounter);
void UpdateEscapeWarp(s16 x, s16 y);
void UpdatePalettesWithTime(u32);
void UpdateTimeOfDay(void);
void WarpIntoMap(void);

#endif //GUARD_OVERWORLD_H
