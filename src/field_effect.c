#include "global.h"
#include "battle_transition.h"
#include "bg.h"
#include "data.h"
#include "decompress.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_camera.h"
#include "field_control_avatar.h"
#include "field_effect_helpers.h"
#include "field_effect.h"
#include "field_fadetransition.h"
#include "field_player_avatar.h"
#include "field_weather.h"
#include "fieldmap.h"
#include "follower_npc.h"
#include "gpu_regs.h"
#include "help_system.h"
#include "malloc.h"
#include "menu.h"
#include "metatile_behavior.h"
#include "oras_dowse.h"
#include "overworld.h"
#include "palette.h"
#include "party_menu.h"
#include "pokemon_storage_system.h"
#include "quest_log_player.h"
#include "quest_log.h"
#include "rtc.h"
#include "script.h"
#include "sound.h"
#include "special_field_anim.h"
#include "task.h"
#include "trainer_pokemon_sprites.h"
#include "trainer_see.h"
#include "trainer.h"
#include "trig.h"
#include "util.h"
#include "constants/event_object_movement.h"
#include "constants/metatile_behaviors.h"
#include "constants/songs.h"
#include "constants/sound.h"

#define subsprite_table(ptr) {.subsprites = ptr, .subspriteCount = (sizeof ptr) / (sizeof(struct Subsprite))}
#define MAX_ACTIVE_FLD_EFFECTS 32

EWRAM_DATA u32 gFieldEffectArguments[8] = {0};

static enum FieldEffect sFieldEffectActiveList[MAX_ACTIVE_FLD_EFFECTS];

static void FieldEffectActiveListAdd(enum FieldEffect fldeff);
static u32 FldEff_Nop(void);

static void Task_PokecenterHeal(u8 taskId);
static void PokecenterHealEffect_Init(struct Task *task);
static void PokecenterHealEffect_WaitForBallPlacement(struct Task *task);
static void PokecenterHealEffect_WaitForBallFlashing(struct Task *task);
static void PokecenterHealEffect_WaitForSoundAndEnd(struct Task *task);

static void Task_HallOfFameRecord(u8 taskId);
static void HallOfFameRecordEffect_Init(struct Task *task);
static void HallOfFameRecordEffect_WaitForBallPlacement(struct Task *task);
static void HallOfFameRecordEffect_WaitForBallFlashing(struct Task *task);
static void HallOfFameRecordEffect_WaitForSoundAndEnd(struct Task *task);

static u8 CreateGlowingPokeballsEffect(s16 duration, s16 x, s16 y, bool16 fanfare);
static u8 CreatePokecenterMonitorSprite(s32 x, s32 y);
static void CreateHofMonitorSprite(s32 x, s32 y);
static void PokeballGlowEffect_Dummy(struct Sprite *sprite);
static void PokeballGlowEffect_FlashFirstThree(struct Sprite *sprite);
static void PokeballGlowEffect_FlashLast(struct Sprite *sprite);
static void PokeballGlowEffect_Idle(struct Sprite *sprite);
static void PokeballGlowEffect_PlaceBalls(struct Sprite *sprite);
static void PokeballGlowEffect_TryPlaySe(struct Sprite *sprite);
static void PokeballGlowEffect_WaitAfterFlash(struct Sprite *sprite);
static void PokeballGlowEffect_WaitForSound(struct Sprite *sprite);
static void SpriteCB_HallOfFameMonitor(struct Sprite *sprite);
static void SpriteCB_PokeballGlow(struct Sprite *sprite);
static void SpriteCB_PokeballGlowEffect(struct Sprite *sprite);
static void SpriteCB_PokecenterMonitor(struct Sprite *sprite);

static void Task_UseFly(u8 taskId);
static void FieldCallback_FlyIntoMap(void);
static void Task_FlyIntoMap(u8 taskId);

// Fall warp
static void Task_FallWarpFieldEffect(u8 taskId);
static bool32 FallWarpEffect_Init(struct Task *task);
static bool32 FallWarpEffect_WaitWeather(struct Task *task);
static bool32 FallWarpEffect_StartFall(struct Task *task);
static bool32 FallWarpEffect_Fall(struct Task *task);
static bool32 FallWarpEffect_Land(struct Task *task);
static bool32 FallWarpEffect_CameraShake(struct Task *task);
static bool32 FallWarpEffect_End(struct Task *task);

// Escalator warp out
static void Task_EscalatorWarpOut(u8 taskId);
static bool32 EscalatorWarpOut_Init(struct Task *task);
static bool32 EscalatorWarpOut_WaitForPlayer(struct Task *task);
static bool32 EscalatorWarpOut_Up_Ride(struct Task *task);
static bool32 EscalatorWarpOut_Up_End(struct Task *task);
static bool32 EscalatorWarpOut_Down_Ride(struct Task *task);
static bool32 EscalatorWarpOut_Down_End(struct Task *task);
static void RideUpEscalatorOut(struct Task *task);
static void RideDownEscalatorOut(struct Task *task);
static void FadeOutAtEndOfEscalator(void);
static void WarpAtEndOfEscalator(void);
static void FieldCallback_EscalatorWarpIn(void);

// Escalator Warp in
static void Task_EscalatorWarpIn(u8 taskId);
static bool32 EscalatorWarpIn_Init(struct Task *task);
static bool32 EscalatorWarpIn_Down_Init(struct Task *task);
static bool32 EscalatorWarpIn_Down_Ride(struct Task *task);
static bool32 EscalatorWarpIn_Up_Init(struct Task *task);
static bool32 EscalatorWarpIn_Up_Ride(struct Task *task);
static bool32 EscalatorWarpIn_WaitForMovement(struct Task *task);
static bool32 EscalatorWarpIn_End(struct Task *task);

// Waterfall
static void Task_UseWaterfall(u8 taskId);
static bool32 WaterfallFieldEffect_Init(struct Task *task, struct ObjectEvent * playerObj);
static bool32 WaterfallFieldEffect_ShowMon(struct Task *task, struct ObjectEvent * playerObj);
static bool32 WaterfallFieldEffect_WaitForShowMon(struct Task *task, struct ObjectEvent * playerObj);
static bool32 WaterfallFieldEffect_RideUp(struct Task *task, struct ObjectEvent * playerObj);
static bool32 WaterfallFieldEffect_ContinueRideOrEnd(struct Task *task, struct ObjectEvent * playerObj);

// Dive
static void Task_UseDive(u8 taskId);
static bool32 DiveFieldEffect_Init(struct Task *task);
static bool32 DiveFieldEffect_ShowMon(struct Task *task);
static bool32 DiveFieldEffect_TryWarp(struct Task *task);

// Lavaridge 1BF warp
static void Task_LavaridgeGymB1FWarp(u8 taskId);
static bool32 LavaridgeGymB1FWarpEffect_Init(struct Task *task, struct ObjectEvent * objectEvent, struct Sprite *sprite);
static bool32 LavaridgeGymB1FWarpEffect_CameraShake(struct Task *task, struct ObjectEvent * objectEvent, struct Sprite *sprite);
static bool32 LavaridgeGymB1FWarpEffect_Launch(struct Task *task, struct ObjectEvent * objectEvent, struct Sprite *sprite);
static bool32 LavaridgeGymB1FWarpEffect_Rise(struct Task *task, struct ObjectEvent * objectEvent, struct Sprite *sprite);
static bool32 LavaridgeGymB1FWarpEffect_FadeOut(struct Task *task, struct ObjectEvent * objectEvent, struct Sprite *sprite);
static bool32 LavaridgeGymB1FWarpEffect_Warp(struct Task *task, struct ObjectEvent * objectEvent, struct Sprite *sprite);

// Lavaridge 1BF warp exit
static void FieldCB_LavaridgeGymB1FWarpExit(void);
static void Task_LavaridgeGymB1FWarpExit(u8 taskId);
static bool32 LavaridgeGymB1FWarpExitEffect_Init(struct Task *task, struct ObjectEvent * objectEvent, struct Sprite *sprite);
static bool32 LavaridgeGymB1FWarpExitEffect_StartPopOut(struct Task *task, struct ObjectEvent * objectEvent, struct Sprite *sprite);
static bool32 LavaridgeGymB1FWarpExitEffect_PopOut(struct Task *task, struct ObjectEvent * objectEvent, struct Sprite *sprite);
static bool32 LavaridgeGymB1FWarpExitEffect_End(struct Task *task, struct ObjectEvent * objectEvent, struct Sprite *sprite);

// Lavaridge 1F warp
static void Task_LavaridgeGym1FWarp(u8 taskId);
static bool32 LavaridgeGym1FWarpEffect_Init(struct Task *task, struct ObjectEvent * objectEvent, struct Sprite *sprite);
static bool32 LavaridgeGym1FWarpEffect_AshPuff(struct Task *task, struct ObjectEvent * objectEvent, struct Sprite *sprite);
static bool32 LavaridgeGym1FWarpEffect_Disappear(struct Task *task, struct ObjectEvent * objectEvent, struct Sprite *sprite);
static bool32 LavaridgeGym1FWarpEffect_FadeOut(struct Task *task, struct ObjectEvent * objectEvent, struct Sprite *sprite);
static bool32 LavaridgeGym1FWarpEffect_Warp(struct Task *task, struct ObjectEvent * objectEvent, struct Sprite *sprite);

// Escape Rope warp
static void Task_EscapeRopeWarpOut(u8 taskId);
static void EscapeRopeWarpOutEffect_Init(struct Task *task);
static void EscapeRopeWarpOutEffect_HideFollowerNPC(struct Task *);
static void EscapeRopeWarpOutEffect_Spin(struct Task *task);
static u8 SpinObjectEvent(struct ObjectEvent *playerObj, s16 *timer, s16 *numTurns);
static bool32 WarpOutObjectEventUpwards(struct ObjectEvent *playerObj, s16 *movingState, s16 *offsetY);
static void FieldCallback_EscapeRopeExit(void);
static void Task_EscapeRopeWarpIn(u8 taskId);
static void EscapeRopeWarpInEffect_Init(struct Task *task);
static void EscapeRopeWarpInEffect_Spin(struct Task *task);

// Teleport warp out
static void Task_DoTeleportFieldEffect(u8 taskId);
static void TeleportWarpOutFieldEffect_Init(struct Task *task);
static void TeleportWarpOutFieldEffect_SpinGround(struct Task *task);
static void TeleportWarpOutFieldEffect_SpinExit(struct Task *task);
static void TeleportWarpOutFieldEffect_End(struct Task *task);

// Teleport warp in
static void FieldCallback_TeleportWarpIn(void);
static void Task_TeleportWarpIn(u8 taskId);
static void TeleportWarpInFieldEffect_Init(struct Task *task);
static void TeleportWarpInFieldEffect_SpinEnter(struct Task *task);
static void TeleportWarpInFieldEffect_SpinGround(struct Task *task);

// Show Mon Outdoors
static void Task_FieldMoveShowMonOutdoors(u8 taskId);
static void FieldMoveShowMonOutdoorsEffect_Init(struct Task *task);
static void FieldMoveShowMonOutdoorsEffect_LoadGfx(struct Task *task);
static void FieldMoveShowMonOutdoorsEffect_CreateBanner(struct Task *task);
static void FieldMoveShowMonOutdoorsEffect_WaitForMon(struct Task *task);
static void FieldMoveShowMonOutdoorsEffect_ShrinkBanner(struct Task *task);
static void FieldMoveShowMonOutdoorsEffect_RestoreBg(struct Task *task);
static void FieldMoveShowMonOutdoorsEffect_End(struct Task *task);
static void VBlankCB_ShowMonEffect_Outdoors(void);
static void LoadFieldMoveOutdoorStreaksTilemap(u16 screenbase);

// Show Mon Indoors
static void Task_FieldMoveShowMonIndoors(u8 taskId);
static void FieldMoveShowMonIndoorsEffect_Init(struct Task *task);
static void FieldMoveShowMonIndoorsEffect_LoadGfx(struct Task *task);
static void FieldMoveShowMonIndoorsEffect_SlideBannerOn(struct Task *task);
static void FieldMoveShowMonIndoorsEffect_WaitForMon(struct Task *task);
static void FieldMoveShowMonIndoorsEffect_RestoreBg(struct Task *task);
static void FieldMoveShowMonIndoorsEffect_SlideBannerOff(struct Task *task);
static void FieldMoveShowMonIndoorsEffect_End(struct Task *task);
static void VBlankCB_ShowMonEffect_Indoors(void);
static void AnimateIndoorShowMonBg(struct Task *task);
static bool8 SlideIndoorBannerOnscreen(struct Task *task);
static bool8 SlideIndoorBannerOffscreen(struct Task *task);
static u8 InitFieldMoveMonSprite(u32 species, bool32 isShiny, u32 personality);
static void SpriteCB_FieldMoveMonSlideOnscreen(struct Sprite *sprite);
static void SpriteCB_FieldMoveMonWaitAfterCry(struct Sprite *sprite);
static void SpriteCB_FieldMoveMonSlideOffscreen(struct Sprite *sprite);

// Surf
static void Task_SurfFieldEffect(u8 taskId);
static void SurfFieldEffect_Init(struct Task *task);
static void SurfFieldEffect_FieldMovePose(struct Task *task);
static void SurfFieldEffect_ShowMon(struct Task *task);
static void SurfFieldEffect_JumpOnSurfBlob(struct Task *task);
static void SurfFieldEffect_End(struct Task *task);

// Vs Seeker
static void Task_FldEffUseVsSeeker(u8 taskId);
static void UseVsSeeker_StopPlayerMovement(struct Task *task);
static void UseVsSeeker_DoPlayerAnimation(struct Task *task);
static void UseVsSeeker_ResetPlayerGraphics(struct Task *task);
static void UseVsSeeker_End(struct Task *task);

// Fly out
static void Task_FlyOut(u8 taskId);
static void FlyOutFieldEffect_FieldMovePose(struct Task *task);
static void FlyOutFieldEffect_ShowMon(struct Task *task);
static void FlyOutFieldEffect_BirdLeaveBall(struct Task *task);
static void FlyOutFieldEffect_WaitBirdLeave(struct Task *task);
static void FlyOutFieldEffect_BirdSwoopDown(struct Task *task);
static void FlyOutFieldEffect_JumpOnBird(struct Task *task);
static void FlyOutFieldEffect_FlyOffWithBird(struct Task *task);
static void FlyOutFieldEffect_WaitFlyOff(struct Task *task);
static void FlyOutFieldEffect_End(struct Task *task);
static void SpriteCB_NPCFlyOut(struct Sprite *sprite);

// Fly in
static void Task_FlyIn(u8 taskId);
static void FlyInFieldEffect_BirdSwoopDown(struct Task *task);
static void FlyInFieldEffect_FlyInWithBird(struct Task *task);
static void FlyInFieldEffect_JumpOffBird(struct Task *task);
static void FlyInFieldEffect_FieldMovePose(struct Task *task);
static void FlyInFieldEffect_BirdReturnToBall(struct Task *task);
static void FlyInFieldEffect_WaitBirdReturn(struct Task *task);
static void FlyInFieldEffect_End(struct Task *task);
static void TryChangeBirdSprite(struct Sprite *sprite);

// Fly
static bool8 GetFlyBirdAnimCompleted(u8 flyBlobSpriteId);
static u8 CreateFlyBirdSprite(void);
static void DoBirdSpriteWithPlayerAffineAnim(struct Sprite *sprite, u8 affineAnimId);
static void SetFlyBirdPlayerSpriteId(u8 flyBlobSpriteId, u8 playerSpriteId);
static void SpriteCB_FlyBirdLeaveBall(struct Sprite *sprite);
static void SpriteCB_FlyBirdSwoopDown(struct Sprite *sprite);
static void SpriteCB_FlyBirdWithPlayer(struct Sprite *sprite);
static void StartFlyBirdSwoopDown(u8 flyBlobSpriteId);

// Move Deoxys rock
static void Task_MoveDeoxysRock(u8 taskId);

// Destroy Deoxys rock
static void Task_DestroyDeoxysRock(u8 taskId);
static void DestroyDeoxysRockEffect_CameraShake(u8 taskId);
static void DestroyDeoxysRockEffect_RockFragments(u8 taskId);
static void DestroyDeoxysRockEffect_WaitAndEnd(u8 taskId);
static void CreateDeoxysRockFragments(struct Sprite *sprite);
static void SpriteCB_DeoxysRockFragment(struct Sprite *sprite);
static void Task_DeoxysRockCameraShake(u8 taskId);
static void StartEndingDeoxysRockCameraShake(u8 taskId);

static const u16 sPokeballGlow_Gfx[] = INCBIN_U16("graphics/field_effects/pics/pokeball_glow.4bpp");
static const u16 sPokeballGlow_Pal[] = INCBIN_U16("graphics/field_effects/pics/pokeball_glow.gbapal");
static const u16 sPokecenterMonitor_Gfx[] = INCBIN_U16("graphics/field_effects/pics/pokemoncenter_monitor.4bpp");
static const u16 sHofMonitor_Pal[] = INCBIN_U16("graphics/field_effects/pics/hof_monitor.gbapal");
static const u16 sHofMonitor_Gfx[] = INCBIN_U16("graphics/field_effects/pics/hof_monitor.4bpp");

static const u16 sFieldMoveStreaksOutdoors_Gfx[] = INCBIN_U16("graphics/field_effects/pics/field_move_streaks_outdoors.4bpp");
static const u16 sFieldMoveStreaksOutdoors_Pal[] = INCBIN_U16("graphics/field_effects/pics/field_move_streaks_outdoors.gbapal");
static const u16 sFieldMoveStreaksOutdoors_Tilemap[] = INCBIN_U16("graphics/field_effects/pics/field_move_streaks_outdoors.bin");

static const u16 sFieldMoveStreaksIndoors_Gfx[] = INCBIN_U16("graphics/field_effects/pics/field_move_streaks_indoors.4bpp");
static const u16 sFieldMoveStreaksIndoors_Pal[] = INCBIN_U16("graphics/field_effects/pics/field_move_streaks_indoors.gbapal");
static const u16 sFieldMoveStreaksIndoors_Tilemap[] = INCBIN_U16("graphics/field_effects/pics/field_move_streaks_indoors.bin");

static const u16 sRockFragment_TopLeft[] = INCBIN_U16("graphics/field_effects/pics/deoxys_rock_fragment_top_left.4bpp");
static const u16 sRockFragment_TopRight[] = INCBIN_U16("graphics/field_effects/pics/deoxys_rock_fragment_top_right.4bpp");
static const u16 sRockFragment_BottomLeft[] = INCBIN_U16("graphics/field_effects/pics/deoxys_rock_fragment_bottom_left.4bpp");
static const u16 sRockFragment_BottomRight[] = INCBIN_U16("graphics/field_effects/pics/deoxys_rock_fragment_bottom_right.4bpp");

static const u32 (*const sFieldEffectFuncs[FLDEFF_COUNT]) (void) =
{
    [FLDEFF_NONE]                         = FldEff_Nop,
    [FLDEFF_EXCLAMATION_MARK_ICON]        = FldEff_ExclamationMarkIcon,
    [FLDEFF_USE_CUT_ON_GRASS]             = FldEff_UseCutOnGrass,
    [FLDEFF_USE_CUT_ON_TREE]              = FldEff_UseCutOnTree,
    [FLDEFF_SHADOW]                       = FldEff_Shadow,
    [FLDEFF_TALL_GRASS]                   = FldEff_TallGrass,
    [FLDEFF_RIPPLE]                       = FldEff_Ripple,
    [FLDEFF_FIELD_MOVE_SHOW_MON]          = FldEff_FieldMoveShowMon,
    [FLDEFF_ASH]                          = FldEff_Ash,
    [FLDEFF_SURF_BLOB]                    = FldEff_SurfBlob,
    [FLDEFF_USE_SURF]                     = FldEff_UseSurf,
    [FLDEFF_DUST]                         = FldEff_Dust,
    [FLDEFF_USE_SECRET_POWER_CAVE]        = FldEff_Nop,
    [FLDEFF_JUMP_TALL_GRASS]              = FldEff_JumpTallGrass,
    [FLDEFF_SAND_FOOTPRINTS]              = FldEff_SandFootprints,
    [FLDEFF_JUMP_BIG_SPLASH]              = FldEff_JumpBigSplash,
    [FLDEFF_SPLASH]                       = FldEff_Splash,
    [FLDEFF_JUMP_SMALL_SPLASH]            = FldEff_JumpSmallSplash,
    [FLDEFF_LONG_GRASS]                   = FldEff_LongGrass,
    [FLDEFF_JUMP_LONG_GRASS]              = FldEff_JumpLongGrass,
    [FLDEFF_SHAKING_GRASS]                = FldEff_ShakingGrass,
    [FLDEFF_SHAKING_LONG_GRASS]           = FldEff_ShakingGrass2,
    [FLDEFF_SAND_HOLE]                    = FldEff_UnusedSand,
    [FLDEFF_UNUSED_WATER_SURFACING]       = FldEff_UnusedWaterSurfacing,
    [FLDEFF_BERRY_TREE_GROWTH_SPARKLE]    = FldEff_BerryTreeGrowthSparkle,
    [FLDEFF_DEEP_SAND_FOOTPRINTS]         = FldEff_DeepSandFootprints,
    [FLDEFF_POKECENTER_HEAL]              = FldEff_PokecenterHeal,
    [FLDEFF_USE_SECRET_POWER_TREE]        = FldEff_Nop,
    [FLDEFF_USE_SECRET_POWER_SHRUB]       = FldEff_Nop,
    [FLDEFF_TREE_DISGUISE]                = FldEff_TreeDisguise,
    [FLDEFF_MOUNTAIN_DISGUISE]            = FldEff_MountainDisguise,
    [FLDEFF_NPCFLY_OUT]                   = FldEff_NPCFlyOut,
    [FLDEFF_USE_FLY]                      = FldEff_UseFly,
    [FLDEFF_FLY_IN]                       = FldEff_FlyIn,
    [FLDEFF_QUESTION_MARK_ICON_AND_EMOTE] = FldEff_QuestionMarkIcon,
    [FLDEFF_FEET_IN_FLOWING_WATER]        = FldEff_FeetInFlowingWater,
    [FLDEFF_BIKE_TIRE_TRACKS]             = FldEff_BikeTireTracks,
    [FLDEFF_SAND_DISGUISE]                = FldEff_SandDisguise,
    [FLDEFF_USE_ROCK_SMASH]               = FldEff_UseRockSmash,
    [FLDEFF_USE_DIG]                      = FldEff_UseDig,
    [FLDEFF_SAND_PILE]                    = FldEff_SandPile,
    [FLDEFF_USE_STRENGTH]                 = FldEff_UseStrength,
    [FLDEFF_SHORT_GRASS]                  = FldEff_ShortGrass,
    [FLDEFF_HOT_SPRINGS_WATER]            = FldEff_HotSpringsWater,
    [FLDEFF_USE_WATERFALL]                = FldEff_UseWaterfall,
    [FLDEFF_USE_DIVE]                     = FldEff_UseDive,
    [FLDEFF_POKEBALL_TRAIL]               = FldEff_PokeballTrail,
    [FLDEFF_X_ICON]                       = FldEff_XIcon,
    [FLDEFF_NOP_47]                       = FldEff_Nop,
    [FLDEFF_NOP_48]                       = FldEff_Nop,
    [FLDEFF_POP_OUT_OF_ASH]               = FldEff_PopOutOfAsh,
    [FLDEFF_LAVARIDGE_GYM_WARP]           = FldEff_LavaridgeGymWarp,
    [FLDEFF_SWEET_SCENT]                  = FldEff_SweetScent,
    [FLDEFF_SAND_PILLAR]                  = FldEff_Nop,
    [FLDEFF_BUBBLES]                      = FldEff_Bubbles,
    [FLDEFF_SPARKLE]                      = FldEff_Sparkle,
    [FLDEFF_SECRET_POWER_CAVE]            = FldEff_Nop,
    [FLDEFF_SECRET_POWER_TREE]            = FldEff_Nop,
    [FLDEFF_SECRET_POWER_SHRUB]           = FldEff_Nop,
    [FLDEFF_CUT_GRASS]                    = FldEff_CutGrass,
    [FLDEFF_FIELD_MOVE_SHOW_MON_INIT]     = FldEff_FieldMoveShowMonInit,
    [FLDEFF_USE_FLY_ANCIENT_TOMB]         = FldEff_Nop,
    [FLDEFF_PCTURN_ON]                    = FldEff_Nop,
    [FLDEFF_HALL_OF_FAME_RECORD]          = FldEff_HallOfFameRecord,
    [FLDEFF_USE_TELEPORT]                 = FldEff_UseTeleport,
    [FLDEFF_SMILEY_FACE_ICON]             = FldEff_SmileyFaceIcon,
    [FLDEFF_USE_VS_SEEKER]                = FldEff_UseVsSeeker,
    [FLDEFF_DOUBLE_EXCL_MARK_ICON]        = FldEff_DoubleExclMarkIcon,
    [FLDEFF_MOVE_DEOXYS_ROCK]             = FldEff_MoveDeoxysRock,
    [FLDEFF_DESTROY_DEOXYS_ROCK]          = FldEff_DestroyDeoxysRock,
    [FLDEFF_PHOTO_FLASH]                  = FldEff_PhotoFlash,
    [FLDEFF_TRACKS_SLITHER]               = FldEff_TracksSlither,
    [FLDEFF_TRACKS_BUG]                   = FldEff_TracksBug,
    [FLDEFF_TRACKS_SPOT]                  = FldEff_TracksSpot,
    [FLDEFF_SNOW_FOOTPRINTS]              = FldEff_SnowFootprints,
    [FLDEFF_SNOW_BIKE_TIRE_TRACKS]        = FldEff_SnowBikeTireTracks,
    [FLDEFF_SNOW_TRACKS_SLITHER]          = FldEff_SnowTracksSlither,
    [FLDEFF_SNOW_TRACKS_BUG]              = FldEff_SnowTracksBug,
    [FLDEFF_SNOW_TRACKS_SPOT]             = FldEff_SnowTracksSpot,
    [FLDEFF_CAVE_DUST]                    = FldEff_CaveDust,
    [FLDEFF_USE_ROCK_CLIMB]               = FldEff_UseRockClimb,
    [FLDEFF_ROCK_CLIMB_DUST]              = FldEff_RockClimbDust,
    [FLDEFF_DEFOG]                        = FldEff_Defog,
    [FLDEFF_ORAS_DOWSE]                   = FldEff_ORASDowsing,
};

