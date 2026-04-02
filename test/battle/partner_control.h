//
// DO NOT MODIFY THIS FILE! It is auto-generated from test/battle/partner_control.party
//
// If you want to modify this file see expansion PR #7154
//

#line 1 "test/battle/partner_control.party"

#line 1
    [DIFFICULTY_NORMAL][PARTNER_NONE] =
    {
#line 3
        .trainerClass = TRAINER_CLASS_PKMN_TRAINER_1,
#line 4
        .trainerPic = TRAINER_PIC_RS_BRENDAN,
#line 5
        .gender = TRAINER_GENDER_MALE,
#line 6
        .encounterMusic = TRAINER_ENCOUNTER_MUSIC_MALE,
        .partySize = 0,
        .party = (const struct TrainerMon[])
        {
        },
    },
#line 8
    [DIFFICULTY_NORMAL][PARTNER_STEVEN_TEST] =
    {
#line 9
        .trainerName = _("STEVEN"),
#line 10
        .trainerClass = TRAINER_CLASS_RIVAL,
#line 11
        .trainerPic = TRAINER_PIC_STEVEN,
#line 12
        .gender = TRAINER_GENDER_MALE,
#line 13
        .encounterMusic = TRAINER_ENCOUNTER_MUSIC_MALE,
#line 14
        .aiFlags = AI_FLAG_BASIC_TRAINER,
        .partySize = 3,
        .party = (const struct TrainerMon[])
        {
            {
#line 16
            .species = SPECIES_METANG,
            .gender = TRAINER_MON_RANDOM_GENDER,
#line 20
            .ev = TRAINER_PARTY_EVS(0, 252, 252, 0, 6, 0),
#line 19
            .iv = TRAINER_PARTY_IVS(31, 31, 31, 31, 31, 31),
#line 18
            .lvl = 42,
            .ball = POKEBALL_COUNT,
#line 17
            .nature = NATURE_BRAVE,
            .dynamaxLevel = MAX_DYNAMAX_LEVEL,
            .moves = {
#line 21
                MOVE_LIGHT_SCREEN,
                MOVE_PSYCHIC,
                MOVE_REFLECT,
                MOVE_METAL_CLAW,
            },
            },
            {
#line 26
            .species = SPECIES_SKARMORY,
            .gender = TRAINER_MON_RANDOM_GENDER,
#line 30
            .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 6, 252),
#line 29
            .iv = TRAINER_PARTY_IVS(31, 31, 31, 31, 31, 31),
#line 28
            .lvl = 43,
            .ball = POKEBALL_COUNT,
#line 27
            .nature = NATURE_IMPISH,
            .dynamaxLevel = MAX_DYNAMAX_LEVEL,
            .moves = {
#line 31
                MOVE_TOXIC,
                MOVE_AERIAL_ACE,
                MOVE_PROTECT,
                MOVE_STEEL_WING,
            },
            },
            {
#line 36
            .species = SPECIES_AGGRON,
            .gender = TRAINER_MON_RANDOM_GENDER,
#line 40
            .ev = TRAINER_PARTY_EVS(0, 252, 0, 0, 252, 6),
#line 39
            .iv = TRAINER_PARTY_IVS(31, 31, 31, 31, 31, 31),
#line 38
            .lvl = 44,
            .ball = POKEBALL_COUNT,
#line 37
            .nature = NATURE_ADAMANT,
            .dynamaxLevel = MAX_DYNAMAX_LEVEL,
            .moves = {
#line 41
                MOVE_THUNDER,
                MOVE_PROTECT,
                MOVE_SOLAR_BEAM,
                MOVE_DRAGON_CLAW,
            },
            },
        },
    },
#line 46
#line 52
    [DIFFICULTY_NORMAL][2] =
    {
#line 47
        .trainerName = _("Test2"),
#line 48
        .trainerClass = TRAINER_CLASS_RIVAL,
#line 49
        .trainerPic = TRAINER_PIC_STEVEN,
#line 50
        .gender = TRAINER_GENDER_MALE,
#line 51
        .encounterMusic = TRAINER_ENCOUNTER_MUSIC_MALE,
        .partySize = 1,
        .party = (const struct TrainerMon[])
        {
            {
#line 54
            .species = SPECIES_MEWTWO,
            .gender = TRAINER_MON_RANDOM_GENDER,
#line 56
            .iv = TRAINER_PARTY_IVS(31, 31, 31, 31, 31, 31),
#line 55
            .lvl = 50,
            .ball = POKEBALL_COUNT,
            .nature = NATURE_HARDY,
            .dynamaxLevel = MAX_DYNAMAX_LEVEL,
            },
        },
    },
#line 57
#line 64
    [DIFFICULTY_EASY][2] =
    {
#line 58
        .trainerName = _("Test2"),
#line 59
        .trainerClass = TRAINER_CLASS_RIVAL,
#line 60
        .trainerPic = TRAINER_PIC_STEVEN,
#line 61
        .gender = TRAINER_GENDER_MALE,
#line 62
        .encounterMusic = TRAINER_ENCOUNTER_MUSIC_MALE,
#line 63
        .battleType = TRAINER_BATTLE_TYPE_SINGLES,
        .partySize = 1,
        .party = (const struct TrainerMon[])
        {
            {
#line 66
            .species = SPECIES_METAPOD,
            .gender = TRAINER_MON_RANDOM_GENDER,
#line 68
            .iv = TRAINER_PARTY_IVS(31, 31, 31, 31, 31, 31),
#line 67
            .lvl = 1,
            .ball = POKEBALL_COUNT,
            .nature = NATURE_HARDY,
            .dynamaxLevel = MAX_DYNAMAX_LEVEL,
            },
        },
    },
#line 69
#line 76
    [DIFFICULTY_HARD][2] =
    {
#line 70
        .trainerName = _("Test2"),
#line 71
        .trainerClass = TRAINER_CLASS_RIVAL,
#line 72
        .trainerPic = TRAINER_PIC_STEVEN,
#line 73
        .gender = TRAINER_GENDER_MALE,
#line 74
        .encounterMusic = TRAINER_ENCOUNTER_MUSIC_MALE,
#line 75
        .battleType = TRAINER_BATTLE_TYPE_SINGLES,
        .partySize = 1,
        .party = (const struct TrainerMon[])
        {
            {
#line 78
            .species = SPECIES_ARCEUS,
            .gender = TRAINER_MON_RANDOM_GENDER,
#line 80
            .iv = TRAINER_PARTY_IVS(31, 31, 31, 31, 31, 31),
#line 79
            .lvl = 99,
            .ball = POKEBALL_COUNT,
            .nature = NATURE_HARDY,
            .dynamaxLevel = MAX_DYNAMAX_LEVEL,
            },
        },
    },
