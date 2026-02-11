#include "global.h"
#include "apprentice.h"
#include "battle_dome.h"
#include "battle_frontier.h"
#include "battle_records.h"
#include "battle_tower.h"
#include "event_data.h"
#include "field_specials.h"
#include "frontier_util.h"
#include "international_string_util.h"
#include "load_save.h"
#include "malloc.h"
#include "menu.h"
#include "move.h"
#include "party_menu.h"
#include "random.h"
#include "record_mixing.h"
#include "save.h"
#include "string_util.h"
#include "strings.h"
#include "window.h"
#include "constants/battle_frontier.h"
#include "constants/battle_move_effects.h"
#include "constants/event_objects.h"
#include "constants/frontier_util.h"

struct FrontierBrainMon
{
    u16 species;
    u16 heldItem;
    u8 fixedIV;
    u8 nature;
    u8 evs[NUM_STATS];
    u16 moves[MAX_MON_MOVES];
};

struct FrontierBrain
{
    u16 trainerId;
    u8 objEventGfx;
    u8 isFemale;
    const u8 *lostTexts[2];
    const u8 *wonTexts[2];
    u16 battledBit[2];
    u8 streakAppearances[4];
};

// This file's functions.
static void GetChallengeStatus(void);
static void SetFrontierData(void);
static void SaveSelectedParty(void);

// battledBit: Flags to change the conversation when the Frontier Brain is encountered for a battle
// First bit is has battled them before and not won yet, second bit is has battled them and won (obtained a Symbol)
const struct FrontierBrain gFrontierBrainInfo[NUM_FRONTIER_FACILITIES] =
{
    [FRONTIER_FACILITY_TOWER] =
    {
        .trainerId = TRAINER_ANABEL,
        .objEventGfx = OBJ_EVENT_GFX_ANABEL,
        .isFemale = TRUE,
        .lostTexts = {
            COMPOUND_STRING("Okay, I understand…"), //Silver
            COMPOUND_STRING("Thank you…")           //Gold
        },
        .wonTexts = {
            COMPOUND_STRING("It's very disappointing…"), //Silver
            COMPOUND_STRING("I'm terribly sorry…")       //Gold
        },
        .battledBit = {1 << 0, 1 << 1},
        .streakAppearances = {35, 70, 35, 1},
    },
    [FRONTIER_FACILITY_DOME] =
    {
        .trainerId = TRAINER_TUCKER,
        .objEventGfx = OBJ_EVENT_GFX_TUCKER,
        .isFemale = FALSE,
        .lostTexts = {
            COMPOUND_STRING(
                "Grr…\n"
                "What the…"),        //Silver
            COMPOUND_STRING(
                "Ahahaha!\n"
                "You're inspiring!") //Gold
        },
        .wonTexts = {
            COMPOUND_STRING(
                "Ahahaha! Aren't you embarrassed?\n"
                "Everyone's watching!"),                              //Silver
            COMPOUND_STRING("My DOME ACE title isn't just for show!") //Gold
        },
        .battledBit = {1 << 2, 1 << 3},
        .streakAppearances = {1, 2, 5, 0},
    },
    [FRONTIER_FACILITY_PALACE] =
    {
        .trainerId = TRAINER_SPENSER,
        .objEventGfx = OBJ_EVENT_GFX_SPENSER,
        .isFemale = FALSE,
        .lostTexts = {
            COMPOUND_STRING(
                "Ah…\n"
                "Now this is something else…"), //Silver
            COMPOUND_STRING(
                "Gwah!\n"
                "Hahahaha!")                    //Gold
        },
        .wonTexts = {
            COMPOUND_STRING(
                "Your POKéMON are wimpy because\n"
                "you're wimpy as a TRAINER!"),           //Silver
            COMPOUND_STRING(
                "Gwahahaha!\n"
                "My brethren, we have nothing to fear!") //Gold
        },
        .battledBit = {1 << 4, 1 << 5},
        .streakAppearances = {21, 42, 21, 1},
    },
    [FRONTIER_FACILITY_ARENA] =
    {
        .trainerId = TRAINER_GRETA,
        .objEventGfx = OBJ_EVENT_GFX_GRETA,
        .isFemale = TRUE,
        .lostTexts = {
            COMPOUND_STRING(
                "No way!\n"
                "Good job!"),        //Silver
            COMPOUND_STRING(
                "Huh?\n"
                "Are you serious?!") //Gold
        },
        .wonTexts = {
            COMPOUND_STRING(
                "Oh, come on!\n"
                "You have to try harder than that!"), //Silver
            COMPOUND_STRING(
                "Heheh!\n"
                "What did you expect?")               //Gold
        },
        .battledBit = {1 << 6, 1 << 7},
        .streakAppearances = {28, 56, 28, 1},
    },
    [FRONTIER_FACILITY_FACTORY] =
    {
        .trainerId = TRAINER_NOLAND,
        .objEventGfx = OBJ_EVENT_GFX_NOLAND,
        .isFemale = FALSE,
        .lostTexts = {
            COMPOUND_STRING(
                "Good job!\n"
                "You know what you're doing!"),    //Silver
            COMPOUND_STRING("What happened here?") //Gold
        },
        .wonTexts = {
            COMPOUND_STRING(
                "Way to work!\n"
                "That was a good lesson, eh?"), //Silver
            COMPOUND_STRING(
                "Hey, hey, hey!\n"
                "You're finished already?")     //Gold
        },
        .battledBit = {1 << 8, 1 << 9},
        .streakAppearances = {21, 42, 21, 1},
    },
    [FRONTIER_FACILITY_PIKE] =
    {
        .trainerId = TRAINER_LUCY,
        .objEventGfx = OBJ_EVENT_GFX_LUCY,
        .isFemale = TRUE,
        .lostTexts = {
            COMPOUND_STRING("Urk…"), //Silver
            COMPOUND_STRING("Darn!") //Gold
        },
        .wonTexts = {
            COMPOUND_STRING("Humph…"), //Silver
            COMPOUND_STRING("Hah!")    //Gold
        },
        .battledBit = {1 << 10, 1 << 11},
        .streakAppearances = {28, 140, 56, 1},
    },
    [FRONTIER_FACILITY_PYRAMID] =
    {
        .trainerId = TRAINER_BRANDON,
        .objEventGfx = OBJ_EVENT_GFX_BRANDON,
        .isFemale = FALSE,
        .lostTexts = {
            COMPOUND_STRING(
                "That's it! You've done great!\n"
                "You've worked hard for this!"), //Silver
            COMPOUND_STRING(
                "That's it! You've done it!\n"
                "You kept working for this!")    //Gold
        },
        .wonTexts = {
            COMPOUND_STRING(
                "Hey! What's wrong with you!\n"
                "Let's see some effort! Get up!"),       //Silver
            COMPOUND_STRING(
                "Hey! Don't you give up now!\n"
                "Get up! Don't lose faith in yourself!") //Gold
        },
        .battledBit = {1 << 12, 1 << 13},
        .streakAppearances = {21, 70, 35, 0},
    },
};

