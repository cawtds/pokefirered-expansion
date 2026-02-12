#include "global.h"
#include "apprentice.h"
#include "string_util.h"
#include "constants/characters.h"


#include "data/battle_frontier/apprentice.h"


void BufferApprenticeChallengeText(u8 saveApprenticeId)
{
    u8 i, num;
    const u8 *challengeText;

    num = gSaveBlock1Ptr->apprentices[saveApprenticeId].number;
    for (i = 0; num != 0 && i < APPRENTICE_COUNT; num /= 10, i++)
        ;

    StringCopy_PlayerName(gStringVar1, gSaveBlock1Ptr->apprentices[saveApprenticeId].playerName);
    ConvertInternationalString(gStringVar1, gSaveBlock1Ptr->apprentices[saveApprenticeId].language);
    ConvertIntToDecimalStringN(gStringVar2, gSaveBlock1Ptr->apprentices[saveApprenticeId].number, STR_CONV_MODE_RIGHT_ALIGN, i);
    challengeText = sApprenticeChallengeTexts[gSaveBlock1Ptr->apprentices[saveApprenticeId].id];
    StringExpandPlaceholders(gStringVar4, challengeText);
}

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