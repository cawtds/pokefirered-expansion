#ifndef GUARD_CONSTANTS_BERRY_H
#define GUARD_CONSTANTS_BERRY_H

#define FOREACH_BERRY(F) \
    F(CHERI) \
    F(CHESTO) \
    F(PECHA) \
    F(RAWST) \
    F(ASPEAR) \
    F(LEPPA) \
    F(ORAN) \
    F(PERSIM) \
    F(LUM) \
    F(SITRUS) \
    F(FIGY) \
    F(WIKI) \
    F(MAGO) \
    F(AGUAV) \
    F(IAPAPA) \
    F(RAZZ) \
    F(BLUK) \
    F(NANAB) \
    F(WEPEAR) \
    F(PINAP) \
    F(POMEG) \
    F(KELPSY) \
    F(QUALOT) \
    F(HONDEW) \
    F(GREPA) \
    F(TAMATO) \
    F(CORNN) \
    F(MAGOST) \
    F(RABUTA) \
    F(NOMEL) \
    F(SPELON) \
    F(PAMTRE) \
    F(WATMEL) \
    F(DURIN) \
    F(BELUE) \
    F(OCCA) \
    F(PASSHO) \
    F(WACAN) \
    F(RINDO) \
    F(YACHE) \
    F(CHOPLE) \
    F(KEBIA) \
    F(SHUCA) \
    F(COBA) \
    F(PAYAPA) \
    F(TANGA) \
    F(CHARTI) \
    F(KASIB) \
    F(HABAN) \
    F(COLBUR) \
    F(BABIRI) \
    F(CHILAN) \
    F(LIECHI) \
    F(GANLON) \
    F(SALAC) \
    F(PETAYA) \
    F(APICOT) \
    F(LANSAT) \
    F(STARF) \
    F(ENIGMA) \
    F(MICLE) \
    F(CUSTAP) \
    F(JABOCA) \
    F(ROWAP) \
    F(ROSELI) \
    F(KEE) \
    F(MARANGA)

#define UNPACK_BERRY_ID(_berry) BERRY_ID_##_berry,

enum BerryId
{
    BERRY_ID_NONE,
    FOREACH_BERRY(UNPACK_BERRY_ID)
    BERRY_ID_ENIGMA_E_READER,
    NUM_BERRIES = BERRY_ID_ENIGMA_E_READER,
};

#undef UNPACK_BERRY_ID

enum BerryFirmness
{
    BERRY_FIRMNESS_UNKNOWN,
    BERRY_FIRMNESS_VERY_SOFT,
    BERRY_FIRMNESS_SOFT,
    BERRY_FIRMNESS_HARD,
    BERRY_FIRMNESS_VERY_HARD,
    BERRY_FIRMNESS_SUPER_HARD,
};

enum BerryColor
{
    BERRY_COLOR_RED,
    BERRY_COLOR_BLUE,
    BERRY_COLOR_PURPLE,
    BERRY_COLOR_GREEN,
    BERRY_COLOR_YELLOW,
    BERRY_COLOR_PINK,
};

enum __attribute__((__packed__)) Flavor
{
    FLAVOR_SPICY,
    FLAVOR_DRY,
    FLAVOR_SWEET,
    FLAVOR_BITTER,
    FLAVOR_SOUR,
    FLAVOR_COUNT,
};

#define BERRY_STAGE_NO_BERRY    0  // there is no tree planted and the soil is completely flat.
#define BERRY_STAGE_PLANTED     1
#define BERRY_STAGE_SPROUTED    2
#define BERRY_STAGE_TALLER      3
#define BERRY_STAGE_FLOWERING   4
#define BERRY_STAGE_BERRIES     5
#define BERRY_STAGE_TRUNK       6 // These follow BERRY_STAGE_BERRIES to preserve save compatibility
#define BERRY_STAGE_BUDDING     7
#define BERRY_STAGE_SPARKLING   255

// Berries can be watered in the following stages:
// - BERRY_STAGE_PLANTED
// - BERRY_STAGE_SPROUTED
// - BERRY_STAGE_TALLER
// - BERRY_STAGE_FLOWERING
#define NUM_WATER_STAGES 4

// IDs for berry tree objects, indexes into berryTrees in SaveBlock1
// Named for whatever berry is initially planted there on a new game
// Those with no initial berry are named "soil"
// 90 reserved
enum BerryTreeId
{
    BERRY_TREES_COUNT = 90,
};


#endif // GUARD_CONSTANTS_BERRY_H
