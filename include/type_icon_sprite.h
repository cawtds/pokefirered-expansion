#ifndef GUARD_TYPE_ICON_SPRITE_H
#define GUARD_TYPE_ICON_SPRITE_H

#include "gba/types.h"

#define TAG_MOVE_TYPES   33333
#define TAG_MOVE_TYPES_1 33334
#define TAG_MOVE_TYPES_2 33335
#define TAG_MOVE_TYPES_3 33336

void InitTypeIconGfx(void);
u32 CreateTypeIconSprite(void);
void ShowTypeIcon(struct Sprite *icon, enum Type type, s32 x, s32 y);

#endif // GUARD_TYPE_ICON_SPRITE_H
