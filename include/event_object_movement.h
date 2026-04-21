#ifndef GUARD_EVENT_OBJECT_MOVEMENT_H
#define GUARD_EVENT_OBJECT_MOVEMENT_H

#include "constants/event_object_movement.h"

#if OW_BATTLE_ONLY_FORMS && !OW_POKEMON_OBJECT_EVENTS
#error "OW_POKEMON_OBJECT_EVENTS needs to be TRUE in order for OW_BATTLE_ONLY_FORMS to work."
#endif

#if OW_POKEMON_OBJECT_EVENTS == FALSE && OW_FOLLOWERS_ENABLED == TRUE
#error "OW_POKEMON_OBJECT_EVENTS needs to be TRUE in order for OW_FOLLOWERS_ENABLED to work."
#endif

// Palette slots for overworld NPCs.
// The same standard set of palettes for overworld objects are normally always loaded at the same
// time while walking around the overworld. The only exceptions are the palettes for the player and
// the "special" NPC, which can be swapped out. This also means that e.g. two "special" NPCs
// with competing palettes cannot be properly loaded at the same time.
enum {
    PALSLOT_PLAYER,
    PALSLOT_PLAYER_REFLECTION,
    PALSLOT_NPC_1,
    PALSLOT_NPC_2,
    PALSLOT_NPC_3,
    PALSLOT_NPC_4,
    PALSLOT_NPC_1_REFLECTION,
    PALSLOT_NPC_2_REFLECTION,
    PALSLOT_NPC_3_REFLECTION,
    PALSLOT_NPC_4_REFLECTION,
    PALSLOT_NPC_SPECIAL,
    PALSLOT_NPC_SPECIAL_REFLECTION,
    OBJ_PALSLOT_COUNT
    // the remaining sprite palette slots (12-15) are used by field effects, the interface, etc.
};

enum SpinnerRunnerFollowPatterns
{
    RUNFOLLOW_ANY,
    RUNFOLLOW_NORTH_SOUTH,
    RUNFOLLOW_EAST_WEST,
    RUNFOLLOW_NORTH_WEST,
    RUNFOLLOW_NORTH_EAST,
    RUNFOLLOW_SOUTH_WEST,
    RUNFOLLOW_SOUTH_EAST,
    RUNFOLLOW_NORTH_SOUTH_WEST,
    RUNFOLLOW_NORTH_SOUTH_EAST,
    RUNFOLLOW_NORTH_EAST_WEST,
    RUNFOLLOW_SOUTH_EAST_WEST
};

enum FollowerTransformTypes
{
    TRANSFORM_TYPE_NONE,
    TRANSFORM_TYPE_PERMANENT,
    TRANSFORM_TYPE_RANDOM_WILD,
    TRANSFORM_TYPE_WEATHER,
};

// Sprite data for the CameraObject functions
#define sCamera_FollowSpriteId data[0]
#define sCamera_State          data[1]
#define sCamera_MoveX          data[2]
#define sCamera_MoveY          data[3]

struct StepAnimTable
{
    const union AnimCmd *const *anims;
    u8 animPos[4];
};

typedef void (*MovementAction)(enum Direction direction);

enum GroundEffect
{
    GROUND_EFFECT_TALL_GRASS_ON_SPAWN,
    GROUND_EFFECT_TALL_GRASS_ON_MOVE,
    GROUND_EFFECT_LONG_GRASS_ON_SPAWN,
    GROUND_EFFECT_LONG_GRASS_ON_MOVE,
    GROUND_EFFECT_WATER_REFLECTION,
    GROUND_EFFECT_ICE_REFLECTION,
    GROUND_EFFECT_SHALLOW_FLOWING_WATER,
    GROUND_EFFECT_SAND,
    GROUND_EFFECT_DEEP_SAND,
    GROUND_EFFECT_RIPPLES,
    GROUND_EFFECT_PUDDLE,
    GROUND_EFFECT_SAND_PILE,
    GROUND_EFFECT_LAND_IN_TALL_GRASS,
    GROUND_EFFECT_LAND_IN_LONG_GRASS,
    GROUND_EFFECT_LAND_IN_SHALLOW_WATER,
    GROUND_EFFECT_LAND_IN_DEEP_WATER,
    GROUND_EFFECT_LAND_ON_NORMAL_GROUND,
    GROUND_EFFECT_SHORT_GRASS,
    GROUND_EFFECT_HOT_SPRINGS,
    GROUND_EFFECT_SEAWEED,
    GROUND_EFFECT_SNOW,
};