static const struct FrontierBrainMon sFrontierBrainsMons[][2][FRONTIER_PARTY_SIZE] =
{
    [FRONTIER_FACILITY_TOWER] =
    {
        // Silver Symbol.
        {
            {
                .species = SPECIES_ALAKAZAM,
                .heldItem = ITEM_BRIGHT_POWDER,
                .fixedIV = 24,
                .nature = NATURE_MODEST,
                .evs = {106, 0, 152, 152, 100, 0},
                .moves = {MOVE_THUNDER_PUNCH, MOVE_FIRE_PUNCH, MOVE_ICE_PUNCH, MOVE_DISABLE},
            },
            {
                .species = SPECIES_ENTEI,
                .heldItem = ITEM_LUM_BERRY,
                .fixedIV = 24,
                .nature = NATURE_LONELY,
                .evs = {100, 152, 152, 0, 100, 6},
                .moves = {MOVE_FIRE_BLAST, MOVE_CALM_MIND, MOVE_RETURN, MOVE_ROAR},
            },
            {
                .species = SPECIES_SNORLAX,
                .heldItem = ITEM_QUICK_CLAW,
                .fixedIV = 24,
                .nature = NATURE_ADAMANT,
                .evs = {152, 152, 0, 0, 106, 100},
                .moves = {MOVE_BODY_SLAM, MOVE_BELLY_DRUM, MOVE_YAWN, MOVE_SHADOW_BALL},
            },
        },
        // Gold Symbol.
        {
            {
                .species = SPECIES_RAIKOU,
                .heldItem = ITEM_LUM_BERRY,
                .fixedIV = MAX_PER_STAT_IVS,
                .nature = NATURE_MODEST,
                .evs = {158, 0, 252, 100, 0, 0},
                .moves = {MOVE_THUNDERBOLT, MOVE_CALM_MIND, MOVE_REFLECT, MOVE_REST},
            },
            {
                .species = SPECIES_LATIOS,
                .heldItem = ITEM_BRIGHT_POWDER,
                .fixedIV = MAX_PER_STAT_IVS,
                .nature = NATURE_MODEST,
                .evs = {252, 0, 252, 6, 0, 0},
                .moves = {MOVE_PSYCHIC, MOVE_CALM_MIND, MOVE_RECOVER, MOVE_DRAGON_CLAW},
            },
            {
                .species = SPECIES_SNORLAX,
                .heldItem = ITEM_CHESTO_BERRY,
                .fixedIV = MAX_PER_STAT_IVS,
                .nature = NATURE_ADAMANT,
                .evs = {252, 252, 0, 0, 6, 0},
                .moves = {MOVE_CURSE, MOVE_RETURN, MOVE_REST, MOVE_SHADOW_BALL},
            },
        },
    },
    [FRONTIER_FACILITY_DOME] =
    {
        // Silver Symbol.
        {
            {
                .species = SPECIES_SWAMPERT,
                .heldItem = ITEM_FOCUS_BAND,
                .fixedIV = 20,
                .nature = NATURE_BRAVE,
                .evs = {152, 152, 106, 0, 100, 0},
                .moves = {MOVE_SURF, MOVE_EARTHQUAKE, MOVE_ICE_BEAM, MOVE_COUNTER},
            },
            {
                .species = SPECIES_SALAMENCE,
                .heldItem = ITEM_LUM_BERRY,
                .fixedIV = 20,
                .nature = NATURE_ADAMANT,
                .evs = {152, 152, 106, 100, 0, 0},
                .moves = {MOVE_EARTHQUAKE, MOVE_BRICK_BREAK, MOVE_DRAGON_CLAW, MOVE_AERIAL_ACE},
            },
            {
                .species = SPECIES_CHARIZARD,
                .heldItem = ITEM_WHITE_HERB,
                .fixedIV = 20,
                .nature = NATURE_QUIET,
                .evs = {100, 152, 106, 152, 0, 0},
                .moves = {MOVE_OVERHEAT, MOVE_ROCK_SLIDE, MOVE_AERIAL_ACE, MOVE_EARTHQUAKE},
            },
        },
        // Gold Symbol.
        {
            {
                .species = SPECIES_SWAMPERT,
                .heldItem = ITEM_LEFTOVERS,
                .fixedIV = MAX_PER_STAT_IVS,
                .nature = NATURE_BRAVE,
                .evs = {252, 252, 6, 0, 0, 0},
                .moves = {MOVE_SURF, MOVE_EARTHQUAKE, MOVE_ICE_BEAM, MOVE_MIRROR_COAT},
            },
            {
                .species = SPECIES_METAGROSS,
                .heldItem = ITEM_QUICK_CLAW,
                .fixedIV = MAX_PER_STAT_IVS,
                .nature = NATURE_BRAVE,
                .evs = {252, 252, 6, 0, 0, 0},
                .moves = {MOVE_PSYCHIC, MOVE_METEOR_MASH, MOVE_EARTHQUAKE, MOVE_PROTECT},
            },
            {
                .species = SPECIES_LATIAS,
                .heldItem = ITEM_CHESTO_BERRY,
                .fixedIV = MAX_PER_STAT_IVS,
                .nature = NATURE_MODEST,
                .evs = {252, 0, 252, 6, 0, 0},
                .moves = {MOVE_THUNDERBOLT, MOVE_PSYCHIC, MOVE_CALM_MIND, MOVE_REST},
            },
        },
    },
    [FRONTIER_FACILITY_PALACE] =
    {
        // Silver Symbol.
        {
            {
                .species = SPECIES_CROBAT,
                .heldItem = ITEM_BRIGHT_POWDER,
                .fixedIV = 16,
                .nature = NATURE_ADAMANT,
                .evs = {152, 0, 0, 152, 100, 106},
                .moves = {MOVE_CONFUSE_RAY, MOVE_DOUBLE_TEAM, MOVE_TOXIC, MOVE_FLY},
            },
            {
                .species = SPECIES_SLAKING,
                .heldItem = ITEM_SCOPE_LENS,
                .fixedIV = 16,
                .nature = NATURE_HARDY,
                .evs = {152, 152, 0, 106, 100, 0},
                .moves = {MOVE_EARTHQUAKE, MOVE_SWAGGER, MOVE_SHADOW_BALL, MOVE_BRICK_BREAK},
            },
            {
                .species = SPECIES_LAPRAS,
                .heldItem = ITEM_QUICK_CLAW,
                .fixedIV = 16,
                .nature = NATURE_QUIET,
                .evs = {0, 0, 252, 0, 106, 152},
                .moves = {MOVE_ICE_BEAM, MOVE_HORN_DRILL, MOVE_CONFUSE_RAY, MOVE_PROTECT},
            },
        },
        // Gold Symbol.
        {
            {
                .species = SPECIES_ARCANINE,
                .heldItem = ITEM_WHITE_HERB,
                .fixedIV = MAX_PER_STAT_IVS,
                .nature = NATURE_HASTY,
                .evs = {6, 252, 252, 0, 0, 0},
                .moves = {MOVE_OVERHEAT, MOVE_EXTREME_SPEED, MOVE_ROAR, MOVE_PROTECT},
            },
            {
                .species = SPECIES_SLAKING,
                .heldItem = ITEM_SCOPE_LENS,
                .fixedIV = MAX_PER_STAT_IVS,
                .nature = NATURE_HARDY,
                .evs = {6, 252, 0, 252, 0, 0},
                .moves = {MOVE_HYPER_BEAM, MOVE_EARTHQUAKE, MOVE_SHADOW_BALL, MOVE_YAWN},
            },
            {
                .species = SPECIES_SUICUNE,
                .heldItem = ITEM_KINGS_ROCK,
                .fixedIV = MAX_PER_STAT_IVS,
                .nature = NATURE_HASTY,
                .evs = {252, 0, 252, 6, 0, 0},
                .moves = {MOVE_BLIZZARD, MOVE_SURF, MOVE_BITE, MOVE_CALM_MIND},
            },
        },
    },
    [FRONTIER_FACILITY_ARENA] =
    {
        // Silver Symbol.
        {
            {
                .species = SPECIES_HERACROSS,
                .heldItem = ITEM_SALAC_BERRY,
                .fixedIV = 20,
                .nature = NATURE_JOLLY,
                .evs = {106, 152, 0, 152, 0, 100},
                .moves = {MOVE_MEGAHORN, MOVE_ROCK_TOMB, MOVE_ENDURE, MOVE_REVERSAL},
            },
            {
                .species = SPECIES_UMBREON,
                .heldItem = ITEM_LEFTOVERS,
                .fixedIV = 20,
                .nature = NATURE_CALM,
                .evs = {152, 0, 100, 0, 152, 106},
                .moves = {MOVE_BODY_SLAM, MOVE_CONFUSE_RAY, MOVE_PSYCHIC, MOVE_FEINT_ATTACK},
            },
            {
                .species = SPECIES_SHEDINJA,
                .heldItem = ITEM_BRIGHT_POWDER,
                .fixedIV = 20,
                .nature = NATURE_ADAMANT,
                .evs = {0, 252, 6, 252, 0, 0},
                .moves = {MOVE_SHADOW_BALL, MOVE_RETURN, MOVE_CONFUSE_RAY, MOVE_AERIAL_ACE},
            },
        },
        // Gold Symbol.
        {
            {
                .species = SPECIES_UMBREON,
                .heldItem = ITEM_CHESTO_BERRY,
                .fixedIV = MAX_PER_STAT_IVS,
                .nature = NATURE_CALM,
                .evs = {252, 0, 0, 0, 252, 6},
                .moves = {MOVE_DOUBLE_EDGE, MOVE_CONFUSE_RAY, MOVE_REST, MOVE_PSYCHIC},
            },
            {
                .species = SPECIES_GENGAR,
                .heldItem = ITEM_LEFTOVERS,
                .fixedIV = MAX_PER_STAT_IVS,
                .nature = NATURE_MODEST,
                .evs = {252, 0, 252, 0, 6, 0},
                .moves = {MOVE_PSYCHIC, MOVE_HYPNOSIS, MOVE_DREAM_EATER, MOVE_DESTINY_BOND},
            },
            {
                .species = SPECIES_BRELOOM,
                .heldItem = ITEM_LUM_BERRY,
                .fixedIV = MAX_PER_STAT_IVS,
                .nature = NATURE_JOLLY,
                .evs = {6, 252, 0, 252, 0, 0},
                .moves = {MOVE_SPORE, MOVE_FOCUS_PUNCH, MOVE_GIGA_DRAIN, MOVE_HEADBUTT},
            },
        },
    },
    [FRONTIER_FACILITY_FACTORY] =
    {
        // Because Factory's Pokémon are random, this facility's Brain also uses random Pokémon.
        // What is interesting, this team is actually the one Steven uses in the multi tag battle alongside the player.
        {
            {
                .species = SPECIES_METANG,
                .heldItem = ITEM_SITRUS_BERRY,
                .fixedIV = MAX_PER_STAT_IVS,
                .nature = NATURE_BRAVE,
                .evs = {0, 252, 252, 0, 6, 0},
                .moves = {MOVE_LIGHT_SCREEN, MOVE_PSYCHIC, MOVE_REFLECT, MOVE_METAL_CLAW},
            },
            {
                .species = SPECIES_SKARMORY,
                .heldItem = ITEM_SITRUS_BERRY,
                .fixedIV = MAX_PER_STAT_IVS,
                .nature = NATURE_IMPISH,
                .evs = {252, 0, 0, 0, 6, 252},
                .moves = {MOVE_TOXIC, MOVE_AERIAL_ACE, MOVE_PROTECT, MOVE_STEEL_WING},
            },
            {
                .species = SPECIES_AGGRON,
                .heldItem = ITEM_SITRUS_BERRY,
                .fixedIV = MAX_PER_STAT_IVS,
                .nature = NATURE_ADAMANT,
                .evs = {0, 252, 0, 0, 252, 6},
                .moves = {MOVE_THUNDERBOLT, MOVE_PROTECT, MOVE_SOLAR_BEAM, MOVE_DRAGON_CLAW},
            },
        },
        {
            {
                .species = SPECIES_METANG,
                .heldItem = ITEM_SITRUS_BERRY,
                .fixedIV = MAX_PER_STAT_IVS,
                .nature = NATURE_BRAVE,
                .evs = {0, 252, 252, 0, 6, 0},
                .moves = {MOVE_LIGHT_SCREEN, MOVE_PSYCHIC, MOVE_REFLECT, MOVE_METAL_CLAW},
            },
            {
                .species = SPECIES_SKARMORY,
                .heldItem = ITEM_SITRUS_BERRY,
                .fixedIV = MAX_PER_STAT_IVS,
                .nature = NATURE_IMPISH,
                .evs = {252, 0, 0, 0, 6, 252},
                .moves = {MOVE_TOXIC, MOVE_AERIAL_ACE, MOVE_PROTECT, MOVE_STEEL_WING},
            },
            {
                .species = SPECIES_AGGRON,
                .heldItem = ITEM_SITRUS_BERRY,
                .fixedIV = MAX_PER_STAT_IVS,
                .nature = NATURE_ADAMANT,
                .evs = {0, 252, 0, 0, 252, 6},
                .moves = {MOVE_THUNDERBOLT, MOVE_PROTECT, MOVE_SOLAR_BEAM, MOVE_DRAGON_CLAW},
            },
        },
    },
    [FRONTIER_FACILITY_PIKE] =
    {
        // Silver Symbol.
        {
            {
                .species = SPECIES_SEVIPER,
                .heldItem = ITEM_QUICK_CLAW,
                .fixedIV = 16,
                .nature = NATURE_BRAVE,
                .evs = {252, 0, 252, 0, 6, 0},
                .moves = {MOVE_SWAGGER, MOVE_CRUNCH, MOVE_POISON_FANG, MOVE_GIGA_DRAIN},
            },
            {
                .species = SPECIES_SHUCKLE,
                .heldItem = ITEM_CHESTO_BERRY,
                .fixedIV = 16,
                .nature = NATURE_BOLD,
                .evs = {252, 0, 0, 0, 106, 252},
                .moves = {MOVE_TOXIC, MOVE_SANDSTORM, MOVE_PROTECT, MOVE_REST},
            },
            {
                .species = SPECIES_MILOTIC,
                .heldItem = ITEM_LEFTOVERS,
                .fixedIV = 16,
                .nature = NATURE_MODEST,
                .evs = {152, 0, 100, 0, 152, 106},
                .moves = {MOVE_ICE_BEAM, MOVE_MIRROR_COAT, MOVE_SURF, MOVE_RECOVER},
            },
        },
        // Gold Symbol.
        {
            {
                .species = SPECIES_SEVIPER,
                .heldItem = ITEM_FOCUS_BAND,
                .fixedIV = MAX_PER_STAT_IVS,
                .nature = NATURE_BOLD,
                .evs = {252, 0, 0, 0, 252, 6},
                .moves = {MOVE_SWAGGER, MOVE_CRUNCH, MOVE_SLUDGE_BOMB, MOVE_GIGA_DRAIN},
            },
            {
                .species = SPECIES_STEELIX,
                .heldItem = ITEM_BRIGHT_POWDER,
                .fixedIV = MAX_PER_STAT_IVS,
                .nature = NATURE_BRAVE,
                .evs = {252, 0, 0, 0, 6, 252},
                .moves = {MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE, MOVE_EXPLOSION, MOVE_SCREECH},
            },
            {
                .species = SPECIES_GYARADOS,
                .heldItem = ITEM_CHESTO_BERRY,
                .fixedIV = MAX_PER_STAT_IVS,
                .nature = NATURE_ADAMANT,
                .evs = {252, 6, 0, 0, 0, 252},
                .moves = {MOVE_DRAGON_DANCE, MOVE_RETURN, MOVE_ROAR, MOVE_REST},
            },
        },
    },
    [FRONTIER_FACILITY_PYRAMID] =
    {
        // Silver Symbol.
        {
            {
                .species = SPECIES_REGIROCK,
                .heldItem = ITEM_QUICK_CLAW,
                .fixedIV = 16,
                .nature = NATURE_ADAMANT,
                .evs = {152, 152, 0, 0, 106, 100},
                .moves = {MOVE_EXPLOSION, MOVE_SUPERPOWER, MOVE_EARTHQUAKE, MOVE_ANCIENT_POWER},
            },
            {
                .species = SPECIES_REGISTEEL,
                .heldItem = ITEM_LEFTOVERS,
                .fixedIV = 16,
                .nature = NATURE_ADAMANT,
                .evs = {152, 152, 0, 0, 6, 200},
                .moves = {MOVE_EARTHQUAKE, MOVE_METAL_CLAW, MOVE_TOXIC, MOVE_IRON_DEFENSE},
            },
            {
                .species = SPECIES_REGICE,
                .heldItem = ITEM_CHESTO_BERRY,
                .fixedIV = 16,
                .nature = NATURE_MODEST,
                .evs = {106, 0, 152, 0, 100, 152},
                .moves = {MOVE_ICE_BEAM, MOVE_AMNESIA, MOVE_THUNDER, MOVE_REST},
            },
        },
        // Gold Symbol.
        {
            {
                .species = SPECIES_ARTICUNO,
                .heldItem = ITEM_SCOPE_LENS,
                .fixedIV = MAX_PER_STAT_IVS,
                .nature = NATURE_MILD,
                .evs = {6, 0, 252, 252, 0, 0},
                .moves = {MOVE_BLIZZARD, MOVE_WATER_PULSE, MOVE_AERIAL_ACE, MOVE_REFLECT},
            },
            {
                .species = SPECIES_ZAPDOS,
                .heldItem = ITEM_LUM_BERRY,
                .fixedIV = MAX_PER_STAT_IVS,
                .nature = NATURE_MILD,
                .evs = {6, 0, 252, 252, 0, 0},
                .moves = {MOVE_THUNDER, MOVE_DETECT, MOVE_DRILL_PECK, MOVE_LIGHT_SCREEN},
            },
            {
                .species = SPECIES_MOLTRES,
                .heldItem = ITEM_BRIGHT_POWDER,
                .fixedIV = MAX_PER_STAT_IVS,
                .nature = NATURE_MILD,
                .evs = {6, 0, 252, 252, 0, 0},
                .moves = {MOVE_FIRE_BLAST, MOVE_HYPER_BEAM, MOVE_AERIAL_ACE, MOVE_SAFEGUARD},
            },
        },
    },
};

