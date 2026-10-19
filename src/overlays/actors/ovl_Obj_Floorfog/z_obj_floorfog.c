/*
 * File: z_object_kankyo.c
 * Overlay: ovl_Object_Kankyo
 * Description: Environmental Effects
 */

#include "z_obj_floorfog.h"

#include "libc64/qrand.h"
#include "array_count.h"
#include "gfx.h"
#include "gfx_setupdl.h"
#include "segmented_address.h"
#include "sequence.h"
#include "sfx.h"
#include "sys_math3d.h"
#include "sys_math.h"
#include "sys_matrix.h"
#include "z_lib.h"
#include "audio.h"
#include "play_state.h"
#include "player.h"
#include "save.h"
#include "attributes.h"
#include "z_debug.h"

#include "assets/objects/gameplay_keep/gameplay_keep.h"

#define FLAGS (ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_DRAW_CULLING_DISABLED | ACTOR_FLAG_UPDATE_DURING_OCARINA)

void ObjFloorfog_Init(Actor* thisx, PlayState* play);
void ObjFloorfog_Destroy(Actor* thisx, PlayState* play);
void ObjFloorfog_Update(Actor* thisx, PlayState* play);
void ObjFloorfog_Draw(Actor* thisx, PlayState* play);

ActorProfile Obj_Floorfog_Profile = {
    /**/ ACTOR_OBJ_FLOORFOG,
    /**/ ACTORCAT_ITEMACTION,
    /**/ FLAGS,
    /**/ OBJECT_GAMEPLAY_KEEP,
    /**/ sizeof(ObjFloorfog),
    /**/ ObjFloorfog_Init,
    /**/ ObjFloorfog_Destroy,
    /**/ ObjFloorfog_Update,
    /**/ ObjFloorfog_Draw,
};

void ObjFloorfog_Init(Actor* thisx, PlayState* play) {
    ObjFloorfog* this = (ObjFloorfog*)thisx;

    this->effectPrimColor.r = this->effectPrimColor.g = this->effectPrimColor.b = 200;
    this->effectEnvColor.r = this->effectEnvColor.g = this->effectEnvColor.b = 200;

    // add params for individual radii and for smaller variant, which can be easily used in different areas
}

void ObjFloorfog_Destroy(Actor* thisx, PlayState* play) {
    Actor_Kill(thisx);
}

void ObjFloorfog_Update(Actor* thisx, PlayState* play) {
    ObjFloorfog* this = (ObjFloorfog*)thisx;
}

#define EFFECT_DIST_MAX 250.0f
#define EFFECT_COUNT_MAX 70 // for testing

