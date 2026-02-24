#ifndef GUARD_BATTLE_SPECIAL_H
#define GUARD_BATTLE_SPECIAL_H

void ClearEReaderTrainer(struct BattleTowerEReaderTrainer *);
void ValidateEReaderTrainer(void);
u8 GetEreaderTrainerFrontSpriteId(void);
void GetEreaderTrainerName(u8 *dest);
enum TrainerClassID GetEreaderTrainerClassId(void);

#endif // GUARD_BATTLE_SPECIAL_H
