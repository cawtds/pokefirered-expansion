#ifndef GUARD_POKEMON_ICON_H
#define GUARD_POKEMON_ICON_H

#include "global.h"

// extern const u8 *const gMonIconTable[];
extern const struct SpritePalette gMonIconPaletteTable[];
extern const u16 gMonIconPalettes[][16];
// extern const u8 gMonIconPaletteIndices[];

const u8 *GetMonIconPtr(enum Species species, u32 personality);
const u8 *GetMonIconPtrIsEgg(enum Species species, u32 personality, bool32 isEgg);
const u8 *GetMonIconTiles(enum Species species, bool32 extra);
const u8 *GetMonIconTilesIsEgg(enum Species species, u32 personality, bool32 isEgg);
const u16 *GetValidMonIconPalettePtr(enum Species species);
void LoadMonIconPalettes(void);
void FreeMonIconPalettes(void);
u16 GetIconSpeciesNoPersonality(enum Species species);
u8 CreateMonIconNoPersonality(enum Species species, SpriteCallback callback, s16 x, s16 y, u8 subpriority);
void FreeAndDestroyMonIconSprite(struct Sprite *);
u16 GetUnownLetterByPersonality(u32 personality);
void SpriteCB_MonIcon(struct Sprite *);
u8 CreateMonIcon(enum Species species, SpriteCallback callback, s16 x, s16 y, u8 subpriority, u32 personality);
u8 CreateMonIconIsEgg(enum Species species, void (*callback)(struct Sprite *), s16 x, s16 y, u8 subpriority, u32 personality, bool32 isEgg);
u8 UpdateMonIconFrame(struct Sprite *sprite);
void LoadMonIconPalette(enum Species species);
void LoadMonIconPalettePersonality(enum Species species, u32 personality);
void FreeMonIconPalette(enum Species species);
void SetPartyHPBarSprite(struct Sprite *sprite, u8 animNum);
u8 GetMonIconPaletteIndexFromSpecies(enum Species species);
enum Species GetIconSpecies(enum Species species, u32 personality);
void TryLoadAllMonIconPalettesAtOffset(u16 offset);
u8 GetValidMonIconPalIndex(enum Species species);
void SafeLoadMonIconPalette(enum Species species);
void SafeFreeMonIconPalette(enum Species species);

#endif // GUARD_POKEMON_ICON_H
