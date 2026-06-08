#ifndef Z_EFF_SS_BOMB_H
#define Z_EFF_SS_BOMB_H

#include "ultra64.h"
#include "z_math.h"
#include "color.h"

typedef struct {
    Vec3f pos;
    Vec3f velocity;
    Vec3f accel;
    Color_RGBA8 primColor;
    s16 scale;
    s16 scaleStep;
    s16 life;
    s16 type; // 0 - scale up and fade, 1 - scale up, down and fade
    s16 lighting;
    s16 rotation;
} EffectSsBombInitParams; // size = 0x24

#endif
