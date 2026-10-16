#ifndef Z_OBJ_FLOORFOG_H
#define Z_OBJ_FLOORFOG_H

#include "ultra64.h"
#include "actor.h"

#define FLOORFOG_EFFECT_MAX 99

struct ObjFloorfog;

typedef struct ObjFloorfogEffect {
    /* 0x00 */ u8 state;
    /* 0x04 */ Vec3f pos;
    /* 0x40 */ u8 alpha;
               f32 scale;
               s16 angle;
} ObjFloorfogEffect;

typedef struct ObjFloorfog {
    /* 0x0000 */ Actor actor;
    /* 0x014C */ ObjFloorfogEffect effects[FLOORFOG_EFFECT_MAX];
} ObjFloorfog;

#endif
