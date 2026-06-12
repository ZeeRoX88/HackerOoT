/*
 * File: z_eff_ss_bomb.c
 * Overlay: ovl_Effect_Ss_Bomb
 * Description: Bomb Blast. Unused in the orignal game.
 */

#include "z_eff_ss_bomb.h"

#include "libc64/qrand.h"
#include "array_count.h"
#include "gfx.h"
#include "gfx_setupdl.h"
#include "segmented_address.h"
#include "sys_matrix.h"
#include "z_lib.h"
#include "effect.h"
#include "play_state.h"
#include "skin_matrix.h"

#include "assets/objects/gameplay_keep/gameplay_keep.h"
#include "assets/objects/gameplay_hacker_keep/gameplay_hacker_keep.h"

#define rPrimColorR regs[0]
#define rPrimColorG regs[1]
#define rPrimColorB regs[2]
#define rPrimColorA regs[3]

#define rLighting regs[4]

#define rEnvColorG regs[5]
#define rEnvColorB regs[6]
#define rEnvColorA regs[7]

#define rRotationDirection regs[8]
#define rScale regs[9]
#define rScaleStep regs[10]
#define rRotation regs[11]
#define rLifespan regs[12]

u32 EffectSsBomb_Init(PlayState* play, u32 index, EffectSs* this, void* initParamsx);
void EffectSsBomb_Draw(PlayState* play, u32 index, EffectSs* this);
void EffectSsBomb_Update(PlayState* play, u32 index, EffectSs* this);

EffectSsProfile Effect_Ss_Bomb_Profile = {
    EFFECT_SS_BOMB,
    EffectSsBomb_Init,
};

u32 EffectSsBomb_Init(PlayState* play, u32 index, EffectSs* this, void* initParamsx) {
    EffectSsBombInitParams* initParams = (EffectSsBombInitParams*)initParamsx;

    Math_Vec3f_Copy(&this->pos, &initParams->pos);
    Math_Vec3f_Copy(&this->velocity, &initParams->velocity);
    Math_Vec3f_Copy(&this->accel, &initParams->accel);
    this->life = initParams->life;
    this->update = EffectSsBomb_Update;
    this->draw = EffectSsBomb_Draw;
    this->type = initParams->type;

    this->rPrimColorR = initParams->primColor.r;
    this->rPrimColorG = initParams->primColor.g;
    this->rPrimColorB = initParams->primColor.b;
    this->rLighting = initParams->lighting;

    this->rPrimColorA = initParams->primColor.a;
    this->rScale = initParams->scale;
    this->rScaleStep = initParams->scaleStep;
    this->rLifespan = initParams->life;
    this->rRotation = (s16)(Rand_Next() >> 16); // random initial rotation
    this->rRotationDirection = initParams->rotation;

    return 1;
}

void EffectSsBomb_Draw(PlayState* play, u32 index, EffectSs* this) {
    f32 scale;
    GraphicsContext* gfxCtx = play->state.gfxCtx;

    OPEN_DISPS(gfxCtx, "../z_eff_ss_bomb.c", 168);

    scale = this->rScale * 0.0025f;
    Matrix_Translate(this->pos.x, this->pos.y, this->pos.z, MTXMODE_NEW);
    Matrix_Scale(scale, scale, 1.0f, MTXMODE_APPLY);
    Matrix_ReplaceRotation(&play->billboardMtxF);
    Matrix_RotateZ(BINANG_TO_RAD(this->rRotation), MTXMODE_APPLY);
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx, "../z_kankyo.c", 2364);

    if (this->rLighting == 0) { // lighting off
        gDPSetCombineLERP(POLY_XLU_DISP++, 0, 0, 0, TEXEL0, 0, 0, 0, TEXEL0, COMBINED, 0, PRIMITIVE, 0, COMBINED, 0, PRIMITIVE, 0);
    } else { // lighting on
        gDPSetCombineLERP(POLY_XLU_DISP++, TEXEL0, 0, SHADE, 0, 0, 0, 0, TEXEL0, COMBINED, 0, PRIMITIVE, 0, COMBINED, 0, PRIMITIVE, 0);
    }

    gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, this->rPrimColorR, this->rPrimColorG, this->rPrimColorB, this->rPrimColorA);
    // gDPSetEnvColor(POLY_XLU_DISP++, this->rEnvColorR, this->rEnvColorG, this->rEnvColorB, this->rEnvColorA);
    gSPDisplayList(POLY_XLU_DISP++, gOEffDustDL);

    CLOSE_DISPS(gfxCtx, "../z_eff_ss_bomb.c", 214);
}

void EffectSsBomb_Update(PlayState* play, u32 index, EffectSs* this) {
    this->accel.x = (Rand_ZeroOne() * 0.4f) - 0.2f;
    this->accel.z = (Rand_ZeroOne() * 0.4f) - 0.2f;

    if (this->type == 1 && (this->life < (this->rLifespan >> 1) /* this->rLifespan / 2 */)) {
        this->rScale -= CLAMP_MIN(this->rScaleStep, 0);
    } else {
        this->rScale += this->rScaleStep;
    }

    if (this->rRotationDirection == 0) {
        this->rRotation += 0x900;
    } else {
        this->rRotation -= 0x900;
    }
    // if (this->life < (this->rLifespan >> 1)/* this->rLifespan / 2 */) {
    //     this->rPrimColorA = CLAMP_MIN(this->rPrimColorA - (255 / (this->rLifespan >> 1)), 0);
    // }
}
