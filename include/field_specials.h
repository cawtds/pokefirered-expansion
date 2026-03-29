#ifndef GUARD_FIELD_SPECIALS_H
#define GUARD_FIELD_SPECIALS_H

#include "global.h"

extern u16 gScrollableMultichoice_ScrollOffset;

bool8 CutMoveRuinValleyCheck(void);
bool8 InMultiPartnerRoom(void);
bool8 InPokemonCenter(void);
bool8 IsDestinationBoxFull(void);
bool8 ShouldShowBoxWasFullMessage(void);
bool8 UsedPokemonCenterWarp(void);
enum Species GetStarterSpecies(void);
size_t CountDigits(s32 value);
u16 GetHiddenItemAttr(u32 hiddenItem, u8 attr);
u16 GetPCBoxToSendMon(void);
u32 GetPlayerTrainerId(void);
u8 ContextNpcGetTextColor(void);
u8 GetLeadMonIndex(void);
u8 GetUnlockedSeviiAreas(void);
void CutMoveOpenDottedHoleDoor(void);
void DoPicboxCancel(void);
void FieldCB_ShowPortholeView(void);
void FrontierGamblerSetWonOrLost(bool8 won);
void IncrementBirthIslandRockStepCount(void);
void IncrementResortGorgeousStepCounter(void);
void QuestLog_CheckDepartingIndoorsMap(void);
void QuestLog_TryRecordDepartedLocation(void);
void ResetCyclingRoadChallengeData(void);
void ResetFieldTasksArgs(void);
void RunMassageCooldownStepCounter(void);
void SetPCBoxToSendMon(u8);
void StopPokemonLeagueLightingEffectTask(void);
void TV_PrintIntToStringVar(u8 varidx, s32 number);
void UpdateFrontierGambler(u16 daysSince);
void UpdateFrontierManiac(u16 daysSince);

#endif // GUARD_FIELD_SPECIALS_H