#define GROUND_EFFECT_FLAG_TALL_GRASS_ON_SPAWN   (1 << GROUND_EFFECT_TALL_GRASS_ON_SPAWN)
#define GROUND_EFFECT_FLAG_TALL_GRASS_ON_MOVE    (1 << GROUND_EFFECT_TALL_GRASS_ON_MOVE)
#define GROUND_EFFECT_FLAG_LONG_GRASS_ON_SPAWN   (1 << GROUND_EFFECT_LONG_GRASS_ON_SPAWN)
#define GROUND_EFFECT_FLAG_LONG_GRASS_ON_MOVE    (1 << GROUND_EFFECT_LONG_GRASS_ON_MOVE)
#define GROUND_EFFECT_FLAG_WATER_REFLECTION      (1 << GROUND_EFFECT_WATER_REFLECTION)
#define GROUND_EFFECT_FLAG_ICE_REFLECTION        (1 << GROUND_EFFECT_ICE_REFLECTION)
#define GROUND_EFFECT_FLAG_SHALLOW_FLOWING_WATER (1 << GROUND_EFFECT_SHALLOW_FLOWING_WATER)
#define GROUND_EFFECT_FLAG_SAND                  (1 << GROUND_EFFECT_SAND)
#define GROUND_EFFECT_FLAG_DEEP_SAND             (1 << GROUND_EFFECT_DEEP_SAND)
#define GROUND_EFFECT_FLAG_RIPPLES               (1 << GROUND_EFFECT_RIPPLES)
#define GROUND_EFFECT_FLAG_PUDDLE                (1 << GROUND_EFFECT_PUDDLE)
#define GROUND_EFFECT_FLAG_SAND_PILE             (1 << GROUND_EFFECT_SAND_PILE)
#define GROUND_EFFECT_FLAG_LAND_IN_TALL_GRASS    (1 << GROUND_EFFECT_LAND_IN_TALL_GRASS)
#define GROUND_EFFECT_FLAG_LAND_IN_LONG_GRASS    (1 << GROUND_EFFECT_LAND_IN_LONG_GRASS)
#define GROUND_EFFECT_FLAG_LAND_IN_SHALLOW_WATER (1 << GROUND_EFFECT_LAND_IN_SHALLOW_WATER)
#define GROUND_EFFECT_FLAG_LAND_IN_DEEP_WATER    (1 << GROUND_EFFECT_LAND_IN_DEEP_WATER)
#define GROUND_EFFECT_FLAG_LAND_ON_NORMAL_GROUND (1 << GROUND_EFFECT_LAND_ON_NORMAL_GROUND)
#define GROUND_EFFECT_FLAG_SHORT_GRASS           (1 << GROUND_EFFECT_SHORT_GRASS)
#define GROUND_EFFECT_FLAG_HOT_SPRINGS           (1 << GROUND_EFFECT_HOT_SPRINGS)
#define GROUND_EFFECT_FLAG_SEAWEED               (1 << GROUND_EFFECT_SEAWEED)
#define GROUND_EFFECT_FLAG_SNOW                  (1 << GROUND_EFFECT_SNOW)

enum JumpType
{
    JUMP_TYPE_HIGH,
    JUMP_TYPE_LOW,
    JUMP_TYPE_NORMAL,
    JUMP_TYPE_FAST,
    JUMP_TYPE_FASTER,
};

enum MoveSpeed
{
    MOVE_SPEED_NORMAL, // walking
    MOVE_SPEED_FAST_1, // running / surfing / sliding (ice tile)
    MOVE_SPEED_FAST_2, // water current / bicycle
    MOVE_SPEED_FASTER, // going down cycling road on bicycle
    MOVE_SPEED_FASTEST,
};

struct PairedPalettes
{
    u16 tag;
    const u16 *data;
};

