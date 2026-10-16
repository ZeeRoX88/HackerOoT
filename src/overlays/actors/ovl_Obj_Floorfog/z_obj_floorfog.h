#ifndef Z_OBJ_FLOORFOG_H
#define Z_OBJ_FLOORFOG_H

#include "ultra64.h"
#include "actor.h"

#define FLOORFOG_EFFECT_MAX 70

struct ObjFloorfog;

typedef struct ObjFloorfogEffect {
    u8 state;
    Vec3f pos;
    u8 alpha;
    f32 scale;
    s16 angle;
} ObjFloorfogEffect;

typedef struct ObjFloorfog {
    Actor actor;
    ObjFloorfogEffect effects[FLOORFOG_EFFECT_MAX];
    Color_RGB8 effectPrimColor;
    Color_RGB8 effectEnvColor;
} ObjFloorfog;

#endif
