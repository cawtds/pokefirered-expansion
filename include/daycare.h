#ifndef GUARD_DAYCARE_H
#define GUARD_DAYCARE_H

#include "constants/daycare.h"

u8 *GetMonNick(struct Pokemon *mon, u8 *dest);
u8 *GetBoxMonNickname(struct BoxPokemon *mon, u8 *dest);
u8 *GetBoxMonNick(struct BoxPokemon *mon, u8 *dest);
u8 CountPokemonInDaycare(struct DayCare *daycare);
void InitDaycareMailRecordMixing(struct DayCare *daycare, struct RecordMixingDayCareMail *daycareMail);
s8 Daycare_FindEmptySpot(struct DayCare *daycare);
void StoreSelectedPokemonInDaycare(void);
enum Species TakePokemonFromDaycare(void);
void GetDaycareCost(void);
u8 GetNumLevelsGainedFromDaycare(void);
void TriggerPendingDaycareEgg(void);
void RejectEggFromDayCare(void);
void CreateEgg(struct Pokemon *mon, enum Species species, bool8 setHotSpringsLocation);
void GiveEggFromDaycare(void);
bool8 DoEggActions_CheckHatch(void);
u16 GetSelectedMonNickAndSpecies(void);
void GetDaycareMonNicknames(void);
u8 GetDaycareState(void);
u8 GetDaycareCompatibilityScore(struct DayCare *daycare);
void SetDaycareCompatibilityString(void);
bool8 NameHasGenderSymbol(const u8 *name, u8 genderRatio);
void ShowDaycareLevelMenu(void);
void ChooseSendDaycareMon(void);

void ScriptHatchMon(void);
void EggHatch(void);
u8 GetEggStepsToSubtract(void);
bool8 ShouldEggHatch(void);
u8 GetEggCyclesToSubtract(void);
u8 GetEggMovesBySpecies(enum Species species, u16 *eggMoves);
void StorePokemonInDaycare(struct Pokemon *mon, struct DaycareMon *daycareMon);
u8 GetEggMoves(struct Pokemon *pokemon, u16 *eggMoves);

#endif // GUARD_DAYCARE_H
