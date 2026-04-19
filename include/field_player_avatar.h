#ifndef GUARD_FIELD_PLAYER_AVATAR_H
#define GUARD_FIELD_PLAYER_AVATAR_H

#include "global.h"
#include "constants/event_objects.h"

bool32 IsPlayerBiking(void);
bool32 ObjectMovingOnRockStairs(struct ObjectEvent *objectEvent, u8 direction);
bool32 TestPlayerAvatarState(enum PlayerState state);
bool32 WaitTeleportInPlayerAnim(void);
bool32 WaitTeleportWarpOutPlayerAnim(void);
bool8 IsPlayerFacingSurfableFishableWater(void);
bool8 IsPlayerSurfingNorth(void);
bool8 TryDoMetatileBehaviorForcedMovement(void);
enum Gender GetPlayerAvatarGenderByGraphicsId(enum ObjectEventGfx graphicsId);
enum ObjectEventGfx GetPlayerAvatarGraphicsIdByCurrentState(void);
enum PlayerState GetPlayerAvatarState(void);
u8 CheckForObjectEventCollision(struct ObjectEvent *objectEvent, s16 x, s16 y, u8 direction, u8 metatileBehavior);
enum ObjectEventGfx GetPlayerAvatarGraphicsIdByStateId(enum PlayerState state);
enum ObjectEventGfx GetPlayerAvatarGraphicsIdByStateIdAndGender(enum PlayerState state, enum Gender gender);
u8 GetPlayerAvatarObjectId(void);
u8 GetPlayerFacingDirection(void);
u8 GetPlayerMovementDirection(void);
enum ObjectEventGfx GetRivalAvatarGraphicsIdByStateIdAndGender(enum PlayerState state, enum Gender gender);
enum ObjectEventGfx GetRSAvatarGraphicsIdByGender(enum Gender gender);
u8 player_get_pos_including_state_based_drift(s16 *x, s16 *y);
u8 PlayerGetCopyableMovement(void);
u8 PlayerGetElevation(void);
void ClearPlayerAvatarInfo(void);
void CreateStopSurfingTask_NoMusicChange(u8 direction);
void GetXYCoordsOneStepInFrontOfPlayer(s16 *xPtr, s16 *yPtr);
void InitPlayerAvatar(s16 x, s16 y, u8 direction, enum Gender gender);
void MovementType_Player(struct Sprite *sprite);
void MovePlayerNotOnBike(u8 direction, u16 heldKeys);
void player_step(u8 direction, u16 newKeys, u16 heldKeys);
void PlayerFaceDirection(u8 direction);
void PlayerFaceDirectionFast(u8 direction);
void PlayerFreeze(void);
void PlayerGetDestCoords(s16 *, s16 *);
void PlayerGlide(u8 direction);
void PlayerJumpLedge(u8 direction);
void PlayerNotOnBikeCollide(u8 direction);
void PlayerOnBikeCollide(u8 direction);
void PlayerRideWaterCurrent(u8 direction);
void PlayerRun(u8 direction);
void PlayerRunSlow(u8 direction);
void PlayerShakeHeadOrWalkInPlace(void);
void PlayerTurnInPlace(u8 direction);
void PlayerUseAcroBikeOnBumpySlope(u8 direction);
void PlayerWalkFast(u8 direction);
void PlayerWalkFaster(u8 direction);
void PlayerWalkNormal(u8 direction);
void PlayerWalkSlow(u8 direction);
void PlayerWalkSlower(u8 direction);
void SavePlayerFacingDirectionForTeleport(u8 direction);
void SetPlayerAvatarExtraStateTransition(enum ObjectEventGfx graphicsId);
void SetPlayerAvatarState(enum PlayerState playerState);
void SetPlayerAvatarTransitionFlags(u16 flags);
void SetPlayerAvatarTransitionState(enum PlayerState transitionState);
void SetPlayerAvatarWatering(u8 direction);
void SetPlayerInvisibility(bool8);
void StartFishing(u8 secondaryId);
void StartPlayerAvatarSummonMonForFieldMoveAnim(void);
void StartPlayerAvatarVsSeekerAnim(void);
void StartTeleportInPlayerAnim(void);
void StartTeleportWarpOutPlayerAnim(void);
void StopPlayerAvatar(void);
void UpdatePlayerAvatarTransitionState(void);
//sideways stairs
u8 GetRightSideStairsDirection(u8 direction);
u8 GetLeftSideStairsDirection(u8 direction);
void SetPlayerAvatarFishing(u8 direction);
bool32 MetatileAtCoordsIsWaterTile(s16 x, s16 y);

#endif //GUARD_FIELD_PLAYER_AVATAR_H