static const struct OamData sOamData_8x8 =
{
    .y = 0,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(8x8),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(8x8),
    .tileNum = 0x000,
    .priority = 0,
    .paletteNum = 0,
    .affineParam = 0
};

static const struct OamData sOamData_16x16 =
{
    .y = 0,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(16x16),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(16x16),
    .tileNum = 0x000,
    .priority = 0,
    .paletteNum = 0,
    .affineParam = 0
};

const struct SpritePalette gSpritePalette_PokeballGlow =
{
    sPokeballGlow_Pal, FLDEFF_PAL_TAG_POKEBALL_GLOW
};

const struct SpritePalette gSpritePalette_HofMonitor =
{
    sHofMonitor_Pal, FLDEFF_PAL_TAG_HOF_MONITOR
};

static const struct OamData sOamData_32x16 =
{
    .y = 0,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(32x16),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(32x16),
    .tileNum = 0x000,
    .priority = 0,
    .paletteNum = 0,
    .affineParam = 0
};

static const struct SpriteFrameImage sPicTable_PokeballGlow[] =
{
    {sPokeballGlow_Gfx, 0x20}
};

static const struct SpriteFrameImage sPicTable_PokecenterMonitor[] =
{
    {sPokecenterMonitor_Gfx + 0x000, 0x100},
    {sPokecenterMonitor_Gfx + 0x080, 0x100},
    {sPokecenterMonitor_Gfx + 0x100, 0x100},
    {sPokecenterMonitor_Gfx + 0x180, 0x100}
};

static const struct SpriteFrameImage sPicTable_HofMonitor[] =
{
    {sHofMonitor_Gfx + 0x00, 0x80},
    {sHofMonitor_Gfx + 0x40, 0x80},
    {sHofMonitor_Gfx + 0x80, 0x80},
    {sHofMonitor_Gfx + 0xC0, 0x80}
};

static const union AnimCmd sAnim_Static[] = {
    ANIMCMD_FRAME(0, 1),
    ANIMCMD_JUMP(0)
};

static const union AnimCmd sAnim_Flicker[] =
{
    ANIMCMD_FRAME(1, 5),
    ANIMCMD_FRAME(2, 5),
    ANIMCMD_FRAME(3, 7),
    ANIMCMD_FRAME(2, 5),
    ANIMCMD_FRAME(1, 5),
    ANIMCMD_FRAME(0, 5),
    ANIMCMD_LOOP(3),
    ANIMCMD_END
};

static const union AnimCmd *const sAnims_Flicker[] =
{
    sAnim_Static,
    sAnim_Flicker
};

static const union AnimCmd sAnim_HofMonitor[] =
{
    ANIMCMD_FRAME(3, 8),
    ANIMCMD_FRAME(2, 8),
    ANIMCMD_FRAME(1, 8),
    ANIMCMD_FRAME(0, 8),
    ANIMCMD_FRAME(1, 8),
    ANIMCMD_FRAME(2, 8),
    ANIMCMD_LOOP(2),
    ANIMCMD_FRAME(1, 8),
    ANIMCMD_FRAME(0, 8),
    ANIMCMD_END
};

static const union AnimCmd *const sAnims_HofMonitor[] =
{
    sAnim_HofMonitor
};

static const struct SpriteTemplate sSpriteTemplate_PokeballGlow =
{
    .tileTag = TAG_NONE,
    .paletteTag = FLDEFF_PAL_TAG_POKEBALL_GLOW,
    .oam = &sOamData_8x8,
    .anims = sAnims_Flicker,
    .images = sPicTable_PokeballGlow,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_PokeballGlow
};

static const struct SpriteTemplate sSpriteTemplate_PokecenterMonitor =
{
    .tileTag = TAG_NONE,
    .paletteTag = FLDEFF_PAL_TAG_POKEBALL_GLOW,
    .oam = &sOamData_32x16,
    .anims = sAnims_Flicker,
    .images = sPicTable_PokecenterMonitor,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_PokecenterMonitor
};

static const struct SpriteTemplate sSpriteTemplate_HofMonitor =
{
    .tileTag = TAG_NONE,
    .paletteTag = FLDEFF_PAL_TAG_HOF_MONITOR,
    .oam = &sOamData_16x16,
    .anims = sAnims_HofMonitor,
    .images = sPicTable_HofMonitor,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_HallOfFameMonitor
};

static const union AffineAnimCmd sAffineAnim_FlyBirdLeaveBall[] =
{
    AFFINEANIMCMD_FRAME( 8,  8, -30,  0),
    AFFINEANIMCMD_FRAME(28, 28,   0, 30),
    AFFINEANIMCMD_END
};

static const union AffineAnimCmd sAffineAnim_FlyBirdReturnToBall[] =
{
    AFFINEANIMCMD_FRAME(256, 256, 64,  0),
    AFFINEANIMCMD_FRAME(-10, -10,  0, 22),
    AFFINEANIMCMD_END
};

static const union AffineAnimCmd *const sAffineAnims_FlyBirdBall[] =
{
    sAffineAnim_FlyBirdLeaveBall,
    sAffineAnim_FlyBirdReturnToBall
};

static const union AffineAnimCmd sAffineAnim_FlyBirdOutOfMap[] =
{
    AFFINEANIMCMD_FRAME(24, 24, 0, 1),
    AFFINEANIMCMD_JUMP(0)
};

static const union AffineAnimCmd sAffineAnim_FlyBirdIntoMap[] =
{
    AFFINEANIMCMD_FRAME(512, 512, 0, 1),
    AFFINEANIMCMD_FRAME(-16, -16, 0, 1),
    AFFINEANIMCMD_JUMP(1)
};

static const union AffineAnimCmd *const sAffineAnims_FlyBirdWithPlayer[] =
{
    sAffineAnim_FlyBirdOutOfMap,
    sAffineAnim_FlyBirdIntoMap
};

static const struct SpriteFrameImage sImages_DeoxysRockFragment[] =
{
    {sRockFragment_TopLeft, 0x20},
    {sRockFragment_TopRight, 0x20},
    {sRockFragment_BottomLeft, 0x20},
    {sRockFragment_BottomRight, 0x20}
};

static const union AnimCmd sAnim_RockFragment_TopLeft[] =
{
    ANIMCMD_FRAME(0, 0),
    ANIMCMD_END
};

static const union AnimCmd sAnim_RockFragment_TopRight[] =
{
    ANIMCMD_FRAME(1, 0),
    ANIMCMD_END
};

static const union AnimCmd sAnim_RockFragment_BottomLeft[] =
{
    ANIMCMD_FRAME(2, 0),
    ANIMCMD_END
};

static const union AnimCmd sAnim_RockFragment_BottomRight[] =
{
    ANIMCMD_FRAME(3, 0),
    ANIMCMD_END
};

static const union AnimCmd *const sAnims_DeoxysRockFragment[] =
{
    sAnim_RockFragment_TopLeft,
    sAnim_RockFragment_TopRight,
    sAnim_RockFragment_BottomLeft,
    sAnim_RockFragment_BottomRight
};

static const struct SpriteTemplate sSpriteTemplate_DeoxysRockFragment =
{
    .tileTag = TAG_NONE,
    .paletteTag = 4371,
    .oam = &sOamData_8x8,
    .anims = sAnims_DeoxysRockFragment,
    .images = sImages_DeoxysRockFragment,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_DeoxysRockFragment
};

enum PokecenterHealEffectState
{
    POKECENTER_HEAL_EFFECT_INIT,
    POKECENTER_HEAL_EFFECT_WAIT_PLACEMENT,
    POKECENTER_HEAL_EFFECT_WAIT_FLASHING,
    POKECENTER_HEAL_EFFECT_END,
};

static void (*const sPokecenterHealEffectFuncs[])(struct Task *) =
{
    [POKECENTER_HEAL_EFFECT_INIT]           = PokecenterHealEffect_Init,
    [POKECENTER_HEAL_EFFECT_WAIT_PLACEMENT] = PokecenterHealEffect_WaitForBallPlacement,
    [POKECENTER_HEAL_EFFECT_WAIT_FLASHING]  = PokecenterHealEffect_WaitForBallFlashing,
    [POKECENTER_HEAL_EFFECT_END]            = PokecenterHealEffect_WaitForSoundAndEnd
};

enum HofRecordEffectState
{
    HOF_RECORD_EFFECT_INIT,
    HOF_RECORD_EFFECT_WAIT_PLACEMENT,
    HOF_RECORD_EFFECT_WAIT_FLASHING,
    HOF_RECORD_EFFECT_END,
};

static void (*const sHallOfFameRecordEffectFuncs[])(struct Task *) =
{
    [HOF_RECORD_EFFECT_INIT]           = HallOfFameRecordEffect_Init,
    [HOF_RECORD_EFFECT_WAIT_PLACEMENT] = HallOfFameRecordEffect_WaitForBallPlacement,
    [HOF_RECORD_EFFECT_WAIT_FLASHING]  = HallOfFameRecordEffect_WaitForBallFlashing,
    [HOF_RECORD_EFFECT_END]            = HallOfFameRecordEffect_WaitForSoundAndEnd
};

enum PokeballGlowEffectState
{
    POKEBALL_GLOW_EFFECT_PLACE_BALLS,
    POKEBALL_GLOW_EFFECT_TRY_PLAY_SE,
    POKEBALL_GLOW_EFFECT_FLASH_FIRST_THREE,
    POKEBALL_GLOW_EFFECT_FLASH_LAST,
    POKEBALL_GLOW_EFFECT_WAIT_AFTER_FLASH,
    POKEBALL_GLOW_EFFECT_DUMMY,
    POKEBALL_GLOW_EFFECT_WAIT_FOR_SOUND,
    POKEBALL_GLOW_EFFECT_IDLE,
};

static void (*const sPokeballGlowEffectFuncs[])(struct Sprite *) =
{
    [POKEBALL_GLOW_EFFECT_PLACE_BALLS]       = PokeballGlowEffect_PlaceBalls,
    [POKEBALL_GLOW_EFFECT_TRY_PLAY_SE]       = PokeballGlowEffect_TryPlaySe,
    [POKEBALL_GLOW_EFFECT_FLASH_FIRST_THREE] = PokeballGlowEffect_FlashFirstThree,
    [POKEBALL_GLOW_EFFECT_FLASH_LAST]        = PokeballGlowEffect_FlashLast,
    [POKEBALL_GLOW_EFFECT_WAIT_AFTER_FLASH]  = PokeballGlowEffect_WaitAfterFlash,
    [POKEBALL_GLOW_EFFECT_DUMMY]             = PokeballGlowEffect_Dummy,
    [POKEBALL_GLOW_EFFECT_WAIT_FOR_SOUND]    = PokeballGlowEffect_WaitForSound,
    [POKEBALL_GLOW_EFFECT_IDLE]              = PokeballGlowEffect_Idle
};

u32 FieldEffectStart(enum FieldEffect fldeff)
{
    FieldEffectActiveListAdd(fldeff);
    return sFieldEffectFuncs[fldeff]();
}

void ApplyGlobalFieldPaletteTint(u8 paletteIdx)
{
    switch (gGlobalFieldTintMode)
    {
    case 0:
        return;
    case 1:
        TintPalette_GrayScale(&gPlttBufferUnfaded[OBJ_PLTT_ID2(paletteIdx)], 16);
        break;
    case 2:
        TintPalette_SepiaTone(&gPlttBufferUnfaded[OBJ_PLTT_ID2(paletteIdx)], 16);
        break;
    case 3:
        QuestLog_BackUpPalette(OBJ_PLTT_ID2(paletteIdx), 16);
        TintPalette_GrayScale(&gPlttBufferUnfaded[OBJ_PLTT_ID2(paletteIdx)], 16);
        break;
    default:
        return;
    }
    CpuFastCopy(&gPlttBufferUnfaded[OBJ_PLTT_ID2(paletteIdx)], &gPlttBufferFaded[OBJ_PLTT_ID2(paletteIdx)], PLTT_SIZE_4BPP);
}

void FieldEffectScript_LoadFadedPal(const struct SpritePalette *spritePalette)
{
    bool32 isTagNew = IndexOfSpritePaletteTag(spritePalette->tag) == 0xFF;
    u32 paletteSlot = LoadSpritePalette(spritePalette);

    SetPaletteColorMapType(paletteSlot + 16, COLOR_MAP_DARK_CONTRAST);
    if (isTagNew)
        ApplyGlobalFieldPaletteTint(paletteSlot);
    UpdateSpritePaletteWithWeather(paletteSlot, TRUE);
}

void FieldEffectScript_LoadPal(const struct SpritePalette *spritePalette)
{
    u8 idx = IndexOfSpritePaletteTag(spritePalette->tag);
    LoadSpritePalette(spritePalette);
    if (idx != 0xFF)
        ApplyGlobalFieldPaletteTint(IndexOfSpritePaletteTag(spritePalette->tag));
}

void FieldEffectFreeGraphicsResources(struct Sprite *sprite)
{
    u16 tileStart = sprite->sheetTileStart;
    u8 paletteNum = sprite->oam.paletteNum;
    DestroySprite(sprite);
    FieldEffectFreeTilesIfUnused(tileStart);
    FieldEffectFreePaletteIfUnused(paletteNum);
}

void FieldEffectStop(struct Sprite *sprite, enum FieldEffect fldeff)
{
    FieldEffectFreeGraphicsResources(sprite);
    FieldEffectActiveListRemove(fldeff);
}

void FieldEffectFreeTilesIfUnused(u16 tileStart)
{
    u8 i;
    u16 tag = GetSpriteTileTagByTileStart(tileStart);

    if (tag != TAG_NONE)
    {
        for (i = 0; i < MAX_SPRITES; i++)
            if (gSprites[i].inUse && gSprites[i].usingSheet && tileStart == gSprites[i].sheetTileStart)
                return;
        FreeSpriteTilesByTag(tag);
    }
}

void FieldEffectFreePaletteIfUnused(u8 paletteNum)
{
    u8 i;
    u16 tag = GetSpritePaletteTagByPaletteNum(paletteNum);

    if (tag != TAG_NONE)
    {
        for (i = 0; i < MAX_SPRITES; i++)
            if (gSprites[i].inUse && gSprites[i].oam.paletteNum == paletteNum)
                return;
        FreeSpritePaletteByTag(tag);
    }
}

void FieldEffectActiveListClear(void)
{
    u8 i;
    for (i = 0; i < MAX_ACTIVE_FLD_EFFECTS; i++)
    {
        sFieldEffectActiveList[i] = FLDEFF_NONE;
    }
}

static void FieldEffectActiveListAdd(enum FieldEffect fldeff)
{
    u8 i;
    for (i = 0; i < MAX_ACTIVE_FLD_EFFECTS; i++)
    {
        if (sFieldEffectActiveList[i] == FLDEFF_NONE)
        {
            sFieldEffectActiveList[i] = fldeff;
            return;
        }
    }
}

void FieldEffectActiveListRemove(enum FieldEffect fldeff)
{
    u8 i;
    for (i = 0; i < MAX_ACTIVE_FLD_EFFECTS; i++)
    {
        if (sFieldEffectActiveList[i] == fldeff)
        {
            sFieldEffectActiveList[i] = FLDEFF_NONE;
            return;
        }
    }
}

bool8 FieldEffectActiveListContains(enum FieldEffect fldeff)
{
    u8 i;
    for (i = 0; i < MAX_ACTIVE_FLD_EFFECTS; i++)
    {
        if (sFieldEffectActiveList[i] == fldeff)
        {
            return TRUE;
        }
    }
    return FALSE;
}

u8 CreateMonSprite_PicBox(enum Species species, s16 x, s16 y, u8 subpriority)
{
    u16 spriteId = CreateMonFrontPicSprite(species, FALSE, 0x8000, x, y, 0, species);

    PreservePaletteInWeather(IndexOfSpritePaletteTag(species) + 0x10);
    if (spriteId == 0xFFFF)
        return MAX_SPRITES;
    else
        return spriteId;
}

static u8 CreateMonSprite_FieldMove(enum Species species, bool32 isShiny, u32 personality, s16 x, s16 y, u8 subpriority)
{
    u16 spriteId = CreateMonFrontPicSprite(species, isShiny, personality, x, y, 0, species);
    PreservePaletteInWeather(IndexOfSpritePaletteTag(species) + 0x10);
    if (spriteId == 0xFFFF)
        return MAX_SPRITES;
    else
        return spriteId;
}

void FreeResourcesAndDestroySprite(struct Sprite *sprite, u8 spriteId)
{
    u8 paletteNum = sprite->oam.paletteNum;
    ResetPreservedPalettesInWeather();
    if (sprite->oam.affineMode != ST_OAM_AFFINE_OFF)
    {
        FreeOamMatrix(sprite->oam.matrixNum);
    }
    FreeAndDestroyMonPicSpriteNoPalette(spriteId);
    FieldEffectFreePaletteIfUnused(paletteNum); // Clear palette only if unused, in case follower is using it
}

// r, g, b are between 0 and 16
void MultiplyInvertedPaletteRGBComponents(u16 i, u8 r, u8 g, u8 b)
{
    int curRed, curGreen, curBlue;
    u16 color = gPlttBufferUnfaded[i];

    curRed   = (color & RGB_RED);
    curGreen = (color & RGB_GREEN) >>  5;
    curBlue  = (color & RGB_BLUE)  >> 10;

    curRed   += (((0x1F - curRed)   * r) >> 4);
    curGreen += (((0x1F - curGreen) * g) >> 4);
    curBlue  += (((0x1F - curBlue)  * b) >> 4);

    color  = curRed;
    color |= (curGreen <<  5);
    color |= (curBlue  << 10);

    gPlttBufferFaded[i] = color;
}

// Task data for Task_PokecenterHeal and Task_HallOfFameRecord
#define tState              data[0]
#define tNumMons            data[1]
#define tFirstBallX         data[2]
#define tFirstBallY         data[3]
#define tMonitorX           data[4]
#define tMonitorY           data[5]
#define tBallSpriteId       data[6]
#define tMonitorSpriteId    data[7]

// Sprite data for SpriteCB_PokeballGlowEffect
#define sState      data[0]
#define sTimer      data[1]
#define sCounter    data[2]
#define sNumFlashed data[3]
#define sPlayHealSe data[5]
#define sNumMons    data[6]
#define sSpriteId   data[7]

// Sprite data for SpriteCB_PokeballGlow
#define sEffectSpriteId data[0]

// Sprite data for SpriteCB_PokecenterMonitor
#define sStartFlash data[0]

u32 FldEff_PokecenterHeal(void)
{
    u8 nPokemon;
    struct Task *task;

    FieldEffectScript_LoadFadedPal(&gSpritePalette_PokeballGlow);
    FieldEffectScript_LoadFadedPal(&gSpritePalette_GeneralFieldEffect0);
    nPokemon = (OW_IGNORE_EGGS_ON_HEAL <= GEN_3) ? CalculatePlayerPartyCount() : CountPartyNonEggMons();
    task = &gTasks[CreateTask(Task_PokecenterHeal, 0xFF)];
    task->tNumMons = nPokemon;
    task->tFirstBallX = 93;
    task->tFirstBallY = 36;
    task->tMonitorX = 128;
    task->tMonitorY = 24;
    return FALSE;
}

static void Task_PokecenterHeal(u8 taskId)
{
    struct Task *task = &gTasks[taskId];
    sPokecenterHealEffectFuncs[task->tState](task);
}

static void PokecenterHealEffect_Init(struct Task *task)
{
    task->tState = POKECENTER_HEAL_EFFECT_WAIT_PLACEMENT;
    task->tBallSpriteId = CreateGlowingPokeballsEffect(task->tNumMons, task->tFirstBallX, task->tFirstBallY, TRUE);
    task->tMonitorSpriteId = CreatePokecenterMonitorSprite(task->tMonitorX, task->tMonitorY);
}

static void PokecenterHealEffect_WaitForBallPlacement(struct Task *task)
{
    if (gSprites[task->tBallSpriteId].sState >= 2)
    {
        gSprites[task->tMonitorSpriteId].sStartFlash++;
        task->tState = POKECENTER_HEAL_EFFECT_WAIT_FLASHING;
    }
}

static void PokecenterHealEffect_WaitForBallFlashing(struct Task *task)
{
    if (gSprites[task->tBallSpriteId].sState > 4)
        task->tState = POKECENTER_HEAL_EFFECT_END;
}

static void PokecenterHealEffect_WaitForSoundAndEnd(struct Task *task)
{
    if (gSprites[task->tBallSpriteId].sState > 6)
    {
        DestroySprite(&gSprites[task->tBallSpriteId]);
        FieldEffectActiveListRemove(FLDEFF_POKECENTER_HEAL);
        DestroyTask(FindTaskIdByFunc(Task_PokecenterHeal));
    }
}

u32 FldEff_HallOfFameRecord(void)
{
    u8 nPokemon;
    struct Task *task;

    FieldEffectScript_LoadFadedPal(&gSpritePalette_PokeballGlow);
    FieldEffectScript_LoadFadedPal(&gSpritePalette_HofMonitor);
    nPokemon = CalculatePlayerPartyCount();
    task = &gTasks[CreateTask(Task_HallOfFameRecord, 0xFF)];
    task->tNumMons = nPokemon;
    task->tFirstBallX = 117;
    task->tFirstBallY = 60;
    return FALSE;
}

static void Task_HallOfFameRecord(u8 taskId)
{
    struct Task *task;
    task = &gTasks[taskId];
    sHallOfFameRecordEffectFuncs[task->tState](task);
}

static void HallOfFameRecordEffect_Init(struct Task *task)
{
    task->tState = HOF_RECORD_EFFECT_WAIT_PLACEMENT;
    task->tBallSpriteId = CreateGlowingPokeballsEffect(task->tNumMons, task->tFirstBallX, task->tFirstBallY, FALSE);
}

static void HallOfFameRecordEffect_WaitForBallPlacement(struct Task *task)
{
    if (gSprites[task->tBallSpriteId].sState > 1)
    {
        CreateHofMonitorSprite(120, 25);
        task->data[15]++; // unused, leftover from RSE
        task->tState = HOF_RECORD_EFFECT_WAIT_FLASHING;
    }
}

