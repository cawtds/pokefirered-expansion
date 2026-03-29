#ifndef GUARD_TRAINER_POKEMON_SPRITES_H
#define GUARD_TRAINER_POKEMON_SPRITES_H

// For the flags argument of CreateMonPicSprite_Affine
#define MON_PIC_AFFINE_BACK   0
#define MON_PIC_AFFINE_FRONT  1
#define MON_PIC_AFFINE_NONE   3
#define F_MON_PIC_NO_AFFINE (1 << 7)

u16 CreateMonFrontPicSprite(enum Species species, bool32 isShiny, u32 personality, s16 x, s16 y, u8 paletteSlot, u16 paletteTag);
u16 CreateMonPicSprite_Affine(enum Species species, bool8 isShiny, u32 personality, u8 flags, s16 x, s16 y, u8 paletteSlot, u16 paletteTag);
u16 CreateTrainerFrontPicSprite(enum TrainerPicID trainerPicId, s16 x, s16 y, u8 paletteSlot);
u8 CreateTrainerSprite(enum TrainerPicID trainerPicId, s16 x, s16 y, u8 subpriority, u8 *buffer);
void CopyTrainerBackspriteFramesToDest(u8 trainerPicId, u8 *dest);
void FreeAndDestroyMonPicSprite(u16 spriteId);
void FreeAndDestroyMonPicSpriteNoPalette(u16 spriteId);
void FreeAndDestroyTrainerPicSprite(u16);
void LoadMonFrontPicInWindow(enum Species species, bool32 isShiny, u32 personality, u8 paletteSlot, u8 windowId);
void LoadTrainerFrontPicInWindow(enum TrainerPicID trainerPicID, u16 destX, u16 destY, u8 paletteSlot, u8 windowId);
void ResetAllPicSprites(void);

#endif // GUARD_TRAINER_POKEMON_SPRITES_H
