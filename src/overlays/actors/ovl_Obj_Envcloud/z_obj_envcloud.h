#ifndef Z_OBJ_ENVCLOUD_H
#define Z_OBJ_ENVCLOUD_H

#include "ultra64.h"
#include "actor.h"

struct ObjEnvcloud;

typedef void (*ObjEnvcloudActionFunc)(struct ObjEnvcloud*, struct PlayState*);

typedef struct ObjEnvcloudEffect {
    /* 0x00 */ u8 state;
    /* 0x04 */ Vec3f pos; // relative to base
    /* 0x10 */ Vec3f prevPos;
    /* 0x1C */ Vec3f base;
    /* 0x34 */ f32 speed;
    /* 0x3E */ u16 angle;
    /* 0x40 */ u8 alpha;
    /* 0x44 */ f32 size;
} ObjEnvcloudEffect; // size = 0x54

typedef struct ObjEnvcloud {
    /* 0x0000 */ Actor actor;
    /* 0x014C */ ObjEnvcloudEffect effects[64];
    /* 0x165C */ ObjEnvcloudActionFunc actionFunc;
} ObjEnvcloud; // size = 0x1660

#endif
