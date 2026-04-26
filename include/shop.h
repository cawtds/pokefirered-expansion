#ifndef GUARD_SHOP_H
#define GUARD_SHOP_H

#include "global.h"
#include "menu_helpers.h"
#include "constants/shops.h"

#define INDEX_CANCEL -2

u8 GetMartFontId(void);
void CreateDecorationShop1Menu(const u16 *);
void CreateDecorationShop2Menu(const u16 *);
void CreatePokemartMenu(enum ShopID shopId);
void RecordItemTransaction(enum Item itemId, u16 quantity, u8 logEventId);

#endif // GUARD_SHOP_H
