#include "global.h"
#include "battle_main.h"
#include "decompress.h"
#include "type_icon_sprite.h"

static const u32 gMoveTypes_Gfx[] = INCBIN_U32("graphics/types/move_types.4bpp.smol");
static const u16 gMoveTypes_Pal1[] = INCBIN_U16("graphics/types/move_types_1.gbapal");
static const u16 gMoveTypes_Pal2[] = INCBIN_U16("graphics/types/move_types_2.gbapal");
static const u16 gMoveTypes_Pal3[] = INCBIN_U16("graphics/types/move_types_3.gbapal");

static const struct OamData sOamData_MoveTypes =
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
    .tileNum = 0,
    .priority = 1,
    .paletteNum = 0,
    .affineParam = 0,
};

static const union AnimCmd sSpriteAnim_TypeNone[] = {
    ANIMCMD_FRAME(TYPE_NONE * 8, 0, FALSE, FALSE),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_TypeNormal[] = {
    ANIMCMD_FRAME(TYPE_NORMAL * 8, 0, FALSE, FALSE),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_TypeFighting[] = {
    ANIMCMD_FRAME(TYPE_FIGHTING * 8, 0, FALSE, FALSE),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_TypeFlying[] = {
    ANIMCMD_FRAME(TYPE_FLYING * 8, 0, FALSE, FALSE),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_TypePoison[] = {
    ANIMCMD_FRAME(TYPE_POISON * 8, 0, FALSE, FALSE),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_TypeGround[] = {
    ANIMCMD_FRAME(TYPE_GROUND * 8, 0, FALSE, FALSE),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_TypeRock[] = {
    ANIMCMD_FRAME(TYPE_ROCK * 8, 0, FALSE, FALSE),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_TypeBug[] = {
    ANIMCMD_FRAME(TYPE_BUG * 8, 0, FALSE, FALSE),
    ANIMCMD_END
};
static const union AnimCmd sSpriteAnim_TypeGhost[] = {
    ANIMCMD_FRAME(TYPE_GHOST * 8, 0, FALSE, FALSE),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_TypeSteel[] = {
    ANIMCMD_FRAME(TYPE_STEEL * 8, 0, FALSE, FALSE),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_TypeMystery[] = {
    ANIMCMD_FRAME(TYPE_MYSTERY * 8, 0, FALSE, FALSE),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_TypeFire[] = {
    ANIMCMD_FRAME(TYPE_FIRE * 8, 0, FALSE, FALSE),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_TypeWater[] = {
    ANIMCMD_FRAME(TYPE_WATER * 8, 0, FALSE, FALSE),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_TypeGrass[] = {
    ANIMCMD_FRAME(TYPE_GRASS * 8, 0, FALSE, FALSE),
    ANIMCMD_END
};
static const union AnimCmd sSpriteAnim_TypeElectric[] = {
    ANIMCMD_FRAME(TYPE_ELECTRIC * 8, 0, FALSE, FALSE),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_TypePsychic[] = {
    ANIMCMD_FRAME(TYPE_PSYCHIC * 8, 0, FALSE, FALSE),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_TypeIce[] = {
    ANIMCMD_FRAME(TYPE_ICE * 8, 0, FALSE, FALSE),
    ANIMCMD_END
};
static const union AnimCmd sSpriteAnim_TypeDragon[] = {
    ANIMCMD_FRAME(TYPE_DRAGON * 8, 0, FALSE, FALSE),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_TypeDark[] = {
    ANIMCMD_FRAME(TYPE_DARK * 8, 0, FALSE, FALSE),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_TypeFairy[] = {
    ANIMCMD_FRAME(TYPE_FAIRY * 8, 0, FALSE, FALSE),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_TypeStellar[] = {
    ANIMCMD_FRAME(TYPE_STELLAR * 8, 0, FALSE, FALSE),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_CategoryCool[] = {
    ANIMCMD_FRAME((CONTEST_CATEGORY_COOL + NUMBER_OF_MON_TYPES) * 8, 0, FALSE, FALSE),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_CategoryBeauty[] = {
    ANIMCMD_FRAME((CONTEST_CATEGORY_BEAUTY + NUMBER_OF_MON_TYPES) * 8, 0, FALSE, FALSE),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_CategoryCute[] = {
    ANIMCMD_FRAME((CONTEST_CATEGORY_CUTE + NUMBER_OF_MON_TYPES) * 8, 0, FALSE, FALSE),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_CategorySmart[] = {
    ANIMCMD_FRAME((CONTEST_CATEGORY_SMART + NUMBER_OF_MON_TYPES) * 8, 0, FALSE, FALSE),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_CategoryTough[] = {
    ANIMCMD_FRAME((CONTEST_CATEGORY_TOUGH + NUMBER_OF_MON_TYPES) * 8, 0, FALSE, FALSE),
    ANIMCMD_END
};

static const union AnimCmd *const sSpriteAnimTable_MoveTypes[NUMBER_OF_MON_TYPES + CONTEST_CATEGORIES_COUNT] = {
    [TYPE_NONE] = sSpriteAnim_TypeNone,
    [TYPE_NORMAL] = sSpriteAnim_TypeNormal,
    [TYPE_FIGHTING] = sSpriteAnim_TypeFighting,
    [TYPE_FLYING] = sSpriteAnim_TypeFlying,
    [TYPE_POISON] = sSpriteAnim_TypePoison,
    [TYPE_GROUND] = sSpriteAnim_TypeGround,
    [TYPE_ROCK] = sSpriteAnim_TypeRock,
    [TYPE_BUG] = sSpriteAnim_TypeBug,
    [TYPE_GHOST] = sSpriteAnim_TypeGhost,
    [TYPE_STEEL] = sSpriteAnim_TypeSteel,
    [TYPE_MYSTERY] = sSpriteAnim_TypeMystery,
    [TYPE_FIRE] = sSpriteAnim_TypeFire,
    [TYPE_WATER] = sSpriteAnim_TypeWater,
    [TYPE_GRASS] = sSpriteAnim_TypeGrass,
    [TYPE_ELECTRIC] = sSpriteAnim_TypeElectric,
    [TYPE_PSYCHIC] = sSpriteAnim_TypePsychic,
    [TYPE_ICE] = sSpriteAnim_TypeIce,
    [TYPE_DRAGON] = sSpriteAnim_TypeDragon,
    [TYPE_DARK] = sSpriteAnim_TypeDark,
    [TYPE_FAIRY] = sSpriteAnim_TypeFairy,
    [TYPE_STELLAR] = sSpriteAnim_TypeStellar,
    [NUMBER_OF_MON_TYPES + CONTEST_CATEGORY_COOL] = sSpriteAnim_CategoryCool,
    [NUMBER_OF_MON_TYPES + CONTEST_CATEGORY_BEAUTY] = sSpriteAnim_CategoryBeauty,
    [NUMBER_OF_MON_TYPES + CONTEST_CATEGORY_CUTE] = sSpriteAnim_CategoryCute,
    [NUMBER_OF_MON_TYPES + CONTEST_CATEGORY_SMART] = sSpriteAnim_CategorySmart,
    [NUMBER_OF_MON_TYPES + CONTEST_CATEGORY_TOUGH] = sSpriteAnim_CategoryTough,
};

static const struct SpritePalette gSpritePalette_MoveTypes1 =
{
    .data = gMoveTypes_Pal1,
    .tag = TAG_MOVE_TYPES_1,
};

static const struct SpritePalette gSpritePalette_MoveTypes2 =
{
    .data = gMoveTypes_Pal2,
    .tag = TAG_MOVE_TYPES_2,
};

static const struct SpritePalette gSpritePalette_MoveTypes3 =
{
    .data = gMoveTypes_Pal3,
    .tag = TAG_MOVE_TYPES_3,
};

static const struct CompressedSpriteSheet gSpriteSheet_MoveTypes =
{
    .data = gMoveTypes_Gfx,
    .size = (NUMBER_OF_MON_TYPES + CONTEST_CATEGORIES_COUNT) * 0x100,
    .tag = TAG_MOVE_TYPES,
};

static const struct SpriteTemplate gSpriteTemplate_MoveTypes =
{
    .tileTag = TAG_MOVE_TYPES,
    .paletteTag = TAG_NONE,
    .oam = &sOamData_MoveTypes,
    .anims = sSpriteAnimTable_MoveTypes,
};

void InitTypeIconGfx(void)
{
    LoadSpritePalette(&gSpritePalette_MoveTypes1);
    LoadSpritePalette(&gSpritePalette_MoveTypes2);
    LoadSpritePalette(&gSpritePalette_MoveTypes3);
    LoadCompressedSpriteSheet(&gSpriteSheet_MoveTypes);
}

u32 CreateTypeIconSprite(void)
{
    u32 spriteId;
    struct Sprite *sprite;

    spriteId = CreateSprite(&gSpriteTemplate_MoveTypes, 0, 0, 2);
    sprite = &gSprites[spriteId];
    sprite->invisible = TRUE;

    return spriteId;
}

void ShowTypeIcon(struct Sprite *icon, enum Type type, s32 x, s32 y)
{
    icon->invisible = FALSE;
    icon->oam.paletteNum = IndexOfSpritePaletteTag(gTypesInfo[type].paletteTag);
    icon->x = x;
    icon->y = y;
    StartSpriteAnim(icon, type);
}