static const u8 *const sHallFacilityToRecordsText[] =
{
    [RANKING_HALL_TOWER_SINGLES] = gText_FrontierFacilityWinStreak,
    [RANKING_HALL_TOWER_DOUBLES] = gText_FrontierFacilityWinStreak,
    [RANKING_HALL_TOWER_MULTIS]  = gText_FrontierFacilityWinStreak,
    [RANKING_HALL_DOME]          = gText_FrontierFacilityClearStreak,
    [RANKING_HALL_PALACE]        = gText_FrontierFacilityWinStreak,
    [RANKING_HALL_ARENA]         = gText_FrontierFacilityKOsStreak,
    [RANKING_HALL_FACTORY]       = gText_FrontierFacilityWinStreak,
    [RANKING_HALL_PIKE]          = gText_FrontierFacilityRoomsCleared,
    [RANKING_HALL_PYRAMID]       = gText_FrontierFacilityFloorsCleared,
    [RANKING_HALL_TOWER_LINK]    = gText_FrontierFacilityWinStreak,
};

static const u8 *const sRecordsWindowChallengeTexts[][2] =
{
    [RANKING_HALL_TOWER_SINGLES] = {gText_BattleTower2,  gText_FacilitySingle},
    [RANKING_HALL_TOWER_DOUBLES] = {gText_BattleTower2,  gText_FacilityDouble},
    [RANKING_HALL_TOWER_MULTIS]  = {gText_BattleTower2,  gText_FacilityMulti},
    [RANKING_HALL_DOME]          = {gText_BattleDome,    gText_FacilitySingle},
    [RANKING_HALL_PALACE]        = {gText_BattlePalace,  gText_FacilitySingle},
    [RANKING_HALL_ARENA]         = {gText_BattleArena,   gText_Facility},
    [RANKING_HALL_FACTORY]       = {gText_BattleFactory, gText_FacilitySingle},
    [RANKING_HALL_PIKE]          = {gText_BattlePike,    gText_Facility},
    [RANKING_HALL_PYRAMID]       = {gText_BattlePyramid, gText_Facility},
    [RANKING_HALL_TOWER_LINK]    = {gText_BattleTower2,  gText_FacilityLink},
};

