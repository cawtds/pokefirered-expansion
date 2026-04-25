#ifndef GUARD_POKEDEX_AREA_MARKERS_H
#define GUARD_POKEDEX_AREA_MARKERS_H

#include "constants/rtc.h"

void GetAreaMarkerSubsprite(s32 i, s32 dexArea, struct Subsprite *subsprites);
void DestroyPokedexAreaMarkers(u8 taskId);
void DestroyPokedexAreaMarkerSprites(u8 taskId);
u8 CreatePokedexAreaMarkers(enum Species species, u16 tilesTag, u8 palIdx, u8 y, enum Season season, enum TimeOfDay timeOfDay);
u8 GetNumPokedexAreaMarkers(u8 taskId);

#endif //GUARD_POKEDEX_AREA_MARKERS_H
