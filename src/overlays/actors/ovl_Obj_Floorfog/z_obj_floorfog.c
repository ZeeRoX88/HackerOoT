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
#include "sys_matrix.h"
#include "z_lib.h"
#include "audio.h"
#include "play_state.h"
#include "player.h"
#include "save.h"

#include "assets/objects/gameplay_keep/gameplay_keep.h"

#define FLAGS (ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_DRAW_CULLING_DISABLED | ACTOR_FLAG_UPDATE_DURING_OCARINA)

void ObjFloorfog_Init(Actor* thisx, PlayState* play);
void ObjFloorfog_Destroy(Actor* thisx, PlayState* play);
void ObjFloorfog_Update(Actor* thisx, PlayState* play);
void ObjFloorfog_Draw(Actor* thisx, PlayState* play);

void ObjFloorfog_SetupAction(ObjFloorfog* this, ObjFloorfogActionFunc action);
void ObjFloorfog_Snow(ObjFloorfog* this, PlayState* play);

void ObjFloorfog_DrawClouds(Actor* thisx, PlayState* play);

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

static u8 sIsSpawned = false;

void ObjFloorfog_SetupAction(ObjFloorfog* this, ObjFloorfogActionFunc action) {
    this->actionFunc = action;
}

void ObjFloorfog_Init(Actor* thisx, PlayState* play) {
    ObjFloorfog* this = (ObjFloorfog*)thisx;
    s16 i;

    this->actor.room = -1;

    if (!sIsSpawned) {
        s32 bgId;
        Vec3f checkPos;

        checkPos.x = this->actor.world.pos.x;
        checkPos.y = this->actor.world.pos.y + 50.0f;
        checkPos.z = this->actor.world.pos.z;

        this->actor.floorHeight =
        BgCheck_EntityRaycastDown4(&play->colCtx, &this->actor.floorPoly, &bgId, &this->actor, &checkPos);

        for (i = 0; i < ARRAY_COUNT(this->effects); i++) {
            this->effects[i].state = 0;
        }

        ObjFloorfog_SetupAction(this, ObjFloorfog_Snow);
        sIsSpawned = true;
    } else {
        Actor_Kill(&this->actor);
    }
}

void ObjFloorfog_Destroy(Actor* thisx, PlayState* play) {
    Actor_Kill(thisx);
}

void ObjFloorfog_Snow(ObjFloorfog* this, PlayState* play) {
}

void ObjFloorfog_Update(Actor* thisx, PlayState* play) {
    ObjFloorfog* this = (ObjFloorfog*)thisx;

    this->actionFunc(this, play);
}

#define EFFECT_DIST_MAX 250.0f

void ObjFloorfog_Draw(Actor* thisx, PlayState* play) {
    ObjFloorfog* this = (ObjFloorfog*)thisx;
    Player* player = GET_PLAYER(play);
    u8 i;

    if (!(play->cameraPtrs[CAM_ID_MAIN]->stateFlags & CAM_STATE_CAMERA_IN_WATER)) {
        OPEN_DISPS(play->state.gfxCtx, "../z_object_kankyo.c", 958);

        for (i = 0; i < FLOORFOG_EFFECT_MAX; i++) {
            switch (this->effects[i].state) {
                s16 randAngle;
                s16 randDist;
                s16 effectRot;
                s16 diff;
                u8 test;

                case 0: // init
                    randAngle = (s16)(Rand_Next() >> 16); // fastest way to calculate random binary angle, Thar0
                    randDist = (s16)(EFFECT_DIST_MAX * sqrtf(Rand_ZeroOne()));

                    this->effects[i].pos.x = this->actor.world.pos.x + (Math_SinS(randAngle) * randDist);
                    this->effects[i].pos.y = this->actor.floorHeight + 10.0f;
                    this->effects[i].pos.z = this->actor.world.pos.z + (Math_CosS(randAngle) * randDist);

                    this->effects[i].alpha = 255;
                    this->effects[i].scale = Rand_ZeroOne() + 1.5f;

                    this->effects[i].state++;
                    break;
                case 1: // moving
                    this->effects[i].pos.x += (Math_SinS(this->actor.world.rot.y) * 0.5f);
                    this->effects[i].pos.z += (Math_CosS(this->actor.world.rot.y) * 0.5f);

                    if (Math_Vec3f_DistXZ(&this->effects[i].pos, &this->actor.world.pos) > EFFECT_DIST_MAX) {
                        this->effects[i].alpha = CLAMP_MIN(this->effects[i].alpha - 10, 0);
                        if (this->effects[i].alpha == 0) {
                            this->effects[i].state++;
                        }
                    } else {
                        if (Math_Vec3f_DistXZ(&this->effects[i].pos, &player->actor.world.pos) < 35.0f) {
                            this->effects[i].alpha = CLAMP_MIN(this->effects[i].alpha - 50, 0);
                        } else {
                            test = (u8)(127.0f * Math_SinS((play->gameplayFrames + (s16)this->effects[i].pos.x) * 0x200) + 127.0f);
                            this->effects[i].alpha = CLAMP(this->effects[i].alpha + 10, 0, test);
                        }
                    }
                    break;
                case 2: // reset location
                    effectRot = Math_Vec3f_Yaw(&this->actor.world.pos, &this->effects[i].pos);
                    diff = this->actor.world.rot.y - effectRot;
                    this->effects[i].pos.x = this->actor.world.pos.x + (Math_SinS(this->actor.world.rot.y + 0x8000 + diff) * 250.0f);
                    this->effects[i].pos.z = this->actor.world.pos.z + (Math_CosS(this->actor.world.rot.y + 0x8000 + diff) * 250.0f);
                    this->effects[i].state = 1;
                    break;
                default:
                    break;
            }

            Matrix_Translate(this->effects[i].pos.x,
                             this->effects[i].pos.y,
                             this->effects[i].pos.z, MTXMODE_NEW);
            Matrix_RotateX(BINANG_TO_RAD(-0x4000), MTXMODE_APPLY);
            Matrix_Scale(this->effects[i].scale, this->effects[i].scale, this->effects[i].scale, MTXMODE_APPLY);
            gDPPipeSync(POLY_XLU_DISP++);

            gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 200, 200, 200, this->effects[i].alpha);
            gDPSetEnvColor(POLY_XLU_DISP++, 200, 200, 200, this->effects[i].alpha);

            MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx, "../z_object_kankyo.c", 1107);

            gSPSegment(POLY_XLU_DISP++, 0x08, SEGMENTED_TO_VIRTUAL(gDust5Tex));

            Gfx_SetupDL_61Xlu(play->state.gfxCtx);
            gDPSetAlphaCompare(POLY_XLU_DISP++, G_AC_THRESHOLD); // disable this for emulator
            // gDPSetAlphaDither(POLY_XLU_DISP++, G_AD_DISABLE); // set this for emulator

            // gSPMatrix(POLY_XLU_DISP++, &D_01000000, G_MTX_NOPUSH | G_MTX_MUL | G_MTX_MODELVIEW); // billboard mtx

            gDPPipeSync(POLY_XLU_DISP++);

            gSPDisplayList(POLY_XLU_DISP++, gEffDustDL);

            gDPPipeSync(POLY_XLU_DISP++);
        }

        CLOSE_DISPS(play->state.gfxCtx, "../z_object_kankyo.c", 1127);
    }
}
