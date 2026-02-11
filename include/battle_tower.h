#ifndef GUARD_BATTLE_TOWER_H
#define GUARD_BATTLE_TOWER_H

#include "data.h"
#include "battle_frontier.h"

extern const u8 gTowerMaleFacilityClasses[30];
extern const u16 gTowerMaleTrainerGfxIds[30];
extern const u8 gTowerFemaleFacilityClasses[20];
extern const u16 gTowerFemaleTrainerGfxIds[20];
extern const struct TrainerMon gSlateportBattleTentMons[];
extern const struct BattleFrontierTrainer gSlateportBattleTentTrainers[];

u16 GetCurrentBattleTowerWinStreak(enum FrontierLevelMode lvlMode, u8 battleMode);

#endif //GUARD_BATTLE_TOWER_H