// Trainer ID ranges for possible frontier trainers to encounter on particular challenges
// Trainers are scaled by difficulty, so higher trainer IDs have better teams
static const u16 sFrontierTrainerIdRanges[][2] =
{
    {FRONTIER_TRAINER_BRADY,   FRONTIER_TRAINER_JILL},   //   0 -  99
    {FRONTIER_TRAINER_TREVIN,  FRONTIER_TRAINER_CHLOE},  //  80 - 119
    {FRONTIER_TRAINER_ERIK,    FRONTIER_TRAINER_SOFIA},  // 100 - 139
    {FRONTIER_TRAINER_NORTON,  FRONTIER_TRAINER_JAZLYN}, // 120 - 159
    {FRONTIER_TRAINER_BRADEN,  FRONTIER_TRAINER_ALISON}, // 140 - 179
    {FRONTIER_TRAINER_ZACHERY, FRONTIER_TRAINER_LAMAR},  // 160 - 199
    {FRONTIER_TRAINER_HANK,    FRONTIER_TRAINER_TESS},   // 180 - 219
    {FRONTIER_TRAINER_JAXON,   FRONTIER_TRAINER_GRETEL}, // 200 - 299
};

static const u16 sFrontierTrainerIdRangesHard[][2] =
{
    {FRONTIER_TRAINER_ERIK,    FRONTIER_TRAINER_CHLOE},  // 100 - 119
    {FRONTIER_TRAINER_NORTON,  FRONTIER_TRAINER_SOFIA},  // 120 - 139
    {FRONTIER_TRAINER_BRADEN,  FRONTIER_TRAINER_JAZLYN}, // 140 - 159
    {FRONTIER_TRAINER_ZACHERY, FRONTIER_TRAINER_ALISON}, // 160 - 179
    {FRONTIER_TRAINER_HANK,    FRONTIER_TRAINER_LAMAR},  // 180 - 199
    {FRONTIER_TRAINER_JAXON,   FRONTIER_TRAINER_TESS},   // 200 - 219
    {FRONTIER_TRAINER_LEON,    FRONTIER_TRAINER_RAUL},   // 220 - 239
    {FRONTIER_TRAINER_JAXON,   FRONTIER_TRAINER_GRETEL}, // 200 - 299
};

