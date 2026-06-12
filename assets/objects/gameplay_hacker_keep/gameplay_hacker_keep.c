#include "config.h"

#if MM_BOTTLE_MODEL
#include "assets/objects/gameplay_hacker_keep/bottle.c"
#endif

/* u64 gMoonPhase00Tex[] = {
#include "assets/objects/gameplay_hacker_keep/moon_00.ia8.inc.c"
}; */

u64 gMoonPhase01Tex[] = {
#include "assets/objects/gameplay_hacker_keep/moon_01.ia8.inc.c"
};

u64 gMoonPhase02Tex[] = {
#include "assets/objects/gameplay_hacker_keep/moon_02.ia8.inc.c"
};

u64 gMoonPhase03Tex[] = {
#include "assets/objects/gameplay_hacker_keep/moon_03.ia8.inc.c"
};

u64 gEffKusaTex[] = {
#include "assets/objects/gameplay_hacker_keep/eff_kusa.ia8.inc.c"
};

Vtx gOEffDustDL_gOEffDustDL_mesh_layer_Transparent_vtx_cull[8] = {
	{{ {-20, -20, 0}, 0, {-16, -16}, {0, 0, 0, 0} }},
	{{ {-20, -20, 0}, 0, {-16, -16}, {0, 0, 0, 0} }},
	{{ {-20, 20, 0}, 0, {-16, -16}, {0, 0, 0, 0} }},
	{{ {-20, 20, 0}, 0, {-16, -16}, {0, 0, 0, 0} }},
	{{ {20, -20, 0}, 0, {-16, -16}, {0, 0, 0, 0} }},
	{{ {20, -20, 0}, 0, {-16, -16}, {0, 0, 0, 0} }},
	{{ {20, 20, 0}, 0, {-16, -16}, {0, 0, 0, 0} }},
	{{ {20, 20, 0}, 0, {-16, -16}, {0, 0, 0, 0} }},
};

Vtx gOEffDustDL_gOEffDustDL_mesh_layer_Transparent_vtx_0[4] = {
	{{ {-20, -20, 0}, 0, {-16, 1008}, {0, 0, 127, 255} }},
	{{ {20, -20, 0}, 0, {1008, 1008}, {0, 0, 127, 255} }},
	{{ {20, 20, 0}, 0, {1008, -16}, {0, 0, 127, 255} }},
	{{ {-20, 20, 0}, 0, {-16, -16}, {0, 0, 127, 255} }},
};

Gfx gOEffDustDL_gOEffDustDL_mesh_layer_Transparent_tri_0[] = {
	gsSPVertex(gOEffDustDL_gOEffDustDL_mesh_layer_Transparent_vtx_0 + 0, 4, 0),
	gsSP2Triangles(0, 1, 2, 0, 0, 2, 3, 0),
	gsSPEndDisplayList(),
};

Gfx mat_gOEffDustDL_f3dlite_material_layerTransparent[] = {
	gsDPPipeSync(),
	// gsDPSetCombineLERP(TEXEL0, 0, SHADE, 0, 0, 0, 0, TEXEL0, COMBINED, 0, PRIMITIVE, 0, COMBINED, 0, PRIMITIVE, 0),
	gsSPLoadGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_FOG | G_LIGHTING | G_SHADING_SMOOTH),
	gsSPSetOtherMode(G_SETOTHERMODE_H, 4, 20, G_AD_NOISE | G_CD_MAGICSQ | G_CK_NONE | G_TC_FILT | G_TF_BILERP | G_TL_TILE | G_TD_CLAMP | G_TP_PERSP | G_CYC_2CYCLE | G_PM_NPRIMITIVE),
	gsSPSetOtherMode(G_SETOTHERMODE_L, 0, 32, G_AC_THRESHOLD | G_ZS_PIXEL | G_RM_FOG_SHADE_A | G_RM_AA_ZB_XLU_SURF2),
	gsSPTexture(65535, 65535, 0, 0, 1),
	gsDPSetTextureLUT(G_TT_NONE),
	gsDPSetTextureImage(G_IM_FMT_IA, G_IM_SIZ_8b_LOAD_BLOCK, 1, gEffKusaTex),
	gsDPSetTile(G_IM_FMT_IA, G_IM_SIZ_8b_LOAD_BLOCK, 0, 0, 7, 0, G_TX_CLAMP | G_TX_NOMIRROR, 5, 0, G_TX_CLAMP | G_TX_NOMIRROR, 5, 0),
	gsDPLoadBlock(7, 0, 0, 511, 512),
	gsDPSetTile(G_IM_FMT_IA, G_IM_SIZ_8b, 4, 0, 0, 0, G_TX_CLAMP | G_TX_NOMIRROR, 5, 0, G_TX_CLAMP | G_TX_NOMIRROR, 5, 0),
	gsDPSetTileSize(0, 0, 0, 124, 124),
	gsSPEndDisplayList(),
};

Gfx gOEffDustDL[] = {
	gsSPClearGeometryMode(G_LIGHTING),
	gsSPVertex(gOEffDustDL_gOEffDustDL_mesh_layer_Transparent_vtx_cull + 0, 8, 0),
	gsSPCullDisplayList(0, 7),
	gsSPDisplayList(mat_gOEffDustDL_f3dlite_material_layerTransparent),
	gsSPDisplayList(gOEffDustDL_gOEffDustDL_mesh_layer_Transparent_tri_0),
	gsSPEndDisplayList(),
};
