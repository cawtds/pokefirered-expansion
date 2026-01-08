# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is **pokefirered-expansion**, a decompilation and expansion of Pokemon FireRed. It's based on the pokeemerald-expansion project and includes modern features, quest log, and help system. The project compiles C and assembly source code into a playable GBA ROM.

## Build Commands

- Claude is NOT to use the make command. After edits are finished, you do not need to compile the project into a GBA ROM.

## Architecture

### Directory Structure

- `src/` - C source code (game logic, battle system, UI, etc.)
- `src/data/` - Game data as C headers (items, Pokemon, moves, abilities)
- `data/` - Scripts, map data, and battle scripts in assembly
- `data/maps/<MapName>/` - Per-map scripts, text, and events
- `include/` - Header files for all source modules
- `include/constants/` - Game constants (items, flags, species, etc.)
- `graphics/` - Image assets organized by category
- `sound/` - Music and sound effect data
- `tools/` - Custom build tools (gbagfx, jsonproc, mapjson, etc.)

### Key Systems

**Items**:
- Constants in `include/constants/items.h`
- Data definitions in `src/data/items.h`
- Use functions in `src/item_use.c` (declare in `include/item_use.h`)
- Icon graphics reference existing icons or add to `graphics/items/`

**Flags and Variables**:
- System flags in `include/constants/flags.h`
- Variables in `include/constants/vars.h`
- Flags are persistent boolean states; variables hold numeric values

**Scripts** (map events, NPCs):
- Assembly files in `data/maps/<MapName>/scripts.inc`
- Text strings in `data/maps/<MapName>/text.inc`
- Use scripting macros from `asm/macros/event.inc`

**Pokemon/Species**:
- Species constants in `include/constants/species.h`
- Data in `src/data/pokemon/` subdirectories

**Battle System**:
- Battle scripts in `data/battle_scripts_1.s` and `data/battle_scripts_2.s`
- Move effects in `src/battle_script_commands.c`
- AI logic in `src/battle_ai_main.c`

### Common Patterns

**Adding a Key Item**:
1. Add constant in `include/constants/items.h`, update `ITEMS_COUNT`
2. Add item data entry in `src/data/items.h`
3. If it has a use function: declare in `include/item_use.h`, implement in `src/item_use.c`
4. Use `ITEM_USE_FIELD` type for items usable from bag, `ITEM_USE_PARTY_MENU` for party selection

**Adding a Flag**:
1. Find unused flag in `include/constants/flags.h` (FLAG_0x8XX series)
2. Rename to descriptive name like `FLAG_SYS_FEATURE_ACTIVE`
3. Use `FlagGet()`, `FlagSet()`, `FlagClear()`, `FlagToggle()` in C code

**String Formatting**:
- Define strings in `src/strings.c`, declare in `include/strings.h`
- Use `gStringVar1`, `gStringVar2`, etc. for dynamic content
- Call `StringExpandPlaceholders(gStringVar4, gText_Template)` to expand `{STR_VAR_1}` etc.
- Pokemon names: `GetMonNickname(mon, gStringVar1)`

**Map Script Events**:
- Use `giveitem_msg` for giving items with message
- Text labels follow pattern: `MapName_Text_Description`
- Script labels follow pattern: `MapName_EventScript_Name`

## String Encoding

The game uses a custom character encoding defined in `charmap.txt`. Special characters:
- `{PLAYER}` - Player's name
- `{STR_VAR_1}`, `{STR_VAR_2}`, `{STR_VAR_3}` - Dynamic string slots
- `{PAUSE_UNTIL_PRESS}` - Wait for button press
- `$` - String terminator in .inc text files