static void HallOfFameRecordEffect_WaitForBallFlashing(struct Task *task)
{
    if (gSprites[task->tBallSpriteId].sState > 4)
        task->tState = HOF_RECORD_EFFECT_END;
}

static void HallOfFameRecordEffect_WaitForSoundAndEnd(struct Task *task)
{
    if (gSprites[task->tBallSpriteId].sState > 6)
    {
        DestroySprite(&gSprites[task->tBallSpriteId]);
        FieldEffectActiveListRemove(FLDEFF_HALL_OF_FAME_RECORD);
        DestroyTask(FindTaskIdByFunc(Task_HallOfFameRecord));
    }
}

static u8 CreateGlowingPokeballsEffect(s16 numMons, s16 x, s16 y, bool16 playHealSe)
{
    u8 spriteId;
    struct Sprite *sprite;
    spriteId = CreateInvisibleSprite(SpriteCB_PokeballGlowEffect);
    sprite = &gSprites[spriteId];
    sprite->x2 = x;
    sprite->y2 = y;
    sprite->subpriority = 0xFF;
    sprite->sPlayHealSe = playHealSe;
    sprite->sNumMons = numMons;
    sprite->sSpriteId = spriteId;
    return spriteId;
}

static void SpriteCB_PokeballGlowEffect(struct Sprite *sprite)
{
    sPokeballGlowEffectFuncs[sprite->sState](sprite);
}

static const struct Coords16 sPokeballCoordOffsets[] = {
    {0, 0},
    {6, 0},
    {0, 4},
    {6, 4},
    {0, 8},
    {6, 8}
};

static const u8 sPokeballGlowReds[]   = {16, 12,  8,  0};
static const u8 sPokeballGlowGreens[] = {16, 12,  8,  0};
static const u8 sPokeballGlowBlues[]  = { 0,  0,  0,  0};

static void PokeballGlowEffect_PlaceBalls(struct Sprite *sprite)
{
    u8 spriteId;
    if (sprite->sTimer == 0 || (--sprite->sTimer) == 0)
    {
        sprite->sTimer = 25;
        spriteId = CreateSpriteAtEnd(&sSpriteTemplate_PokeballGlow, sPokeballCoordOffsets[sprite->sCounter].x + sprite->x2, sPokeballCoordOffsets[sprite->sCounter].y + sprite->y2, 0xFF);
        gSprites[spriteId].oam.priority = 2;
        gSprites[spriteId].sEffectSpriteId = sprite->sSpriteId;
        sprite->sCounter++;
        sprite->sNumMons--;
        PlaySE(SE_BALL);
    }
    if (sprite->sNumMons == 0)
    {
        sprite->sTimer = 32;
        sprite->sState = POKEBALL_GLOW_EFFECT_TRY_PLAY_SE;
    }
}

static void PokeballGlowEffect_TryPlaySe(struct Sprite *sprite)
{
    if ((--sprite->sTimer) == 0)
    {
        sprite->sState = POKEBALL_GLOW_EFFECT_FLASH_FIRST_THREE;
        sprite->sTimer = 8;
        sprite->sCounter = 0;
        sprite->sNumFlashed = 0;
        if (sprite->sPlayHealSe)
            PlayFanfare(MUS_HEAL);
    }
}

static void PokeballGlowEffect_FlashFirstThree(struct Sprite *sprite)
{
    u8 phase;
    if ((--sprite->sTimer) == 0)
    {
        sprite->sTimer = 8;
        sprite->sCounter++;
        sprite->sCounter &= 3;
        if (sprite->sCounter == 0)
            sprite->sNumFlashed++;
    }
    phase = (sprite->sCounter + 3) & 3;
    MultiplyInvertedPaletteRGBComponents(OBJ_PLTT_ID(IndexOfSpritePaletteTag(0x1007)) + 8, sPokeballGlowReds[phase], sPokeballGlowGreens[phase], sPokeballGlowBlues[phase]);
    phase = (sprite->sCounter + 2) & 3;
    MultiplyInvertedPaletteRGBComponents(OBJ_PLTT_ID(IndexOfSpritePaletteTag(0x1007)) + 6, sPokeballGlowReds[phase], sPokeballGlowGreens[phase], sPokeballGlowBlues[phase]);
    phase = (sprite->sCounter + 1) & 3;
    MultiplyInvertedPaletteRGBComponents(OBJ_PLTT_ID(IndexOfSpritePaletteTag(0x1007)) + 2, sPokeballGlowReds[phase], sPokeballGlowGreens[phase], sPokeballGlowBlues[phase]);
    phase = sprite->sCounter;
    MultiplyInvertedPaletteRGBComponents(OBJ_PLTT_ID(IndexOfSpritePaletteTag(0x1007)) + 5, sPokeballGlowReds[phase], sPokeballGlowGreens[phase], sPokeballGlowBlues[phase]);
    MultiplyInvertedPaletteRGBComponents(OBJ_PLTT_ID(IndexOfSpritePaletteTag(0x1007)) + 3, sPokeballGlowReds[phase], sPokeballGlowGreens[phase], sPokeballGlowBlues[phase]);
    if (sprite->sNumFlashed >= 3)
    {
        sprite->sState = POKEBALL_GLOW_EFFECT_FLASH_LAST;
        sprite->sTimer = 8;
        sprite->sCounter = 0;
    }
}

static void PokeballGlowEffect_FlashLast(struct Sprite *sprite)
{
    u8 phase;
    if ((--sprite->sTimer) == 0)
    {
        sprite->sTimer = 8;
        sprite->sCounter++;
        sprite->sCounter &= 3;
        if (sprite->sCounter == 3)
        {
            sprite->sState = POKEBALL_GLOW_EFFECT_WAIT_AFTER_FLASH;
            sprite->sTimer = 30;
        }
    }
    phase = sprite->sCounter;
    MultiplyInvertedPaletteRGBComponents(OBJ_PLTT_ID(IndexOfSpritePaletteTag(0x1007)) + 8, sPokeballGlowReds[phase], sPokeballGlowGreens[phase], sPokeballGlowBlues[phase]);
    MultiplyInvertedPaletteRGBComponents(OBJ_PLTT_ID(IndexOfSpritePaletteTag(0x1007)) + 6, sPokeballGlowReds[phase], sPokeballGlowGreens[phase], sPokeballGlowBlues[phase]);
    MultiplyInvertedPaletteRGBComponents(OBJ_PLTT_ID(IndexOfSpritePaletteTag(0x1007)) + 2, sPokeballGlowReds[phase], sPokeballGlowGreens[phase], sPokeballGlowBlues[phase]);
    MultiplyInvertedPaletteRGBComponents(OBJ_PLTT_ID(IndexOfSpritePaletteTag(0x1007)) + 5, sPokeballGlowReds[phase], sPokeballGlowGreens[phase], sPokeballGlowBlues[phase]);
    MultiplyInvertedPaletteRGBComponents(OBJ_PLTT_ID(IndexOfSpritePaletteTag(0x1007)) + 3, sPokeballGlowReds[phase], sPokeballGlowGreens[phase], sPokeballGlowBlues[phase]);
}

static void PokeballGlowEffect_WaitAfterFlash(struct Sprite *sprite)
{
    if ((--sprite->sTimer) == 0)
        sprite->sState = POKEBALL_GLOW_EFFECT_DUMMY;
}

static void PokeballGlowEffect_Dummy(struct Sprite *sprite)
{
    sprite->sState = POKEBALL_GLOW_EFFECT_WAIT_FOR_SOUND;
}

static void PokeballGlowEffect_WaitForSound(struct Sprite *sprite)
{
    if (sprite->sPlayHealSe == FALSE || IsFanfareTaskInactive())
        sprite->sState = POKEBALL_GLOW_EFFECT_IDLE;
}

static void PokeballGlowEffect_Idle(struct Sprite *sprite)
{
}

static void SpriteCB_PokeballGlow(struct Sprite *sprite)
{
    if (gSprites[sprite->sEffectSpriteId].sState > 4)
        FieldEffectFreeGraphicsResources(sprite);
}

static u8 CreatePokecenterMonitorSprite(s32 x, s32 y)
{
    u8 spriteId;
    struct Sprite *sprite;
    spriteId = CreateSpriteAtEnd(&sSpriteTemplate_PokecenterMonitor, x, y, 0);
    sprite = &gSprites[spriteId];
    sprite->oam.priority = 2;
    sprite->invisible = TRUE;
    return spriteId;
}

static void SpriteCB_PokecenterMonitor(struct Sprite *sprite)
{
    if (sprite->sStartFlash != FALSE)
    {
        sprite->sStartFlash = FALSE;
        sprite->invisible = FALSE;
        StartSpriteAnim(sprite, 1);
    }
    if (sprite->animEnded)
        FieldEffectFreeGraphicsResources(sprite);
}


static void CreateHofMonitorSprite(s32 x, s32 y)
{
    CreateSpriteAtEnd(&sSpriteTemplate_HofMonitor, x, y, 0);
}

static void SpriteCB_HallOfFameMonitor(struct Sprite *sprite)
{
    if (sprite->animEnded)
        FieldEffectFreeGraphicsResources(sprite);
}

#undef tState
#undef tNumMons
#undef tFirstBallX
#undef tFirstBallY
#undef tMonitorX
#undef tMonitorY
#undef tBallSpriteId
#undef tMonitorSpriteId

#undef sState
#undef sTimer
#undef sCounter
#undef sNumFlashed
#undef sPlayHealSe
#undef sNumMons
#undef sSpriteId

#undef sEffectSpriteId
#undef sStartFlash

void ReturnToFieldFromFlyMapSelect(void)
{
    SetMainCallback2(CB2_ReturnToField);
    gFieldCallback = FieldCallback_UseFly;
}

void FieldCallback_UseFly(void)
{
    FadeInFromBlack();
    CreateTask(Task_UseFly, 0);
    LockPlayerFieldControls();
    FreezeObjectEvents();
    gFieldCallback = NULL;
}

#define taskState           task->data[3]
#define fieldEffectStarted  task->data[0]

static void Task_UseFly(u8 taskId)
{
    struct ObjectEvent *follower = &gObjectEvents[GetFollowerNPCObjectId()];
    struct Task *task;
    task = &gTasks[taskId];
    if (taskState == 0)
    {
        if (!PlayerHasFollowerNPC())
        {
            taskState = 2;
        }
        else
        {
            FollowerNPCWalkIntoPlayerForLeaveMap();
            taskState++;
        }
    }
    if (taskState == 1)
    {
        if (ObjectEventClearHeldMovementIfFinished(follower))
        {
            FollowerNPCHideForLeaveMap(follower);
            taskState++;
        }
    }
    if (taskState == 2)
    {
        if (!fieldEffectStarted)
        {
            if (!IsWeatherNotFadingIn())
                return;

            gFieldEffectArguments[0] = GetCursorSelectionMonId();
            if ((int)gFieldEffectArguments[0] >= PARTY_SIZE)
                gFieldEffectArguments[0] = 0;

            FieldEffectStart(FLDEFF_USE_FLY);
            fieldEffectStarted = TRUE;
        }
        if (!FieldEffectActiveListContains(FLDEFF_USE_FLY))
        {
            Overworld_ResetStateAfterFly();
            WarpIntoMap();
            SetMainCallback2(CB2_LoadMap);
            gFieldCallback = FieldCallback_FlyIntoMap;
            DestroyTask(taskId);
        }
    }
}

#undef taskState
#undef fieldEffectStarted

static void FieldCallback_FlyIntoMap(void)
{
    Overworld_PlaySpecialMapMusic();
    FadeInFromBlack();
    CreateTask(Task_FlyIntoMap, 0);
    gObjectEvents[gPlayerAvatar.objectEventId].invisible = TRUE;
    if (gPlayerAvatar.flags & PLAYER_AVATAR_FLAG_SURFING)
        ObjectEventTurn(&gObjectEvents[gPlayerAvatar.objectEventId], DIR_WEST);

    LockPlayerFieldControls();
    FreezeObjectEvents();
    gFieldCallback = NULL;
}

#define taskState               task->data[0]
#define tWaitPaletteFadeIn      0
#define tWaitFieldEffectEnd     1
#define tNPCFollowerFacePlayer  2
#define tTaskEnd                3

static void Task_FlyIntoMap(u8 taskId)
{
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    struct ObjectEvent *follower = &gObjectEvents[GetFollowerNPCObjectId()];
    struct Task *task;
    task = &gTasks[taskId];
    if (taskState == tWaitPaletteFadeIn)
    {
        if (gPaletteFade.active)
            return;
        FieldEffectStart(FLDEFF_FLY_IN);
        taskState++;
    }
    if (taskState == tWaitFieldEffectEnd)
    {
        if (!FieldEffectActiveListContains(FLDEFF_FLY_IN))
        {
            if (FNPC_NPC_FOLLOWER_SHOW_AFTER_LEAVE_ROUTE)
                FollowerNPCReappearAfterLeaveMap(follower, player);

            taskState++;
        }
    }
    if (taskState == tNPCFollowerFacePlayer)
    {
        if (PlayerHasFollowerNPC() && ObjectEventClearHeldMovementIfFinished(follower))
        {
            if (FNPC_NPC_FOLLOWER_SHOW_AFTER_LEAVE_ROUTE)
                FollowerNPCFaceAfterLeaveMap();
            taskState++;
        }
        else if (!PlayerHasFollowerNPC())
        {
            taskState++;
        }
    }
    if (taskState == tTaskEnd)
    {
        UnlockPlayerFieldControls();
        UnfreezeObjectEvents();
        DestroyTask(taskId);
    }
}

#undef taskState
#undef tWaitPaletteFadeIn
#undef tWaitFieldEffectEnd
#undef tNPCFollowerFacePlayer
#undef tTaskEnd

#define tState      data[0]
#define tFallOffset data[1]
#define tTotalFall  data[2]
#define tSetTrigger data[3]
#define tSubsprMode data[4]

#define tVertShake  data[1] // re-used
#define tNumShakes  data[2]

enum FallWarpEffectState
{
    FALL_WARP_EFFECT_INIT,
    FALL_WARP_EFFECT_WAIT_WEATHER,
    FALL_WARP_EFFECT_START_FALL,
    FALL_WARP_EFFECT_FALL,
    FALL_WARP_EFFECT_LAND,
    FALL_WARP_EFFECT_CAMERA_SHAKE,
    FALL_WARP_EFFECT_END,
};

static bool32 (*const sFallWarpEffectCBPtrs[])(struct Task *task) =
{
    [FALL_WARP_EFFECT_INIT]         = FallWarpEffect_Init,
    [FALL_WARP_EFFECT_WAIT_WEATHER] = FallWarpEffect_WaitWeather,
    [FALL_WARP_EFFECT_START_FALL]   = FallWarpEffect_StartFall,
    [FALL_WARP_EFFECT_FALL]         = FallWarpEffect_Fall,
    [FALL_WARP_EFFECT_LAND]         = FallWarpEffect_Land,
    [FALL_WARP_EFFECT_CAMERA_SHAKE] = FallWarpEffect_CameraShake,
    [FALL_WARP_EFFECT_END]          = FallWarpEffect_End
};

void FieldCB_FallWarpExit(void)
{
    Overworld_PlaySpecialMapMusic();
    WarpFadeInScreen();
    QuestLog_DrawPreviouslyOnQuestHeaderIfInPlaybackMode();
    LockPlayerFieldControls();
    FreezeObjectEvents();
    CreateTask(Task_FallWarpFieldEffect, 0);
    gFieldCallback = NULL;
}

static void Task_FallWarpFieldEffect(u8 taskId)
{
    struct Task *task = &gTasks[taskId];
    while (sFallWarpEffectCBPtrs[task->tState](task))
        ;
}

static bool32 FallWarpEffect_Init(struct Task *task)
{
    struct ObjectEvent *playerObject = &gObjectEvents[gPlayerAvatar.objectEventId];
    struct Sprite *playerSprite = &gSprites[gPlayerAvatar.spriteId];

    CameraObjectFreeze();
    gObjectEvents[gPlayerAvatar.objectEventId].invisible = TRUE;
    gPlayerAvatar.preventStep = TRUE;
    ObjectEventSetHeldMovement(playerObject, GetFaceDirectionMovementAction(GetPlayerFacingDirection()));
    task->tSubsprMode = playerSprite->subspriteMode;
    playerObject->fixedPriority = TRUE;
    playerSprite->oam.priority = 1;
    playerSprite->subspriteMode = SUBSPRITES_IGNORE_PRIORITY;
    task->tState = FALL_WARP_EFFECT_WAIT_WEATHER;

    return TRUE;
}

static bool32 FallWarpEffect_WaitWeather(struct Task *task)
{
    if (IsWeatherNotFadingIn())
        task->tState = FALL_WARP_EFFECT_START_FALL;

    return FALSE;
}

static bool32 FallWarpEffect_StartFall(struct Task *task)
{
    struct Sprite *sprite = &gSprites[gPlayerAvatar.spriteId];
    s16 centerToCornerVecY = -(sprite->centerToCornerVecY << 1);

    sprite->y2 = -(sprite->y + sprite->centerToCornerVecY + gSpriteCoordOffsetY + centerToCornerVecY);
    task->tFallOffset = 1;
    task->tTotalFall = 0;
    gObjectEvents[gPlayerAvatar.objectEventId].invisible = FALSE;
    PlaySE(SE_FALL);
    task->tState = FALL_WARP_EFFECT_FALL;

    return FALSE;
}

static bool32 FallWarpEffect_Fall(struct Task *task)
{
    struct ObjectEvent * objectEvent;
    struct Sprite *sprite;

    objectEvent = &gObjectEvents[gPlayerAvatar.objectEventId];
    sprite = &gSprites[gPlayerAvatar.spriteId];
    sprite->y2 += task->tFallOffset;
    if (task->tFallOffset < 8)
    {
        task->tTotalFall += task->tFallOffset;
        if (task->tTotalFall & 0xf)
            task->tFallOffset <<= 1;
    }
    if (!task->tSetTrigger && sprite->y2 >= -16)
    {
        task->tSetTrigger = TRUE;
        objectEvent->fixedPriority = FALSE;
        sprite->subspriteMode = task->tSubsprMode;
        objectEvent->triggerGroundEffectsOnMove = TRUE;
    }
    if (sprite->y2 >= 0)
    {
        PlaySE(SE_M_STRENGTH);
        objectEvent->triggerGroundEffectsOnStop = TRUE;
        objectEvent->landingJump = TRUE;
        sprite->y2 = 0;
        task->tState = FALL_WARP_EFFECT_LAND;
    }

    return FALSE;
}

static bool32 FallWarpEffect_Land(struct Task *task)
{
    task->tState = FALL_WARP_EFFECT_CAMERA_SHAKE;
    task->tVertShake = 4;
    task->tNumShakes = 0;
    SetCameraPanningCallback(NULL);

    return TRUE;
}

static bool32 FallWarpEffect_CameraShake(struct Task *task)
{
    SetCameraPanning(0, task->tVertShake);
    task->tVertShake = -task->tVertShake;
    task->tNumShakes++;
    if ((task->tNumShakes & 3) == 0)
        task->tVertShake >>= 1;

    if (task->tVertShake == 0)
        task->tState = FALL_WARP_EFFECT_END;

    return FALSE;
}

static bool32 FallWarpEffect_End(struct Task *task)
{
    s16 x, y;

    gPlayerAvatar.preventStep = FALSE;
    UnlockPlayerFieldControls();
    CameraObjectReset();
    UnfreezeObjectEvents();
    InstallCameraPanAheadCallback();
    PlayerGetDestCoords(&x, &y);
    if (MetatileBehavior_IsSurfableInSeafoamIslands(MapGridGetMetatileBehaviorAt(x, y)) == TRUE)
    {
        VarSet(VAR_TEMP_1, 1);
        SetPlayerAvatarTransitionFlags(PLAYER_AVATAR_FLAG_SURFING);
        SetHelpContext(HELPCONTEXT_SURFING);
    }
    DestroyTask(FindTaskIdByFunc(Task_FallWarpFieldEffect));
    FollowerNPC_WarpSetEnd();

    return FALSE;
}

#undef tState
#undef tFallOffset
#undef tTotalFall
#undef tSetTrigger
#undef tSubsprMode
#undef tVertShake
#undef tNumShakes

enum EscalatorWarpOutEffectState
{
    ESCALATOR_WARP_OUT_EFFECT_INIT,
    ESCALATOR_WARP_OUT_EFFECT_WAIT_FOR_PLAYER,
    ESCALATOR_WARP_OUT_EFFECT_UP_RIDE,
    ESCALATOR_WARP_OUT_EFFECT_UP_END,
    ESCALATOR_WARP_OUT_EFFECT_DOWN_RIDE,
    ESCALATOR_WARP_OUT_EFFECT_DOWN_END,
};

static bool32 (*const sEscalatorWarpOutFieldEffectFuncs[])(struct Task *task) =
{
    [ESCALATOR_WARP_OUT_EFFECT_INIT]            = EscalatorWarpOut_Init,
    [ESCALATOR_WARP_OUT_EFFECT_WAIT_FOR_PLAYER] = EscalatorWarpOut_WaitForPlayer,
    [ESCALATOR_WARP_OUT_EFFECT_UP_RIDE]         = EscalatorWarpOut_Up_Ride,
    [ESCALATOR_WARP_OUT_EFFECT_UP_END]          = EscalatorWarpOut_Up_End,
    [ESCALATOR_WARP_OUT_EFFECT_DOWN_RIDE]       = EscalatorWarpOut_Down_Ride,
    [ESCALATOR_WARP_OUT_EFFECT_DOWN_END]        = EscalatorWarpOut_Down_End,
};

#define tState   data[0]
#define tGoingUp data[1]
#define tTimer1  data[2]
#define tTimer2  data[3]

void HideFollowerForFieldEffect(void)
{
    struct ObjectEvent *followerObj = GetFollowerObject();
    if (!followerObj || followerObj->invisible)
        return;

    ClearObjectEventMovement(followerObj, &gSprites[followerObj->spriteId]);
    ObjectEventSetHeldMovement(followerObj, MOVEMENT_ACTION_ENTER_POKEBALL);
}

void StartEscalatorWarp(u8 metatileBehavior, u8 priority)
{
    u8 taskId = CreateTask(Task_EscalatorWarpOut, priority);
    gTasks[taskId].tGoingUp = FALSE;
    if (metatileBehavior == MB_UP_ESCALATOR)
        gTasks[taskId].tGoingUp = TRUE;

    EndORASDowsing();
}

static void Task_EscalatorWarpOut(u8 taskId)
{
    struct Task *task = &gTasks[taskId];
    while (sEscalatorWarpOutFieldEffectFuncs[task->tState](task))
        ;
}

static bool32 EscalatorWarpOut_Init(struct Task *task)
{
    FreezeObjectEvents();
    CameraObjectFreeze();
    StartEscalator(task->tGoingUp);
    HideFollowerForFieldEffect(); // Hide follower before warping
    QuestLog_OnEscalatorWarp(QL_ESCALATOR_OUT);
    task->tState = ESCALATOR_WARP_OUT_EFFECT_WAIT_FOR_PLAYER;

    return FALSE;
}

static bool32 EscalatorWarpOut_WaitForPlayer(struct Task *task)
{
    struct ObjectEvent * objectEvent;
    objectEvent = &gObjectEvents[gPlayerAvatar.objectEventId];
    if (!ObjectEventIsMovementOverridden(objectEvent) || ObjectEventClearHeldMovementIfFinished(objectEvent))
    {
        ObjectEventSetHeldMovement(objectEvent, GetFaceDirectionMovementAction(GetPlayerFacingDirection()));
        objectEvent->noShadow = TRUE; // hide shadow for cleaner movement
        task->tState = ESCALATOR_WARP_OUT_EFFECT_UP_RIDE;
        task->tTimer1 = 0;
        task->tTimer2 = 0;
        EscalatorMoveFollowerNPC(task->tGoingUp);

        if (!task->tGoingUp)
            task->tState = ESCALATOR_WARP_OUT_EFFECT_DOWN_RIDE;

        PlaySE(SE_ESCALATOR);
    }
    return FALSE;
}

static bool32 EscalatorWarpOut_Up_Ride(struct Task *task)
{
    RideUpEscalatorOut(task);
    if (task->tTimer1 > 3)
    {
        FadeOutAtEndOfEscalator();
        task->tState = ESCALATOR_WARP_OUT_EFFECT_UP_END;
    }
    return FALSE;
}

