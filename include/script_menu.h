#ifndef GUARD_SCRIPT_MENU_H
#define GUARD_SCRIPT_MENU_H

#include "global.h"
#include "list_menu.h"
#include "menu.h"
#include "constants/script_menu.h"

// The default size the stack for dynamic multichoice is initialized to
// If you try to push an element when the stack is full, it will be reallocated
// With increasing capacity of MULTI_DYNAMIC_STACK_INC

#define MULTI_DYNAMIC_STACK_SIZE 5
#define MULTI_DYNAMIC_STACK_INC 5

extern const u8 *const gStdStrings[];

struct DynamicMultichoiceStack
{
    s32 top;
    u32 capacity;
    struct ListMenuItem *elements;
};

bool32 MultichoiceDynamic_StackEmpty(void);
bool32 MultichoiceDynamic_StackFull(void);
bool32 ScriptMenu_CreatePCMultichoice(void);
bool32 ScriptMenu_MultichoiceWithDefault(u8 left, u8 top, enum MultichoiceID multichoiceId, bool32 ignoreBPress, u8 defaultChoice);
bool8 (*ScriptMenu_HidePokemonPic(void))(void);
bool8 ScriptMenu_Multichoice(u8 left, u8 top, enum MultichoiceID multichoiceId, bool8 ignoreBPress);
bool8 ScriptMenu_MultichoiceDynamic(u8 left, u8 top, u8 argc, struct ListMenuItem *items, bool8 ignoreBPress, u8 maxBeforeScroll, u32 initialRow, u32 callbackSet);
bool8 ScriptMenu_MultichoiceGrid(u8 left, u8 top, enum MultichoiceID multichoiceId, bool8 ignoreBpress, u8 columnCount);
bool8 ScriptMenu_ShowPokemonPic(enum Species species, u8 x, u8 y);
bool8 ScriptMenu_YesNo(u8 var1, u8 var2);
int ConvertPixelWidthToTileWidth(int width);
int DisplayTextAndGetWidth(const u8 *str, int width);
int ScriptMenu_AdjustLeftCoordFromWidth(int left, int width);
struct ListMenuItem *MultichoiceDynamic_PeekElement(void);
struct ListMenuItem *MultichoiceDynamic_PeekElementAt(u32 index);
struct ListMenuItem *MultichoiceDynamic_PopElement(void);
u32 MultichoiceDynamic_StackSize(void);
void DrawMultichoiceMenuInternal(u8 left, u8 top, enum MultichoiceID multichoiceId, bool8 ignoreBPress, u8 cursorPos, const struct MenuAction *actions, int count);
void MultichoiceDynamic_DestroyStack(void);
void MultichoiceDynamic_InitStack(u32 capacity);
void MultichoiceDynamic_PushElement(struct ListMenuItem item);
void MultichoiceDynamic_ReallocStack(u32 newCapacity);
void PicboxCancel(void);
void QL_DestroyAbortedDisplay(void);
void ScriptMenu_DisplayPCStartupPrompt(void);

#endif //GUARD_SCRIPT_MENU_H
