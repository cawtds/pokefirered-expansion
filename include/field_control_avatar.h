#ifndef GUARD_FIELD_CONTROL_AVATAR_H
#define GUARD_FIELD_CONTROL_AVATAR_H

#include "global.h"

struct FieldInput
{
    bool8 pressedAButton:1;
    bool8 checkStandardWildEncounter:1;
    bool8 pressedStartButton:1;
    bool8 pressedSelectButton:1;
    bool8 heldDirection:1;
    bool8 heldDirection2:1;
    bool8 tookStep:1;
    bool8 pressedBButton:1;
    bool8 pressedRButton:1;
    bool8 input_field_1_0:1;
    bool8 input_field_1_1:1;
    bool8 input_field_1_2:1;
    bool8 input_field_1_3:1;
    enum Direction dpadDirection;
};

extern struct FieldInput gFieldInputRecord;

void RestartWildEncounterImmunitySteps(void);
void ClearPoisonStepCounter(void);
int SetCableClubWarp(void);
void HandleBoulderFallThroughHole(struct ObjectEvent *);
void GetInFrontOfPlayerPosition(struct MapPosition * position);
bool8 TryDoDiveWarp(struct MapPosition * pos, u16 behavior);
bool8 IsDirectionalStairWarpMetatileBehavior(u16 metatileBehavior, enum Direction playerDirection);
const u8 *GetInteractedLinkPlayerScript(struct MapPosition *position, u8 metatileBehavior, enum Direction direction);
const u8 *GetCoordEventScriptAtMapPosition(struct MapPosition *position);
void FieldClearPlayerInput(struct FieldInput *input);
int ProcessPlayerFieldInput(struct FieldInput *input);
void FieldInput_HandleCancelSignpost(struct FieldInput * input);
void FieldGetPlayerInput(struct FieldInput *input, u16 newKeys, u16 heldKeys);
void HandleBoulderActivateVictoryRoadSwitch(u16 x, u16 y);
const u8 *GetObjectEventScriptPointerPlayerFacing(void);

#endif //GUARD_FIELD_CONTROL_AVATAR_H
