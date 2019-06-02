// #define REGION_JAPAN
// #define REGION_USA
#define REGION_EUROPE

#if defined(REGION_JAPAN)
#define REGION_SAVE_FILENAME "BICHEN-00001_%d"
#elif defined(REGION_USA)
#define REGION_SAVE_FILENAME "BACHEN-00001_%d"
#elif defined(REGION_EUROPE)
#define TV_PAL
#define REGION_SAVE_FILENAME "BECHEN-00001_%d"
#else
#error No defined region!
#endif

#if 1
#define VID_WIDTH 640
#define VID_WIDTH_MULTIPLIER 2
#else
#define VID_WIDTH 320
#define VID_WIDTH_MULTIPLIER 1
#endif
#define VID_HEIGHT 240
#define VID_HEIGHT_MULTIPLIER 1

#ifdef TV_PAL
#define VBLANKS_PER_SEC 50
#else
#define VBLANKS_PER_SEC 60
#endif

#define LAVA_ANIMATION_START 80
#define LAVA_ANIMATION_FRAMES 20
#define WATER_ANIMATION_START 100
#define WATER_ANIMATION_FRAMES 32
