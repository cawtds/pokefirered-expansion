#ifndef GUARD_SCRIPT_POKEMON_UTIL_H
#define GUARD_SCRIPT_POKEMON_UTIL_H

u32 ScriptGiveMon(enum Species species, u8 level, enum Item item);
bool8 ScriptGiveEgg(enum Species species);
void ScriptSetMonMoveSlot(u8 partyIdx, enum Move move, u8 slot);
void HealPlayerParty(void);
void ReducePlayerPartyToSelectedMons(void);
void CreateScriptedWildMon(enum Species species, u8 level, enum Item item);
void CreateScriptedDoubleWildMon(enum Species species1, u8 level1, enum Item item1, enum Species species2, u8 level2, enum Item item2);

#endif //GUARD_SCRIPT_POKEMON_UTIL_H
