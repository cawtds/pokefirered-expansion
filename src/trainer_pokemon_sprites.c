#include "global.h"
#include "data.h"
#include "decompress.h"
#include "malloc.h"
#include "palette.h"
#include "trainer_pokemon_sprites.h"
#include "window.h"

#define PICS_COUNT 8

// Needs to be large enough to store either a decompressed Pokémon pic or trainer pic
#define PIC_SPRITE_SIZE max(MON_PIC_SIZE, TRAINER_PIC_SIZE)
#define MAX_PIC_FRAMES  max(MAX_MON_PIC_FRAMES, MAX_TRAINER_PIC_FRAMES)

struct PicData
{
    u8 *frames;
    struct SpriteFrameImage *images;
    u16 paletteTag;
    u8 spriteId;
    u8 active;
};

static EWRAM_DATA struct SpriteTemplate sCreatingSpriteTemplate = {};
static EWRAM_DATA struct PicData sSpritePics[PICS_COUNT] = {};

static const struct PicData sDummyPicData = {};

static const struct OamData sOamData_Normal =
{
    .shape = SPRITE_SHAPE(64x64),
    .size = SPRITE_SIZE(64x64)
};

static const struct OamData sOamData_Affine =
{
    .affineMode = ST_OAM_AFFINE_NORMAL,
    .shape = SPRITE_SHAPE(64x64),
    .size = SPRITE_SIZE(64x64)
};

void DummyPicSpriteCallback(struct Sprite *sprite)
{

}

bool16 ResetAllPicSprites(void)
{
    int i;

    for (i = 0; i < PICS_COUNT; i ++)
        sSpritePics[i] = sDummyPicData;

    return FALSE;
}

void LoadPicPaletteByTagOrSlot(u16 species, bool32 isShiny, u32 personality, u8 paletteSlot, u16 paletteTag)
{
    if (paletteTag == TAG_NONE)
    {
        sCreatingSpriteTemplate.paletteTag = TAG_NONE;
        LoadPalette(GetMonSpritePalFromSpeciesAndPersonality(species, isShiny, personality), OBJ_PLTT_ID(paletteSlot), PLTT_SIZE_4BPP);
    }
    else
    {
        sCreatingSpriteTemplate.paletteTag = paletteTag;
        LoadSpritePaletteWithTag(GetMonSpritePalFromSpeciesAndPersonality(species, isShiny, personality), species);
    }
}

u16 CreateMonPicSprite(u16 species, bool32 isShiny, u32 personality, bool8 isFrontPic, s16 x, s16 y, u8 paletteSlot, u16 paletteTag)
{
    u8 i;
    u8 *framePics;
    struct SpriteFrameImage *images;
    int j;
    u8 spriteId;

    for (i = 0; i < PICS_COUNT; i ++)
    {
        if (!sSpritePics[i].active)
            break;
    }
    if (i == PICS_COUNT)
        return 0xFFFF;

    framePics = Alloc(PIC_SPRITE_SIZE * MAX_PIC_FRAMES);
    if (!framePics)
        return 0xFFFF;

    images = Alloc(sizeof(struct SpriteFrameImage) * MAX_PIC_FRAMES);
    if (!images)
    {
        Free(framePics);
        return 0xFFFF;
    }

    LoadSpecialPokePic(framePics, species, personality, isFrontPic);

    for (j = 0; j < MAX_PIC_FRAMES; j ++)
    {
        images[j].data = framePics + PIC_SPRITE_SIZE * j;
        images[j].size = PIC_SPRITE_SIZE;
    }
    sCreatingSpriteTemplate.tileTag = TAG_NONE;
    sCreatingSpriteTemplate.oam = &sOamData_Normal;
    sCreatingSpriteTemplate.anims = gAnims_MonPic;
    sCreatingSpriteTemplate.images = images;
    sCreatingSpriteTemplate.affineAnims = gDummySpriteAffineAnimTable;
    sCreatingSpriteTemplate.callback = DummyPicSpriteCallback;
    LoadPicPaletteByTagOrSlot(species, isShiny, personality, paletteSlot, paletteTag);
    spriteId = CreateSprite(&sCreatingSpriteTemplate, x, y, 0);
    if (paletteTag == TAG_NONE)
        gSprites[spriteId].oam.paletteNum = paletteSlot;
    sSpritePics[i].frames = framePics;
    sSpritePics[i].images = images;
    sSpritePics[i].paletteTag = paletteTag;
    sSpritePics[i].spriteId = spriteId;
    sSpritePics[i].active = TRUE;
    return spriteId;
}

