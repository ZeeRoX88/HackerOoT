/*
 * File: z_object_kankyo.c
 * Overlay: ovl_Object_Kankyo
 * Description: Environmental Effects
 */

#include "z_obj_envcloud.h"

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

void ObjEnvcloud_Init(Actor* thisx, PlayState* play);
void ObjEnvcloud_Destroy(Actor* thisx, PlayState* play);
void ObjEnvcloud_Update(Actor* thisx, PlayState* play);
void ObjEnvcloud_Draw(Actor* thisx, PlayState* play);

void ObjEnvcloud_SetupAction(ObjEnvcloud* this, ObjEnvcloudActionFunc action);
void ObjEnvcloud_Snow(ObjEnvcloud* this, PlayState* play);

void ObjEnvcloud_DrawClouds(Actor* thisx, PlayState* play);

ActorProfile Obj_Envcloud_Profile = {
    /**/ ACTOR_OBJ_ENVCLOUD,
    /**/ ACTORCAT_ITEMACTION,
    /**/ FLAGS,
    /**/ OBJECT_GAMEPLAY_KEEP,
    /**/ sizeof(ObjEnvcloud),
    /**/ ObjEnvcloud_Init,
    /**/ ObjEnvcloud_Destroy,
    /**/ ObjEnvcloud_Update,
    /**/ ObjEnvcloud_Draw,
};

static u8 sIsSpawned = false;
static s16 sTrailingFairies = 0;

void ObjEnvcloud_SetupAction(ObjEnvcloud* this, ObjEnvcloudActionFunc action) {
    this->actionFunc = action;
}

void ObjEnvcloud_Init(Actor* thisx, PlayState* play) {
    ObjEnvcloud* this = (ObjEnvcloud*)thisx;
    s32 pad;
    s16 i;

    for (i = 0; i < ARRAY_COUNT(this->effects); i++) {
        this->effects[i].state = 0;
    }

    this->actor.room = -1;

    if (!sIsSpawned) {
        ObjEnvcloud_SetupAction(this, ObjEnvcloud_Snow);
        sIsSpawned = true;
    } else {
        Actor_Kill(&this->actor);
    }
}

void ObjEnvcloud_Destroy(Actor* thisx, PlayState* play) {
    Actor_Kill(thisx);
}

void ObjEnvcloud_Snow(ObjEnvcloud* this, PlayState* play) {
}

void ObjEnvcloud_Update(Actor* thisx, PlayState* play) {
    ObjEnvcloud* this = (ObjEnvcloud*)thisx;

    this->actionFunc(this, play);
}