struct FollowerSpriteVisualizerData
{
    u16 currentmonId;
    bool8 isShiny;
    bool8 isFemale;
};

u8 GetObjectEventIdByLocalIdAndMap(u8, u8, u8);
bool8 TryGetObjectEventIdByLocalIdAndMap(u8, u8, u8, u8 *);
u8 GetObjectEventIdByXY(s16, s16);
void SetObjectEventDirection(struct ObjectEvent *objectEvent, enum Direction direction);
void RemoveObjectEvent(struct ObjectEvent *objectEvent);
void RemoveObjectEventByLocalIdAndMap(u8 localId, u8 mapNum, u8 mapGroup);
u16 GetBoulderRevealFlagByLocalIdAndMap(u8 localId, u8 mapNum, u8 mapGroup);
void LoadPlayerObjectReflectionPalette(u16, u8);
void LoadSpecialObjectReflectionPalette(u16, u8);
void TryMoveObjectEventToMapCoords(u8, u8, u8, s16, s16);
void PatchObjectPalette(u16, u8);
void SpawnObjectEventsOnReturnToField(s16 x, s16 y);
void GetMapCoordsFromSpritePos(s16, s16, s16 *, s16 *);
u8 GetFaceDirectionAnimNum(enum Direction direction);
void SetSpritePosToOffsetMapCoords(s16 *, s16 *, s16, s16);
void ObjectEventClearHeldMovement(struct ObjectEvent *);
void ObjectEventClearHeldMovementIfActive(struct ObjectEvent *);
struct Pokemon *GetFirstLiveMon(void);
void UpdateFollowingPokemon(void);
void RemoveFollowingPokemon(void);
struct ObjectEvent *GetFollowerObject(void);
u8 CreateVirtualObject(u16 graphicsId, u8 virtualObjId, s16 x, s16 y, u8 elevation, enum Direction direction);
u8 CreateObjectGraphicsSprite(u16 graphicsId, SpriteCallback callback, s16 x, s16 y, u8 subpriority);
u8 TrySpawnObjectEvent(u8 localId, u8 mapNum, u8 mapGroup);
u32 SpawnSpecialObjectEventParameterized(u16 graphicsId, u8 movementBehavior, u8 localId, s16 x, s16 y, u8 elevation);
u8 SpawnSpecialObjectEvent(struct ObjectEventTemplate *);
void CameraObjectReset(void);
void CameraObjectFreeze(void);
u8 UpdateSpritePaletteByTemplate(const struct SpriteTemplate *template, struct Sprite *sprite);
void ObjectEventSetGraphicsId(struct ObjectEvent *objectEvent, u16 graphicsId);
void ObjectEventTurn(struct ObjectEvent *objectEvent, enum Direction direction);
void ObjectEventTurnByLocalIdAndMap(u8 localId, u8 mapNum, u8 mapGroup, enum Direction direction);
void ObjectEventForceSetHeldMovement(struct ObjectEvent *, u8);
const struct ObjectEventGraphicsInfo *GetObjectEventGraphicsInfo(u16 graphicsId);
void SetObjectInvisibility(u8 localId, u8 mapNum, u8 mapGroup, u8 state);
void FreeAndReserveObjectSpritePalettes(void);
u8 LoadObjectEventPalette(u16 paletteTag);
u8 LoadObjectEventPaletteCopy(u16 originalTag, u16 copyTag);
u8 LoadPlayerObjectEventPalette(u8 gender);
void SetObjectPositionByLocalIdAndMap(u8 localId, u8 mapNum, u8 mapGroup, s16 x, s16 y);
void ResetObjectSubpriority(u8 localId, u8 mapNum, u8 mapGroup);
void SetObjectSubpriority(u8 localId, u8 mapNum, u8 mapGroup, u8 subpriority);
void ObjectEventGetLocalIdAndMap(struct ObjectEvent *objectEvent, void *localId, void *mapNum, void *mapGroup);
void ShiftObjectEventCoords(struct ObjectEvent *, s16, s16);
void TryOverrideObjectEventTemplateCoords(u8, u8, u8);
void UpdateObjectEventCurrentMovement(struct ObjectEvent *, struct Sprite *, bool8(struct ObjectEvent *, struct Sprite *));
bool8 ObjectEventFaceOppositeDirection(struct ObjectEvent *objectEvent, enum Direction direction);
enum Direction GetOppositeDirection(enum Direction direction);
u8 GetWalkInPlaceFasterMovementAction(enum Direction direction);
u8 GetStepInPlaceDelay8AnimId(u32);
u8 GetWalkInPlaceNormalMovementAction(enum Direction direction);
u8 GetWalkInPlaceSlowMovementAction(enum Direction direction);
u8 GetWalkInPlaceFastMovementAction(enum Direction direction);
enum Collision GetCollisionAtCoords(struct ObjectEvent *, s16, s16, enum Direction direction);
void MoveCoords(enum Direction direction, s16 *, s16 *);
bool8 ObjectEventIsHeldMovementActive(struct ObjectEvent *);
u8 ObjectEventClearHeldMovementIfFinished(struct ObjectEvent *);
u8 GetObjectEventIdByPosition(u16 x, u16 y, u8 elevation);
void QL_UpdateObjectEventCurrentMovement(struct ObjectEvent *, struct Sprite *);
bool8 ObjectEventSetHeldMovement(struct ObjectEvent *, u8);
void ShiftStillObjectEventCoords(struct ObjectEvent *);
void OverrideMovementTypeForObjectEvent(const struct ObjectEvent *, u8);
void SetTrainerMovementType(struct ObjectEvent *, u8);
u8 GetFishingDirectionAnimNum(enum Direction direction);
u8 GetFishingNoCatchDirectionAnimNum(enum Direction direction);
u8 CreateFameCheckerObject(u16 graphicsId, u8 localId, s16 x, s16 y);
void InitObjectEventPalettes(u8 mode);
bool8 ObjectEventIsMovementOverridden(struct ObjectEvent *objectEvent);
u8 ObjectEventCheckHeldMovementStatus(struct ObjectEvent *objectEvent);
u8 GetWalkNormalMovementAction(enum Direction direction);
u8 GetWalkFastMovementAction(enum Direction direction);
u8 GetWalkFasterMovementAction(enum Direction direction);
u8 GetWalkSlowerMovementAction(enum Direction direction);
u8 GetTrainerFacingDirectionMovementType(enum Direction direction);
u8 GetFaceDirectionMovementAction(enum Direction direction);
u8 GetFaceDirectionFastMovementAction(enum Direction direction);
void CameraObjectSetFollowedObjectId(u8 objectId);
void UnfreezeObjectEvents(void);
u8 GetSlideMovementAction(enum Direction direction);
u8 GetJumpInPlaceMovementAction(enum Direction direction);
u8 GetJumpMovementAction(enum Direction direction);
u8 GetJump2MovementAction(enum Direction direction);
bool8 UpdateWalkSlowerAnim(struct Sprite *sprite);
void SetJumpSpriteData(struct Sprite *sprite, enum Direction direction, u8 distance, enum JumpType jumpType);
u8 DoJumpSpriteMovement(struct Sprite *);
u8 DoJumpSpecialSpriteMovement(struct Sprite *);
void TurnVirtualObject(u8 virtualObjId, enum Direction direction);
const u8 *GetObjectEventScriptPointerByObjectEventId(u8 objectEventId);
u8 GetFirstInactiveObjectEventId(void);
u8 GetObjectEventIdByLocalId(u8);
bool32 IsFollowerVisible(void);
u8 GetCollisionFlagsAtCoords(struct ObjectEvent * objectEvent, s16 x, s16 y, enum Direction direction);
void OverrideTemplateCoordsForObjectEvent(const struct ObjectEvent *objectEvent);
void CancelPlayerForcedMovement(void);
void FreezeObjectEvents(void);
bool8 FreezeObjectEvent(struct ObjectEvent *);
void UnfreezeObjectEvent(struct ObjectEvent *);
void FreezeObjectEventsExceptOne(u8 objEventId);
void FreezeObjectEventsExceptTwo(u8 objectEventId1, u8 objectEventId2);
void SetVirtualObjectGraphics(u8 virtualObjId, u16 graphicsId);
void SetVirtualObjectInvisibility(u8 virtualObjId, bool32 invisible);
bool32 IsVirtualObjectInvisible(u8 virtualObjId);
void SetVirtualObjectSpriteAnim(u8 virtualObjId, u8 animNo);
bool32 IsVirtualObjectAnimating(u8 virtualObjId);
u8 GetJumpSpecialMovementAction(enum Direction direction);
void EnableObjectGroundEffectsByXY(s16 x, s16 y);
void MoveObjectEventToMapCoords(struct ObjectEvent *objectEvent, s16 x, s16 y);
u8 CreateCopySpriteAt(struct Sprite *sprite, s16 x, s16 y, u8 subpriority);
bool8 IsElevationMismatchAt(u8 elevation, s16 x, s16 y);
u16 GetObjectPaletteTag(u8 paletteIndex);
void SetSpritePosToMapCoords(s16 x, s16 y, s16 *x2, s16 *y2);
void UpdateObjectEventSpriteInvisibility(struct Sprite *sprite, bool8 invisible);
u8 ElevationToPriority(u8 elevation);
void ObjectEventUpdateElevation(struct ObjectEvent *objEvent, struct Sprite *sprite);
void SetObjectSubpriorityByElevation(u8 elevation, struct Sprite *sprite, u8 offset);
void CopyObjectGraphicsInfoToSpriteTemplate(u16 graphicsId, void (*callback)(struct Sprite *), struct SpriteTemplate *spriteTemplate, const struct SubspriteTable **subspriteTables);
u8 AddCameraObject(u8 trackedSpriteId);
void UpdateObjectEventsForCameraUpdate(s16 x, s16 y);
void SpriteCB_VirtualObject(struct Sprite *);
void SetMovementDelay(struct Sprite *, s16);
bool8 WaitForMovementDelay(struct Sprite *);
void SetSpriteDataForNormalStep(struct Sprite *sprite, enum Direction direction, enum MoveSpeed speed);
bool8 NpcTakeStep(struct Sprite *sprite);
void SetWalkSlowerSpriteData(struct Sprite *sprite, enum Direction direction);
void SetWalkSlowestSpriteData(struct Sprite *sprite, enum Direction direction);
bool8 UpdateWalkSlowestAnim(struct Sprite *sprite);
void SetWalkSlowSpriteData(struct Sprite *sprite, enum Direction direction);
bool8 UpdateWalkSlowAnim(struct Sprite *sprite);
void DoShadowFieldEffect(struct ObjectEvent *);
void SetRunSlowSpriteData(struct Sprite *sprite, enum Direction direction);
bool8 UpdateRunSlowAnim(struct Sprite *sprite);
void SetAndStartSpriteAnim(struct Sprite *, u8, u8);
bool8 SpriteAnimEnded(struct Sprite *);
u8 ObjectEventGetHeldMovementActionId(struct ObjectEvent *objectEvent);
u8 GetMoveDirectionAnimNum(enum Direction direction);
u8 CopySprite(struct Sprite *sprite, s16 x, s16 y, u8 subpriority);
void FieldEffectFreeTilesIfUnused(u16 tileStart);
enum Species GetOverworldWeatherSpecies(enum Species species);
const struct ObjectEventGraphicsInfo *SpeciesToGraphicsInfo(enum Species species, bool32 shiny, bool32 female);

