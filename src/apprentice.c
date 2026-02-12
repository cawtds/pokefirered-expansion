#include "global.h"
#include "apprentice.h"
#include "constants/characters.h"


#include "data/battle_frontier/apprentice.h"


void ResetApprenticeStruct(struct Apprentice *apprentice)
{
    u8 i;

    for (i = 0; i < ARRAY_COUNT(apprentice->speechWon); i++)
        apprentice->speechWon[i] = EC_EMPTY_WORD;

    apprentice->playerName[0] = EOS;
    apprentice->id = NUM_APPRENTICES;
}

const u8 *GetApprenticeNameInLanguage(u32 apprenticeId, u8 language)
{
    const struct ApprenticeTrainer *apprentice = &gApprentices[apprenticeId];

    switch (language)
    {
    case LANGUAGE_JAPANESE:
        return apprentice->name[0];
    case LANGUAGE_ENGLISH:
        return apprentice->name[1];
    case LANGUAGE_FRENCH:
        return apprentice->name[2];
    case LANGUAGE_ITALIAN:
        return apprentice->name[3];
    case LANGUAGE_GERMAN:
        return apprentice->name[4];
    case LANGUAGE_SPANISH:
    default:
        return apprentice->name[5];
    }
}