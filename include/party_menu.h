#ifndef GUARD_PARTY_MENU_H
#define GUARD_PARTY_MENU_H

#include "main.h"
#include "task.h"
#include "constants/party_menu.h"

struct PartyMenu
{
    MainCallback exitCallback;
    TaskFunc task;
    u8 menuType:4;
    u8 layout:2;
    u8 unused:2;
    s8 slotId;
    s8 slotId2;
    u8 action;
    enum Item bagItem;
    s16 data[2];
};

extern struct PartyMenu gPartyMenu;
extern bool8 gPartyMenuUseExitCallback;
extern u8 gSelectedMonPartyId;
extern MainCallback gPostMenuFieldCallback;
extern u8 gSelectedOrderFromParty[MAX_FRONTIER_PARTY_SIZE];
extern u8 gBattlePartyCurrentOrder[PARTY_SIZE / 2];

extern const struct SpriteSheet gSpriteSheet_HeldItem;
extern const u16 gHeldItemPalette[];

extern void (*gItemUseCB)(u8, TaskFunc);
extern const struct SpriteTemplate gSpriteTemplate_StatusIcons;

bool32 BoxMonKnowsMove(struct BoxPokemon *boxMon, enum Move move);
bool32 MonKnowsMove(struct Pokemon *mon, enum Move move);
bool8 CB2_FadeFromPartyMenu(void);
bool8 FieldCallback_PrepareFadeInFromMenu(void);
bool8 IsMultiBattle(void);
bool8 IsPartyMenuTextPrinterActive(void);
enum Move ItemIdToBattleMoveId(enum Item item);
u32 Party_FirstMonWithMove(enum Move moveId);
u8 *GetMonNickname(struct Pokemon *mon, u8 *dest);
u8 DisplayPartyMenuMessage(const u8 *str, bool8 keepOpen);
u8 GetAilmentFromStatus(u32 status);
u8 GetCursorSelectionMonId(void);
u8 GetItemEffectType(enum Item item);
u8 GetMonAilment(struct Pokemon *mon);
u8 GetPartyIdFromBattlePartyId(u8 battlePartyId);
u8 GetPartyMenuType(void);
void AnimatePartySlot(u8 slot, u8 animNum);
void BufferBattlePartyCurrentOrder(void);
void BufferBattlePartyCurrentOrderBySide(u8 battlerId, u8 flankId);
void CB2_ChooseMonToGiveItem(void);
void CB2_GiveHoldItem(void);
void CB2_PartyMenuFromStartMenu(void);
void CB2_ReturnToPartyMenuFromFlyMap(void);
void CB2_ReturnToPartyMenuFromSummaryScreen(void);
void CB2_SelectBagItemToGive(void);
void CB2_ShowPartyMenuForItemUse(void);
void ChooseMonForDaycare(void);
void ChooseMonForInBattleItem(void);
void ChooseMonForMoveRelearner(void);
void ChooseMonForMoveTutor(void);
void ChooseMonForTradingBoard(u8 menuType, MainCallback callback);
void ChooseMonForWirelessMinigame(void);
void ChooseMonToGiveMailFromMailbox(void);
void ChoosePartyMon(void);
void ClearSelectedPartyOrder(void);
void DisplayPartyMenuStdMessage(u32 stringId);
void DrawHeldItemIconsForTrade(u8 *partyCounts, u8 *partySpriteIds, u8 whichParty);
void InitChooseMonsForBattle(u8 unused);
void InitPartyMenu(u8 menuType, u8 layout, u8 partyAction, bool8 keepCursorPos, u8 messageId, TaskFunc task, MainCallback callback);
void ItemUseCB_AbilityCapsule(u8 taskId, TaskFunc task);
void ItemUseCB_AbilityPatch(u8 taskId, TaskFunc task);
void ItemUseCB_BattleChooseMove(u8 taskId, TaskFunc task);
void ItemUseCB_BattleScript(u8 taskId, TaskFunc task);
void ItemUseCB_DynamaxCandy(u8 taskId, TaskFunc task);
void ItemUseCB_EvolutionStone(u8 taskId, TaskFunc func);
void ItemUseCB_FormChange_ConsumedOnUse(u8 taskId, TaskFunc task);
void ItemUseCB_FormChange(u8 taskId, TaskFunc task);
void ItemUseCB_Fusion(u8 taskId, TaskFunc task);
void ItemUseCB_Medicine(u8 taskId, TaskFunc func);
void ItemUseCB_Mint(u8 taskId, TaskFunc task);
void ItemUseCB_PPRecovery(u8 taskId, TaskFunc func);
void ItemUseCB_PPUp(u8 taskId, TaskFunc func);
void ItemUseCB_RareCandy(u8 taskId, TaskFunc func);
void ItemUseCB_ReduceEV(u8 taskId, TaskFunc task);
void ItemUseCB_ResetEVs(u8 taskId, TaskFunc task);
void ItemUseCB_RotomCatalog(u8 taskId, TaskFunc task);
void ItemUseCB_SacredAsh(u8 taskId, TaskFunc func);
void ItemUseCB_TMHM(u8 taskId, TaskFunc func);
void ItemUseCB_ZygardeCube(u8 taskId, TaskFunc task);
void LoadHeldItemIcons(void);
void LoadPartyMenuAilmentGfx(void);
void OpenPartyMenuInBattle(u8 partyAction);
void PartyMenuModifyHP(u8 taskId, u8 slot, s8 hpIncrement, s16 hpDifference, TaskFunc task);
void Pokedude_ChooseMonForInBattleItem(void);
void Pokedude_OpenPartyMenuInBattle(void);
void SetUsedFlyQuestLogEvent(const u8 *healLocCtrlData);
void ShowPartyMenuToShowcaseMultiBattleParty(void);
void SpriteCB_BounceConfirmCancelButton(u8 spriteId, u8 spriteId2, u8 animNum);
void SwitchPartyMonSlots(u8 slot, u8 slot2);
void SwitchPartyOrderLinkMulti(u8 battlerId, u8 slot, u8 slot2);
void Task_HandleChooseMonInput(u8 taskId);

#endif // GUARD_PARTY_MENU_H