static const u8 *const sLevelModeText[] =
{
    [FRONTIER_LVL_50]   = gText_RecordsLv50,
    [FRONTIER_LVL_OPEN] = gText_RecordsOpenLevel,
};

static const struct WindowTemplate sRankingHallRecordsWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 2,
    .tilemapTop = 1,
    .width = 26,
    .height = 17,
    .paletteNum = 15,
    .baseBlock = 1
};

static void (*const sFrontierUtilFuncs[])(void) =
{
    [FRONTIER_UTIL_FUNC_GET_STATUS]            = GetChallengeStatus,
    // [FRONTIER_UTIL_FUNC_GET_DATA]              = GetFrontierData,
    [FRONTIER_UTIL_FUNC_SET_DATA]              = SetFrontierData,
    // [FRONTIER_UTIL_FUNC_SET_PARTY_ORDER]       = SetSelectedPartyOrder,
    // [FRONTIER_UTIL_FUNC_SOFT_RESET]            = DoSoftReset_,
    // [FRONTIER_UTIL_FUNC_SET_TRAINERS]          = SetFrontierTrainers,
    [FRONTIER_UTIL_FUNC_SAVE_PARTY]            = SaveSelectedParty,
    // [FRONTIER_UTIL_FUNC_RESULTS_WINDOW]        = ShowFacilityResultsWindow,
    // [FRONTIER_UTIL_FUNC_CHECK_AIR_TV_SHOW]     = CheckPutFrontierTVShowOnAir,
    // [FRONTIER_UTIL_FUNC_GET_BRAIN_STATUS]      = Script_GetFrontierBrainStatus,
    // [FRONTIER_UTIL_FUNC_IS_BRAIN]              = IsTrainerFrontierBrain,
    // [FRONTIER_UTIL_FUNC_GIVE_BATTLE_POINTS]    = GiveBattlePoints,
    // [FRONTIER_UTIL_FUNC_GET_FACILITY_SYMBOLS]  = GetFacilitySymbolCount,
    // [FRONTIER_UTIL_FUNC_GIVE_FACILITY_SYMBOL]  = GiveFacilitySymbol,
    // [FRONTIER_UTIL_FUNC_CHECK_BATTLE_TYPE]     = CheckBattleTypeFlag,
    // [FRONTIER_UTIL_FUNC_CHECK_INELIGIBLE]      = CheckPartyIneligibility,
    // [FRONTIER_UTIL_FUNC_CHECK_VISIT_TRAINER]   = ValidateVisitingTrainer,
    // [FRONTIER_UTIL_FUNC_INCREMENT_STREAK]      = IncrementWinStreak,
    // [FRONTIER_UTIL_FUNC_RESTORE_HELD_ITEMS]    = RestoreHeldItems,
    // [FRONTIER_UTIL_FUNC_SAVE_BATTLE]           = SaveRecordBattle,
    // [FRONTIER_UTIL_FUNC_BUFFER_TRAINER_NAME]   = BufferFrontierTrainerName,
    // [FRONTIER_UTIL_FUNC_RESET_SKETCH_MOVES]    = ResetSketchedMoves,
    // [FRONTIER_UTIL_FUNC_SET_BRAIN_OBJECT]      = SetFacilityBrainObjectEvent,
};

// code
void CallFrontierUtilFunc(void)
{
    sFrontierUtilFuncs[gSpecialVar_0x8004]();
}

// VAR_TEMP_CHALLENGE_STATUS is used to react to the status in OnFrame map scripts
static void GetChallengeStatus(void)
{
    VarSet(VAR_TEMP_CHALLENGE_STATUS, 0xFF);
    switch (gSaveBlock2Ptr->frontier.challengeStatus)
    {
    case 0:
        break;
    case CHALLENGE_STATUS_SAVING:
        FrontierGamblerSetWonOrLost(FALSE);
        VarSet(VAR_TEMP_CHALLENGE_STATUS, gSaveBlock2Ptr->frontier.challengeStatus);
        break;
    case CHALLENGE_STATUS_LOST:
        FrontierGamblerSetWonOrLost(FALSE);
        VarSet(VAR_TEMP_CHALLENGE_STATUS, gSaveBlock2Ptr->frontier.challengeStatus);
        break;
    case CHALLENGE_STATUS_WON:
        FrontierGamblerSetWonOrLost(TRUE);
        VarSet(VAR_TEMP_CHALLENGE_STATUS, gSaveBlock2Ptr->frontier.challengeStatus);
        break;
    case CHALLENGE_STATUS_PAUSED:
        VarSet(VAR_TEMP_CHALLENGE_STATUS, gSaveBlock2Ptr->frontier.challengeStatus);
        break;
    }
}

static void SetFrontierData(void)
{
    s32 i;
    u8 facility = VarGet(VAR_FRONTIER_FACILITY);
    u8 hasSymbol = GetPlayerSymbolCountForFacility(facility);
    if (hasSymbol == 2)
        hasSymbol = 1;

    switch (gSpecialVar_0x8005)
    {
    case FRONTIER_DATA_CHALLENGE_STATUS:
        gSaveBlock2Ptr->frontier.challengeStatus = gSpecialVar_0x8006;
        break;
    case FRONTIER_DATA_LVL_MODE:
        gSaveBlock2Ptr->frontier.lvlMode = gSpecialVar_0x8006;
        break;
    case FRONTIER_DATA_BATTLE_NUM:
        gSaveBlock2Ptr->frontier.curChallengeBattleNum = gSpecialVar_0x8006;
        break;
    case FRONTIER_DATA_PAUSED:
        gSaveBlock2Ptr->frontier.challengePaused = gSpecialVar_0x8006;
        break;
    case FRONTIER_DATA_SELECTED_MON_ORDER:
        for (i = 0; i < MAX_FRONTIER_PARTY_SIZE; i++)
            gSaveBlock2Ptr->frontier.selectedPartyMons[i] = gSelectedOrderFromParty[i];
        break;
    case FRONTIER_DATA_RECORD_DISABLED:
        gSaveBlock2Ptr->frontier.disableRecordBattle = gSpecialVar_0x8006;
        break;
    case FRONTIER_DATA_HEARD_BRAIN_SPEECH:
        gSaveBlock2Ptr->frontier.battledBrainFlags |= gFrontierBrainInfo[facility].battledBit[hasSymbol];
        break;
    }
}

static void SaveSelectedParty(void)
{
    u8 i;

    for (i = 0; i < MAX_FRONTIER_PARTY_SIZE; i++)
    {
        u16 monId = gSaveBlock2Ptr->frontier.selectedPartyMons[i] - 1;
        if (monId < PARTY_SIZE)
            SavePlayerPartyMon(gSaveBlock2Ptr->frontier.selectedPartyMons[i] - 1, &gPlayerParty[i]);
    }
}

u8 GetPlayerSymbolCountForFacility(u8 facility)
{
    return FlagGet(FLAG_SYS_TOWER_SILVER + facility * 2)
         + FlagGet(FLAG_SYS_TOWER_GOLD + facility * 2);
}