// Exported data declarations
extern const struct OamData gObjectEventBaseOam_32x32;
extern const struct OamData gObjectEventBaseOam_64x64;
extern const struct SubspriteTable sOamTables_32x32[];
extern const struct SubspriteTable sOamTables_64x64[];
extern const union AnimCmd *const sAnimTable_Following[];
extern const union AnimCmd *const sAnimTable_Following_Asym[];
extern const u16 gFieldEffectObjectPic_CutGrass[];
extern const u16 gFieldEffectPal_CutGrass[];
extern const u8 gReflectionEffectPaletteMap[];
u8 GetLedgeJumpDirection(s16 x, s16 y, enum Direction direction);
u8 GetGlideMovementAction(enum Direction direction);
u8 GetRideWaterCurrentMovementAction(enum Direction direction);
u8 GetPlayerRunMovementAction(enum Direction direction);
u8 GetPlayerRunSlowMovementAction(enum Direction direction);
u8 GetWalkSlowMovementAction(enum Direction direction);
u8 GetSpinMovementAction(enum Direction direction);
u8 GetAcroWheelieFaceDirectionMovementAction(enum Direction direction);
u8 GetAcroPopWheelieFaceDirectionMovementAction(enum Direction direction);
u8 GetAcroEndWheelieFaceDirectionMovementAction(enum Direction direction);
u8 GetAcroWheelieHopFaceDirectionMovementAction(enum Direction direction);
u8 GetAcroWheelieHopMovementAction(enum Direction direction);
u8 GetAcroPopWheelieMoveMovementAction(enum Direction direction);
u8 GetAcroWheelieJumpMovementAction(enum Direction direction);
u8 GetJumpInPlaceTurnAroundMovementAction(enum Direction direction);
u8 GetAcroWheelieInPlaceMovementAction(enum Direction direction);
u8 GetAcroPopWheelieMoveMovementAction(enum Direction direction);
u8 GetAcroWheelieMoveMovementAction(enum Direction direction);
u8 GetJumpSpecialWithEffectMovementAction(enum Direction direction);
u8 GetFishingBiteDirectionAnimNum(enum Direction direction);
void TrySpawnObjectEvents(s16 cameraX, s16 cameraY);
void ResetObjectEvents(void);
void ClearObjectEventMovement(struct ObjectEvent *, struct Sprite *);