u16 CreateMonPicSprite_Affine(u16 species, bool8 isShiny, u32 personality, u8 flags, s16 x, s16 y, u8 paletteSlot, u16 paletteTag)
{
    u8 *framePics;
    struct SpriteFrameImage *images;
    int j;
    u8 i;
    u8 spriteId;
    u8 type;
    species = SanitizeSpeciesId(species);

    for (i = 0; i < PICS_COUNT; i++)
    {
        if (!sSpritePics[i].active)
            break;
    }
    if (i == PICS_COUNT)
        return 0xFFFF;

    framePics = Alloc(MON_PIC_SIZE * MAX_MON_PIC_FRAMES);
    if (!framePics)
        return 0xFFFF;

    if (flags & F_MON_PIC_NO_AFFINE)
    {
        flags &= ~F_MON_PIC_NO_AFFINE;
        type = MON_PIC_AFFINE_NONE;
    }
    else
    {
        type = flags;
    }
    images = Alloc(sizeof(struct SpriteFrameImage) * MAX_MON_PIC_FRAMES);
    if (!images)
    {
        Free(framePics);
        return 0xFFFF;
    }

    LoadSpecialPokePic(framePics, species, personality, flags);

    for (j = 0; j < MAX_MON_PIC_FRAMES; j ++)
    {
        images[j].data = framePics + MON_PIC_SIZE * j;
        images[j].size = MON_PIC_SIZE;
    }
    sCreatingSpriteTemplate.tileTag = TAG_NONE;
    sCreatingSpriteTemplate.anims = gSpeciesInfo[species].frontAnimFrames;
    sCreatingSpriteTemplate.images = images;
    if (type == MON_PIC_AFFINE_FRONT)
    {
        sCreatingSpriteTemplate.affineAnims = gAffineAnims_BattleSpriteOpponentSide;
        sCreatingSpriteTemplate.oam = &sOamData_Affine;
    }
    else if (type == MON_PIC_AFFINE_BACK)
    {
        sCreatingSpriteTemplate.affineAnims = gAffineAnims_BattleSpritePlayerSide;
        sCreatingSpriteTemplate.oam = &sOamData_Affine;
    }
    else // MON_PIC_AFFINE_NONE
    {
        sCreatingSpriteTemplate.oam = &sOamData_Normal;
        sCreatingSpriteTemplate.affineAnims = gDummySpriteAffineAnimTable;
    }
    sCreatingSpriteTemplate.callback = DummyPicSpriteCallback;
    LoadPicPaletteByTagOrSlot(species, isShiny, personality, paletteSlot, paletteTag);
    spriteId = CreateSprite(&sCreatingSpriteTemplate, x, y, 0);
    if (paletteTag == TAG_NONE)
        gSprites[spriteId].oam.paletteNum = paletteSlot;
    sSpritePics[i].frames = framePics;
    sSpritePics[i].images = images;
    sSpritePics[i].paletteTag = paletteTag;
    sSpritePics[i].spriteId = spriteId;
    sSpritePics[i].active = TRUE;
    return spriteId;
}

static u16 FreeAndDestroyPicSpriteInternal(u16 spriteId, bool8 clearPalette)
{
    u8 i;
    u8 *framePics;
    struct SpriteFrameImage *images;

    for (i = 0; i < PICS_COUNT; i ++)
    {
        if (sSpritePics[i].spriteId == spriteId)
            break;
    }
    if (i == PICS_COUNT)
        return 0xFFFF;

    framePics = sSpritePics[i].frames;
    images = sSpritePics[i].images;
    if (clearPalette && sSpritePics[i].paletteTag != TAG_NONE)
        FreeSpritePaletteByTag(GetSpritePaletteTagByPaletteNum(gSprites[spriteId].oam.paletteNum));
    DestroySprite(&gSprites[spriteId]);
    Free(framePics);
    Free(images);
    sSpritePics[i] = sDummyPicData;
    return 0;
}

u16 FreeAndDestroyMonPicSprite(u16 spriteId)
{
    return FreeAndDestroyPicSpriteInternal(spriteId, TRUE);
}

u16 FreeAndDestroyMonPicSpriteNoPalette(u16 spriteId)
{
    return FreeAndDestroyPicSpriteInternal(spriteId, FALSE);
}