static bool32 EscalatorWarpOut_Up_End(struct Task *task)
{
    RideUpEscalatorOut(task);
    WarpAtEndOfEscalator();
    return FALSE;
}

static bool32 EscalatorWarpOut_Down_Ride(struct Task *task)
{
    RideDownEscalatorOut(task);
    if (task->tTimer1 > 3)
    {
        FadeOutAtEndOfEscalator();
        task->tState = ESCALATOR_WARP_OUT_EFFECT_DOWN_END;
    }
    return FALSE;
}

static bool32 EscalatorWarpOut_Down_End(struct Task *task)
{
    RideDownEscalatorOut(task);
    WarpAtEndOfEscalator();

    return FALSE;
}


static void RideUpEscalatorOut(struct Task *task)
{
    struct Sprite *sprite = &gSprites[gPlayerAvatar.spriteId];

    sprite->x2 = Cos(132, task->tTimer1);
    sprite->y2 = Sin(148, task->tTimer1);
    task->tTimer2++;
    if (task->tTimer2 & 1)
        task->tTimer1++;
}

static void RideDownEscalatorOut(struct Task *task)
{
    struct Sprite *sprite = &gSprites[gPlayerAvatar.spriteId];

    sprite->x2 = Cos(124, task->tTimer1);
    sprite->y2 = Sin(118, task->tTimer1);
    task->tTimer2++;
    if (task->tTimer2 & 1)
        task->tTimer1++;
}

static void FadeOutAtEndOfEscalator(void)
{
    TryFadeOutOldMapMusic();
    WarpFadeOutScreen();
}

static void WarpAtEndOfEscalator(void)
{
    if (!gPaletteFade.active && BGMusicStopped() == TRUE)
    {
        StopEscalator();
        WarpIntoMap();
        gFieldCallback = FieldCallback_EscalatorWarpIn;
        SetMainCallback2(CB2_LoadMap);
        DestroyTask(FindTaskIdByFunc(Task_EscalatorWarpOut));
    }
}

#undef tState
#undef tGoingUp
#undef tTimer1
#undef tTimer2

enum EscalatorWarpInEffectState
{
    ESCALATOR_WARP_IN_INIT,
    ESCALATOR_WARP_IN_DOWN_INIT,
    ESCALATOR_WARP_IN_DOWN_RIDE,
    ESCALATOR_WARP_IN_UP_INIT,
    ESCALATOR_WARP_IN_UP_RIDE,
    ESCALATOR_WARP_IN_WAIT_FOR_MOVEMENT,
    ESCALATOR_WARP_IN_END,
};

static bool32 (*const sEscalatorWarpInFieldEffectFuncs[])(struct Task *task) = {
    [ESCALATOR_WARP_IN_INIT]              = EscalatorWarpIn_Init,
    [ESCALATOR_WARP_IN_DOWN_INIT]         = EscalatorWarpIn_Down_Init,
    [ESCALATOR_WARP_IN_DOWN_RIDE]         = EscalatorWarpIn_Down_Ride,
    [ESCALATOR_WARP_IN_UP_INIT]           = EscalatorWarpIn_Up_Init,
    [ESCALATOR_WARP_IN_UP_RIDE]           = EscalatorWarpIn_Up_Ride,
    [ESCALATOR_WARP_IN_WAIT_FOR_MOVEMENT] = EscalatorWarpIn_WaitForMovement,
    [ESCALATOR_WARP_IN_END]               = EscalatorWarpIn_End,
};

static void FieldCallback_EscalatorWarpIn(void)
{
    Overworld_PlaySpecialMapMusic();
    WarpFadeInScreen();
    QuestLog_DrawPreviouslyOnQuestHeaderIfInPlaybackMode();
    LockPlayerFieldControls();
    FreezeObjectEvents();
    CreateTask(Task_EscalatorWarpIn, 0);
    gFieldCallback = NULL;
}

#define tState   data[0]
#define tTimer1  data[1]
#define tTimer2  data[2]

static void Task_EscalatorWarpIn(u8 taskId)
{
    struct Task *task = &gTasks[taskId];
    while (sEscalatorWarpInFieldEffectFuncs[task->tState](task))
        ;
}

static bool32 EscalatorWarpIn_Init(struct Task *task)
{
    struct ObjectEvent *objectEvent;
    s16 x, y;
    u32 behavior;

    CameraObjectFreeze();
    objectEvent = &gObjectEvents[gPlayerAvatar.objectEventId];
    objectEvent->noShadow = TRUE;
    ObjectEventSetHeldMovement(objectEvent, GetFaceDirectionMovementAction(DIR_EAST));
    PlayerGetDestCoords(&x, &y);
    behavior = MapGridGetMetatileBehaviorAt(x, y);
    EscalatorMoveFollowerNPCFinish();
    task->tState = ESCALATOR_WARP_IN_DOWN_INIT;
    task->tTimer1 = 16;
    if (behavior == MB_DOWN_ESCALATOR)
    {
        task->tState = ESCALATOR_WARP_IN_UP_INIT;
        StartEscalator(TRUE);
    }
    else
    {
        StartEscalator(FALSE);
    }

    return TRUE;
}

static bool32 EscalatorWarpIn_Down_Init(struct Task *task)
{
    struct Sprite *sprite = &gSprites[gPlayerAvatar.spriteId];

    sprite->x2 = Cos(132, task->tTimer1);
    sprite->y2 = Sin(148, task->tTimer1);
    task->tState = ESCALATOR_WARP_IN_DOWN_RIDE;

    return FALSE;
}

static bool32 EscalatorWarpIn_Down_Ride(struct Task *task)
{
    struct Sprite *sprite = &gSprites[gPlayerAvatar.spriteId];

    sprite->x2 = Cos(132, task->tTimer1);
    sprite->y2 = Sin(148, task->tTimer1);
    task->tTimer2++;
    if (task->tTimer2 & 1)
        task->tTimer1--;

    if (task->tTimer1 == 0)
    {
        sprite->x2 = 0;
        sprite->y2 = 0;
        task->tState = ESCALATOR_WARP_IN_WAIT_FOR_MOVEMENT;
    }

    return FALSE;
}


static bool32 EscalatorWarpIn_Up_Init(struct Task *task)
{
    struct Sprite *sprite = &gSprites[gPlayerAvatar.spriteId];

    sprite->x2 = Cos(124, task->tTimer1);
    sprite->y2 = Sin(118, task->tTimer1);
    task->tState = ESCALATOR_WARP_IN_UP_RIDE;

    return FALSE;
}

static bool32 EscalatorWarpIn_Up_Ride(struct Task *task)
{
    struct Sprite *sprite = &gSprites[gPlayerAvatar.spriteId];

    sprite->x2 = Cos(124, task->tTimer1);
    sprite->y2 = Sin(118, task->tTimer1);
    task->tTimer2++;
    if (task->tTimer2 & 1)
        task->tTimer1--;

    if (task->tTimer1 == 0)
    {
        sprite->x2 = 0;
        sprite->y2 = 0;
        task->tState = ESCALATOR_WARP_IN_WAIT_FOR_MOVEMENT;
    }

    return FALSE;
}

static bool32 EscalatorWarpIn_WaitForMovement(struct Task *task)
{
    if (IsEscalatorMoving())
        return FALSE;

    StopEscalator();
    task->tState = ESCALATOR_WARP_IN_END;

    return TRUE;
}

static bool32 EscalatorWarpIn_End(struct Task *task)
{
    struct ObjectEvent *objectEvent = &gObjectEvents[gPlayerAvatar.objectEventId];

    objectEvent->noShadow = FALSE;
    if (ObjectEventClearHeldMovementIfFinished(objectEvent))
    {
        CameraObjectReset();
        UnlockPlayerFieldControls();
        UnfreezeObjectEvents();
        ObjectEventSetHeldMovement(objectEvent, GetWalkNormalMovementAction(DIR_EAST));
        DestroyTask(FindTaskIdByFunc(Task_EscalatorWarpIn));
        QuestLog_OnEscalatorWarp(QL_ESCALATOR_IN);
    }

    return FALSE;
}

#undef tState
#undef tTimer1
#undef tTimer2

enum WaterfallState
{
    WATERFALL_INIT,
    WATERFALL_SHOWMON,
    WATERFALL_WAITMON,
    WATERFALL_RIDE_UP,
    WATERFALL_CONTINUE_OR_END,
};

static bool32 (*const sWaterfallFieldEffectFuncs[])(struct Task *task, struct ObjectEvent * playerObj) = {
    [WATERFALL_INIT]            = WaterfallFieldEffect_Init,
    [WATERFALL_SHOWMON]         = WaterfallFieldEffect_ShowMon,
    [WATERFALL_WAITMON]         = WaterfallFieldEffect_WaitForShowMon,
    [WATERFALL_RIDE_UP]         = WaterfallFieldEffect_RideUp,
    [WATERFALL_CONTINUE_OR_END] = WaterfallFieldEffect_ContinueRideOrEnd
};

#define tState data[0]
#define tMonId data[1]

u32 FldEff_UseWaterfall(void)
{
    u8 taskId = CreateTask(Task_UseWaterfall, 0xFF);
    gTasks[taskId].tMonId = gFieldEffectArguments[0];
    Task_UseWaterfall(taskId);

    return 0;
}

static void Task_UseWaterfall(u8 taskId)
{
    while (sWaterfallFieldEffectFuncs[gTasks[taskId].tState](&gTasks[taskId], &gObjectEvents[gPlayerAvatar.objectEventId]))
        ;
}

static bool32 WaterfallFieldEffect_Init(struct Task *task, struct ObjectEvent * playerObj)
{
    LockPlayerFieldControls();
    gPlayerAvatar.preventStep = TRUE;
    task->tState = WATERFALL_SHOWMON;

    return FALSE;
}

static bool32 WaterfallFieldEffect_ShowMon(struct Task *task, struct ObjectEvent * playerObj)
{
    LockPlayerFieldControls();
    if (!ObjectEventIsMovementOverridden(playerObj))
    {
        ObjectEventClearHeldMovementIfFinished(playerObj);
        gFieldEffectArguments[0] = task->tMonId;
        FieldEffectStart(FLDEFF_FIELD_MOVE_SHOW_MON_INIT);
        task->tState = WATERFALL_WAITMON;
    }
    return FALSE;
}

static bool32 WaterfallFieldEffect_WaitForShowMon(struct Task *task, struct ObjectEvent * playerObj)
{
    if (FieldEffectActiveListContains(FLDEFF_FIELD_MOVE_SHOW_MON))
        return FALSE;

    task->tState = WATERFALL_RIDE_UP;
    return TRUE;
}

static bool32 WaterfallFieldEffect_RideUp(struct Task *task, struct ObjectEvent * playerObj)
{
    ObjectEventSetHeldMovement(playerObj, GetWalkSlowerMovementAction(DIR_NORTH));
    task->tState = WATERFALL_CONTINUE_OR_END;
    return FALSE;
}

static bool32 WaterfallFieldEffect_ContinueRideOrEnd(struct Task *task, struct ObjectEvent * playerObj)
{
    if (!ObjectEventClearHeldMovementIfFinished(playerObj))
        return FALSE;
    if (MetatileBehavior_IsWaterfall(playerObj->currentMetatileBehavior))
    {
        task->tState = WATERFALL_RIDE_UP;
        return TRUE;
    }
    UnlockPlayerFieldControls();
    gPlayerAvatar.preventStep = FALSE;
    DestroyTask(FindTaskIdByFunc(Task_UseWaterfall));
    FieldEffectActiveListRemove(FLDEFF_USE_WATERFALL);
    return FALSE;
}

#undef tState
#undef tMonId

enum DiveEffectState
{
    DIVE_INIT,
    DIVE_SHOW_MON,
    DIVE_TRY_WARP,
};

static bool32 (*const sDiveFieldEffectFuncs[])(struct Task *task) =
{
    [DIVE_INIT]     = DiveFieldEffect_Init,
    [DIVE_SHOW_MON] = DiveFieldEffect_ShowMon,
    [DIVE_TRY_WARP] = DiveFieldEffect_TryWarp
};

#define tState data[0]
#define tMonId data[15]

u32 FldEff_UseDive(void)
{
    u8 taskId = CreateTask(Task_UseDive, 0xFF);
    gTasks[taskId].tMonId = gFieldEffectArguments[0]; // party index of pokemon with dive
    Task_UseDive(taskId);

    return 0;
}

static void Task_UseDive(u8 taskId)
{
    while (sDiveFieldEffectFuncs[gTasks[taskId].tState](&gTasks[taskId]))
        ;
}

static bool32 DiveFieldEffect_Init(struct Task *task)
{
    gPlayerAvatar.preventStep = TRUE;
    task->tState = DIVE_SHOW_MON;

    return FALSE;
}

static bool32 DiveFieldEffect_ShowMon(struct Task *task)
{
    LockPlayerFieldControls();
    gFieldEffectArguments[0] = task->tMonId;
    FieldEffectStart(FLDEFF_FIELD_MOVE_SHOW_MON_INIT);
    task->tState = DIVE_TRY_WARP;

    return FALSE;
}

static bool32 DiveFieldEffect_TryWarp(struct Task *task)
{
    struct MapPosition pos;

    PlayerGetDestCoords(&pos.x, &pos.y);
    if (!FieldEffectActiveListContains(FLDEFF_FIELD_MOVE_SHOW_MON))
    {

        TryDoDiveWarp(&pos, gObjectEvents[gPlayerAvatar.objectEventId].currentMetatileBehavior);
        DestroyTask(FindTaskIdByFunc(Task_UseDive));
        FieldEffectActiveListRemove(FLDEFF_USE_DIVE);
    }

    return FALSE;
}

#undef tState
#undef tMonId

enum LavaridgeB1FWarpState
{
    LAVARIDGE_B1F_WARP_INIT,
    LAVARIDGE_B1F_WARP_CAMERA_SHAKE,
    LAVARIDGE_B1F_WARP_LAUNCH,
    LAVARIDGE_B1F_WARP_RISE,
    LAVARIDGE_B1F_WARP_FADE_OUT,
    LAVARIDGE_B1F_WARP_WARP,
};

static bool32 (*const sLavaridgeGymB1FWarpEffectFuncs[])(struct Task *task, struct ObjectEvent *objectEvent, struct Sprite *sprite) =
{
    [LAVARIDGE_B1F_WARP_INIT]         = LavaridgeGymB1FWarpEffect_Init,
    [LAVARIDGE_B1F_WARP_CAMERA_SHAKE] = LavaridgeGymB1FWarpEffect_CameraShake,
    [LAVARIDGE_B1F_WARP_LAUNCH]       = LavaridgeGymB1FWarpEffect_Launch,
    [LAVARIDGE_B1F_WARP_RISE]         = LavaridgeGymB1FWarpEffect_Rise,
    [LAVARIDGE_B1F_WARP_FADE_OUT]     = LavaridgeGymB1FWarpEffect_FadeOut,
    [LAVARIDGE_B1F_WARP_WARP]         = LavaridgeGymB1FWarpEffect_Warp
};

#define tState             data[0]
#define tVertShake         data[1]
#define tTimer             data[2]
#define tSpriteY           data[3]
#define tYMovementFinished data[4]
#define tSetTrigger        data[5]

void StartLavaridgeGymB1FWarp(u8 priority)
{
    EndORASDowsing();
    CreateTask(Task_LavaridgeGymB1FWarp, priority);
}

static void Task_LavaridgeGymB1FWarp(u8 taskId)
{
    struct Task *task = &gTasks[taskId];
    struct Sprite *sprite = &gSprites[gPlayerAvatar.spriteId];
    struct ObjectEvent *objEvent = &gObjectEvents[gPlayerAvatar.objectEventId];

    while (sLavaridgeGymB1FWarpEffectFuncs[task->tState](task, objEvent, sprite));
}

static bool32 LavaridgeGymB1FWarpEffect_Init(struct Task *task, struct ObjectEvent *objEvent, struct Sprite *sprite)
{
    FreezeObjectEvents();
    CameraObjectFreeze();
    SetCameraPanningCallback(NULL);
    gPlayerAvatar.preventStep = TRUE;
    objEvent->fixedPriority = TRUE;
    task->tVertShake = 1;
    task->tState = LAVARIDGE_B1F_WARP_CAMERA_SHAKE;

    return TRUE;
}

static bool32 LavaridgeGymB1FWarpEffect_CameraShake(struct Task *task, struct ObjectEvent *objEvent, struct Sprite *sprite)
{
    SetCameraPanning(0, task->tVertShake);
    task->tVertShake = -task->tVertShake;
    task->tTimer++;
    if (task->tTimer > 7)
    {
        task->tTimer = 0;
        task->tState = LAVARIDGE_B1F_WARP_LAUNCH;
    }

    return FALSE;
}

static bool32 LavaridgeGymB1FWarpEffect_Launch(struct Task *task, struct ObjectEvent *objEvent, struct Sprite *sprite)
{
    sprite->y2 = 0;
    task->tSpriteY = 1;
    gFieldEffectArguments[0] = objEvent->currentCoords.x;
    gFieldEffectArguments[1] = objEvent->currentCoords.y;
    gFieldEffectArguments[2] = sprite->subpriority - 1;
    gFieldEffectArguments[3] = sprite->oam.priority;
    FieldEffectStart(FLDEFF_LAVARIDGE_GYM_WARP);
    PlaySE(SE_M_EXPLOSION);
    task->tState = LAVARIDGE_B1F_WARP_RISE;

    return TRUE;
}

static bool32 LavaridgeGymB1FWarpEffect_Rise(struct Task *task, struct ObjectEvent *objEvent, struct Sprite *sprite)
{
    s16 centerToCornerVecY;
    SetCameraPanning(0, task->tVertShake);
    task->tVertShake = -task->tVertShake;
    if (++task->tTimer <= 17)
    {
        if (!(task->tTimer & 1) && (task->tVertShake <= 3))
            task->tVertShake <<= 1;
    }
    else if (!(task->tTimer & 4) && (task->tVertShake > 0))
    {
        task->tVertShake >>= 1;
    }

    if (task->tTimer > 6)
    {
        centerToCornerVecY = -(sprite->centerToCornerVecY << 1);
        if (sprite->y2 > -(sprite->y + sprite->centerToCornerVecY + gSpriteCoordOffsetY + centerToCornerVecY))
        {
            sprite->y2 -= task->tSpriteY;
            if (task->tSpriteY <= 7)
            {
                task->tSpriteY++;
            }
        } else
        {
            task->tYMovementFinished = TRUE;
        }
    }

    if (!task->tSetTrigger && sprite->y2 < -16)
    {
        task->tSetTrigger = TRUE;
        objEvent->fixedPriority = TRUE;
        sprite->oam.priority = 1;
        sprite->subspriteMode = SUBSPRITES_IGNORE_PRIORITY;
    }

    if (task->tVertShake == 0 && task->tYMovementFinished)
        task->tState = LAVARIDGE_B1F_WARP_FADE_OUT;

    return FALSE;
}

static bool32 LavaridgeGymB1FWarpEffect_FadeOut(struct Task *task, struct ObjectEvent * objEvent, struct Sprite *sprite)
{
    TryFadeOutOldMapMusic();
    WarpFadeOutScreen();
    task->tState = LAVARIDGE_B1F_WARP_WARP;

    return FALSE;
}

static bool32 LavaridgeGymB1FWarpEffect_Warp(struct Task *task, struct ObjectEvent * objEvent, struct Sprite *sprite)
{
    if (!gPaletteFade.active && BGMusicStopped() == TRUE)
    {
        WarpIntoMap();
        gFieldCallback = FieldCB_LavaridgeGymB1FWarpExit;
        SetMainCallback2(CB2_LoadMap);
        DestroyTask(FindTaskIdByFunc(Task_LavaridgeGymB1FWarp));
    }

    return FALSE;
}

#undef tState
#undef tVertShake
#undef tTimer
#undef tSpriteY
#undef tYMovementFinished
#undef tSetTrigger

enum LavaridgeB1FWarpExitState
{
    LAVARIDGE_B1F_WARP_EXIT_INIT,
    LAVARIDGE_B1F_WARP_EXIT_START_POP_OUT,
    LAVARIDGE_B1F_WARP_EXIT_POP_OUT,
    LAVARIDGE_B1F_WARP_EXIT_END,
};

static bool32 (*const sLavaridgeGymB1FWarpExitEffectFuncs[])(struct Task *task, struct ObjectEvent *objectEvent, struct Sprite *sprite) =
{
    [LAVARIDGE_B1F_WARP_EXIT_INIT]          = LavaridgeGymB1FWarpExitEffect_Init,
    [LAVARIDGE_B1F_WARP_EXIT_START_POP_OUT] = LavaridgeGymB1FWarpExitEffect_StartPopOut,
    [LAVARIDGE_B1F_WARP_EXIT_POP_OUT]       = LavaridgeGymB1FWarpExitEffect_PopOut,
    [LAVARIDGE_B1F_WARP_EXIT_END]           = LavaridgeGymB1FWarpExitEffect_End,
};

#define tState    data[0]
#define tSpriteId data[1]

static void FieldCB_LavaridgeGymB1FWarpExit(void)
{
    Overworld_PlaySpecialMapMusic();
    WarpFadeInScreen();
    QuestLog_DrawPreviouslyOnQuestHeaderIfInPlaybackMode();
    LockPlayerFieldControls();
    gFieldCallback = NULL;
    CreateTask(Task_LavaridgeGymB1FWarpExit, 0);
}

static void Task_LavaridgeGymB1FWarpExit(u8 taskId)
{
    while (sLavaridgeGymB1FWarpExitEffectFuncs[gTasks[taskId].tState](&gTasks[taskId], &gObjectEvents[gPlayerAvatar.objectEventId], &gSprites[gPlayerAvatar.spriteId]));
}

static bool32 LavaridgeGymB1FWarpExitEffect_Init(struct Task *task, struct ObjectEvent *objectEvent, struct Sprite *sprite)
{
    CameraObjectFreeze();
    FreezeObjectEvents();
    gPlayerAvatar.preventStep = TRUE;
    objectEvent->invisible = TRUE;
    task->tState = LAVARIDGE_B1F_WARP_EXIT_START_POP_OUT;
    return FALSE;
}

static bool32 LavaridgeGymB1FWarpExitEffect_StartPopOut(struct Task *task, struct ObjectEvent *objectEvent, struct Sprite *sprite)
{
    if (IsWeatherNotFadingIn())
    {
        gFieldEffectArguments[0] = objectEvent->currentCoords.x;
        gFieldEffectArguments[1] = objectEvent->currentCoords.y;
        gFieldEffectArguments[2] = sprite->subpriority - 1;
        gFieldEffectArguments[3] = sprite->oam.priority;
        task->tSpriteId = FieldEffectStart(FLDEFF_POP_OUT_OF_ASH);
        task->tState = LAVARIDGE_B1F_WARP_EXIT_POP_OUT;
    }
    return FALSE;
}

static bool32 LavaridgeGymB1FWarpExitEffect_PopOut(struct Task *task, struct ObjectEvent *objectEvent, struct Sprite *sprite)
{
    sprite = &gSprites[task->tSpriteId];
    if (sprite->animCmdIndex > 1)
    {
        task->tState = LAVARIDGE_B1F_WARP_EXIT_END;
        objectEvent->invisible = FALSE;
        CameraObjectReset();
        PlaySE(SE_M_DIG);
        ObjectEventSetHeldMovement(objectEvent, GetJumpMovementAction(DIR_EAST));
    }
    return FALSE;
}

static bool32 LavaridgeGymB1FWarpExitEffect_End(struct Task *task, struct ObjectEvent *objectEvent, struct Sprite *sprite)
{
    if (ObjectEventClearHeldMovementIfFinished(objectEvent))
    {
        gPlayerAvatar.preventStep = FALSE;
        UnlockPlayerFieldControls();
        UnfreezeObjectEvents();
        DestroyTask(FindTaskIdByFunc(Task_LavaridgeGymB1FWarpExit));
    }
    return FALSE;
}

#undef tState
#undef tSpriteId

enum LavaridgeBFWarpState
{
    LAVARIDGE_1F_WARP_INIT,
    LAVARIDGE_1F_WARP_ASH_PUFF,
    LAVARIDGE_1F_WARP_DISAPPEAR,
    LAVARIDGE_1F_WARP_FADE_OUT,
    LAVARIDGE_1F_WARP_WARP,
};