u8 MovementType_FollowPlayer_Shadow(struct ObjectEvent *, struct Sprite *);
u8 MovementType_FollowPlayer_Active(struct ObjectEvent *, struct Sprite *);
u8 MovementType_FollowPlayer_Moving(struct ObjectEvent *, struct Sprite *);
void StartSpriteAnimInDirection(struct ObjectEvent *objectEvent, struct Sprite *sprite, enum Direction direction, u8 animNum);

bool8 FollowablePlayerMovement_Idle(struct ObjectEvent *, struct Sprite *, u8, bool8(u8));
bool8 FollowablePlayerMovement_Step(struct ObjectEvent *, struct Sprite *, u8, bool8(u8));
bool8 FollowablePlayerMovement_GoSpeed1(struct ObjectEvent *, struct Sprite *, u8, bool8(u8));
bool8 FollowablePlayerMovement_GoSpeed2(struct ObjectEvent *, struct Sprite *, u8, bool8(u8));
bool8 FollowablePlayerMovement_Slide(struct ObjectEvent *, struct Sprite *, u8, bool8(u8));
bool8 FollowablePlayerMovement_JumpInPlace(struct ObjectEvent *, struct Sprite *, u8, bool8(u8));
bool8 FollowablePlayerMovement_GoSpeed4(struct ObjectEvent *, struct Sprite *, u8, bool8(u8));

u8 GetObjectEventBerryTreeId(u8 objectEventId);
void SetBerryTreeJustPicked(u8 mapId, u8 mapNumber, u8 mapGroup);
bool8 IsBerryTreeSparkling(u8 localId, u8 mapNum, u8 mapGroup);
const struct ObjectEventTemplate *GetObjectEventTemplateByLocalIdAndMap(u8 localId, u8 mapNum, u8 mapGroup);
u16 GetObjectEventFlagIdByLocalIdAndMap(u8 localId, u8 mapNum, u8 mapGroup);
u8 TrySpawnObjectEventTemplate(const struct ObjectEventTemplate *objectEventTemplate, u8 mapNum, u8 mapGroup, s16 cameraX, s16 cameraY);
enum Collision GetSidewaysStairsCollision(struct ObjectEvent *objectEvent, enum Direction dir, u8 currentBehavior, u8 nextBehavior, enum Collision collision);
u32 GetObjectObjectCollidesWith(struct ObjectEvent *objectEvent, s16 x, s16 y, bool32 addCoords);

void MovementType_None(struct Sprite *sprite);

#endif // GUARD_EVENT_OBJECT_MOVEMENT_H
