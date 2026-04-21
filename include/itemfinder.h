#ifndef GUARD_ITEMFINDER_H
#define GUARD_ITEMFINDER_H

bool8 HiddenItemIsWithinRangeOfPlayer(const struct MapEvents *events, u8 taskId);
enum Direction GetPlayerDirectionTowardsHiddenItem(s16 itemX, s16 itemY);
void ItemUseOnFieldCB_Itemfinder(u8 taskId);

#endif //GUARD_ITEMFINDER_H