static bool32 (*const sLavaridgeGym1FWarpEffectFuncs[])(struct Task *task, struct ObjectEvent *objectEvent, struct Sprite *sprite) = {
    [LAVARIDGE_1F_WARP_INIT]      = LavaridgeGym1FWarpEffect_Init,
    [LAVARIDGE_1F_WARP_ASH_PUFF]  = LavaridgeGym1FWarpEffect_AshPuff,
    [LAVARIDGE_1F_WARP_DISAPPEAR] = LavaridgeGym1FWarpEffect_Disappear,
    [LAVARIDGE_1F_WARP_FADE_OUT]  = LavaridgeGym1FWarpEffect_FadeOut,
    [LAVARIDGE_1F_WARP_WARP]      = LavaridgeGym1FWarpEffect_Warp
};

// For the ash puff effect when warping off the B1F ash tiles
u32 FldEff_LavaridgeGymWarp(void)
{
    u8 spriteId;

    FieldEffectScript_LoadFadedPal(&gSpritePalette_Ash);
    SetSpritePosToOffsetMapCoords((s16 *)&gFieldEffectArguments[0], (s16 *)&gFieldEffectArguments[1], 8, 8);
    spriteId = CreateSpriteAtEnd(&gFieldEffectObjectTemplate_AshLaunch, gFieldEffectArguments[0], gFieldEffectArguments[1], gFieldEffectArguments[2]);
    gSprites[spriteId].oam.priority = gFieldEffectArguments[3];
    gSprites[spriteId].coordOffsetEnabled = TRUE;

    return spriteId;
}

void SpriteCB_AshLaunch(struct Sprite *sprite)
{
    if (sprite->animEnded)
        FieldEffectStop(sprite, FLDEFF_LAVARIDGE_GYM_WARP);
}

#define tState    data[0]
#define tTimer    data[1]
#define tSpriteId data[1] // reused

void StartLavaridgeGym1FWarp(u8 priority)
{
    EndORASDowsing();
    CreateTask(Task_LavaridgeGym1FWarp, priority);
}

static void Task_LavaridgeGym1FWarp(u8 taskId)
{
    while(sLavaridgeGym1FWarpEffectFuncs[gTasks[taskId].tState](&gTasks[taskId], &gObjectEvents[gPlayerAvatar.objectEventId], &gSprites[gPlayerAvatar.spriteId]));
}

static bool32 LavaridgeGym1FWarpEffect_Init(struct Task *task, struct ObjectEvent *objEvent, struct Sprite *sprite)
{
    FreezeObjectEvents();
    CameraObjectFreeze();
    gPlayerAvatar.preventStep = TRUE;
    objEvent->fixedPriority = TRUE;
    task->tState = LAVARIDGE_1F_WARP_ASH_PUFF;

    return FALSE;
}

static bool32 LavaridgeGym1FWarpEffect_AshPuff(struct Task *task, struct ObjectEvent *objEvent, struct Sprite *sprite)
{
    if (!ObjectEventClearHeldMovementIfFinished(objEvent))
        return FALSE;

    if (task->tTimer > 3)
    {
        gFieldEffectArguments[0] = objEvent->currentCoords.x;
        gFieldEffectArguments[1] = objEvent->currentCoords.y;
        gFieldEffectArguments[2] = sprite->subpriority - 1;
        gFieldEffectArguments[3] = sprite->oam.priority;
        task->tSpriteId = FieldEffectStart(FLDEFF_POP_OUT_OF_ASH);
        task->tState = LAVARIDGE_1F_WARP_DISAPPEAR;
    }
    else
    {
        task->tTimer++;
        ObjectEventSetHeldMovement(objEvent, GetWalkInPlaceFasterMovementAction(objEvent->facingDirection));
        PlaySE(SE_LAVARIDGE_FALL_WARP);
    }

    return FALSE;
}

static bool32 LavaridgeGym1FWarpEffect_Disappear(struct Task *task, struct ObjectEvent *objEvent, struct Sprite *sprite)
{
    if (gSprites[task->tSpriteId].animCmdIndex != 2)
        return FALSE;

    objEvent->invisible = TRUE;
    task->tState = LAVARIDGE_1F_WARP_FADE_OUT;

    return FALSE;
}

static bool32 LavaridgeGym1FWarpEffect_FadeOut(struct Task *task, struct ObjectEvent *objEvent, struct Sprite *sprite)
{
    if (FieldEffectActiveListContains(FLDEFF_POP_OUT_OF_ASH))
        return FALSE;

    TryFadeOutOldMapMusic();
    WarpFadeOutScreen();
    task->tState = LAVARIDGE_1F_WARP_WARP;

    return FALSE;
}

static bool32 LavaridgeGym1FWarpEffect_Warp(struct Task *task, struct ObjectEvent *objEvent, struct Sprite *sprite)
{
    if (gPaletteFade.active || !BGMusicStopped())
        return FALSE;

    WarpIntoMap();
    gFieldCallback = FieldCB_FallWarpExit;
    SetMainCallback2(CB2_LoadMap);
    DestroyTask(FindTaskIdByFunc(Task_LavaridgeGym1FWarp));

    return FALSE;
}

#undef tState
#undef tTimer
#undef tSpriteId

u32 FldEff_PopOutOfAsh(void)
{
    u8 spriteId;

    FieldEffectScript_LoadFadedPal(&gSpritePalette_Ash);
    SetSpritePosToOffsetMapCoords((s16 *)&gFieldEffectArguments[0], (s16 *)&gFieldEffectArguments[1], 8, 8);
    spriteId = CreateSpriteAtEnd(&gFieldEffectObjectTemplate_AshPuff, gFieldEffectArguments[0], gFieldEffectArguments[1], gFieldEffectArguments[2]);
    gSprites[spriteId].oam.priority = gFieldEffectArguments[3];
    gSprites[spriteId].coordOffsetEnabled = TRUE;

    return spriteId;
}

void SpriteCB_PopOutOfAsh(struct Sprite *sprite)
{
    if (sprite->animEnded)
        FieldEffectStop(sprite, FLDEFF_POP_OUT_OF_ASH);
}

// Task data for Task_EscapeRopeWarpOut
#define tState        data[0]
#define tSpinDelay    data[1]
#define tNumTurns     data[2]
#define tTimer        data[3]
#define tOffscreen    data[4]
#define tMovingState  data[5]
#define tOffsetY      data[6]
#define tHideFollower data[7]
#define tDirection    data[15]

enum
{
    START_MOVEMENT,
    WAIT_MOVEMENT_END
};

enum EscapeRopeWarpOutState
{
    ESCAPE_ROPE_WARP_OUT_INIT,
    ESCAPE_ROPE_WARP_OUT_HIDE_FOLLOWER,
    ESCAPE_ROPE_WARP_OUT_SPIN,
};

static void (*const sEscapeRopeWarpOutEffectFuncs[])(struct Task *task) =
{
    [ESCAPE_ROPE_WARP_OUT_INIT]          = EscapeRopeWarpOutEffect_Init,
    [ESCAPE_ROPE_WARP_OUT_HIDE_FOLLOWER] = EscapeRopeWarpOutEffect_HideFollowerNPC,
    [ESCAPE_ROPE_WARP_OUT_SPIN]          = EscapeRopeWarpOutEffect_Spin
};

void StartEscapeRopeFieldEffect(void)
{
    LockPlayerFieldControls();
    FreezeObjectEvents();
    HideFollowerForFieldEffect(); // hide follower before warping
    EndORASDowsing();
    CreateTask(Task_EscapeRopeWarpOut, 80);
}

static void Task_EscapeRopeWarpOut(u8 taskId)
{
    sEscapeRopeWarpOutEffectFuncs[gTasks[taskId].tState](&gTasks[taskId]);
}

static void EscapeRopeWarpOutEffect_Init(struct Task *task)
{
    if (PlayerHasFollowerNPC())
        task->tState = ESCAPE_ROPE_WARP_OUT_HIDE_FOLLOWER;
    else
        task->tState = ESCAPE_ROPE_WARP_OUT_SPIN;

    task->data[13] = 64; // unused
    task->data[14] = GetPlayerFacingDirection(); // unused
    task->tDirection = DIR_NONE;
}

static void EscapeRopeWarpOutEffect_HideFollowerNPC(struct Task *task)
{
    struct ObjectEvent *follower = &gObjectEvents[GetFollowerNPCObjectId()];
    if (task->tHideFollower == START_MOVEMENT)
    {
        if (!PlayerHasFollowerNPC())
        {
            task->tState = ESCAPE_ROPE_WARP_OUT_SPIN;
            return;
        }

        FollowerNPCWalkIntoPlayerForLeaveMap();
        task->tHideFollower = WAIT_MOVEMENT_END;
    }

    if (task->tHideFollower == WAIT_MOVEMENT_END)
    {
        if (!ObjectEventClearHeldMovementIfFinished(follower))
            return;

        FollowerNPCHideForLeaveMap(follower);
        task->tState = ESCAPE_ROPE_WARP_OUT_SPIN;
    }
}

static void EscapeRopeWarpOutEffect_Spin(struct Task *task)
{
    struct ObjectEvent *playerObj = &gObjectEvents[gPlayerAvatar.objectEventId];

    SpinObjectEvent(playerObj, &task->tSpinDelay, &task->tNumTurns);
    if (task->tTimer < 60)
    {
        task->tTimer++;
        if (task->tTimer == 20)
            PlaySE(SE_WARP_IN);
    }
    else if (task->tOffscreen == FALSE && !WarpOutObjectEventUpwards(playerObj, &task->tMovingState, &task->tOffsetY))
    {
        TryFadeOutOldMapMusic();
        WarpFadeOutScreen();
        task->tOffscreen = TRUE;
    }

    if (task->tOffscreen == TRUE && !gPaletteFade.active && BGMusicStopped() == TRUE)
    {
        SetObjectEventDirection(playerObj, task->tDirection); // always DIR_NONE
        SetWarpDestinationToEscapeWarp();
        WarpIntoMap();
        gFieldCallback = FieldCallback_EscapeRopeExit;
        SetMainCallback2(CB2_LoadMap);
        DestroyTask(FindTaskIdByFunc(Task_EscapeRopeWarpOut));
    }
}

static const enum Direction sSpinDirections[] =
{
    [DIR_NONE]  = DIR_SOUTH,
    [DIR_SOUTH] = DIR_WEST,
    [DIR_NORTH] = DIR_EAST,
    [DIR_WEST]  = DIR_NORTH,
    [DIR_EAST]  = DIR_SOUTH,
};

static u8 SpinObjectEvent(struct ObjectEvent *playerObj, s16 *spinDelay, s16 *numTurns)
{
    if (!ObjectEventIsMovementOverridden(playerObj) || ObjectEventClearHeldMovementIfFinished(playerObj))
    {
        if (*spinDelay != 0 && --(*spinDelay) != 0)
            return playerObj->facingDirection;
        ObjectEventSetHeldMovement(playerObj, GetFaceDirectionMovementAction(sSpinDirections[playerObj->facingDirection]));
        if (*numTurns < 12)
            (*numTurns)++;
        *spinDelay = 12 >> (*numTurns);
        return sSpinDirections[playerObj->facingDirection];
    }
    return playerObj->facingDirection;
}

static bool32 WarpOutObjectEventUpwards(struct ObjectEvent *playerObj, s16 *movingState, s16 *offsetY)
{
    struct Sprite *sprite = &gSprites[playerObj->spriteId];
    switch (*movingState)
    {
    case 0:
        CameraObjectFreeze();
        (*movingState)++;
        // fallthrough
    case 1:
        sprite->y2 -= 8;
        (*offsetY) -= 8;
        if (*offsetY <= -16)
        {
            playerObj->fixedPriority = TRUE;
            sprite->oam.priority = 1;
            sprite->subpriority = 0;
            sprite->subspriteMode = SUBSPRITES_OFF;
            (*movingState)++;
        }
        break;
    case 2:
        sprite->y2 -= 8;
        (*offsetY) -= 8;
        if (*offsetY <= -88)
        {
            (*movingState)++;
            return FALSE;
        }
        break;
    case 3:
        return FALSE;
    }
    return TRUE;
}

#undef tState
#undef tSpinDelay
#undef tNumTurns
#undef tTimer
#undef tOffscreen
#undef tMovingState
#undef tOffsetY
#undef tDirection


enum EscapeRopeWarpInState
{
    ESCAPE_ROPE_WARP_IN_INIT,
    ESCAPE_ROPE_WARP_IN_SPIN,
};

enum EscapeRopeWarpFollowerState
{
    ESCAPE_ROPE_FOLLOWER_WAIT_FOR_PLAYER,
    ESCAPE_ROPE_FOLLOWER_REAPPEAR,
    ESCAPE_ROPE_FOLLOWER_FACE_PLAYER,
    ESCAPE_ROPE_FOLLOWER_END,
};

// Task data for Task_EscapeRopeWarpIn
#define tState         data[0]
#define tMovingState   data[1]
#define tOffsetY       data[2]
#define tPriority      data[3]
#define tSubpriority   data[4]
#define tSubspriteMode data[5]
#define tTimer         data[6]
#define tSpinEnded     data[7]
#define tCurrentDir    data[8]
#define tSpinDelay     data[9]
#define tNumTurns      data[10]
#define tFollowerState data[11]
#define tStartDir   data[15]


static void (*const sEscapeRopeWarpInEffectFuncs[])(struct Task *task) =
{
    [ESCAPE_ROPE_WARP_IN_INIT] = EscapeRopeWarpInEffect_Init,
    [ESCAPE_ROPE_WARP_IN_SPIN] = EscapeRopeWarpInEffect_Spin
};

static bool32 WarpInObjectEventDownwards(struct ObjectEvent *playerObj, s16 *movingState, s16 *offsetY, s16 *priority, s16 *subpriority, s16 *subspriteMode)
{
    struct Sprite *sprite = &gSprites[playerObj->spriteId];
    switch (*movingState)
    {
    case 0:
        CameraObjectFreeze();
        *offsetY = -88;
        sprite->y2 -= 88;
        *priority = sprite->oam.priority;
        *subpriority = sprite->subpriority;
        *subspriteMode = sprite->subspriteMode;
        playerObj->fixedPriority = TRUE;
        sprite->oam.priority = 1;
        sprite->subpriority = 0;
        sprite->subspriteMode = SUBSPRITES_OFF;
        (*movingState)++;
        // fallthrough
    case 1:
        sprite->y2 += 4;
        (*offsetY) += 4;
        if (*offsetY >= -16)
        {
            sprite->oam.priority = *priority;
            sprite->subpriority = *subpriority;
            sprite->subspriteMode = *subspriteMode;
            (*movingState)++;
        }
        break;
    case 2:
        sprite->y2 += 4;
        (*offsetY) += 4;
        if (*offsetY >= 0)
        {
            PlaySE(SE_CLICK);
            CameraObjectReset();
            (*movingState)++;
            return FALSE;
        }
        break;
    case 3:
        return FALSE;
    }
    return TRUE;
}

static void FieldCallback_EscapeRopeExit(void)
{
    Overworld_PlaySpecialMapMusic();
    WarpFadeInScreen();
    QuestLog_DrawPreviouslyOnQuestHeaderIfInPlaybackMode();
    LockPlayerFieldControls();
    FreezeObjectEvents();
    gFieldCallback = NULL;
    gObjectEvents[gPlayerAvatar.objectEventId].invisible = TRUE;
    CreateTask(Task_EscapeRopeWarpIn, 0);
}

static void Task_EscapeRopeWarpIn(u8 taskId)
{
    sEscapeRopeWarpInEffectFuncs[gTasks[taskId].tState](&gTasks[taskId]);
}

static void EscapeRopeWarpInEffect_Init(struct Task *task)
{
    if (!IsWeatherNotFadingIn())
        return;

    PlaySE(SE_WARP_OUT);
    task->tStartDir = GetPlayerFacingDirection();
    task->tState = ESCAPE_ROPE_WARP_IN_SPIN;
    task->tFollowerState = ESCAPE_ROPE_FOLLOWER_WAIT_FOR_PLAYER;

}

static void EscapeRopeWarpInEffect_Spin(struct Task *task)
{
    struct ObjectEvent *playerObj = &gObjectEvents[gPlayerAvatar.objectEventId];
    struct ObjectEvent *followerObj = &gObjectEvents[GetFollowerNPCObjectId()];
    bool32 moving = WarpInObjectEventDownwards(playerObj, &task->tMovingState, &task->tOffsetY, &task->tPriority, &task->tSubpriority, &task->tSubspriteMode);

    playerObj->invisible = FALSE;
    if (task->tTimer < 8)
    {
        task->tTimer++;
    }
    else if (task->tSpinEnded == FALSE)
    {
        task->tTimer++;
        task->tCurrentDir = SpinObjectEvent(playerObj, &task->tSpinDelay, &task->tNumTurns);
        if (task->tTimer >= 50 && task->tCurrentDir == task->tStartDir)
            task->tSpinEnded = TRUE;
    }

    if (!moving && task->tCurrentDir == task->tStartDir && ObjectEventCheckHeldMovementStatus(playerObj) == TRUE && task->tFollowerState == ESCAPE_ROPE_FOLLOWER_WAIT_FOR_PLAYER)
        task->tFollowerState = ESCAPE_ROPE_FOLLOWER_REAPPEAR;

    if (task->tFollowerState == ESCAPE_ROPE_FOLLOWER_REAPPEAR)
    {
        if (FNPC_NPC_FOLLOWER_SHOW_AFTER_LEAVE_ROUTE)
            FollowerNPCReappearAfterLeaveMap(followerObj, playerObj);

        task->tFollowerState = ESCAPE_ROPE_FOLLOWER_FACE_PLAYER;
    }

    if (task->tFollowerState == ESCAPE_ROPE_FOLLOWER_FACE_PLAYER)
    {
        if (PlayerHasFollowerNPC() && ObjectEventClearHeldMovementIfFinished(followerObj))
        {
            if (FNPC_NPC_FOLLOWER_SHOW_AFTER_LEAVE_ROUTE)
                FollowerNPCFaceAfterLeaveMap();

            task->tFollowerState = ESCAPE_ROPE_FOLLOWER_END;
        }
        else if (!PlayerHasFollowerNPC())
        {
            task->tFollowerState = ESCAPE_ROPE_FOLLOWER_END;
        }
    }

    if (task->tFollowerState == ESCAPE_ROPE_FOLLOWER_END)
    {
        playerObj->invisible = FALSE;
        playerObj->fixedPriority = FALSE;
        UnlockPlayerFieldControls();
        UnfreezeObjectEvents();
        DestroyTask(FindTaskIdByFunc(Task_EscapeRopeWarpIn));
    }
}

#undef tState
#undef tMovingState
#undef tOffsetY
#undef tPriority
#undef tSubpriority
#undef tSubspriteMode
#undef tTimer
#undef tSpinEnded
#undef tCurrentDir
#undef tSpinDelay
#undef tNumTurns
#undef tFollowerState
#undef tStartDir

enum TeleportWarpOutState
{
    TELEPORT_WARP_OUT_INIT,
    TELEPORT_WARP_OUT_SPIN_GROUND,
    TELEPORT_WARP_OUT_SPIN_EXIT,
    TELEPORT_WARP_OUT_END,
};

static void (*const sTeleportWarpOutFieldEffectFuncs[])(struct Task *) =
{
    [TELEPORT_WARP_OUT_INIT]        = TeleportWarpOutFieldEffect_Init,
    [TELEPORT_WARP_OUT_SPIN_GROUND] = TeleportWarpOutFieldEffect_SpinGround,
    [TELEPORT_WARP_OUT_SPIN_EXIT]   = TeleportWarpOutFieldEffect_SpinExit,
    [TELEPORT_WARP_OUT_END]         = TeleportWarpOutFieldEffect_End
};

#define tState        data[0]
#define tSpinTimer    data[1]
#define tSpinCounter  data[2]
#define tIncTimer     data[2]
#define tYIncrement   data[3]
#define tTotalYChange data[4]
#define tStartDir     data[15]

void CreateTeleportFieldEffectTask(void)
{
    CreateTask(Task_DoTeleportFieldEffect, 0);
}

static void Task_DoTeleportFieldEffect(u8 taskId)
{
    sTeleportWarpOutFieldEffectFuncs[gTasks[taskId].tState](&gTasks[taskId]);
}

static void TeleportWarpOutFieldEffect_Init(struct Task *task)
{
    LockPlayerFieldControls();
    FreezeObjectEvents();
    CameraObjectFreeze();
    EndORASDowsing();
    task->tStartDir = GetPlayerFacingDirection();
    task->tState = TELEPORT_WARP_OUT_SPIN_GROUND;
}

static void TeleportWarpOutFieldEffect_SpinGround(struct Task *task)
{
    struct ObjectEvent * objectEvent = &gObjectEvents[gPlayerAvatar.objectEventId];
    if (task->tSpinTimer == 0 || (--task->tSpinTimer) == 0)
    {
        ObjectEventTurn(objectEvent, sSpinDirections[objectEvent->facingDirection]);
        task->tSpinTimer = 8;
        task->tSpinCounter++;
    }

    if (task->tSpinCounter > 7 && task->tStartDir == objectEvent->facingDirection)
    {
        task->tState = TELEPORT_WARP_OUT_SPIN_EXIT;
        task->tSpinTimer = 4;
        task->data[2] = 8;
        task->tYIncrement = 1;
        PlaySE(SE_WARP_IN);
    }
}

static void TeleportWarpOutFieldEffect_SpinExit(struct Task *task)
{
    struct ObjectEvent * objectEvent = &gObjectEvents[gPlayerAvatar.objectEventId];
    struct Sprite *sprite = &gSprites[gPlayerAvatar.spriteId];
    if ((--task->tSpinTimer) <= 0)
    {
        task->tSpinTimer = 4;
        ObjectEventTurn(objectEvent, sSpinDirections[objectEvent->facingDirection]);
    }
    sprite->y -= task->tYIncrement;
    task->tTotalYChange += task->tYIncrement;
    if ((--task->tIncTimer) <= 0)
    {
        task->tIncTimer = 4;
        if ((task->tYIncrement < 8))
            task->tYIncrement <<= 1;
    }

    if (task->tTotalYChange > 8 && (sprite->oam.priority = 1, sprite->subspriteMode != SUBSPRITES_OFF))
        sprite->subspriteMode = SUBSPRITES_IGNORE_PRIORITY;

    if (task->tTotalYChange >= 168)
    {
        task->tState = TELEPORT_WARP_OUT_END;
        TryFadeOutOldMapMusic();
        WarpFadeOutScreen();
    }
}

static void TeleportWarpOutFieldEffect_End(struct Task *task)
{
    if (gPaletteFade.active || !BGMusicStopped())
        return;

    SetWarpDestinationToLastHealLocation();
    WarpIntoMap();
    SetMainCallback2(CB2_LoadMap);
    gFieldCallback = FieldCallback_TeleportWarpIn;
    DestroyTask(FindTaskIdByFunc(Task_DoTeleportFieldEffect));
}

#undef tState
#undef tSpinTimer
#undef tSpinCounter
#undef tIncTimer
#undef tYIncrement
#undef tTotalYChange
#undef tStartDir

enum TeleportWarpInState
{
    TELEPORT_WARP_IN_INIT,
    TELEPORT_WARP_IN_SPIN_ENTER,
    TELEPORT_WARP_IN_SPIN_GROUND,
};

enum TeleportWarpFollowerState
{
    TELEPORT_FOLLOWER_WAIT_FOR_PLAYER,
    TELEPORT_FOLLOWER_FACE_PLAYER,
    TELEPORT_FOLLOWER_END,
};

static void (*const sTeleportWarpInFieldEffectFuncs[])(struct Task *) =
{
    [TELEPORT_WARP_IN_INIT]        = TeleportWarpInFieldEffect_Init,
    [TELEPORT_WARP_IN_SPIN_ENTER]  = TeleportWarpInFieldEffect_SpinEnter,
    [TELEPORT_WARP_IN_SPIN_GROUND] = TeleportWarpInFieldEffect_SpinGround,
};