static void Fill1PRecords(struct RankingHall1P *dst, s32 hallFacilityId, s32 lvlMode)
{
#if FREE_RECORD_MIXING_HALL_RECORDS == FALSE
    s32 i, j;
    struct RankingHall1P record1P[HALL_RECORDS_COUNT + 1];
    struct PlayerHallRecords *playerHallRecords = AllocZeroed(sizeof(struct PlayerHallRecords));
    GetPlayerHallRecords(playerHallRecords);

    for (i = 0; i < HALL_RECORDS_COUNT; i++)
        record1P[i] = gSaveBlock2Ptr->hallRecords1P[hallFacilityId][lvlMode][i];

    record1P[HALL_RECORDS_COUNT] = playerHallRecords->onePlayer[hallFacilityId][lvlMode];

    for (i = 0; i < HALL_RECORDS_COUNT; i++)
    {
        s32 highestWinStreak = 0;
        s32 highestId = 0;
        for (j = 0; j < HALL_RECORDS_COUNT + 1; j++)
        {
            if (record1P[j].winStreak > highestWinStreak)
            {
                highestId = j;
                highestWinStreak = record1P[j].winStreak;
            }
        }
        if (record1P[HALL_RECORDS_COUNT].winStreak >= highestWinStreak)
            highestId = HALL_RECORDS_COUNT;

        dst[i] = record1P[highestId];
        record1P[highestId].winStreak = 0;
    }

    Free(playerHallRecords);
#endif //FREE_RECORD_MIXING_HALL_RECORDS
}

static void Fill2PRecords(struct RankingHall2P *dst, s32 lvlMode)
{
#if FREE_RECORD_MIXING_HALL_RECORDS == FALSE
    s32 i, j;
    struct RankingHall2P record2P[HALL_RECORDS_COUNT + 1];
    struct PlayerHallRecords *playerHallRecords = AllocZeroed(sizeof(struct PlayerHallRecords));
    GetPlayerHallRecords(playerHallRecords);

    for (i = 0; i < HALL_RECORDS_COUNT; i++)
        record2P[i] = gSaveBlock2Ptr->hallRecords2P[lvlMode][i];

    record2P[HALL_RECORDS_COUNT] = playerHallRecords->twoPlayers[lvlMode];

    for (i = 0; i < HALL_RECORDS_COUNT; i++)
    {
        s32 highestWinStreak = 0;
        s32 highestId = 0;
        for (j = 0; j < HALL_RECORDS_COUNT; j++)
        {
            if (record2P[j].winStreak > highestWinStreak)
            {
                highestId = j;
                highestWinStreak = record2P[j].winStreak;
            }
        }
        if (record2P[HALL_RECORDS_COUNT].winStreak >= highestWinStreak)
            highestId = HALL_RECORDS_COUNT;

        dst[i] = record2P[highestId];
        record2P[highestId].winStreak = 0;
    }

    Free(playerHallRecords);
#endif //FREE_RECORD_MIXING_HALL_RECORDS
}

// Battle Frontier Ranking Hall records.
static void Print1PRecord(s32 position, s32 x, s32 y, struct RankingHall1P *hallRecord, s32 hallFacilityId)
{
    u8 text[32];
    u16 winStreak;

    AddTextPrinterParameterized(gRecordsWindowId, FONT_NORMAL, gText_123Dot[position], x * 8, (8 * (y + 5 * position)) + 1, TEXT_SKIP_DRAW, NULL);
    hallRecord->name[PLAYER_NAME_LENGTH] = EOS;
    if (hallRecord->winStreak)
    {
        TVShowConvertInternationalString(text, hallRecord->name, hallRecord->language);
        AddTextPrinterParameterized(gRecordsWindowId, FONT_NORMAL, text, (x + 2) * 8, (8 * (y + 5 * position)) + 1, TEXT_SKIP_DRAW, NULL);
        winStreak = hallRecord->winStreak;
        if (winStreak > MAX_STREAK)
            winStreak = MAX_STREAK;
        ConvertIntToDecimalStringN(gStringVar2, winStreak, STR_CONV_MODE_RIGHT_ALIGN, 4);
        StringExpandPlaceholders(gStringVar4, sHallFacilityToRecordsText[hallFacilityId]);
        AddTextPrinterParameterized(gRecordsWindowId, FONT_NORMAL, gStringVar4, GetStringRightAlignXOffset(FONT_NORMAL, sHallFacilityToRecordsText[hallFacilityId], 0xC8), (8 * (y + 5 * position)) + 1, TEXT_SKIP_DRAW, NULL);
    }
}

static void Print2PRecord(s32 position, s32 x, s32 y, struct RankingHall2P *hallRecord)
{
    u8 text[32];
    u16 winStreak;

    AddTextPrinterParameterized(gRecordsWindowId, FONT_NORMAL, gText_123Dot[position], x * 8, (8 * (y + 5 * position)) + 1, TEXT_SKIP_DRAW, NULL);
    if (hallRecord->winStreak)
    {
        hallRecord->name1[PLAYER_NAME_LENGTH] = EOS;
        hallRecord->name2[PLAYER_NAME_LENGTH] = EOS;
        TVShowConvertInternationalString(text, hallRecord->name1, hallRecord->language);
        AddTextPrinterParameterized(gRecordsWindowId, FONT_NORMAL, text, (x + 2) * 8, (8 * (y + 5 * position - 1)) + 1, TEXT_SKIP_DRAW, NULL);
        if (IsStringJapanese(hallRecord->name2))
            TVShowConvertInternationalString(text, hallRecord->name2, LANGUAGE_JAPANESE);
        else
            StringCopy(text, hallRecord->name2);
        AddTextPrinterParameterized(gRecordsWindowId, FONT_NORMAL, text, (x + 4) * 8, (8 * (y + 5 * position + 1)) + 1, TEXT_SKIP_DRAW, NULL);

        winStreak = hallRecord->winStreak;
        if (winStreak > MAX_STREAK)
            winStreak = MAX_STREAK;
        ConvertIntToDecimalStringN(gStringVar2, winStreak, STR_CONV_MODE_RIGHT_ALIGN, 4);
        StringExpandPlaceholders(gStringVar4, sHallFacilityToRecordsText[RANKING_HALL_TOWER_LINK]);
        AddTextPrinterParameterized(gRecordsWindowId, FONT_NORMAL, gStringVar4, GetStringRightAlignXOffset(FONT_NORMAL, sHallFacilityToRecordsText[RANKING_HALL_TOWER_LINK], 0xC8), (8 * (y + 5 * position)) + 1, TEXT_SKIP_DRAW, NULL);
    }
}

static void PrintHallRecords(s32 hallFacilityId, s32 lvlMode)
{
    s32 i;
    s32 x;
    struct RankingHall1P records1P[HALL_RECORDS_COUNT];
    struct RankingHall2P records2P[HALL_RECORDS_COUNT];

    StringCopy(gStringVar1, sRecordsWindowChallengeTexts[hallFacilityId][0]);
    StringExpandPlaceholders(gStringVar4, sRecordsWindowChallengeTexts[hallFacilityId][1]);
    AddTextPrinterParameterized(gRecordsWindowId, FONT_NORMAL, gStringVar4, 0, 1, TEXT_SKIP_DRAW, NULL);
    x = GetStringRightAlignXOffset(FONT_NORMAL, sLevelModeText[lvlMode], DISPLAY_WIDTH - 32);
    AddTextPrinterParameterized(gRecordsWindowId, FONT_NORMAL, sLevelModeText[lvlMode], x, 1, TEXT_SKIP_DRAW, NULL);
    if (hallFacilityId == RANKING_HALL_TOWER_LINK)
    {
        gSaveBlock2Ptr->frontier.opponentNames[0][PLAYER_NAME_LENGTH] = EOS;
        gSaveBlock2Ptr->frontier.opponentNames[1][PLAYER_NAME_LENGTH] = EOS;
        Fill2PRecords(records2P, lvlMode);
        for (i = 0; i < HALL_RECORDS_COUNT; i++)
            Print2PRecord(i, 1, 4, &records2P[i]);
    }
    else
    {
        Fill1PRecords(records1P, hallFacilityId, lvlMode);
        for (i = 0; i < HALL_RECORDS_COUNT; i++)
            Print1PRecord(i, 1, 4, &records1P[i], hallFacilityId);
    }
}

