#ifndef GUARD_FIELD_PLAYER_AVATAR_H
#define GUARD_FIELD_PLAYER_AVATAR_H

#include "global.h"
#include "constants/event_objects.h"

bool32 IsPlayerBiking(void);
bool32 ObjectMovingOnRockStairs(struct ObjectEvent *objectEvent, enum Direction direction);
bool32 TestPlayerAvatarState(enum AvatarState state);
bool32 WaitTeleportInPlayerAnim(void);
bool32 WaitTeleportWarpOutPlayerAnim(void);
bool8 IsPlayerFacingSurfableFishableWater(void);
bool8 IsPlayerSurfingNorth(void);
bool8 TryDoMetatileBehaviorForcedMovement(void);
enum Gender GetPlayerAvatarGenderByGraphicsId(enum ObjectEventGfx graphicsId);
enum ObjectEventGfx GetPlayerAvatarGfxForCurrentState(void);
enum ObjectEventGfx GetPlayerAvatarGfxForState(enum AvatarState state);
enum ObjectEventGfx GetPlayerAvatarGfxForGender(enum AvatarState state, enum Gender gender);
enum ObjectEventGfx GetPlayerAvatarGfxForVersion(enum AvatarState state, enum Gender gender, enum GameVersion version);
enum AvatarState GetPlayerAvatarState(void);
enum Collision CheckForObjectEventCollision(struct ObjectEvent *objectEvent, s16 x, s16 y, enum Direction direction, u8 metatileBehavior);
u8 GetPlayerAvatarObjectId(void);
enum Direction GetPlayerFacingDirection(void);
enum Direction GetPlayerMovementDirection(void);
void GetPlayerPosWithDrift(s16 *x, s16 *y);
u8 PlayerGetCopyableMovement(void);
u8 PlayerGetElevation(void);
void ClearPlayerAvatarInfo(void);
void CreateStopSurfingTask_NoMusicChange(enum Direction direction);
void GetXYCoordsOneStepInFrontOfPlayer(s16 *xPtr, s16 *yPtr);
void InitPlayerAvatar(s16 x, s16 y, enum Direction direction, enum Gender gender);
void MovementType_Player(struct Sprite *sprite);
void MovePlayerNotOnBike(enum Direction direction, u16 heldKeys);
void PlayerStep(enum Direction direction, u16 newKeys, u16 heldKeys);
void PlayerFaceDirection(enum Direction direction);
void PlayerFaceDirectionFast(enum Direction direction);
void PlayerFreeze(void);
void PlayerGetDestCoords(s16 *, s16 *);
void PlayerGlide(enum Direction direction);
void PlayerJumpLedge(enum Direction direction);
void PlayerNotOnBikeCollide(enum Direction direction);
void PlayerOnBikeCollide(enum Direction direction);
void PlayerRideWaterCurrent(enum Direction direction);
void PlayerRun(enum Direction direction);
void PlayerRunSlow(enum Direction direction);
void PlayerShakeHeadOrWalkInPlace(void);
void PlayerTurnInPlace(enum Direction direction);
void PlayerUseAcroBikeOnBumpySlope(enum Direction direction);
void PlayerWalkFast(enum Direction direction);
void PlayerWalkFaster(enum Direction direction);
void PlayerWalkNormal(enum Direction direction);
void PlayerWalkSlow(enum Direction direction);
void PlayerWalkSlower(enum Direction direction);
void SavePlayerFacingDirectionForTeleport(enum Direction direction);
void SetPlayerAvatarExtraStateTransition(enum ObjectEventGfx graphicsId);
void SetPlayerAvatarState(enum AvatarState playerState);
void SetPlayerAvatarTransitionFlags(u16 flags);
void SetPlayerAvatarTransitionState(enum AvatarState transitionState);
void SetPlayerAvatarWatering(enum Direction direction);
void SetPlayerInvisibility(bool8);
void StartFishing(u8 secondaryId);
void StartPlayerAvatarSummonMonForFieldMoveAnim(void);
void StartPlayerAvatarVsSeekerAnim(void);
void StartTeleportInPlayerAnim(void);
void StartTeleportWarpOutPlayerAnim(void);
void StopPlayerAvatar(void);
void UpdatePlayerAvatarTransitionState(void);
//sideways stairs
enum Direction GetRightSideStairsDirection(enum Direction direction);
enum Direction GetLeftSideStairsDirection(enum Direction direction);
void SetPlayerAvatarFishing(enum Direction direction);
bool32 MetatileAtCoordsIsWaterTile(s16 x, s16 y);

#endif //GUARD_FIELD_PLAYER_AVATAR_H