static void FieldCallback_TeleportWarpIn(void)
{
    Overworld_PlaySpecialMapMusic();
    WarpFadeInScreen();
    QuestLog_DrawPreviouslyOnQuestHeaderIfInPlaybackMode();
    LockPlayerFieldControls();
    FreezeObjectEvents();
    gFieldCallback = NULL;
    gObjectEvents[gPlayerAvatar.objectEventId].invisible = TRUE;
    CameraObjectFreeze();
    CreateTask(Task_TeleportWarpIn, 0);
}

#define tState         data[0]
#define tFallOffset    data[1]
#define tSpinTimer2    data[1] // reused
#define tSpinTimer     data[2]
#define tSpinCount     data[2] // reused
#define tFollowerState data[3]
#define tSetTrigger    data[13]
#define tSubsprMode    data[14]
#define tStartDir      data[15]

static void Task_TeleportWarpIn(u8 taskId)
{
    sTeleportWarpInFieldEffectFuncs[gTasks[taskId].tState](&gTasks[taskId]);
}

static void TeleportWarpInFieldEffect_Init(struct Task *task)
{
    struct Sprite *sprite;
    s16 centerToCornerVecY;
    if (IsWeatherNotFadingIn())
    {
        sprite = &gSprites[gPlayerAvatar.spriteId];
        centerToCornerVecY = -(sprite->centerToCornerVecY << 1);
        sprite->y2 = -(sprite->y + sprite->centerToCornerVecY + gSpriteCoordOffsetY + centerToCornerVecY);
        gObjectEvents[gPlayerAvatar.objectEventId].invisible = FALSE;
        task->tState = TELEPORT_WARP_IN_SPIN_ENTER;
        task->tFallOffset = 8;
        task->tSpinTimer = 1;
        task->tSubsprMode = sprite->subspriteMode;
        task->tStartDir = GetPlayerFacingDirection();
        PlaySE(SE_WARP_IN);
    }
}

static void TeleportWarpInFieldEffect_SpinEnter(struct Task *task)
{
    struct ObjectEvent * objectEvent = &gObjectEvents[gPlayerAvatar.objectEventId];
    struct Sprite *sprite = &gSprites[gPlayerAvatar.spriteId];

    sprite->y2 += task->tFallOffset;
    if (sprite->y2 >= -8)
    {
        if (!task->tSetTrigger)
        {
            task->tSetTrigger = TRUE;
            objectEvent->triggerGroundEffectsOnMove = TRUE;
            sprite->subspriteMode = task->tSubsprMode;
        }
    }
    else
    {
        sprite->oam.priority = 1;
        if (sprite->subspriteMode != SUBSPRITES_OFF)
            sprite->subspriteMode = SUBSPRITES_IGNORE_PRIORITY;
    }

    if (sprite->y2 >= -48 && task->tFallOffset > 1 && !(sprite->y2 & 1))
        task->tFallOffset--;

    if ((--task->tSpinTimer) == 0)
    {
        task->tSpinTimer = 4;
        ObjectEventTurn(objectEvent, sSpinDirections[objectEvent->facingDirection]);
    }

    if (sprite->y2 >= 0)
    {
        sprite->y2 = 0;
        task->tState = TELEPORT_WARP_IN_SPIN_GROUND;
        task->tSpinTimer2 = 1;
        task->tSpinCount = 0;
        task->tFollowerState = TELEPORT_FOLLOWER_WAIT_FOR_PLAYER;
    }
}

static void TeleportWarpInFieldEffect_SpinGround(struct Task *task)
{
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    struct ObjectEvent *follower = &gObjectEvents[GetFollowerNPCObjectId()];

    if ((--task->tSpinTimer2) == 0 && task->tFollowerState == TELEPORT_FOLLOWER_WAIT_FOR_PLAYER)
    {
        ObjectEventTurn(player, sSpinDirections[player->facingDirection]);
        task->tSpinTimer2 = 8;
        if ((++task->tSpinCount) > 4 && task->tSubsprMode == player->facingDirection)
        {
            if (FNPC_NPC_FOLLOWER_SHOW_AFTER_LEAVE_ROUTE)
                FollowerNPCReappearAfterLeaveMap(follower, player);

            task->tFollowerState = TELEPORT_FOLLOWER_FACE_PLAYER;
        }
    }

    if (task->tFollowerState == TELEPORT_FOLLOWER_FACE_PLAYER)
    {
        if (PlayerHasFollowerNPC() && ObjectEventClearHeldMovementIfFinished(follower))
        {
            if (FNPC_NPC_FOLLOWER_SHOW_AFTER_LEAVE_ROUTE)
                FollowerNPCFaceAfterLeaveMap();

            task->tFollowerState = TELEPORT_FOLLOWER_END;
        }
        else if (!PlayerHasFollowerNPC())
        {
            task->tFollowerState = TELEPORT_FOLLOWER_END;
        }
    }

    if (task->tFollowerState == TELEPORT_FOLLOWER_END)
    {
        UnlockPlayerFieldControls();
        CameraObjectReset();
        UnfreezeObjectEvents();
        DestroyTask(FindTaskIdByFunc(Task_TeleportWarpIn));
    }
}

#undef tState
#undef tFallOffset
#undef tSpinTimer2
#undef tSpinTimer
#undef tSpinCount
#undef tFollowerState
#undef tSetTrigger
#undef tSubsprMode
#undef tStartDir

enum ShowMonOutdoorsState
{
    SHOW_MON_OUTDOORS_INIT,
    SHOW_MON_OUTDOORS_LOAD_GFX,
    SHOW_MON_OUTDOORS_CREATE_BANNER,
    SHOW_MON_OUTDOORS_WAIT_FOR_MON,
    SHOW_MON_OUTDOORS_SHRINK_BANNER,
    SHOW_MON_OUTDOORS_RESTORE_BG,
    SHOW_MON_OUTDOORS_END,
};

static void (*const sFieldMoveShowMonOutdoorsEffectFuncs[])(struct Task *task) =
{
    [SHOW_MON_OUTDOORS_INIT]          = FieldMoveShowMonOutdoorsEffect_Init,
    [SHOW_MON_OUTDOORS_LOAD_GFX]      = FieldMoveShowMonOutdoorsEffect_LoadGfx,
    [SHOW_MON_OUTDOORS_CREATE_BANNER] = FieldMoveShowMonOutdoorsEffect_CreateBanner,
    [SHOW_MON_OUTDOORS_WAIT_FOR_MON]  = FieldMoveShowMonOutdoorsEffect_WaitForMon,
    [SHOW_MON_OUTDOORS_SHRINK_BANNER] = FieldMoveShowMonOutdoorsEffect_ShrinkBanner,
    [SHOW_MON_OUTDOORS_RESTORE_BG]    = FieldMoveShowMonOutdoorsEffect_RestoreBg,
    [SHOW_MON_OUTDOORS_END]           = FieldMoveShowMonOutdoorsEffect_End,
};

// task data
#define tState        data[0]
#define tWinHoriz     data[1]
#define tWinVert      data[2]
#define tWinIn        data[3]
#define tWinOut       data[4]
#define tBgHoriz      data[5]
#define tBgVert       data[6]
#define tWinInBackup  data[11]
#define tWinOutBackup data[12]
#define tCallback1    data[13]
#define tCallback2    data[14] // used indirectly
#define tMonSpriteId  data[15]

// sprite data
#define sSpecies   data[0]
#define sTimer     data[1]
#define sNoDucking data[6]
#define sSlideDone data[7]

u32 FldEff_FieldMoveShowMon(void)
{
    u8 taskId;

    if (IsMapTypeOutdoors(GetCurrentMapType()) == TRUE)
        taskId = CreateTask(Task_FieldMoveShowMonOutdoors, 0xFF);
    else
        taskId = CreateTask(Task_FieldMoveShowMonIndoors, 0xFF);

    gTasks[taskId].tMonSpriteId = InitFieldMoveMonSprite(gFieldEffectArguments[0], gFieldEffectArguments[1], gFieldEffectArguments[2]);
    return 0;
}


u32 FldEff_FieldMoveShowMonInit(void)
{
    u32 noDucking = gFieldEffectArguments[0] & SHOW_MON_CRY_NO_DUCKING;

    if (gFieldEffectArguments[0] & SHOW_MON_NOT_IN_PARTY)
    {
        u32 species = gFieldEffectArguments[0] & (~SHOW_MON_NOT_IN_PARTY);

        gFieldEffectArguments[0] = species;
        gFieldEffectArguments[1] = FALSE;
        gFieldEffectArguments[2] = SHINY_ODDS;
    }
    else
    {
        u8 partyIdx = gFieldEffectArguments[0];

        gFieldEffectArguments[0] = GetMonData(&gPlayerParty[partyIdx], MON_DATA_SPECIES);
        gFieldEffectArguments[1] = GetMonData(&gPlayerParty[partyIdx], MON_DATA_IS_SHINY, NULL);
        gFieldEffectArguments[2] = GetMonData(&gPlayerParty[partyIdx], MON_DATA_PERSONALITY);
    }
    gFieldEffectArguments[0] |= noDucking;
    FieldEffectStart(FLDEFF_FIELD_MOVE_SHOW_MON);
    FieldEffectActiveListRemove(FLDEFF_FIELD_MOVE_SHOW_MON_INIT);
    return 0;
}

static void Task_FieldMoveShowMonOutdoors(u8 taskId)
{
    sFieldMoveShowMonOutdoorsEffectFuncs[gTasks[taskId].tState](&gTasks[taskId]);
}

static void FieldMoveShowMonOutdoorsEffect_Init(struct Task *task)
{
    task->tWinInBackup = GetGpuReg(REG_OFFSET_WININ);
    task->tWinOutBackup = GetGpuReg(REG_OFFSET_WINOUT);
    StoreWordInTwoHalfwords((u16 *)&task->tCallback1, (u32)gMain.vblankCallback);
    task->tWinHoriz = WIN_RANGE(240, 241);
    task->tWinVert = WIN_RANGE(80, 81);
    task->tWinIn = WININ_WIN0_BG_ALL | WININ_WIN0_OBJ | WININ_WIN0_CLR;
    task->tWinOut = WINOUT_WIN01_BG1 | WINOUT_WIN01_BG2 | WINOUT_WIN01_BG3 | WINOUT_WIN01_OBJ | WINOUT_WIN01_CLR;
    SetGpuReg(REG_OFFSET_WIN0H, task->tWinHoriz);
    SetGpuReg(REG_OFFSET_WIN0V, task->tWinVert);
    SetGpuReg(REG_OFFSET_WININ, task->tWinIn);
    SetGpuReg(REG_OFFSET_WINOUT, task->tWinOut);
    SetVBlankCallback(VBlankCB_ShowMonEffect_Outdoors);
    task->tState = SHOW_MON_OUTDOORS_LOAD_GFX;
}

static void FieldMoveShowMonOutdoorsEffect_LoadGfx(struct Task *task)
{
    u16 charbase = ((GetGpuReg(REG_OFFSET_BG0CNT) >> 2) << 14);
    u16 screenbase = ((GetGpuReg(REG_OFFSET_BG0CNT) >> 8) << 11);

    CpuCopy16(sFieldMoveStreaksOutdoors_Gfx, (void *)(VRAM + charbase), sizeof(sFieldMoveStreaksOutdoors_Gfx));
    CpuFill32(0, (void *)(VRAM + screenbase), 0x800);
    LoadPalette(sFieldMoveStreaksOutdoors_Pal, BG_PLTT_ID(15), sizeof(sFieldMoveStreaksOutdoors_Pal));
    LoadFieldMoveOutdoorStreaksTilemap(screenbase);
    task->tState = SHOW_MON_OUTDOORS_CREATE_BANNER;
}

static void FieldMoveShowMonOutdoorsEffect_CreateBanner(struct Task *task)
{
    s16 win0h_lo;
    s16 win0v_lo;
    s16 win0v_hi;

    task->tBgHoriz -= 16;
    win0h_lo = ((u16)task->tWinHoriz >> 8);
    win0v_lo = ((u16)task->tWinVert >> 8);
    win0v_hi = ((u16)task->tWinVert & 0xff);
    win0h_lo -= 16;
    win0v_lo -= 2;
    win0v_hi += 2;
    if (win0h_lo < 0)
        win0h_lo = 0;

    if (win0v_lo < 40)
        win0v_lo = 40;

    if (win0v_hi > 120)
        win0v_hi = 120;

    task->tWinHoriz = WIN_RANGE(win0h_lo, task->tWinHoriz & 0xFF);
    task->tWinVert = WIN_RANGE(win0v_lo, win0v_hi);

    if (win0h_lo == 0 && win0v_lo == 40 && win0v_hi == 120)
    {
        gSprites[task->tMonSpriteId].callback = SpriteCB_FieldMoveMonSlideOnscreen;
        task->tState = SHOW_MON_OUTDOORS_WAIT_FOR_MON;
    }
}

static void FieldMoveShowMonOutdoorsEffect_WaitForMon(struct Task *task)
{
    task->tBgHoriz -= 16;
    if (gSprites[task->tMonSpriteId].sSlideDone)
        task->tState = SHOW_MON_OUTDOORS_SHRINK_BANNER;
}

static void FieldMoveShowMonOutdoorsEffect_ShrinkBanner(struct Task *task)
{
    s16 win0v_lo;
    s16 win0v_hi;

    task->tBgHoriz -= 16;
    win0v_lo = (task->tWinVert >> 8);
    win0v_hi = (task->tWinVert & 0xFF);
    win0v_lo += 6;
    win0v_hi -= 6;
    if (win0v_lo > 80)
        win0v_lo = 80;

    if (win0v_hi < 81)
        win0v_hi = 81;

    task->tWinVert = WIN_RANGE(win0v_lo, win0v_hi);
    if (win0v_lo == 80 && win0v_hi == 81)
        task->tState = SHOW_MON_OUTDOORS_RESTORE_BG;
}

static void FieldMoveShowMonOutdoorsEffect_RestoreBg(struct Task *task)
{
    u16 bg0cnt = (GetGpuReg(REG_OFFSET_BG0CNT) >> 8) << 11;

    CpuFill32(0, (void *)VRAM + bg0cnt, 0x800);
    task->tWinHoriz = WIN_RANGE(0, 241);
    task->tWinVert = WIN_RANGE(0, 161);
    task->tWinIn = task->tWinInBackup;
    task->tWinOut = task->tWinOutBackup;
    task->tState = SHOW_MON_OUTDOORS_END;
}

static void FieldMoveShowMonOutdoorsEffect_End(struct Task *task)
{
    IntrCallback callback;
    LoadWordFromTwoHalfwords((u16 *)&task->tCallback1, (u32 *)&callback);
    SetVBlankCallback(callback);
    ChangeBgX(0, 0, 0);
    ChangeBgY(0, 0, 0);
    Menu_LoadStdPal();
    FreeResourcesAndDestroySprite(&gSprites[task->tMonSpriteId], task->tMonSpriteId);
    FieldEffectActiveListRemove(FLDEFF_FIELD_MOVE_SHOW_MON);
    DestroyTask(FindTaskIdByFunc(Task_FieldMoveShowMonOutdoors));
}

static void VBlankCB_ShowMonEffect_Outdoors(void)
{
    IntrCallback callback;
    struct Task *task = &gTasks[FindTaskIdByFunc(Task_FieldMoveShowMonOutdoors)];
    LoadWordFromTwoHalfwords((u16 *)&task->tCallback1, (u32 *)&callback);
    callback();
    SetGpuReg(REG_OFFSET_WIN0H, task->tWinHoriz);
    SetGpuReg(REG_OFFSET_WIN0V, task->tWinVert);
    SetGpuReg(REG_OFFSET_WININ, task->tWinIn);
    SetGpuReg(REG_OFFSET_WINOUT, task->tWinOut);
    SetGpuReg(REG_OFFSET_BG0HOFS, task->tBgHoriz);
    SetGpuReg(REG_OFFSET_BG0VOFS, task->tBgVert);
}

static void LoadFieldMoveOutdoorStreaksTilemap(u16 screenbase)
{
    u16 i;
    u16 *dest = (u16 *)(VRAM + ARRAY_COUNT(sFieldMoveStreaksOutdoors_Tilemap) + screenbase);

    for (i = 0; i < ARRAY_COUNT(sFieldMoveStreaksOutdoors_Tilemap); i++, dest++)
        *dest = sFieldMoveStreaksOutdoors_Tilemap[i] | 0xF000;
}

// task data
#undef tState
#undef tWinHoriz
#undef tWinVert
#undef tWinIn
#undef tWinOut
#undef tBgHoriz
#undef tBgVert
#undef tWinInBackup
#undef tWinOutBackup
#undef tCallback1
#undef tCallback2
#undef tMonSpriteId

enum ShowMonIndoorsState
{
    SHOW_MON_INDOORS_INIT,
    SHOW_MON_INDOORS_LOAD_GFX,
    SHOW_MON_INDOORS_SLIDE_BANNER_ON,
    SHOW_MON_INDOORS_WAIT_FOR_MON,
    SHOW_MON_INDOORS_RESTORE_BG,
    SHOW_MON_INDOORS_SLIDE_BANNER_OFF,
    SHOW_MON_INDOORS_END,
};

static void (*const sFieldMoveShowMonIndoorsEffectFuncs[])(struct Task *) =
{
    [SHOW_MON_INDOORS_INIT]             = FieldMoveShowMonIndoorsEffect_Init,
    [SHOW_MON_INDOORS_LOAD_GFX]         = FieldMoveShowMonIndoorsEffect_LoadGfx,
    [SHOW_MON_INDOORS_SLIDE_BANNER_ON]  = FieldMoveShowMonIndoorsEffect_SlideBannerOn,
    [SHOW_MON_INDOORS_WAIT_FOR_MON]     = FieldMoveShowMonIndoorsEffect_WaitForMon,
    [SHOW_MON_INDOORS_RESTORE_BG]       = FieldMoveShowMonIndoorsEffect_RestoreBg,
    [SHOW_MON_INDOORS_SLIDE_BANNER_OFF] = FieldMoveShowMonIndoorsEffect_SlideBannerOff,
    [SHOW_MON_INDOORS_END]              = FieldMoveShowMonIndoorsEffect_End,
};

#define tState       data[0]
#define tBgHoriz     data[1]
#define tBgVert      data[2]
#define tBgOffsetIdx data[3]
#define tBgOffset    data[4]
#define tWinInBackup data[5]
#define tScreenBase  data[12]
#define tCallback1   data[13]
#define tCallback2   data[14] // used indirectly
#define tMonSprite   data[15]

static void Task_FieldMoveShowMonIndoors(u8 taskId)
{
    sFieldMoveShowMonIndoorsEffectFuncs[gTasks[taskId].tState](&gTasks[taskId]);
}

static void FieldMoveShowMonIndoorsEffect_Init(struct Task *task)
{
    SetGpuReg(REG_OFFSET_BG0HOFS, task->tBgHoriz);
    SetGpuReg(REG_OFFSET_BG0VOFS, task->tBgVert);
    StoreWordInTwoHalfwords((u16 *)&task->tCallback1, (u32)gMain.vblankCallback);
    SetVBlankCallback(VBlankCB_ShowMonEffect_Indoors);
    task->tState = SHOW_MON_INDOORS_LOAD_GFX;
}

static void FieldMoveShowMonIndoorsEffect_LoadGfx(struct Task *task)
{
    u16 charbase = ((GetGpuReg(REG_OFFSET_BG0CNT) >> 2) << 14);
    u16 screenbase = ((GetGpuReg(REG_OFFSET_BG0CNT) >> 8) << 11);

    task->tScreenBase = screenbase;
    CpuCopy16(sFieldMoveStreaksIndoors_Gfx, (void *)(VRAM + charbase), sizeof(sFieldMoveStreaksIndoors_Gfx));
    CpuFill32(0, (void *)(VRAM + screenbase), 0x800);
    LoadPalette(sFieldMoveStreaksIndoors_Pal, BG_PLTT_ID(15), sizeof(sFieldMoveStreaksIndoors_Pal));
    task->tState = SHOW_MON_INDOORS_SLIDE_BANNER_ON;
}

static void FieldMoveShowMonIndoorsEffect_SlideBannerOn(struct Task *task)
{
    if (SlideIndoorBannerOnscreen(task))
    {
        task->tWinInBackup = GetGpuReg(REG_OFFSET_WININ);
        SetGpuReg(REG_OFFSET_WININ, (task->tWinInBackup & 0xFF) | WININ_WIN1_BG0 | WININ_WIN1_OBJ);
        SetGpuReg(REG_OFFSET_WIN1H, WIN_RANGE(0, 240));
        SetGpuReg(REG_OFFSET_WIN1V, WIN_RANGE(40, 120));
        gSprites[task->tMonSprite].callback = SpriteCB_FieldMoveMonSlideOnscreen;
        task->tState = SHOW_MON_INDOORS_WAIT_FOR_MON;
    }
    AnimateIndoorShowMonBg(task);
}

static void FieldMoveShowMonIndoorsEffect_WaitForMon(struct Task *task)
{
    AnimateIndoorShowMonBg(task);

    if (gSprites[task->tMonSprite].sSlideDone)
        task->tState = SHOW_MON_INDOORS_RESTORE_BG;
}

static void FieldMoveShowMonIndoorsEffect_RestoreBg(struct Task *task)
{
    AnimateIndoorShowMonBg(task);
    task->tBgOffsetIdx = task->tBgHoriz & 7;
    task->tBgOffset = 0;
    SetGpuReg(REG_OFFSET_WIN1H, WIN_RANGE(0xFF, 0xFF));
    SetGpuReg(REG_OFFSET_WIN1V, WIN_RANGE(0xFF, 0xFF));
    SetGpuReg(REG_OFFSET_WININ, task->tWinInBackup);
    task->tState = SHOW_MON_INDOORS_SLIDE_BANNER_OFF;
}

static void FieldMoveShowMonIndoorsEffect_SlideBannerOff(struct Task *task)
{
    AnimateIndoorShowMonBg(task);

    if (SlideIndoorBannerOffscreen(task))
        task->tState = SHOW_MON_INDOORS_END;
}

static void FieldMoveShowMonIndoorsEffect_End(struct Task *task)
{
    IntrCallback intrCallback;
    u16 charbase = (GetGpuReg(REG_OFFSET_BG0CNT) >> 8) << 11;

    CpuFill32(0, (void *)VRAM + charbase, 0x800);
    LoadWordFromTwoHalfwords((u16 *)&task->tCallback1, (u32 *)&intrCallback);
    SetVBlankCallback(intrCallback);
    ChangeBgX(0, 0, 0);
    ChangeBgY(0, 0, 0);
    Menu_LoadStdPal();
    FreeResourcesAndDestroySprite(&gSprites[task->tMonSprite], task->tMonSprite);
    FieldEffectActiveListRemove(FLDEFF_FIELD_MOVE_SHOW_MON);
    DestroyTask(FindTaskIdByFunc(Task_FieldMoveShowMonIndoors));
}

static void VBlankCB_ShowMonEffect_Indoors(void)
{
    IntrCallback intrCallback;
    struct Task *task;
    task = &gTasks[FindTaskIdByFunc(Task_FieldMoveShowMonIndoors)];
    LoadWordFromTwoHalfwords((u16 *)&task->tCallback1, (u32 *)&intrCallback);
    intrCallback();
    SetGpuReg(REG_OFFSET_BG0HOFS, task->tBgHoriz);
    SetGpuReg(REG_OFFSET_BG0VOFS, task->tBgVert);
}

static void AnimateIndoorShowMonBg(struct Task *task)
{
    task->tBgHoriz -= 16;
    task->tBgOffsetIdx += 16;
}

static bool8 SlideIndoorBannerOnscreen(struct Task *task)
{
    u16 i;
    u16 srcOffs;
    u16 dstOffs;
    u16 *dest;

    if (task->tBgOffset >= 32)
        return TRUE;

    dstOffs = (task->tBgOffsetIdx >> 3) & 0x1F;
    if (dstOffs >= task->tBgOffset)
    {
        dstOffs = (32 - dstOffs) & 0x1F;
        srcOffs = (32 - task->tBgOffset) & 0x1F;
        dest = (u16 *)(VRAM + 0x140 + (u16)task->tScreenBase);
        for (i = 0; i < 10; i++)
        {
            dest[dstOffs + i * 32] = sFieldMoveStreaksIndoors_Tilemap[srcOffs + i * 32];
            dest[dstOffs + i * 32] |= 0xF000;

            dest[((dstOffs + 1) & 0x1F) + i * 32] = sFieldMoveStreaksIndoors_Tilemap[((srcOffs + 1) & 0x1F) + i * 32] | 0xF000;
            dest[((dstOffs + 1) & 0x1F) + i * 32] |= 0xF000;
        }
        task->tBgOffset += 2;
    }
    return FALSE;
}