void LoadMonPicInWindow(u16 species, bool32 isShiny, u32 personality, bool8 isFrontPic, u8 paletteSlot, u8 windowId)
{
    u8 *framePics = Alloc(TRAINER_PIC_SIZE * MAX_TRAINER_PIC_FRAMES);

    if (!framePics)
        return;

    LoadSpecialPokePic(framePics, species, personality, isFrontPic);
    BlitBitmapRectToWindow(windowId, framePics, 0, 0, MON_PIC_WIDTH, MON_PIC_HEIGHT, 0, 0, MON_PIC_WIDTH, MON_PIC_HEIGHT);
    LoadPalette(GetMonSpritePalFromSpeciesAndPersonality(species, isShiny, personality), BG_PLTT_ID(paletteSlot), PLTT_SIZE_4BPP);
    Free(framePics);
}

void LoadTrainerPicInWindow(enum TrainerPicID trainerPicId, u16 destX, u16 destY, u8 paletteSlot, u8 windowId)
{
    u8 *framePics = Alloc(TRAINER_PIC_SIZE * MAX_TRAINER_PIC_FRAMES);

    if (!framePics)
        return;

    DecompressDataWithHeaderWram(GetTrainerFrontPicData(trainerPicId), framePics);
    BlitBitmapRectToWindow(windowId, framePics, 0, 0, TRAINER_PIC_WIDTH, TRAINER_PIC_HEIGHT, destX, destY, TRAINER_PIC_WIDTH, TRAINER_PIC_HEIGHT);
    LoadPalette(GetTrainerFrontPicPalette(trainerPicId), BG_PLTT_ID(paletteSlot), PLTT_SIZE_4BPP);
    Free(framePics);
}

u16 CreateFrontTrainerPicSprite(enum TrainerPicID trainerPicId, s16 x, s16 y, u8 paletteSlot, u16 paletteTag)
{

    u8 i;
    u8 *framePics;
    struct SpriteFrameImage *images;
    int j;
    u8 spriteId;

    for (i = 0; i < PICS_COUNT; i ++)
    {
        if (!sSpritePics[i].active)
            break;
    }
    if (i == PICS_COUNT)
        return 0xFFFF;

    framePics = Alloc(PIC_SPRITE_SIZE * MAX_PIC_FRAMES);
    if (!framePics)
        return 0xFFFF;

    images = Alloc(sizeof(struct SpriteFrameImage) * MAX_PIC_FRAMES);
    if (!images)
    {
        Free(framePics);
        return 0xFFFF;
    }
    DecompressDataWithHeaderWram(GetTrainerFrontPicData(trainerPicId), framePics);

    for (j = 0; j < MAX_PIC_FRAMES; j ++)
    {
        images[j].data = framePics + PIC_SPRITE_SIZE * j;
        images[j].size = PIC_SPRITE_SIZE;
    }

    sCreatingSpriteTemplate.tileTag = TAG_NONE;
    sCreatingSpriteTemplate.oam = &sOamData_Normal;
    sCreatingSpriteTemplate.anims = gAnims_Trainer;
    sCreatingSpriteTemplate.images = images;
    sCreatingSpriteTemplate.affineAnims = gDummySpriteAffineAnimTable;
    sCreatingSpriteTemplate.callback = DummyPicSpriteCallback;
    sCreatingSpriteTemplate.paletteTag = TAG_NONE;

    LoadPalette(GetTrainerFrontPicPalette(trainerPicId), OBJ_PLTT_ID(paletteSlot), PLTT_SIZE_4BPP);
    spriteId = CreateSprite(&sCreatingSpriteTemplate, x, y, 0);
    if (paletteTag == TAG_NONE)
        gSprites[spriteId].oam.paletteNum = paletteSlot;
    sSpritePics[i].frames = framePics;
    sSpritePics[i].images = images;
    sSpritePics[i].paletteTag = paletteTag;
    sSpritePics[i].spriteId = spriteId;
    sSpritePics[i].active = TRUE;
    return spriteId;
}

u16 FreeAndDestroyTrainerPicSprite(u16 spriteId)
{
    return FreeAndDestroyPicSpriteInternal(spriteId, TRUE);
}

void CopyTrainerBackspriteFramesToDest(u8 trainerPicId, u8 *dest)
{
    const struct SpriteFrameImage *frame = GetTrainerBackPicImage(trainerPicId);
    // y_offset is repurposed to indicates how many frames does the trainer pic have.
    u32 size = (frame->size * GetTrainerBackPicCoords(trainerPicId)->y_offset);
    CpuSmartCopy16(frame->data, dest, size);
}