void ObjFloorfog_Draw(Actor* thisx, PlayState* play) {
    ObjFloorfog* this = (ObjFloorfog*)thisx;
    Player* player = GET_PLAYER(play);

    if (!(play->cameraPtrs[CAM_ID_MAIN]->stateFlags & CAM_STATE_CAMERA_IN_WATER)) {
        f32 phi = 1.618f; // (sqrtf(5.0f) + 1.0f) / 2.0f;
        f32 goldenAngle = 2.0f * M_PI / Math_PowF(phi, 2); // 2.3999f
        u8 i;

        OPEN_DISPS(play->state.gfxCtx, "../z_object_kankyo.c", 958);

        POLY_XLU_DISP = Gfx_SetupDL(POLY_XLU_DISP, SETUPDL_0);
        gDPSetCombineLERP(POLY_XLU_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, PRIMITIVE, 0, TEXEL0, 0,
                          COMBINED, 0, PRIMITIVE, 0, 0, 0, 0, COMBINED);
        gDPSetRenderMode(POLY_XLU_DISP++, G_RM_FOG_SHADE_A, G_RM_ZB_CLD_SURF2);
        gSPSetGeometryMode(POLY_XLU_DISP++, G_FOG | G_LIGHTING);

        // gDPSetAlphaCompare(POLY_XLU_DISP++, G_AC_THRESHOLD); // disable this for emulator
        gDPSetAlphaCompare(POLY_XLU_DISP++, G_AC_NONE); // set this for emulator
        gDPSetAlphaDither(POLY_XLU_DISP++, G_AD_DISABLE); // set this for emulator

        for (i = 0; i < EFFECT_COUNT_MAX; i++) {
            this->effects[i].pos.x += (Math_SinS(this->actor.world.rot.y) * 0.5f);
            this->effects[i].pos.z += (Math_CosS(this->actor.world.rot.y) * 0.5f);
            this->effects[i].angle += 0x100;

            switch (this->effects[i].state) {
                f32 r;
                f32 theta;
                f32 dist;
                s16 effectRot;
                s16 diff;
                u8 sineAlpha;

                case 0: // init
                    // sunflower algorithm for uniform positions
                    r = EFFECT_DIST_MAX * sqrtf((f32)i / (f32)(EFFECT_COUNT_MAX - 1));
                    theta = i * goldenAngle;
                    this->effects[i].pos.x = this->actor.world.pos.x + (r * cosf(theta));
                    this->effects[i].pos.y = this->actor.world.pos.y + 10.0f;
                    this->effects[i].pos.z = this->actor.world.pos.z + (r * sinf(theta));

                    this->effects[i].scale = Rand_ZeroOne() + 2.0f;
                    this->effects[i].angle = i * 0x400;

                    this->effects[i].state = 1;
                    break;
                case 1: // inside max distance, regular movement and alpha
                    // when out of boundary area
                    if (Math_Vec3f_DistXZ(&this->effects[i].pos, &this->actor.world.pos) >= EFFECT_DIST_MAX) {
                        this->effects[i].alpha = CLAMP_MIN(this->effects[i].alpha - 10, 0);
                        if (this->effects[i].alpha == 0) {
                            this->effects[i].state = 2;
                        }
                    } else {
                        // player is close
                        if (Math_Vec3f_DistXZ(&this->effects[i].pos, &player->actor.world.pos) < 35.0f) {
                            this->effects[i].alpha = CLAMP_MIN(this->effects[i].alpha - 20, 0);
                        } else { // regular alpha oscillating
                            // sineAlpha = (u8)(100.0f * Math_SinS((play->gameplayFrames - (s16)this->effects[i].pos.x) * 0x200) + 100.0f);
                            sineAlpha = (u8)(50.0f * Math_SinS((play->gameplayFrames - (s16)this->effects[i].pos.x) * 0x200) + 50.0f) + (50.0f * Math_CosS((play->gameplayFrames - (s16)this->effects[i].pos.z) * 0x200) + 50.0f);
                            this->effects[i].alpha = CLAMP(this->effects[i].alpha + 5, 0, sineAlpha);
                        }
                    }
                    break;
                case 2: // reset location
                    dist = Math_Vec3f_DistXZ(&this->effects[i].pos, &this->actor.world.pos);
                    effectRot = Math_Vec3f_Yaw(&this->actor.world.pos, &this->effects[i].pos);
                    diff = this->actor.world.rot.y - effectRot;
                    this->effects[i].pos.x = this->actor.world.pos.x + (Math_SinS(this->actor.world.rot.y + 0x8000 + diff) * dist);
                    this->effects[i].pos.z = this->actor.world.pos.z + (Math_CosS(this->actor.world.rot.y + 0x8000 + diff) * dist);
                    this->effects[i].state = 3;
                    break;
                case 3: // reappearing outside max distance
                    if (Math_Vec3f_DistXZ(&this->effects[i].pos, &this->actor.world.pos) < EFFECT_DIST_MAX) {
                        this->effects[i].state = 1;
                    }
                    break;
            }

            Matrix_Translate(this->effects[i].pos.x,
                             this->effects[i].pos.y,
                             this->effects[i].pos.z, MTXMODE_NEW);
            Matrix_RotateX(BINANG_TO_RAD(-0x4000), MTXMODE_APPLY);
            Matrix_RotateZ(BINANG_TO_RAD(this->effects[i].angle), MTXMODE_APPLY);
            Matrix_Scale(this->effects[i].scale, this->effects[i].scale, this->effects[i].scale, MTXMODE_APPLY);
            gDPPipeSync(POLY_XLU_DISP++);

            gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, this->effectPrimColor.r, this->effectPrimColor.g, this->effectPrimColor.b, this->effects[i].alpha);
            gDPSetEnvColor(POLY_XLU_DISP++, this->effectEnvColor.r, this->effectEnvColor.g, this->effectEnvColor.b, this->effects[i].alpha);

            MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx, "../z_object_kankyo.c", 1107);

            gSPSegment(POLY_XLU_DISP++, 0x08, SEGMENTED_TO_VIRTUAL(gDust5Tex));

            gDPPipeSync(POLY_XLU_DISP++);

            gSPDisplayList(POLY_XLU_DISP++, gEffDustDL);

            gDPPipeSync(POLY_XLU_DISP++);
        }

        CLOSE_DISPS(play->state.gfxCtx, "../z_object_kankyo.c", 1127);
    }
}