static bool8 SlideIndoorBannerOffscreen(struct Task *task)
{
    u16 i;
    u16 dstOffs;
    u16 *dest;
    if (task->tBgOffset >= 32)
    {
        return TRUE;
    }
    dstOffs = task->tBgOffsetIdx >> 3;
    if (dstOffs >= task->tBgOffset)
    {
        dstOffs = (task->tBgHoriz >> 3) & 0x1F;
        dest = (u16 *)(VRAM + 0x140 + (u16)task->tScreenBase);
        for (i = 0; i < 10; i++)
        {
            dest[dstOffs + i * 32] = 0xF000;
            dest[((dstOffs + 1) & 0x1F) + i * 32] = 0xF000;
        }
        task->tBgOffset += 2;
    }
    return FALSE;
}

#undef tState
#undef tBgHoriz
#undef tBgVert
#undef tBgOffsetIdx
#undef tBgOffset
#undef tWinInBackup
#undef tScreenBase
#undef tCallback1
#undef tCallback2
#undef tMonSprite

static u8 InitFieldMoveMonSprite(u32 species, bool32 isShiny, u32 personality)
{
    bool16 playCry = (species & SHOW_MON_CRY_NO_DUCKING) >> 16;
    u8 monSpriteId;
    struct Sprite *sprite;

    species &= ~SHOW_MON_CRY_NO_DUCKING;
    monSpriteId = CreateMonSprite_FieldMove(species, isShiny, personality, 320, 80, 0);
    sprite = &gSprites[monSpriteId];
    sprite->callback = SpriteCallbackDummy;
    sprite->oam.priority = 0;
    sprite->sSpecies = species;
    sprite->sNoDucking = playCry;

    return monSpriteId;
}

static void SpriteCB_FieldMoveMonSlideOnscreen(struct Sprite *sprite)
{
    sprite->x -= 20;
    if (sprite->x <= 120)
    {
        sprite->x = 120;
        sprite->sTimer = 30;
        sprite->callback = SpriteCB_FieldMoveMonWaitAfterCry;
        if (sprite->sNoDucking)
            PlayCry_NormalNoDucking(sprite->sSpecies, 0, CRY_VOLUME_RS, CRY_PRIORITY_NORMAL);
        else
            PlayCry_Normal(sprite->sSpecies, 0);
    }
}

static void SpriteCB_FieldMoveMonWaitAfterCry(struct Sprite *sprite)
{
    if ((--sprite->sTimer) == 0)
        sprite->callback = SpriteCB_FieldMoveMonSlideOffscreen;
}

static void SpriteCB_FieldMoveMonSlideOffscreen(struct Sprite *sprite)
{
    if (sprite->x < -64)
        sprite->sSlideDone = TRUE;
    else
        sprite->x -= 20;
}

// sprite data
#undef sSpecies
#undef sTimer
#undef sNoDucking
#undef sSlideDone

enum SurfState
{
    SURF_INIT,
    SURF_FIELD_MOVE_POSE,
    SURF_SHOW_MON,
    SURF_JUMP_ON_SURF_BLOB,
    SURF_END,
};

static void (*const sSurfFieldEffectFuncs[])(struct Task *) = {
    [SURF_INIT]              = SurfFieldEffect_Init,
    [SURF_FIELD_MOVE_POSE]   = SurfFieldEffect_FieldMovePose,
    [SURF_SHOW_MON]          = SurfFieldEffect_ShowMon,
    [SURF_JUMP_ON_SURF_BLOB] = SurfFieldEffect_JumpOnSurfBlob,
    [SURF_END]               = SurfFieldEffect_End,
};

#define tState data[0]
#define tDestX data[1]
#define tDestY data[2]
#define tMonId data[15]

u32 FldEff_UseSurf(void)
{
    u8 taskId = CreateTask(Task_SurfFieldEffect, 0xFF);
    gTasks[taskId].tMonId = gFieldEffectArguments[0];
    Overworld_ClearSavedMusic();
    if (Overworld_MusicCanOverrideMapMusic(MUS_SURF))
        Overworld_ChangeMusicTo(MUS_SURF);

    return FALSE;
}

static void Task_SurfFieldEffect(u8 taskId)
{
    sSurfFieldEffectFuncs[gTasks[taskId].tState](&gTasks[taskId]);
}

static void SurfFieldEffect_Init(struct Task *task)
{
    LockPlayerFieldControls();
    FreezeObjectEvents();
    // Put follower into pokeball before using Surf
    HideFollowerForFieldEffect();
    gPlayerAvatar.preventStep = TRUE;
    SetPlayerAvatarStateMask(PLAYER_AVATAR_FLAG_SURFING);
    PlayerGetDestCoords(&task->tDestX, &task->tDestY);
    MoveCoords(gObjectEvents[gPlayerAvatar.objectEventId].movementDirection, &task->tDestX, &task->tDestY);
    task->tState = SURF_FIELD_MOVE_POSE;
}

static void SurfFieldEffect_FieldMovePose(struct Task *task)
{
    struct ObjectEvent *objectEvent = &gObjectEvents[gPlayerAvatar.objectEventId];

    if (!ObjectEventIsMovementOverridden(objectEvent) || ObjectEventClearHeldMovementIfFinished(objectEvent))
    {
        StartPlayerAvatarSummonMonForFieldMoveAnim();
        ObjectEventSetHeldMovement(objectEvent, MOVEMENT_ACTION_START_ANIM_IN_DIRECTION);
        task->tState = SURF_SHOW_MON;
    }
}

static void SurfFieldEffect_ShowMon(struct Task *task)
{
    struct ObjectEvent *objectEvent = &gObjectEvents[gPlayerAvatar.objectEventId];

    if (ObjectEventCheckHeldMovementStatus(objectEvent))
    {
        gFieldEffectArguments[0] = task->tMonId | SHOW_MON_CRY_NO_DUCKING;
        FieldEffectStart(FLDEFF_FIELD_MOVE_SHOW_MON_INIT);
        task->tState = SURF_JUMP_ON_SURF_BLOB;
    }
}

static void SurfFieldEffect_JumpOnSurfBlob(struct Task *task)
{
    struct ObjectEvent *objectEvent;

    if (FieldEffectActiveListContains(FLDEFF_FIELD_MOVE_SHOW_MON))
        return;

    objectEvent = &gObjectEvents[gPlayerAvatar.objectEventId];
    ObjectEventSetGraphicsId(objectEvent, GetPlayerAvatarGraphicsIdByStateId(PLAYER_AVATAR_STATE_SURFING));
    ObjectEventClearHeldMovementIfFinished(objectEvent);
    ObjectEventSetHeldMovement(objectEvent, GetJumpSpecialMovementAction(objectEvent->movementDirection));
    FollowerNPC_FollowerToWater();

    gFieldEffectArguments[0] = task->tDestX;
    gFieldEffectArguments[1] = task->tDestY;
    gFieldEffectArguments[2] = gPlayerAvatar.objectEventId;
    objectEvent->fieldEffectSpriteId = FieldEffectStart(FLDEFF_SURF_BLOB);
    task->tState = SURF_END;
}

static void SurfFieldEffect_End(struct Task *task)
{
    struct ObjectEvent *objectEvent = &gObjectEvents[gPlayerAvatar.objectEventId];
    struct ObjectEvent *followerObject = GetFollowerObject();

    if (ObjectEventClearHeldMovementIfFinished(objectEvent))
    {
        gPlayerAvatar.preventStep = FALSE;
        gPlayerAvatar.flags &= ~PLAYER_AVATAR_FLAG_CONTROLLABLE;
        ObjectEventSetHeldMovement(objectEvent, GetFaceDirectionMovementAction(objectEvent->movementDirection));
        if (followerObject)
            ObjectEventClearHeldMovementIfFinished(followerObject);

        SetSurfBlob_BobState(objectEvent->fieldEffectSpriteId, BOB_PLAYER_AND_MON);
        UnfreezeObjectEvents();
        UnlockPlayerFieldControls();
        FieldEffectActiveListRemove(FLDEFF_USE_SURF);
        DestroyTask(FindTaskIdByFunc(Task_SurfFieldEffect));
        SetHelpContext(HELPCONTEXT_SURFING);
    }
}

#undef tState
#undef tDestX
#undef tDestY
#undef tMonId

enum VsSeekerEffectState
{
    VS_SEEKER_STOP_PLAYER_MOVEMENT,
    VS_SEEKER_DO_PLAYER_ANIMATION,
    VS_SEEKER_RESET_PLAYER_GRAPHICS,
    VS_SEEKER_END,
};

static void (*const sUseVsSeekerEffectFuncs[])(struct Task *task) =
{
    [VS_SEEKER_STOP_PLAYER_MOVEMENT]  = UseVsSeeker_StopPlayerMovement,
    [VS_SEEKER_DO_PLAYER_ANIMATION]   = UseVsSeeker_DoPlayerAnimation,
    [VS_SEEKER_RESET_PLAYER_GRAPHICS] = UseVsSeeker_ResetPlayerGraphics,
    [VS_SEEKER_END]                   = UseVsSeeker_End,
};

#define tState data[0]

u32 FldEff_UseVsSeeker(void)
{
    if (gQuestLogState == QL_STATE_RECORDING)
        QuestLogRecordPlayerAvatarGfxTransitionWithDuration(QL_PLAYER_GFX_VSSEEKER, 89);

    CreateTask(Task_FldEffUseVsSeeker, 0xFF);

    return 0;
}

static void Task_FldEffUseVsSeeker(u8 taskId)
{
    sUseVsSeekerEffectFuncs[gTasks[taskId].tState](&gTasks[taskId]);
}

static void UseVsSeeker_StopPlayerMovement(struct Task *task)
{
    LockPlayerFieldControls();
    FreezeObjectEvents();
    gPlayerAvatar.preventStep = TRUE;
    task->tState = VS_SEEKER_DO_PLAYER_ANIMATION;
}

static void UseVsSeeker_DoPlayerAnimation(struct Task *task)
{
    struct ObjectEvent *playerObj = &gObjectEvents[gPlayerAvatar.objectEventId];

    if (!ObjectEventIsMovementOverridden(playerObj) || ObjectEventClearHeldMovementIfFinished(playerObj))
    {
        StartPlayerAvatarVsSeekerAnim();
        ObjectEventSetHeldMovement(playerObj, MOVEMENT_ACTION_START_ANIM_IN_DIRECTION);
        task->tState = VS_SEEKER_RESET_PLAYER_GRAPHICS;
    }
}

static void UseVsSeeker_ResetPlayerGraphics(struct Task *task)
{
    struct ObjectEvent *playerObj = &gObjectEvents[gPlayerAvatar.objectEventId];

    if (!ObjectEventClearHeldMovementIfFinished(playerObj))
        return;

    if (gPlayerAvatar.flags & (PLAYER_AVATAR_FLAG_ACRO_BIKE | PLAYER_AVATAR_FLAG_MACH_BIKE))
        ObjectEventSetGraphicsId(playerObj, GetPlayerAvatarGraphicsIdByStateId(PLAYER_AVATAR_STATE_MACH_BIKE));
    else if (gPlayerAvatar.flags & PLAYER_AVATAR_FLAG_SURFING)
        ObjectEventSetGraphicsId(playerObj, GetPlayerAvatarGraphicsIdByStateId(PLAYER_AVATAR_STATE_SURFING));
    else
        ObjectEventSetGraphicsId(playerObj, GetPlayerAvatarGraphicsIdByStateId(PLAYER_AVATAR_STATE_NORMAL));

    ObjectEventForceSetHeldMovement(playerObj, GetFaceDirectionMovementAction(playerObj->facingDirection));
    task->tState = VS_SEEKER_END;
}

static void UseVsSeeker_End(struct Task *task)
{
    struct ObjectEvent *playerObj = &gObjectEvents[gPlayerAvatar.objectEventId];

    if (!ObjectEventClearHeldMovementIfFinished(playerObj))
        return;

    gPlayerAvatar.preventStep = FALSE;
    FieldEffectActiveListRemove(FLDEFF_USE_VS_SEEKER);
    DestroyTask(FindTaskIdByFunc(Task_FldEffUseVsSeeker));
}

#undef tState

#define sNpcSpriteId data[1]
#define sTime        data[2]

u32 FldEff_NPCFlyOut(void)
{
    u8 spriteId = CreateSprite(&gFieldEffectObjectTemplate_Bird, 120, 0, 1);
    struct Sprite *sprite = &gSprites[spriteId];

    sprite->oam.paletteNum = LoadPlayerObjectEventPalette(gSaveBlock2Ptr->playerGender);
    sprite->oam.priority = 1;
    sprite->callback = SpriteCB_NPCFlyOut;
    sprite->sNpcSpriteId = gFieldEffectArguments[0];
    PlaySE(SE_M_FLY);

    return spriteId;
}

static void SpriteCB_NPCFlyOut(struct Sprite *sprite)
{
    sprite->x2 = Cos(sprite->sTime, 140);
    sprite->y2 = Sin(sprite->sTime, 72);
    sprite->sTime = (sprite->sTime + 4) & 0xFF;

    if (sprite->data[0])
    {
        struct Sprite *npcSprite = &gSprites[sprite->sNpcSpriteId];
        npcSprite->coordOffsetEnabled = FALSE;
        npcSprite->x = sprite->x + sprite->x2;
        npcSprite->y = sprite->y + sprite->y2 - 8;
        npcSprite->x2 = 0;
        npcSprite->y2 = 0;
    }

    if (sprite->sTime >= 128)
        FieldEffectStop(sprite, FLDEFF_NPCFLY_OUT);
}

enum FlyOutEffectState
{
    FLY_OUT_FIELD_MOVE_POSE,
    FLY_OUT_SHOW_MON,
    FLY_OUT_BIRD_LEAVE_BALL,
    FLY_OUT_WAIT_BIRD_LEAVE,
    FLY_OUT_BIRD_SWOOP_DOWN,
    FLY_OUT_JUMP_ON_BIRD,
    FLY_OUT_FLY_OFF_WITH_BIRD,
    FLY_OUT_WAIT_FLY_OFF,
    FLY_OUT_END,
};

static void (*const sFlyOutFieldEffectFuncs[])(struct Task *) =
{
    [FLY_OUT_FIELD_MOVE_POSE]   = FlyOutFieldEffect_FieldMovePose,
    [FLY_OUT_SHOW_MON]          = FlyOutFieldEffect_ShowMon,
    [FLY_OUT_BIRD_LEAVE_BALL]   = FlyOutFieldEffect_BirdLeaveBall,
    [FLY_OUT_WAIT_BIRD_LEAVE]   = FlyOutFieldEffect_WaitBirdLeave,
    [FLY_OUT_BIRD_SWOOP_DOWN]   = FlyOutFieldEffect_BirdSwoopDown,
    [FLY_OUT_JUMP_ON_BIRD]      = FlyOutFieldEffect_JumpOnBird,
    [FLY_OUT_FLY_OFF_WITH_BIRD] = FlyOutFieldEffect_FlyOffWithBird,
    [FLY_OUT_WAIT_FLY_OFF]      = FlyOutFieldEffect_WaitFlyOff,
    [FLY_OUT_END]               = FlyOutFieldEffect_End,
};

// Task data for Task_FlyOut
#define tState        data[0]
#define tMonPartyId   data[1]
#define tBirdSpriteId data[1] // re-used
#define tTimer        data[2]
#define tAvatarFlags  data[15]

u32 FldEff_UseFly(void)
{
    u8 taskId = CreateTask(Task_FlyOut, 0xFE);
    gTasks[taskId].tMonPartyId = gFieldEffectArguments[0];
    return 0;
}

static void Task_FlyOut(u8 taskId)
{
    sFlyOutFieldEffectFuncs[gTasks[taskId].tState](&gTasks[taskId]);
}

static void FlyOutFieldEffect_FieldMovePose(struct Task *task)
{
    struct ObjectEvent *objectEvent = &gObjectEvents[gPlayerAvatar.objectEventId];

    if (ObjectEventIsMovementOverridden(objectEvent) && !ObjectEventClearHeldMovementIfFinished(objectEvent))
        return;

    task->tAvatarFlags = gPlayerAvatar.flags;
    gPlayerAvatar.preventStep = TRUE;
    SetPlayerAvatarStateMask(PLAYER_AVATAR_FLAG_ON_FOOT);
    StartPlayerAvatarSummonMonForFieldMoveAnim();
    ObjectEventSetHeldMovement(objectEvent, MOVEMENT_ACTION_START_ANIM_IN_DIRECTION);
    task->tState = FLY_OUT_SHOW_MON;
}

static void FlyOutFieldEffect_ShowMon(struct Task *task)
{
    struct ObjectEvent *objectEvent = &gObjectEvents[gPlayerAvatar.objectEventId];

    if (!ObjectEventClearHeldMovementIfFinished(objectEvent))
        return;

    task->tState = FLY_OUT_BIRD_LEAVE_BALL;
    gFieldEffectArguments[0] = task->tMonPartyId;
    FieldEffectStart(FLDEFF_FIELD_MOVE_SHOW_MON_INIT);
}

static void FlyOutFieldEffect_BirdLeaveBall(struct Task *task)
{
    if (FieldEffectActiveListContains(FLDEFF_FIELD_MOVE_SHOW_MON))
        return;

    if (task->tAvatarFlags & PLAYER_AVATAR_FLAG_SURFING)
    {
        struct ObjectEvent *objectEvent = &gObjectEvents[gPlayerAvatar.objectEventId];

        SetSurfBlob_BobState(objectEvent->fieldEffectSpriteId, BOB_MON_ONLY);
        SetSurfBlob_DontSyncAnim(objectEvent->fieldEffectSpriteId, FALSE);
    }
    task->tBirdSpriteId = CreateFlyBirdSprite();
    task->tState = FLY_OUT_WAIT_BIRD_LEAVE;
}

static void FlyOutFieldEffect_WaitBirdLeave(struct Task *task)
{
    if (!GetFlyBirdAnimCompleted(task->tBirdSpriteId))
        return;

    task->tState = FLY_OUT_BIRD_SWOOP_DOWN;
    task->tTimer = 16;
    SetPlayerAvatarTransitionFlags(PLAYER_AVATAR_FLAG_ON_FOOT);
    ObjectEventSetHeldMovement(&gObjectEvents[gPlayerAvatar.objectEventId], MOVEMENT_ACTION_FACE_LEFT);
}

static void FlyOutFieldEffect_BirdSwoopDown(struct Task *task)
{
    struct ObjectEvent *objectEvent = &gObjectEvents[gPlayerAvatar.objectEventId];

    if ((task->tTimer != 0 && --task->tTimer != 0) || !ObjectEventClearHeldMovementIfFinished(objectEvent))
        return;

    task->tState = FLY_OUT_JUMP_ON_BIRD;
    PlaySE(SE_M_FLY);
    StartFlyBirdSwoopDown(task->tBirdSpriteId);
}

static void FlyOutFieldEffect_JumpOnBird(struct Task *task)
{
    struct ObjectEvent *objectEvent;

    if ((++task->tTimer) < 8)
        return;

    objectEvent = &gObjectEvents[gPlayerAvatar.objectEventId];
    ObjectEventSetGraphicsId(objectEvent, GetPlayerAvatarGraphicsIdByStateId(PLAYER_AVATAR_STATE_SURFING));
    StartSpriteAnim(&gSprites[objectEvent->spriteId], ANIM_GET_ON_OFF_POKEMON_WEST);
    objectEvent->inanimate = TRUE;
    ObjectEventSetHeldMovement(objectEvent, MOVEMENT_ACTION_JUMP_IN_PLACE_LEFT);
    task->tState = FLY_OUT_FLY_OFF_WITH_BIRD;
    task->tTimer = 0;
}

static void FlyOutFieldEffect_FlyOffWithBird(struct Task *task)
{
    struct ObjectEvent *objectEvent;

    if ((++task->tTimer) < 10)
        return;

    objectEvent = &gObjectEvents[gPlayerAvatar.objectEventId];
    ObjectEventClearHeldMovementIfActive(objectEvent);
    objectEvent->inanimate = FALSE;
    objectEvent->noShadow = TRUE;
    SetFlyBirdPlayerSpriteId(task->tBirdSpriteId, objectEvent->spriteId);
    StartSpriteAnim(&gSprites[task->tBirdSpriteId], gSaveBlock2Ptr->playerGender * 2 + 1);
    DoBirdSpriteWithPlayerAffineAnim(&gSprites[task->tBirdSpriteId], 0);
    gSprites[task->tBirdSpriteId].callback = SpriteCB_FlyBirdWithPlayer;
    CameraObjectFreeze();
    task->tState = FLY_OUT_WAIT_FLY_OFF;
}

static void FlyOutFieldEffect_WaitFlyOff(struct Task *task)
{
    if (!GetFlyBirdAnimCompleted(task->tBirdSpriteId))
        return;

    WarpFadeOutScreen();
    task->tState = FLY_OUT_END;
}

static void FlyOutFieldEffect_End(struct Task *task)
{
    if (gPaletteFade.active)
        return;

        FieldEffectActiveListRemove(FLDEFF_USE_FLY);
        DestroyTask(FindTaskIdByFunc(Task_FlyOut));
}

#undef tState
#undef tMonPartyId
#undef tBirdSpriteId
#undef tTimer
#undef tAvatarFlags

static u8 CreateFlyBirdSprite(void)
{
    u8 spriteId = CreateSprite(&gFieldEffectObjectTemplate_Bird, 255, 180, 1);
    struct Sprite *sprite = &gSprites[spriteId];

    sprite->oam.paletteNum = LoadPlayerObjectEventPalette(gSaveBlock2Ptr->playerGender);
    sprite->oam.priority = 1;
    sprite->callback = SpriteCB_FlyBirdLeaveBall;

    return spriteId;
}

// Sprite data for the bird sprite
#define sInitData       data[0]
#define sTimer1         data[1]
#define sTimer2         data[2]
#define sTimer3         data[3]
#define sTimer4         data[4]
#define sPlayerSpriteId data[6]
#define sAnimCompleted  data[7]

static bool8 GetFlyBirdAnimCompleted(u8 spriteId)
{
    return gSprites[spriteId].sAnimCompleted;
}

static void StartFlyBirdSwoopDown(u8 spriteId)
{
    struct Sprite *sprite;
    sprite = &gSprites[spriteId];
    sprite->callback = SpriteCB_FlyBirdSwoopDown;
    sprite->x = 120;
    sprite->y = 0;
    sprite->x2 = 0;
    sprite->y2 = 0;
    memset(&sprite->data[0], 0, 8 * sizeof(u16) /* zero all data cells */);
    sprite->sPlayerSpriteId = MAX_SPRITES;
}

static void SetFlyBirdPlayerSpriteId(u8 flyBlobSpriteId, u8 playerSpriteId)
{
    gSprites[flyBlobSpriteId].sPlayerSpriteId = playerSpriteId;
}

static void SpriteCB_FlyBirdLeaveBall(struct Sprite *sprite)
{
    if (!sprite->sAnimCompleted)
    {
        if (!sprite->sInitData)
        {
            sprite->oam.affineMode = ST_OAM_AFFINE_DOUBLE;
            sprite->affineAnims = sAffineAnims_FlyBirdBall;
            InitSpriteAffineAnim(sprite);
            StartSpriteAffineAnim(sprite, 0);
            if (gSaveBlock2Ptr->playerGender == MALE)
                sprite->x = 128;
            else
                sprite->x = 118;
            sprite->y = -48;
            sprite->sInitData++;
            sprite->sTimer1 = 64;
            sprite->sTimer2 = 256;
        }

        sprite->sTimer1 += (sprite->sTimer2 >> 8);
        sprite->x2 = Cos(sprite->sTimer1, 120);
        sprite->y2 = Sin(sprite->sTimer1, 120);

        if (sprite->sTimer2 < 2048)
            sprite->sTimer2 += 96;

        if (sprite->sTimer1 > 129)
        {
            sprite->sAnimCompleted = TRUE;
            sprite->oam.affineMode = ST_OAM_AFFINE_OFF;
            FreeOamMatrix(sprite->oam.matrixNum);
            CalcCenterToCornerVec(sprite, sprite->oam.shape, sprite->oam.size, ST_OAM_AFFINE_OFF);
        }
    }
}