void ObjEnvcloud_Draw(Actor* thisx, PlayState* play) {
    ObjEnvcloud* this = (ObjEnvcloud*)thisx;
    f32 dist;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 maxDist;
    f32 baseX;
    f32 baseY;
    f32 baseZ;
    s16 i;
    Vec3f viewForwards;
    Player* player = GET_PLAYER(play);

    if (!(play->cameraPtrs[CAM_ID_MAIN]->stateFlags & CAM_STATE_CAMERA_IN_WATER)) {
        OPEN_DISPS(play->state.gfxCtx, "../z_object_kankyo.c", 958);

        for (i = 0; i < 10; i++) {
            switch (this->effects[i].state) {
                case 0: // init
                    // spawn in front of the camera
                    dx = play->view.at.x - play->view.eye.x;
                    dy = play->view.at.y - play->view.eye.y;
                    dz = play->view.at.z - play->view.eye.z;
                    dist = sqrtf(SQ(dx) + SQ(dy) + SQ(dz));

                    viewForwards.x = dx / dist;
                    viewForwards.y = dy / dist;
                    viewForwards.z = dz / dist;

                    // fake
                    this->effects[i].base.x = play->view.eye.x + viewForwards.x * 120.0f;
                    this->effects[i].base.y = play->view.eye.y + viewForwards.y * 120.0f;
                    this->effects[i].base.z = play->view.eye.z + viewForwards.z * 120.0f;

                    this->effects[i].pos.x = (Rand_ZeroOne() - 0.5f) * 160.0f;
                    this->effects[i].pos.y = (Rand_ZeroOne() - 0.5f) * 80.0f;
                    this->effects[i].pos.z = (Rand_ZeroOne() - 0.5f) * 160.0f;
                    // hmm
                    /* if (this->effects[i].base.y + this->effects[i].pos.y < 50.0f) {
                        this->effects[i].base.y = 50.0f;
                    } */
                    this->effects[i].speed = Rand_ZeroOne() * 5.0f + 0.5f;
                    this->effects[i].dirPhase.x = Rand_ZeroOne() * 360.0f;
                    this->effects[i].dirPhase.z = Rand_ZeroOne() * 360.0f;
                    this->effects[i].alpha = 0;
                    this->effects[i].state++;
                    break;

                case 1: // moving
                    dx = play->view.at.x - play->view.eye.x;
                    dy = play->view.at.y - play->view.eye.y;
                    dz = play->view.at.z - play->view.eye.z;
                    dist = sqrtf(SQ(dx) + SQ(dy) + SQ(dz));

                    viewForwards.x = dx / dist;
                    viewForwards.y = dy / dist;
                    viewForwards.z = dz / dist;

                    baseX = play->view.eye.x + viewForwards.x * 120.0f;
                    baseY = play->view.eye.y + viewForwards.y * 120.0f;
                    baseZ = play->view.eye.z + viewForwards.z * 120.0f;

                    // movement
                    this->effects[i].pos.x += 0.5f; // sinf(0.05f); // direction
                    this->effects[i].pos.z += 0.5f; // cosf(0.05f); // direction

                    maxDist = 120.0f;

                    Vec3f test;
                    test.x = this->effects[i].pos.x + this->effects[i].base.x;
                    test.z = this->effects[i].pos.z + this->effects[i].base.z;
                    if (Math_Vec3f_DistXZ(&test, &play->view.eye) <= 80.0f ||
                        this->effects[i].base.x + this->effects[i].pos.x - baseX > maxDist ||
                        this->effects[i].base.x + this->effects[i].pos.x - baseX < -maxDist ||
                        this->effects[i].base.y + this->effects[i].pos.y - baseY > 50.0f ||
                        this->effects[i].base.y + this->effects[i].pos.y - baseY < -50.0f ||
                        this->effects[i].base.z + this->effects[i].pos.z - baseZ > maxDist ||
                        this->effects[i].base.z + this->effects[i].pos.z - baseZ < -maxDist) {

                        this->effects[i].alpha = CLAMP_MIN(this->effects[i].alpha - 2, 0);

                        if (this->effects[i].alpha == 0) {
                            this->effects[i].state = 0;
                        }

                        // when off screen, wrap around to the other side
                        /* if (this->effects[i].base.x + this->effects[i].pos.x - baseX > maxDist && this->effects[i].alpha == 0) {
                            this->effects[i].base.x = baseX - maxDist;
                            this->effects[i].pos.x = 0.0f;
                        }
                        if (this->effects[i].base.x + this->effects[i].pos.x - baseX < -maxDist && this->effects[i].alpha == 0) {
                            this->effects[i].base.x = baseX + maxDist;
                            this->effects[i].pos.x = 0.0f;
                        }
                        if (this->effects[i].base.z + this->effects[i].pos.z - baseZ > maxDist && this->effects[i].alpha == 0) {
                            this->effects[i].base.z = baseZ - maxDist;
                            this->effects[i].pos.z = 0.0f;
                        }
                        if (this->effects[i].base.z + this->effects[i].pos.z - baseZ < -maxDist && this->effects[i].alpha == 0) {
                            this->effects[i].base.z = baseZ + maxDist;
                            this->effects[i].pos.z = 0.0f;
                        }
                        if (this->effects[i].base.y + this->effects[i].pos.y - baseY > 50.0f && this->effects[i].alpha == 0) {
                            this->effects[i].base.y = baseY - 50.0f;
                            this->effects[i].pos.y = 0.0f;
                        }
                        if (this->effects[i].base.y + this->effects[i].pos.y - baseY < -50.0f && this->effects[i].alpha == 0) {
                            this->effects[i].base.y = baseY + 50.0f;
                            this->effects[i].pos.y = 0.0f;
                        } */
                    } else {
                        this->effects[i].alpha = CLAMP_MAX(this->effects[i].alpha + 1, 20);
                    }
                    break;

                case 2: // reset
                    this->effects[i].state = 0;
                    break;

                default:
                    break;
            }

            Matrix_Translate(this->effects[i].base.x + this->effects[i].pos.x,
                             this->effects[i].base.y + this->effects[i].pos.y,
                             this->effects[i].base.z + this->effects[i].pos.z, MTXMODE_NEW);
            Matrix_Scale(1.5f, 1.5f, 1.5f, MTXMODE_APPLY);
            gDPPipeSync(POLY_XLU_DISP++);

            gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 200, 200, 200, this->effects[i].alpha);
            gDPSetEnvColor(POLY_XLU_DISP++, 200, 200, 200, this->effects[i].alpha);

            MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx, "../z_object_kankyo.c", 1107);

            gSPSegment(POLY_XLU_DISP++, 0x08, SEGMENTED_TO_VIRTUAL(gDust5Tex));

            Gfx_SetupDL_61Xlu(play->state.gfxCtx);
            // gDPSetAlphaCompare(POLY_XLU_DISP++, G_AC_THRESHOLD); // disable this for emulator
            gDPSetAlphaDither(POLY_XLU_DISP++, G_AD_DISABLE); // set this for emulator

            gSPMatrix(POLY_XLU_DISP++, &D_01000000, G_MTX_NOPUSH | G_MTX_MUL | G_MTX_MODELVIEW); // billboard mtx

            gDPPipeSync(POLY_XLU_DISP++);

            gSPDisplayList(POLY_XLU_DISP++, gEffDustDL);

            gDPPipeSync(POLY_XLU_DISP++);
        }

        CLOSE_DISPS(play->state.gfxCtx, "../z_object_kankyo.c", 1127);
    }
}
