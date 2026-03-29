#ifndef GUARD_POKEMON_STORAGE_SYSTEM_H
#define GUARD_POKEMON_STORAGE_SYSTEM_H

#define TOTAL_BOXES_COUNT       14
#define IN_BOX_ROWS             5 // Number of rows, 6 Pokémon per row
#define IN_BOX_COLUMNS          6 // Number of columns, 5 Pokémon per column
#define IN_BOX_COUNT            (IN_BOX_ROWS * IN_BOX_COLUMNS)
#define BOX_NAME_LENGTH         8
#define MAX_FUSION_STORAGE      4

/*
            COLUMNS
ROWS        0   1   2   3   4   5
            6   7   8   9   10  11
            12  13  14  15  16  17
            18  19  20  21  22  23
            24  25  26  27  28  29
*/

struct PokemonStorage
{
    /*0x0000*/ u8 currentBox;
    /*0x0001*/ struct BoxPokemon boxes[TOTAL_BOXES_COUNT][IN_BOX_COUNT];
    /*0x8344*/ u8 boxNames[TOTAL_BOXES_COUNT][BOX_NAME_LENGTH + 1];
    /*0x83C2*/ u8 boxWallpapers[TOTAL_BOXES_COUNT];
    /*0x8432*/ struct Pokemon fusions[MAX_FUSION_STORAGE];
};

extern struct PokemonStorage *gPokemonStoragePtr;

bool32 CheckBoxMonSanityAt(u32 boxId, u32 boxPosition);
bool8 CheckFreePokemonStorageSpace(void);
s16 AdvanceStorageMonIndex(struct BoxPokemon *boxMons, u8 currIndex, u8 maxIndex, u8 mode);
s16 CompactPartySlots(void);
s16 GetFirstFreeBoxSpot(u8 boxId);
struct BoxPokemon *GetBoxedMonPtr(u8 boxId, u8 boxPosition);
u16 CountPartyAliveNonEggMons_IgnoreVar0x8004Slot(void);
u32 CountAllStorageMons(void);
u32 CountPartyNonEggMons(void);
u32 CountStorageNonEggMons(void);
u32 GetAndCopyBoxMonDataAt(u8 boxId, u8 boxPosition, s32 request, void *dst);
u32 GetBoxMonDataAt(u8 boxId, u8 boxPosition, s32 request);
u32 GetBoxMonLevelAt(u8 boxId, u8 boxPosition);
u32 GetCurrentBoxMonData(u8 boxPosition, s32 request);
u8 *GetBoxNamePtr(u8 boxId);
u8 *StringCopyAndFillWithSpaces(u8 *dst, const u8 *src, u16 n);
u8 CountMonsInBox(u8 boxId);
u8 CountPartyAliveNonEggMonsExcept(u8 slotToIgnore);
u8 CountPartyMons(void);
u8 StorageGetCurrentBox(void);
void BoxMonAtToMon(u8 boxId, u8 boxPosition, struct Pokemon *dst);
void CB2_ReturnToPokeStorage(void);
void ChooseMonFromStorage();
void CopyBoxMonAt(u8 boxId, u8 boxPosition, struct BoxPokemon *dst);
void CreateBoxMonAt(u8 boxId, u8 boxPosition, u16 species, u8 level, u8 fixedIV, u8 hasFixedPersonality, u32 personality, u8 otIDType, u32 otID);
void DrawTextWindowAndBufferTiles(const u8 *string, void *dst, u8 zero1, u8 zero2, s32 bytesToBuffer);
void EnterPokeStorage(u8 boxOption);
void GetBoxMonNickAt(u8 boxId, u8 boxPosition, u8 *dst);
void RemoveSelectedPcMon(struct Pokemon *mon);
void ResetPokemonStorageSystem(void);
void SetBoxMonAt(u8 boxId, u8 boxPosition, struct BoxPokemon *src);
void SetBoxMonDataAt(u8 boxId, u8 boxPosition, s32 request, const void *value);
void SetBoxMonNickAt(u8 boxId, u8 boxPosition, const u8 *nick);
void SetCurrentBoxMonData(u8 boxPosition, s32 request, const void *value);
void ShowPokemonStorageSystemPC(void);
void ZeroBoxMonAt(u8 boxId, u8 boxPosition);

#endif // GUARD_POKEMON_STORAGE_SYSTEM_H