static void SpriteCB_FlyBirdSwoopDown(struct Sprite *sprite)
{
    sprite->x2 = Cos(sprite->sTimer2, 140);
    sprite->y2 = Sin(sprite->sTimer2, 72);
    sprite->sTimer2 = (sprite->sTimer2 + 4) & 0xFF;
    if (sprite->sPlayerSpriteId != MAX_SPRITES)
    {
        struct Sprite *playerSprite = &gSprites[sprite->sPlayerSpriteId];
        playerSprite->coordOffsetEnabled = FALSE;
        playerSprite->x = sprite->x + sprite->x2;
        playerSprite->y = sprite->y + sprite->y2 - 8;
        playerSprite->x2 = 0;
        playerSprite->y2 = 0;
    }

    if (sprite->sTimer2 >= 128)
        sprite->sAnimCompleted = TRUE;
}

static void SpriteCB_FlyBirdReturnToBall(struct Sprite *sprite)
{
    if (!sprite->sAnimCompleted)
    {
        if (sprite->sInitData == FALSE)
        {
            sprite->oam.affineMode = ST_OAM_AFFINE_DOUBLE;
            sprite->affineAnims = sAffineAnims_FlyBirdBall;
            InitSpriteAffineAnim(sprite);
            StartSpriteAffineAnim(sprite, 1);
            if (gSaveBlock2Ptr->playerGender == MALE)
                sprite->x = 112;
            else
                sprite->x = 100;
            sprite->y = -32;
            sprite->sInitData++;
            sprite->sTimer1 = 240;
            sprite->sTimer2 = 2048;
            sprite->sTimer4 = 128;
        }
        sprite->sTimer1 += sprite->sTimer2 >> 8;
        sprite->sTimer3 += sprite->sTimer2 >> 8;
        sprite->sTimer1 &= 0xFF;
        sprite->x2 = Cos(sprite->sTimer1, 32);
        sprite->y2 = Sin(sprite->sTimer1, 120);
        if (sprite->sTimer2 > 256)
            sprite->sTimer2 -= sprite->sTimer4;

        if (sprite->sTimer4 < 256)
            sprite->sTimer4 += 24;

        if (sprite->sTimer2 < 256)
            sprite->sTimer2 = 256;

        if (sprite->sTimer3 >= 60)
        {
            sprite->sAnimCompleted = TRUE;
            sprite->oam.affineMode = ST_OAM_AFFINE_OFF;
            FreeOamMatrix(sprite->oam.matrixNum);
            sprite->invisible = TRUE;
        }
    }
}

static void StartFlyBirdReturnToBall(u8 spriteId)
{
    StartFlyBirdSwoopDown(spriteId);
    gSprites[spriteId].callback = SpriteCB_FlyBirdReturnToBall;
}

enum FlyInEffectState
{
    FLY_IN_BIRD_SWOOP_DOWN,
    FLY_IN_FLY_IN_WITH_BIRD,
    FLY_IN_JUMP_OFF_BIRD,
    FLY_IN_FIELD_MOVE_POSE,
    FLY_IN_BIRD_RETURN_TO_BALL,
    FLY_IN_WAIT_BIRD_RETURN,
    FLY_IN_END,
};

static void (*const sFlyInFieldEffectFuncs[])(struct Task *task) =
{
    [FLY_IN_BIRD_SWOOP_DOWN]     = FlyInFieldEffect_BirdSwoopDown,
    [FLY_IN_FLY_IN_WITH_BIRD]    = FlyInFieldEffect_FlyInWithBird,
    [FLY_IN_JUMP_OFF_BIRD]       = FlyInFieldEffect_JumpOffBird,
    [FLY_IN_FIELD_MOVE_POSE]     = FlyInFieldEffect_FieldMovePose,
    [FLY_IN_BIRD_RETURN_TO_BALL] = FlyInFieldEffect_BirdReturnToBall,
    [FLY_IN_WAIT_BIRD_RETURN]    = FlyInFieldEffect_WaitBirdReturn,
    [FLY_IN_END]                 = FlyInFieldEffect_End,
};

// Task data for Task_FlyOut / Task_FlyIn
#define tState        data[0]
#define tMonPartyId   data[1]
#define tBirdSpriteId data[1] // re-used
#define tTimer1       data[1] // re-used
#define tTimer2       data[2]
#define tAvatarFlags  data[15]

u32 FldEff_FlyIn(void)
{
    CreateTask(Task_FlyIn, 0xFE);

    return 0;
}

static void Task_FlyIn(u8 taskId)
{
    sFlyInFieldEffectFuncs[gTasks[taskId].tState](&gTasks[taskId]);
}

static void FlyInFieldEffect_BirdSwoopDown(struct Task *task)
{
    struct ObjectEvent *playerObj = &gObjectEvents[gPlayerAvatar.objectEventId];

    if (ObjectEventIsMovementOverridden(playerObj) && !ObjectEventClearHeldMovementIfFinished(playerObj))
        return;

    task->tState = FLY_IN_FLY_IN_WITH_BIRD;
    task->tTimer2 = 33;
    task->tAvatarFlags = gPlayerAvatar.flags;
    gPlayerAvatar.preventStep = TRUE;
    SetPlayerAvatarStateMask(PLAYER_AVATAR_FLAG_ON_FOOT);
    if (task->tAvatarFlags & PLAYER_AVATAR_FLAG_SURFING)
        SetSurfBlob_BobState(playerObj->fieldEffectSpriteId, BOB_NONE);
    ObjectEventSetGraphicsId(playerObj, GetPlayerAvatarGraphicsIdByStateId(PLAYER_AVATAR_STATE_SURFING));
    CameraObjectFreeze();
    ObjectEventTurn(playerObj, DIR_WEST);
    StartSpriteAnim(&gSprites[playerObj->spriteId], ANIM_GET_ON_OFF_POKEMON_WEST);
    playerObj->invisible = FALSE;
    playerObj->noShadow = TRUE;
    task->tBirdSpriteId = CreateFlyBirdSprite();
    StartFlyBirdSwoopDown(task->tBirdSpriteId);
    SetFlyBirdPlayerSpriteId(task->tBirdSpriteId, playerObj->spriteId);
    StartSpriteAnim(&gSprites[task->tBirdSpriteId], gSaveBlock2Ptr->playerGender * 2 + 2);
    DoBirdSpriteWithPlayerAffineAnim(&gSprites[task->tBirdSpriteId], 1);
    gSprites[task->tBirdSpriteId].callback = SpriteCB_FlyBirdWithPlayer;
}

static void FlyInFieldEffect_FlyInWithBird(struct Task *task)
{
    struct ObjectEvent *playerObj;
    struct Sprite *playerSprite;

    TryChangeBirdSprite(&gSprites[task->tBirdSpriteId]);

    if (task->tTimer2 != 0 && --task->tTimer2 != 0)
        return;

    playerObj= &gObjectEvents[gPlayerAvatar.objectEventId];
    playerSprite = &gSprites[playerObj->spriteId];
    SetFlyBirdPlayerSpriteId(task->tBirdSpriteId, MAX_SPRITES);
    playerSprite->x += playerSprite->x2;
    playerSprite->y += playerSprite->y2;
    playerSprite->x2 = 0;
    playerSprite->y2 = 0;
    task->tState = FLY_IN_JUMP_OFF_BIRD;
    task->tTimer2 = 0;
}

static void FlyInFieldEffect_JumpOffBird(struct Task *task)
{
    s16 yOffsets[18] = {-2, -4, -5, -6, -7, -8, -8, -8, -7, -7, -6, -5, -3, -2, 0, 2, 4, 8};
    struct Sprite *sprite = &gSprites[gPlayerAvatar.spriteId];
    sprite->y2 = yOffsets[task->tTimer2];

    if ((++task->tTimer2) >= 18)
        task->tState = FLY_IN_FIELD_MOVE_POSE;
}

static void FlyInFieldEffect_FieldMovePose(struct Task *task)
{
    struct ObjectEvent *playerObj;
    struct Sprite *playerSprite;

    if (!GetFlyBirdAnimCompleted(task->tBirdSpriteId))
        return;

    playerObj= &gObjectEvents[gPlayerAvatar.objectEventId];
    playerSprite = &gSprites[playerObj->spriteId];
    playerObj->inanimate = FALSE;
    MoveObjectEventToMapCoords(playerObj, playerObj->currentCoords.x, playerObj->currentCoords.y);
    playerSprite->x2 = 0;
    playerSprite->y2 = 0;
    playerSprite->coordOffsetEnabled = TRUE;
    StartPlayerAvatarSummonMonForFieldMoveAnim();
    ObjectEventSetHeldMovement(playerObj, MOVEMENT_ACTION_START_ANIM_IN_DIRECTION);
    task->tState = FLY_IN_BIRD_RETURN_TO_BALL;
}

static void FlyInFieldEffect_BirdReturnToBall(struct Task *task)
{
    if (!ObjectEventClearHeldMovementIfFinished(&gObjectEvents[gPlayerAvatar.objectEventId]))
        return;

    task->tState = FLY_IN_WAIT_BIRD_RETURN;
    StartFlyBirdReturnToBall(task->tBirdSpriteId);
}

static void FlyInFieldEffect_WaitBirdReturn(struct Task *task)
{
    if (!GetFlyBirdAnimCompleted(task->tBirdSpriteId))
        return;

    DestroySprite(&gSprites[task->tBirdSpriteId]);
    task->tState = FLY_IN_END;
    task->tTimer1 = 16;
}

static void FlyInFieldEffect_End(struct Task *task)
{
    struct ObjectEvent *playerObj;
    u8 state;

    if ((--task->tTimer1) != 0)
        return;

    playerObj = &gObjectEvents[gPlayerAvatar.objectEventId];
    state = PLAYER_AVATAR_STATE_NORMAL;

    if (task->tAvatarFlags & PLAYER_AVATAR_FLAG_SURFING)
    {
        state = PLAYER_AVATAR_STATE_SURFING;
        SetSurfBlob_BobState(playerObj->fieldEffectSpriteId, BOB_PLAYER_AND_MON);
    }
    ObjectEventSetGraphicsId(playerObj, GetPlayerAvatarGraphicsIdByStateId(state));
    ObjectEventTurn(playerObj, DIR_SOUTH);
    gPlayerAvatar.flags = task->tAvatarFlags;
    gPlayerAvatar.preventStep = FALSE;
    FieldEffectActiveListRemove(FLDEFF_FLY_IN);
    DestroyTask(FindTaskIdByFunc(Task_FlyIn));
}

#undef tState
#undef tMonPartyId
#undef tBirdSpriteId
#undef tTimer1
#undef tTimer2
#undef tAvatarFlags

static void DoBirdSpriteWithPlayerAffineAnim(struct Sprite *sprite, u8 affineAnimId)
{
    sprite->oam.affineMode = ST_OAM_AFFINE_DOUBLE;
    sprite->affineAnims = sAffineAnims_FlyBirdWithPlayer;
    InitSpriteAffineAnim(sprite);
    StartSpriteAffineAnim(sprite, affineAnimId);
}

static void SpriteCB_FlyBirdWithPlayer(struct Sprite *sprite)
{
    sprite->x2 = Cos(sprite->sTimer2, 180);
    sprite->y2 = Sin(sprite->sTimer2, 72);
    sprite->sTimer2 = (sprite->sTimer2 + 2) & 0xFF;
    if (sprite->sPlayerSpriteId != MAX_SPRITES)
    {
        struct Sprite *playerSprite;
        playerSprite = &gSprites[sprite->sPlayerSpriteId];
        playerSprite->coordOffsetEnabled = FALSE;
        playerSprite->x = sprite->x + sprite->x2;
        playerSprite->y = sprite->y + sprite->y2 - 8;
        playerSprite->x2 = 0;
        playerSprite->y2 = 0;
    }
    if (sprite->sTimer2 >= 128)
    {
        sprite->sAnimCompleted = TRUE;
        sprite->oam.affineMode = ST_OAM_AFFINE_OFF;
        FreeOamMatrix(sprite->oam.matrixNum);
        CalcCenterToCornerVec(sprite, sprite->oam.shape, sprite->oam.size, ST_OAM_AFFINE_OFF);
    }
}

#undef sInitData
#undef sTimer1
#undef sTimer2
#undef sTimer3
#undef sTimer4
#undef sPlayerSpriteId
#undef sAnimCompleted

static void TryChangeBirdSprite(struct Sprite *sprite)
{
    if (sprite->oam.affineMode != ST_OAM_AFFINE_OFF)
    {
        if (gOamMatrices[sprite->oam.matrixNum].a == 0x100 || gOamMatrices[sprite->oam.matrixNum].d == 0x100)
        {
            sprite->oam.affineMode = ST_OAM_AFFINE_OFF;
            FreeOamMatrix(sprite->oam.matrixNum);
            CalcCenterToCornerVec(sprite, sprite->oam.shape, sprite->oam.size, ST_OAM_AFFINE_OFF);
            StartSpriteAnim(sprite, 0);
            sprite->callback = SpriteCB_FlyBirdSwoopDown;
        }
    }
}

// Task data for Task_MoveDeoxysRock
#define tState      data[0]
#define tSpriteId   data[1]
#define tTargetX    data[2]
#define tTargetY    data[3]
#define tCurX       data[4]
#define tCurY       data[5]
#define tVelocityX  data[6]
#define tVelocityY  data[7]
#define tMoveSteps  data[8]
#define tObjEventId data[9]

enum DeoxysMoveRockState
{
    DEOXYS_MOVE_ROCK_INIT,
    DEOXYS_MOVE_ROCK_MOVE,
};

u32 FldEff_MoveDeoxysRock(void)
{
    struct Task *task;
    s32 x, y;
    struct ObjectEvent *objectEvent;
    u8 taskId;
    u8 objectEventIdBuffer;

    if (TryGetObjectEventIdByLocalIdAndMap(gFieldEffectArguments[0], gFieldEffectArguments[1], gFieldEffectArguments[2], &objectEventIdBuffer))
        return FALSE;

    objectEvent = &gObjectEvents[objectEventIdBuffer];
    x = objectEvent->currentCoords.x - 7;
    y = objectEvent->currentCoords.y - 7;
    x = (gFieldEffectArguments[3] - x) * 16;
    y = (gFieldEffectArguments[4] - y) * 16;
    ShiftObjectEventCoords(objectEvent, gFieldEffectArguments[3] + 7, gFieldEffectArguments[4] + 7);

    taskId = CreateTask(Task_MoveDeoxysRock, 80);
    task = &gTasks[taskId];
    task->tSpriteId = objectEvent->spriteId;
    task->tTargetX = gSprites[objectEvent->spriteId].x + x;
    task->tTargetY = gSprites[objectEvent->spriteId].y + y;
    task->tMoveSteps = gFieldEffectArguments[5];
    task->tObjEventId = objectEventIdBuffer;

    return FALSE;
}

static void Task_MoveDeoxysRock(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    struct Sprite *sprite = &gSprites[tSpriteId];
    struct ObjectEvent * objectEvent;

    switch (tState)
    {
    case DEOXYS_MOVE_ROCK_INIT:
        tCurX = sprite->x << 4;
        tCurY = sprite->y << 4;

        // UB: Possible divide by zero
        tVelocityX = SAFE_DIV(((tTargetX << 4) - tCurX), tMoveSteps);
        tVelocityY = SAFE_DIV(((tTargetY << 4) - tCurY), tMoveSteps);
        tState = DEOXYS_MOVE_ROCK_MOVE;
        // fallthrough
    case DEOXYS_MOVE_ROCK_MOVE:
        if (tMoveSteps != 0)
        {
            tMoveSteps--;
            tCurX += tVelocityX;
            tCurY += tVelocityY;
            sprite->x = tCurX >> 4;
            sprite->y = tCurY >> 4;
        }
        else
        {
            objectEvent = &gObjectEvents[tObjEventId];
            sprite->x = tTargetX;
            sprite->y = tTargetY;
            ShiftStillObjectEventCoords(objectEvent);
            objectEvent->triggerGroundEffectsOnStop = TRUE;
            FieldEffectActiveListRemove(FLDEFF_MOVE_DEOXYS_ROCK);
            DestroyTask(taskId);
        }
        break;
    }
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

enum DeoxysDestroyRockState
{
    DEOXYS_DESTROY_ROCK_CAMERA_SHAKE,
    DEOXYS_DESTROY_ROCK_ROCK_FRAGMENTS,
    DEOXYS_DESTROY_ROCK_WAIT_AND_END,
};

static void (*const sDestroyDeoxysRockEffectFuncs[])(u8 taskId) =
{
    [DEOXYS_DESTROY_ROCK_CAMERA_SHAKE]   = DestroyDeoxysRockEffect_CameraShake,
    [DEOXYS_DESTROY_ROCK_ROCK_FRAGMENTS] = DestroyDeoxysRockEffect_RockFragments,
    [DEOXYS_DESTROY_ROCK_WAIT_AND_END]   = DestroyDeoxysRockEffect_WaitAndEnd,
};

// Task data for Task_DestroyDeoxysRock
#define tState         data[1]
#define tObjectEventId data[2]
#define tTimer         data[3]
#define tCameraTaskId  data[5]
#define tLocalId       data[6]
#define tMapNum        data[7]
#define tMapGroup      data[8]

u32 FldEff_DestroyDeoxysRock(void)
{
    u8 taskId;
    u8 objectEventId;
    if (!TryGetObjectEventIdByLocalIdAndMap(gFieldEffectArguments[0], gFieldEffectArguments[1], gFieldEffectArguments[2], &objectEventId))
    {
        taskId = CreateTask(Task_DestroyDeoxysRock, 80);
        gTasks[taskId].tObjectEventId = objectEventId;
        gTasks[taskId].tLocalId = gFieldEffectArguments[0];
        gTasks[taskId].tMapNum = gFieldEffectArguments[1];
        gTasks[taskId].tMapGroup = gFieldEffectArguments[2];
    }
    else
    {
        FieldEffectActiveListRemove(FLDEFF_DESTROY_DEOXYS_ROCK);
    }

    return FALSE;
}

static void Task_DestroyDeoxysRock(u8 taskId)
{
    struct Task *task = &gTasks[taskId];
    InstallCameraPanAheadCallback();
    SetCameraPanningCallback(NULL);
    sDestroyDeoxysRockEffectFuncs[task->tState](taskId);
}

static void DestroyDeoxysRockEffect_CameraShake(u8 taskId)
{
    struct Task *task = &gTasks[taskId];
    u8 newTaskId = CreateTask(Task_DeoxysRockCameraShake, 90);

    PlaySE(SE_THUNDER2);
    task->tCameraTaskId = newTaskId;
    task->tState = DEOXYS_DESTROY_ROCK_ROCK_FRAGMENTS;
}

static void DestroyDeoxysRockEffect_RockFragments(u8 taskId)
{
    struct Task *task = &gTasks[taskId];
    struct Sprite *sprite;

    if (++task->tTimer <= 120)
        return;

    sprite = &gSprites[gObjectEvents[task->tObjectEventId].spriteId];
    gObjectEvents[task->tObjectEventId].invisible = TRUE;
    BlendPalettes(PALETTES_BG, 16, RGB_WHITE);
    BeginNormalPaletteFade(PALETTES_BG, 0, 16, 0, RGB_WHITE);
    CreateDeoxysRockFragments(sprite);
    PlaySE(SE_THUNDER);
    StartEndingDeoxysRockCameraShake(task->tCameraTaskId);
    task->tTimer = 0;
    task->tState = DEOXYS_DESTROY_ROCK_WAIT_AND_END;
}

static void DestroyDeoxysRockEffect_WaitAndEnd(u8 taskId)
{
    struct Task *task;

    if (gPaletteFade.active || FuncIsActiveTask(Task_DeoxysRockCameraShake))
        return;

    task = &gTasks[taskId];
    InstallCameraPanAheadCallback();
    RemoveObjectEventByLocalIdAndMap(task->tLocalId, task->tMapNum, task->tMapGroup);
    FieldEffectActiveListRemove(FLDEFF_DESTROY_DEOXYS_ROCK);
    DestroyTask(taskId);
}

#undef tState
#undef tObjectEventId
#undef tTimer
#undef tCameraTaskId
#undef tLocalId
#undef tMapNum
#undef tMapGroup


// Task data for Task_DeoxysRockCameraShake
#define tShakeDelay data[0]
#define tShakeUp    data[1]
#define tShake      data[5]
#define tEndDelay   data[6]
#define tEnding     data[7]

static void Task_DeoxysRockCameraShake(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    if (tEnding)
    {
        if (++tEndDelay > 20)
        {
            tEndDelay = 0;
            if (tShake != 0)
                tShake--;
        }
    }
    else
    {
        tShake = 4;
    }

    if (++tShakeDelay > 1)
    {
        tShakeDelay = 0;
        if (++tShakeUp & 1)
            SetCameraPanning(0, -tShake);
        else
            SetCameraPanning(0, tShake);
    }
    UpdateCameraPanning();
    if (tShake == 0)
        DestroyTask(taskId);
}

static void StartEndingDeoxysRockCameraShake(u8 taskId)
{
    gTasks[taskId].tEnding = TRUE;
}

#undef tShakeDelay
#undef tShakeUp
#undef tShake
#undef tEndDelay
#undef tEnding

#define NUM_ROCK_FRAGMENTS 4

#define sRockFragmentIndex data[0]

static void CreateDeoxysRockFragments(struct Sprite *sprite)
{
    u32 i;
    s32 xPos = (s16)gTotalCameraPixelOffsetX + sprite->x + sprite->x2;
    s32 yPos = (s16)gTotalCameraPixelOffsetY + sprite->y + sprite->y2 - 4;

    for (i = 0; i < NUM_ROCK_FRAGMENTS; i++)
    {
        u8 spriteId = CreateSprite(&sSpriteTemplate_DeoxysRockFragment, xPos, yPos, 0);
        if (spriteId != MAX_SPRITES)
        {
            StartSpriteAnim(&gSprites[spriteId], i);
            gSprites[spriteId].sRockFragmentIndex = i;
            gSprites[spriteId].oam.paletteNum = sprite->oam.paletteNum;
        }
    }
}

static void SpriteCB_DeoxysRockFragment(struct Sprite *sprite)
{
    switch (sprite->sRockFragmentIndex)
    {
    case 0:
        sprite->x -= 16;
        sprite->y -= 12;
        break;
    case 1:
        sprite->x += 16;
        sprite->y -= 12;
        break;
    case 2:
        sprite->x -= 16;
        sprite->y += 12;
        break;
    case 3:
        sprite->x += 16;
        sprite->y += 12;
        break;
    }

    if (sprite->x < -4 || sprite->x > DISPLAY_WIDTH + 4 || sprite->y < -4 || sprite->y > DISPLAY_HEIGHT + 4)
        DestroySprite(sprite);
}

#undef sRockFragmentIndex

static void Task_PhotoFlash(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        FieldEffectActiveListRemove(FLDEFF_PHOTO_FLASH);
        DestroyTask(taskId);
    }
}

u32 FldEff_PhotoFlash(void)
{
    BlendPalettes(PALETTES_ALL, 16, RGB_WHITE);
    BeginNormalPaletteFade(PALETTES_ALL, -1, 15, 0, RGB_WHITE);
    CreateTask(Task_PhotoFlash, 90);

    return 0;
}

u32 FldEff_CaveDust(void)
{
    u8 spriteId;

    FieldEffectScript_LoadFadedPal(&gSpritePalette_CaveDust);
    SetSpritePosToOffsetMapCoords((s16 *)&gFieldEffectArguments[0], (s16 *)&gFieldEffectArguments[1], 8, 8);
    spriteId = CreateSpriteAtEnd(&gFieldEffectObjectTemplate_CaveDust, gFieldEffectArguments[0], gFieldEffectArguments[1], 0xFF);
    if (spriteId != MAX_SPRITES)
    {
        gSprites[spriteId].coordOffsetEnabled = TRUE;
        gSprites[spriteId].data[0] = 22;
    }

    return spriteId;
}

static u32 FldEff_Nop()
{
    return 0;
}
