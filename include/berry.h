#ifndef GUARD_BERRY_H
#define GUARD_BERRY_H

bool32 BerryTreeGrow(struct BerryTree *tree);
bool32 IsEnigmaBerryValid(void);
bool32 IsPlayerFacingEmptyBerryTreePatch(void);
bool32 ObjectEventInteractionWaterBerryTree(void);
bool32 PlayerHasBerries(void);
bool32 TryToWaterBerryTree(void);
const struct BerryInfo *GetBerryInfo(enum BerryId berry);
enum BerryId GetBerryTypeByBerryTreeId(enum BerryTreeId berryTreeId);
struct BerryTree *GetBerryTreeInfo(enum BerryTreeId berryTreeId);
u8 GetStageByBerryTreeId(enum BerryTreeId berryTreeId);
void Bag_ChooseBerry(void);
void Bag_ChooseMulch(void);
void BerryTreeTimeUpdate(s32 minutes);
void ClearBerryTrees(void);
void GetBerryNameByBerryType(enum BerryId berry, u8 *string);
void ObjectEventInteractionApplyMulch(void);
void ObjectEventInteractionGetBerryTreeData(void);
void ObjectEventInteractionPickBerryTree(void);
void ObjectEventInteractionPlantBerryTree(void);
void ObjectEventInteractionRemoveBerryTree(void);
void PlantBerryTree(enum BerryTreeId berryTreeId, enum BerryId berry, u8 stage, bool8 allowGrowth);
void RemoveBerryTree(enum BerryTreeId berryTreeId);
void SetBerryTreesSeen(void);
void SetEnigmaBerry(u8 *src);

extern const struct Berry gBerries[];

#endif // GUARD_BERRY_H
