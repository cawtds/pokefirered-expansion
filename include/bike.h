#ifndef GUARD_BIKE_H
#define GUARD_BIKE_H

// TODO: Do the constants make sense in FRLG?

// Player speeds
enum
{
    PLAYER_SPEED_STANDING,
    PLAYER_SPEED_NORMAL,
    PLAYER_SPEED_FAST,
    PLAYER_SPEED_FASTER,
    PLAYER_SPEED_FASTEST,
};

enum BikeTransitionId
{
    BIKE_TRANS_FACE_DIRECTION,
    BIKE_TRANS_TURNING,
    BIKE_TRANS_MOVE,
    BIKE_TRANS_DOWNHILL,
    BIKE_TRANS_UPHILL
};

enum {
    BIKE_STATE_NORMAL,
    BIKE_STATE_TURNING,
    BIKE_STATE_SLOPE
};

// Acro bike states
enum
{
    ACRO_STATE_NORMAL,
    ACRO_STATE_TURNING,
    ACRO_STATE_WHEELIE_STANDING,
    ACRO_STATE_BUNNY_HOP,
    ACRO_STATE_WHEELIE_MOVING,
    ACRO_STATE_SIDE_JUMP,
    ACRO_STATE_TURN_JUMP,
};

bool32 IsRunningDisallowed(u8 metatileBehavior);
bool32 IsBikingDisallowedByPlayer(void);
bool32 IsPlayerNotUsingAcroBikeOnBumpySlope(void);
s16 GetPlayerSpeed(void);
void Bike_HandleBumpySlopeJump(void);
void Bike_UpdateBikeCounterSpeed(u8 counter);
void BikeClearState(void);
void GetOnOffBike(enum AvatarState transitionState);
void MovePlayerOnBike(enum Direction direction, u16 newKeys, u16 heldKeys);

#endif //GUARD_BIKE_H
