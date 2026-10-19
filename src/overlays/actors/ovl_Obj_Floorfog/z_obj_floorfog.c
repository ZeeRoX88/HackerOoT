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

#include "assets/objects/gameplay_hacker_keep/gameplay_hacker_keep.h"

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

    this->actor.world.pos.y += 50.0f;
}

void ObjFloorfog_Destroy(Actor* thisx, PlayState* play) {
    Actor_Kill(thisx);
}

void ObjFloorfog_Update(Actor* thisx, PlayState* play) {
    ObjFloorfog* this = (ObjFloorfog*)thisx;
}

void ObjFloorfog_Draw(Actor* thisx, PlayState* play) {
    ObjFloorfog* this = (ObjFloorfog*)thisx;
    Player* player = GET_PLAYER(play);
    Vtx* vtx;
    s16 i;
    s32 gameplayFrames = play->gameplayFrames;

    vtx = SEGMENTED_TO_VIRTUAL(PlaneDL_PlaneDL_mesh_layer_Transparent_vtx_0);

    if (vtx != NULL) {
        for (i = 0; i < ARRAY_COUNT(PlaneDL_PlaneDL_mesh_layer_Transparent_vtx_0); i++) {
            vtx[i].v.ob[1] = (s16)(300.0f * (Math_SinS((gameplayFrames + vtx[i].v.ob[0]) * 0x200) + Math_CosS((gameplayFrames + vtx[i].v.ob[2]) * 0x200))); // y
        }
    }

    OPEN_DISPS(play->state.gfxCtx, "../z_bg_mizu_water.c", 738);
	func_8003435C(&this->actor.world.pos, play);
    Gfx_SetupDL_25Xlu(play->state.gfxCtx);

    gSPSegment(POLY_XLU_DISP++, 0x08,
               Gfx_TwoTexScroll(play->state.gfxCtx, G_TX_RENDERTILE, -gameplayFrames * 1, gameplayFrames * 1, 32, 32, 1,
                                0, -gameplayFrames * 1, 32, 32));

    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx, "../z_bg_mizu_water.c", 749);

	gSPDisplayList(POLY_XLU_DISP++, PlaneDL);

    CLOSE_DISPS(play->state.gfxCtx, "../z_bg_mizu_water.c", 756);
}