void ShowRankingHallRecordsWindow(void)
{
    gRecordsWindowId = AddWindow(&sRankingHallRecordsWindowTemplate);
    DrawStdWindowFrame(gRecordsWindowId, FALSE);
    FillWindowPixelBuffer(gRecordsWindowId, PIXEL_FILL(1));
    PrintHallRecords(gSpecialVar_0x8005, FRONTIER_LVL_50);
    PutWindowTilemap(gRecordsWindowId);
    CopyWindowToVram(gRecordsWindowId, COPYWIN_FULL);
}

void ScrollRankingHallRecordsWindow(void)
{
    FillWindowPixelBuffer(gRecordsWindowId, PIXEL_FILL(1));
    PrintHallRecords(gSpecialVar_0x8005, FRONTIER_LVL_OPEN);
    CopyWindowToVram(gRecordsWindowId, COPYWIN_GFX);
}

void SaveGameFrontier(void)
{
    s32 i;
    struct Pokemon *monsParty = AllocZeroed(sizeof(struct Pokemon) * PARTY_SIZE);

    for (i = 0; i < PARTY_SIZE; i++)
        monsParty[i] = gPlayerParty[i];

    i = gPlayerPartyCount;
    LoadPlayerParty();
    SetContinueGameWarpStatusToDynamicWarp();
    TrySavingData(SAVE_LINK);
    ClearContinueGameWarpStatus2();
    gPlayerPartyCount = i;

    for (i = 0; i < PARTY_SIZE; i++)
        gPlayerParty[i] = monsParty[i];

    Free(monsParty);
}

void ClearEnemyPartyAfterChallenge()
{
    // We zero out the Enemy's party here when the player either wins or loses the challenge since we
    // can't do it the usual way in FreeResetData_ReturnToOvOrDoEvolutions() in battle_main.c due to the
    // way facilities like the Battle Factory and the Slateport Battle Tent work
    if (gSpecialVar_0x8005 == 0)
    {
        ZeroEnemyPartyMons();
    }
}

u32 GetCurrentFacilityWinStreak(void)
{
    s32 lvlMode = gSaveBlock2Ptr->frontier.lvlMode;
    s32 battleMode = VarGet(VAR_FRONTIER_BATTLE_MODE);
    s32 facility = VarGet(VAR_FRONTIER_FACILITY);

    switch (facility)
    {
    case FRONTIER_FACILITY_TOWER:
        return gSaveBlock2Ptr->frontier.towerWinStreaks[battleMode][lvlMode];
    case FRONTIER_FACILITY_DOME:
        return gSaveBlock2Ptr->frontier.domeWinStreaks[battleMode][lvlMode];
    case FRONTIER_FACILITY_PALACE:
        return gSaveBlock2Ptr->frontier.palaceWinStreaks[battleMode][lvlMode];
    case FRONTIER_FACILITY_ARENA:
        return gSaveBlock2Ptr->frontier.arenaWinStreaks[lvlMode];
    case FRONTIER_FACILITY_FACTORY:
        return gSaveBlock2Ptr->frontier.factoryWinStreaks[battleMode][lvlMode];
    case FRONTIER_FACILITY_PIKE:
        return gSaveBlock2Ptr->frontier.pikeWinStreaks[lvlMode];
    case FRONTIER_FACILITY_PYRAMID:
        return gSaveBlock2Ptr->frontier.pyramidWinStreaks[lvlMode];
    default:
        return 0;
    }
}

u16 GetRandomScaledFrontierTrainerId(u8 challengeNum, u8 battleNum)
{
    u16 trainerId;

    if (challengeNum <= 7)
    {
        if (battleNum == FRONTIER_STAGES_PER_CHALLENGE - 1)
        {
            // The last battle in each challenge has a jump in difficulty, pulls from a table with higher ranges
            trainerId = (sFrontierTrainerIdRangesHard[challengeNum][1] - sFrontierTrainerIdRangesHard[challengeNum][0]) + 1;
            trainerId = sFrontierTrainerIdRangesHard[challengeNum][0] + (Random() % trainerId);
        }
        else
        {
            trainerId = (sFrontierTrainerIdRanges[challengeNum][1] - sFrontierTrainerIdRanges[challengeNum][0]) + 1;
            trainerId = sFrontierTrainerIdRanges[challengeNum][0] + (Random() % trainerId);
        }
    }
    else
    {
        // After challenge 7, trainer IDs always come from the last, hardest range, which is the same for both trainer ID tables
        trainerId = (sFrontierTrainerIdRanges[7][1] - sFrontierTrainerIdRanges[7][0]) + 1;
        trainerId = sFrontierTrainerIdRanges[7][0] + (Random() % trainerId);
    }

    return trainerId;
}

void SetBattleFacilityTrainerGfxId(u16 trainerId, u8 tempVarId)
{
    u32 i;
    u8 facilityClass;
    u8 trainerObjectGfxId;

    SetFacilityPtrsGetLevel();
#if FREE_BATTLE_TOWER_E_READER == FALSE
    if (trainerId == TRAINER_EREADER)
    {
        facilityClass = gSaveBlock2Ptr->frontier.ereaderTrainer.facilityClass;
    }
    else if (trainerId == TRAINER_FRONTIER_BRAIN)
#else
    if (trainerId == TRAINER_FRONTIER_BRAIN)
#endif //FREE_BATTLE_TOWER_E_READER
    {
        SetFrontierBrainObjEventGfx_2();
        return;
    }
    else if (trainerId < FRONTIER_TRAINERS_COUNT)
    {
        facilityClass = gFacilityTrainers[trainerId].facilityClass;
    }
    else if (trainerId < TRAINER_RECORD_MIXING_APPRENTICE)
    {
        facilityClass = gSaveBlock2Ptr->frontier.towerRecords[trainerId - TRAINER_RECORD_MIXING_FRIEND].facilityClass;
    }
    else
    {
        facilityClass = gApprentices[gSaveBlock1Ptr->apprentices[trainerId - TRAINER_RECORD_MIXING_APPRENTICE].id].facilityClass;
    }

    // Search male classes.
    for (i = 0; i < ARRAY_COUNT(gTowerMaleFacilityClasses); i++)
    {
        if (gTowerMaleFacilityClasses[i] == facilityClass)
            break;
    }
    if (i != ARRAY_COUNT(gTowerMaleFacilityClasses))
    {
        trainerObjectGfxId = gTowerMaleTrainerGfxIds[i];
        switch (tempVarId)
        {
        case 0:
        default:
            VarSet(VAR_OBJ_GFX_ID_0, trainerObjectGfxId);
            return;
        case 1:
            VarSet(VAR_OBJ_GFX_ID_1, trainerObjectGfxId);
            return;
        case 15:
            VarSet(VAR_OBJ_GFX_ID_E, trainerObjectGfxId);
            return;
        }
    }

    // Search female classes.
    for (i = 0; i < ARRAY_COUNT(gTowerFemaleFacilityClasses); i++)
    {
        if (gTowerFemaleFacilityClasses[i] == facilityClass)
            break;
    }
    if (i != ARRAY_COUNT(gTowerFemaleFacilityClasses))
    {
        trainerObjectGfxId = gTowerFemaleTrainerGfxIds[i];
        switch (tempVarId)
        {
        case 0:
        default:
            VarSet(VAR_OBJ_GFX_ID_0, trainerObjectGfxId);
            return;
        case 1:
            VarSet(VAR_OBJ_GFX_ID_1, trainerObjectGfxId);
            return;
        case 15:
            VarSet(VAR_OBJ_GFX_ID_E, trainerObjectGfxId);
            return;
        }
    }

    switch (tempVarId)
    {
    case 0:
    default:
        VarSet(VAR_OBJ_GFX_ID_0, OBJ_EVENT_GFX_BOY_1);
        return;
    case 1:
        VarSet(VAR_OBJ_GFX_ID_1, OBJ_EVENT_GFX_BOY_1);
        return;
    case 15:
        VarSet(VAR_OBJ_GFX_ID_E, OBJ_EVENT_GFX_BOY_1);
        return;
    }
}

void SetFrontierBrainObjEventGfx_2(void)
{
    s32 facility = VarGet(VAR_FRONTIER_FACILITY);
    VarSet(VAR_OBJ_GFX_ID_0, gFrontierBrainInfo[facility].objEventGfx);
}

#define FRONTIER_BRAIN_OTID 61226

void CreateFrontierBrainPokemon(void)
{
    s32 i, j;
    s32 selectedMonBits;
    s32 monPartyId;
    s32 monLevel = 0;
    u8 friendship;
    s32 facility = VarGet(VAR_FRONTIER_FACILITY);
    s32 symbol = GetFronterBrainSymbol();

    if (facility == FRONTIER_FACILITY_DOME)
        selectedMonBits = GetDomeTrainerSelectedMons(TrainerIdToDomeTournamentId(TRAINER_FRONTIER_BRAIN));
    else
        selectedMonBits = (1 << FRONTIER_PARTY_SIZE) - 1; // all 3 mons selected

    ZeroEnemyPartyMons();
    monPartyId = 0;
    monLevel = SetFacilityPtrsGetLevel();
    for (i = 0; i < FRONTIER_PARTY_SIZE; selectedMonBits >>= 1, i++)
    {
        if (!(selectedMonBits & 1))
            continue;

        u32 personality = GetMonPersonality(sFrontierBrainsMons[facility][symbol][i].species,
                                            MON_GENDER_RANDOM,
                                            sFrontierBrainsMons[facility][symbol][i].nature,
                                            RANDOM_UNOWN_LETTER);
        CreateMonWithIVs(&gEnemyParty[monPartyId],
                  sFrontierBrainsMons[facility][symbol][i].species,
                  monLevel,
                  personality,
                  OTID_STRUCT_PRESET(FRONTIER_BRAIN_OTID),
                  sFrontierBrainsMons[facility][symbol][i].fixedIV);
        SetMonData(&gEnemyParty[monPartyId], MON_DATA_HELD_ITEM, &sFrontierBrainsMons[facility][symbol][i].heldItem);
        for (j = 0; j < NUM_STATS; j++)
            SetMonData(&gEnemyParty[monPartyId], MON_DATA_HP_EV + j, &sFrontierBrainsMons[facility][symbol][i].evs[j]);
        friendship = MAX_FRIENDSHIP;
        for (j = 0; j < MAX_MON_MOVES; j++)
        {
            SetMonMoveSlot(&gEnemyParty[monPartyId], sFrontierBrainsMons[facility][symbol][i].moves[j], j);
            if (GetMoveEffect(sFrontierBrainsMons[facility][symbol][i].moves[j]) == EFFECT_FRUSTRATION)
                friendship = 0;
        }
        SetMonData(&gEnemyParty[monPartyId], MON_DATA_FRIENDSHIP, &friendship);
        CalculateMonStats(&gEnemyParty[monPartyId]);
        monPartyId++;
    }
}

// Frontier Trainer parties are roughly scaled in difficulty with higher trainer IDs, so scale IVs as well
// Duplicated in Battle Dome as GetDomeTrainerMonIvs
u8 GetFrontierTrainerFixedIvs(u16 trainerId)
{
    u8 fixedIv;

    if (trainerId <= FRONTIER_TRAINER_JILL)         // 0 - 99
        fixedIv = 3;
    else if (trainerId <= FRONTIER_TRAINER_CHLOE)   // 100 - 119
        fixedIv = 6;
    else if (trainerId <= FRONTIER_TRAINER_SOFIA)   // 120 - 139
        fixedIv = 9;
    else if (trainerId <= FRONTIER_TRAINER_JAZLYN)  // 140 - 159
        fixedIv = 12;
    else if (trainerId <= FRONTIER_TRAINER_ALISON)  // 160 - 179
        fixedIv = 15;
    else if (trainerId <= FRONTIER_TRAINER_LAMAR)   // 180 - 199
        fixedIv = 18;
    else if (trainerId <= FRONTIER_TRAINER_TESS)    // 200 - 219
        fixedIv = 21;
    else                                            // 220+ (- 299)
        fixedIv = MAX_PER_STAT_IVS;

    return fixedIv;
}

s32 GetFronterBrainSymbol(void)
{
    s32 facility = VarGet(VAR_FRONTIER_FACILITY);
    s32 symbol = GetPlayerSymbolCountForFacility(facility);

    if (symbol == 2)
    {
        u16 winStreak = GetCurrentFacilityWinStreak();
        if (winStreak + gFrontierBrainInfo[facility].streakAppearances[3] == gFrontierBrainInfo[facility].streakAppearances[0])
            symbol = 0;
        else if (winStreak + gFrontierBrainInfo[facility].streakAppearances[3] == gFrontierBrainInfo[facility].streakAppearances[1])
            symbol = 1;
        else if (winStreak + gFrontierBrainInfo[facility].streakAppearances[3] > gFrontierBrainInfo[facility].streakAppearances[1]
                 && (winStreak + gFrontierBrainInfo[facility].streakAppearances[3] - gFrontierBrainInfo[facility].streakAppearances[1]) % gFrontierBrainInfo[facility].streakAppearances[2] == 0)
            symbol = 1;
    }
    return symbol;
}

u8 SetFacilityPtrsGetLevel(void)
{
    // if (gSaveBlock2Ptr->frontier.lvlMode == FRONTIER_LVL_TENT)
    // {
    //     return SetTentPtrsGetLevel();
    // }
    // else
    // {
        gFacilityTrainers = gBattleFrontierTrainers;
        gFacilityTrainerMons = gBattleFrontierMons;
        return GetFrontierEnemyMonLevel(gSaveBlock2Ptr->frontier.lvlMode);
    // }
}

u8 GetFrontierEnemyMonLevel(enum FrontierLevelMode lvlMode)
{
    u8 level;

    switch (lvlMode)
    {
    default:
    case FRONTIER_LVL_50:
        level = FRONTIER_MAX_LEVEL_50;
        break;
    case FRONTIER_LVL_OPEN:
        level = GetHighestLevelInPlayerParty();
        if (level < FRONTIER_MIN_LEVEL_OPEN)
            level = FRONTIER_MIN_LEVEL_OPEN;
        break;
    }

    return level;
}

s32 GetHighestLevelInPlayerParty(void)
{
    s32 highestLevel = 0;
    s32 i;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES)
            && GetMonData(&gPlayerParty[i], MON_DATA_SPECIES_OR_EGG) != SPECIES_EGG)
        {
            s32 level = GetMonData(&gPlayerParty[i], MON_DATA_LEVEL);
            if (level > highestLevel)
                highestLevel = level;
        }
    }

    return highestLevel;
}
