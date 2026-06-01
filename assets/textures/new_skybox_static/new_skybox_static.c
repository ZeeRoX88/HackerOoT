#include "ultra64.h"
#include "gfx.h"
#include "new_skybox_static.h"

u64 skybox_cloud_01_tex[] = {
#include "assets/textures/new_skybox_static/cloud_new_01.ia8.inc.c"
};

u64 skybox_cloud_02_tex[] = {
#include "assets/textures/new_skybox_static/cloud_new_02.ia8.inc.c"
};

u64 skybox_cloud_03_tex[] = {
#include "assets/textures/new_skybox_static/cloud_new_03.ia8.inc.c"
};

u64 skybox_cloud_storm_tex[] = {
#include "assets/textures/new_skybox_static/cloud_storm.ia8.inc.c"
};

u64 skybox_cloud_horizon_tex[] = {
#include "assets/textures/new_skybox_static/cloud_horizon.ia8.inc.c"
};

Vtx skybox_cloud_horizon_skybox_cloud_horizon_mesh_layer_Transparent_vtx_cull[8] = {
	{{ {-5000, -250, -5000}, 0, {0, 0}, {0, 0, 0, 0} }},
	{{ {-5000, -250, 5000}, 0, {0, 0}, {0, 0, 0, 0} }},
	{{ {-5000, 250, 5000}, 0, {0, 0}, {0, 0, 0, 0} }},
	{{ {-5000, 250, -5000}, 0, {0, 0}, {0, 0, 0, 0} }},
	{{ {5000, -250, -5000}, 0, {0, 0}, {0, 0, 0, 0} }},
	{{ {5000, -250, 5000}, 0, {0, 0}, {0, 0, 0, 0} }},
	{{ {5000, 250, 5000}, 0, {0, 0}, {0, 0, 0, 0} }},
	{{ {5000, 250, -5000}, 0, {0, 0}, {0, 0, 0, 0} }},
};

Vtx skybox_cloud_horizon_skybox_cloud_horizon_mesh_layer_Transparent_vtx_0[25] = {
	{{ {0, -250, -5000}, 0, {2032, 496}, {255, 255, 255, 255} }},
	{{ {3536, -250, -3536}, 0, {4080, 495}, {255, 255, 255, 255} }},
	{{ {3536, 250, -3536}, 0, {4069, -16}, {255, 255, 255, 255} }},
	{{ {0, 250, -5000}, 0, {2046, -16}, {255, 255, 255, 255} }},
	{{ {3536, -250, -3536}, 0, {-16, 496}, {255, 255, 255, 255} }},
	{{ {5000, 250, 0}, 0, {2032, -16}, {255, 255, 255, 255} }},
	{{ {3536, 250, -3536}, 0, {-16, -16}, {255, 255, 255, 255} }},
	{{ {5000, -250, 0}, 0, {2032, 496}, {255, 255, 255, 255} }},
	{{ {3536, 250, 3536}, 0, {4080, -16}, {255, 255, 255, 255} }},
	{{ {3536, -250, 3536}, 0, {4080, 496}, {255, 255, 255, 255} }},
	{{ {3536, -250, 3536}, 0, {-16, 496}, {255, 255, 255, 255} }},
	{{ {0, 250, 5000}, 0, {2032, -16}, {255, 255, 255, 255} }},
	{{ {3536, 250, 3536}, 0, {-16, -16}, {255, 255, 255, 255} }},
	{{ {0, -250, 5000}, 0, {2032, 496}, {255, 255, 255, 255} }},
	{{ {-3536, 250, 3536}, 0, {4080, -16}, {255, 255, 255, 255} }},
	{{ {-3536, -250, 3536}, 0, {4080, 496}, {255, 255, 255, 255} }},
	{{ {-3536, -250, 3536}, 0, {-16, 496}, {255, 255, 255, 255} }},
	{{ {-5000, 250, 0}, 0, {2032, -16}, {255, 255, 255, 255} }},
	{{ {-3536, 250, 3536}, 0, {-16, -16}, {255, 255, 255, 255} }},
	{{ {-5000, -250, 0}, 0, {2032, 496}, {255, 255, 255, 255} }},
	{{ {-3536, 250, -3536}, 0, {4080, -16}, {255, 255, 255, 255} }},
	{{ {-3536, -250, -3536}, 0, {4080, 496}, {255, 255, 255, 255} }},
	{{ {-3536, -250, -3536}, 0, {-16, 496}, {255, 255, 255, 255} }},
	{{ {0, 250, -5000}, 0, {2032, -16}, {255, 255, 255, 255} }},
	{{ {-3536, 250, -3536}, 0, {-16, -16}, {255, 255, 255, 255} }},
};

Gfx skybox_cloud_horizon_skybox_cloud_horizon_mesh_layer_Transparent_tri_0[] = {
	gsSPVertex(skybox_cloud_horizon_skybox_cloud_horizon_mesh_layer_Transparent_vtx_0 + 0, 25, 0),
	gsSP2Triangles(0, 1, 2, 0, 0, 2, 3, 0),
	gsSP2Triangles(4, 5, 6, 0, 4, 7, 5, 0),
	gsSP2Triangles(7, 8, 5, 0, 7, 9, 8, 0),
	gsSP2Triangles(10, 11, 12, 0, 10, 13, 11, 0),
	gsSP2Triangles(13, 14, 11, 0, 13, 15, 14, 0),
	gsSP2Triangles(16, 17, 18, 0, 16, 19, 17, 0),
	gsSP2Triangles(19, 20, 17, 0, 19, 21, 20, 0),
	gsSP2Triangles(22, 0, 23, 0, 22, 23, 24, 0),
	gsSPEndDisplayList(),
};

Gfx mat_skybox_cloud_horizon_f3dlite_material_001_layerTransparent[] = {
	gsSPLoadGeometryMode(G_ZBUFFER | G_CULL_BACK),
	gsDPPipeSync(),
	gsDPSetCombineLERP(PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0, COMBINED, 0, 0, 0, COMBINED),
	gsSPSetOtherMode(G_SETOTHERMODE_H, 4, 20, G_AD_NOTPATTERN | G_CD_DISABLE | G_CK_NONE | G_TC_FILT | G_TF_BILERP | G_TT_NONE | G_TL_TILE | G_TD_CLAMP | G_TP_PERSP | G_CYC_2CYCLE | G_PM_NPRIMITIVE),
	gsSPSetOtherMode(G_SETOTHERMODE_L, 0, 32, G_AC_THRESHOLD | G_ZS_PRIM | G_RM_FOG_PRIM_A | G_RM_ZB_XLU_SURF2),
	gsDPSetPrimDepth(32767, 0),
	gsSPTexture(65535, 65535, 0, 0, 1),
	gsDPSetTextureImage(G_IM_FMT_IA, G_IM_SIZ_8b_LOAD_BLOCK, 1, skybox_cloud_horizon_tex),
	gsDPSetTile(G_IM_FMT_IA, G_IM_SIZ_8b_LOAD_BLOCK, 0, 0, 7, 0, G_TX_WRAP | G_TX_NOMIRROR, 0, 0, G_TX_WRAP | G_TX_NOMIRROR, 0, 0),
	gsDPLoadBlock(7, 0, 0, 1023, 128),
	gsDPSetTile(G_IM_FMT_IA, G_IM_SIZ_8b, 16, 0, 0, 0, G_TX_CLAMP | G_TX_NOMIRROR, 4, 0, G_TX_WRAP | G_TX_NOMIRROR, 7, 0),
	gsDPSetTileSize(0, 0, 0, 508, 60),
	gsSPEndDisplayList(),
};

Gfx skybox_cloud_horizon[] = {
	gsSPClearGeometryMode(G_LIGHTING),
	gsSPVertex(skybox_cloud_horizon_skybox_cloud_horizon_mesh_layer_Transparent_vtx_cull + 0, 8, 0),
	gsSPSetGeometryMode(G_LIGHTING),
	gsSPCullDisplayList(0, 7),
	gsSPDisplayList(mat_skybox_cloud_horizon_f3dlite_material_001_layerTransparent),
	gsSPDisplayList(skybox_cloud_horizon_skybox_cloud_horizon_mesh_layer_Transparent_tri_0),
	gsSPEndDisplayList(),
};

Vtx skybox_cloud_skybox_cloud_mesh_layer_Transparent_vtx_cull[8] = {
	{{ {0, -500, -1000}, 0, {0, 0}, {0, 0, 0, 0} }},
	{{ {0, -500, 1000}, 0, {0, 0}, {0, 0, 0, 0} }},
	{{ {0, 500, 1000}, 0, {0, 0}, {0, 0, 0, 0} }},
	{{ {0, 500, -1000}, 0, {0, 0}, {0, 0, 0, 0} }},
	{{ {0, -500, -1000}, 0, {0, 0}, {0, 0, 0, 0} }},
	{{ {0, -500, 1000}, 0, {0, 0}, {0, 0, 0, 0} }},
	{{ {0, 500, 1000}, 0, {0, 0}, {0, 0, 0, 0} }},
	{{ {0, 500, -1000}, 0, {0, 0}, {0, 0, 0, 0} }},
};

Vtx skybox_cloud_skybox_cloud_mesh_layer_Transparent_vtx_0[4] = {
	{{ {0, 500, -1000}, 0, {2032, -16}, {255, 255, 255, 255} }},
	{{ {0, -500, -1000}, 0, {2032, 1008}, {255, 255, 255, 255} }},
	{{ {0, -500, 1000}, 0, {-16, 1008}, {255, 255, 255, 255} }},
	{{ {0, 500, 1000}, 0, {-16, -16}, {255, 255, 255, 255} }},
};

Gfx skybox_cloud_skybox_cloud_mesh_layer_Transparent_tri_0[] = {
	gsSPVertex(skybox_cloud_skybox_cloud_mesh_layer_Transparent_vtx_0 + 0, 4, 0),
	gsSP2Triangles(0, 1, 2, 0, 0, 2, 3, 0),
	gsSPEndDisplayList(),
};

Gfx mat_skybox_cloud_f3dlite_material_006_layerTransparent[] = {
	gsSPLoadGeometryMode(G_ZBUFFER | G_CULL_BACK),
	gsDPPipeSync(),
	gsDPSetCombineLERP(PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0, COMBINED, 0, 0, 0, COMBINED),
	gsSPSetOtherMode(G_SETOTHERMODE_H, 4, 20, G_AD_NOTPATTERN | G_CD_MAGICSQ | G_CK_NONE | G_TC_FILT | G_TF_BILERP | G_TT_NONE | G_TL_TILE | G_TD_CLAMP | G_TP_PERSP | G_CYC_2CYCLE | G_PM_NPRIMITIVE),
	gsSPSetOtherMode(G_SETOTHERMODE_L, 0, 32, G_AC_THRESHOLD | G_ZS_PRIM | G_RM_FOG_PRIM_A | G_RM_ZB_XLU_SURF2),
	gsDPSetPrimDepth(32767, 0),
	gsSPTexture(65535, 65535, 0, 0, 1),
	gsDPSetTextureImage(G_IM_FMT_IA, G_IM_SIZ_8b_LOAD_BLOCK, 1, 0x08000000),
	gsDPSetTile(G_IM_FMT_IA, G_IM_SIZ_8b_LOAD_BLOCK, 0, 0, 7, 0, G_TX_WRAP | G_TX_NOMIRROR, 0, 0, G_TX_WRAP | G_TX_NOMIRROR, 0, 0),
	gsDPLoadBlock(7, 0, 0, 1023, 256),
	gsDPSetTile(G_IM_FMT_IA, G_IM_SIZ_8b, 8, 0, 0, 0, G_TX_CLAMP | G_TX_NOMIRROR, 5, 0, G_TX_CLAMP | G_TX_NOMIRROR, 6, 0),
	gsDPSetTileSize(0, 0, 0, 252, 124),
	gsSPEndDisplayList(),
};

Gfx skybox_cloud[] = {
	gsSPClearGeometryMode(G_LIGHTING),
	gsSPVertex(skybox_cloud_skybox_cloud_mesh_layer_Transparent_vtx_cull + 0, 8, 0),
	gsSPSetGeometryMode(G_LIGHTING),
	gsSPCullDisplayList(0, 7),
	gsSPDisplayList(mat_skybox_cloud_f3dlite_material_006_layerTransparent),
	gsSPDisplayList(skybox_cloud_skybox_cloud_mesh_layer_Transparent_tri_0),
	gsSPEndDisplayList(),
};

/* Vtx skybox_cylinder_b_skybox_cylinder_b_mesh_layer_Opaque_vtx_cull[8] = {
	{{ {-100, -35, -100}, 0, {0, 0}, {0, 0, 0, 0} }},
	{{ {-100, -35, 100}, 0, {0, 0}, {0, 0, 0, 0} }},
	{{ {-100, 35, 100}, 0, {0, 0}, {0, 0, 0, 0} }},
	{{ {-100, 35, -100}, 0, {0, 0}, {0, 0, 0, 0} }},
	{{ {100, -35, -100}, 0, {0, 0}, {0, 0, 0, 0} }},
	{{ {100, -35, 100}, 0, {0, 0}, {0, 0, 0, 0} }},
	{{ {100, 35, 100}, 0, {0, 0}, {0, 0, 0, 0} }},
	{{ {100, 35, -100}, 0, {0, 0}, {0, 0, 0, 0} }},
};

Gfx skybox_cylinder_b_skybox_cylinder_b_mesh_layer_Opaque_tri_0[] = {
	gsSPVertex(skybox_cylinder_b_skybox_cylinder_b_mesh_layer_Opaque_vtx_0 + 0, 32, 0),
	gsSP2Triangles(0, 1, 2, 0, 0, 3, 1, 0),
	gsSP2Triangles(4, 3, 0, 0, 4, 5, 3, 0),
	gsSP2Triangles(5, 6, 3, 0, 5, 7, 6, 0),
	gsSP2Triangles(7, 8, 6, 0, 7, 9, 8, 0),
	gsSP2Triangles(9, 10, 8, 0, 9, 11, 10, 0),
	gsSP2Triangles(11, 12, 10, 0, 11, 13, 12, 0),
	gsSP2Triangles(13, 14, 12, 0, 13, 15, 14, 0),
	gsSP2Triangles(15, 16, 14, 0, 15, 17, 16, 0),
	gsSP2Triangles(17, 18, 16, 0, 17, 19, 18, 0),
	gsSP2Triangles(16, 18, 20, 0, 16, 20, 21, 0),
	gsSP2Triangles(14, 16, 21, 0, 14, 21, 22, 0),
	gsSP2Triangles(12, 14, 22, 0, 12, 22, 23, 0),
	gsSP2Triangles(10, 12, 23, 0, 10, 23, 24, 0),
	gsSP2Triangles(8, 10, 24, 0, 8, 24, 25, 0),
	gsSP2Triangles(6, 8, 25, 0, 6, 25, 26, 0),
	gsSP2Triangles(3, 6, 26, 0, 3, 26, 1, 0),
	gsSP2Triangles(27, 28, 29, 0, 27, 29, 30, 0),
	gsSP1Triangle(29, 31, 30, 0),
	gsSPVertex(skybox_cylinder_b_skybox_cylinder_b_mesh_layer_Opaque_vtx_0 + 32, 5, 0),
	gsSP2Triangles(0, 1, 2, 0, 0, 3, 1, 0),
	gsSP1Triangle(1, 4, 2, 0),
	gsSPEndDisplayList(),
}; */

Gfx mat_skybox_cylinder_b_f3dlite_material_008_layerOpaque[] = {
	gsSPLoadGeometryMode(G_SHADE | G_CULL_BACK | G_FOG | G_SHADING_SMOOTH),
	gsDPPipeSync(),
	gsDPSetCombineLERP(PRIMITIVE, 0, SHADE, 0, 0, 0, 0, 1, COMBINED, 0, PRIMITIVE, 0, 0, 0, 0, COMBINED),
	gsSPSetOtherMode(G_SETOTHERMODE_H, 4, 20, G_AD_NOTPATTERN | G_CD_MAGICSQ | G_CK_NONE | G_TC_FILT | G_TF_BILERP | G_TT_NONE | G_TL_TILE | G_TD_CLAMP | G_TP_PERSP | G_CYC_2CYCLE | G_PM_NPRIMITIVE),
	gsSPSetOtherMode(G_SETOTHERMODE_L, 0, 32, G_AC_NONE | G_ZS_PIXEL | G_RM_FOG_PRIM_A | G_RM_OPA_SURF2),
	gsSPTexture(65535, 65535, 0, 0, 1),
	gsDPSetPrimColor(0, 0, 255, 255, 255, 255),
	gsSPEndDisplayList(),
};

/* Gfx skybox_cylinder_b[] = {
	gsSPClearGeometryMode(G_LIGHTING),
	gsSPVertex(skybox_cylinder_b_skybox_cylinder_b_mesh_layer_Opaque_vtx_cull + 0, 8, 0),
	gsSPSetGeometryMode(G_LIGHTING),
	gsSPCullDisplayList(0, 7),
	gsSPDisplayList(mat_skybox_cylinder_b_f3dlite_material_008_layerOpaque),
	gsSPDisplayList(skybox_cylinder_b_skybox_cylinder_b_mesh_layer_Opaque_tri_0),
	gsSPEndDisplayList(),
}; */

Vtx skybox_storm_cloud_skybox_storm_cloud_mesh_layer_Transparent_vtx_cull[8] = {
	{{ {-1000, 0, -1000}, 0, {0, 0}, {0, 0, 0, 0} }},
	{{ {-1000, 0, 1000}, 0, {0, 0}, {0, 0, 0, 0} }},
	{{ {-1000, 0, 1000}, 0, {0, 0}, {0, 0, 0, 0} }},
	{{ {-1000, 0, -1000}, 0, {0, 0}, {0, 0, 0, 0} }},
	{{ {1000, 0, -1000}, 0, {0, 0}, {0, 0, 0, 0} }},
	{{ {1000, 0, 1000}, 0, {0, 0}, {0, 0, 0, 0} }},
	{{ {1000, 0, 1000}, 0, {0, 0}, {0, 0, 0, 0} }},
	{{ {1000, 0, -1000}, 0, {0, 0}, {0, 0, 0, 0} }},
};

Vtx skybox_storm_cloud_skybox_storm_cloud_mesh_layer_Transparent_vtx_0[4] = {
	{{ {-1000, 0, -1000}, 0, {2032, -16}, {255, 255, 255, 255} }},
	{{ {1000, 0, -1000}, 0, {2032, 2032}, {255, 255, 255, 255} }},
	{{ {1000, 0, 1000}, 0, {-16, 2032}, {255, 255, 255, 255} }},
	{{ {-1000, 0, 1000}, 0, {-16, -16}, {255, 255, 255, 255} }},
};

Gfx skybox_storm_cloud_skybox_storm_cloud_mesh_layer_Transparent_tri_0[] = {
	gsSPVertex(skybox_storm_cloud_skybox_storm_cloud_mesh_layer_Transparent_vtx_0 + 0, 4, 0),
	gsSP2Triangles(0, 1, 2, 0, 0, 2, 3, 0),
	gsSPEndDisplayList(),
};

Gfx mat_skybox_storm_cloud_f3dlite_material_009_layerTransparent[] = {
	gsSPLoadGeometryMode(G_ZBUFFER | G_CULL_BACK),
	gsDPPipeSync(),
	gsDPSetCombineLERP(PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0, COMBINED, 0, 0, 0, COMBINED),
	gsSPSetOtherMode(G_SETOTHERMODE_H, 4, 20, G_AD_NOTPATTERN | G_CD_DISABLE | G_CK_NONE | G_TC_FILT | G_TF_BILERP | G_TT_NONE | G_TL_TILE | G_TD_CLAMP | G_TP_PERSP | G_CYC_2CYCLE | G_PM_NPRIMITIVE),
	gsSPSetOtherMode(G_SETOTHERMODE_L, 0, 32, G_AC_THRESHOLD | G_ZS_PRIM | G_RM_FOG_PRIM_A | G_RM_ZB_XLU_SURF2),
	gsDPSetPrimDepth(32767, 0),
	gsSPTexture(65535, 65535, 0, 0, 1),
	gsDPSetTextureImage(G_IM_FMT_IA, G_IM_SIZ_8b_LOAD_BLOCK, 1, skybox_cloud_storm_tex),
	gsDPSetTile(G_IM_FMT_IA, G_IM_SIZ_8b_LOAD_BLOCK, 0, 0, 7, 0, G_TX_WRAP | G_TX_NOMIRROR, 0, 0, G_TX_WRAP | G_TX_NOMIRROR, 0, 0),
	gsDPLoadBlock(7, 0, 0, 2047, 256),
	gsDPSetTile(G_IM_FMT_IA, G_IM_SIZ_8b, 8, 0, 0, 0, G_TX_WRAP | G_TX_NOMIRROR, 6, 0, G_TX_WRAP | G_TX_NOMIRROR, 6, 0),
	gsDPSetTileSize(0, 0, 0, 252, 252),
	gsSPEndDisplayList(),
};

Gfx skybox_storm_cloud[] = {
	gsSPClearGeometryMode(G_LIGHTING),
	gsSPVertex(skybox_storm_cloud_skybox_storm_cloud_mesh_layer_Transparent_vtx_cull + 0, 8, 0),
	gsSPSetGeometryMode(G_LIGHTING),
	gsSPCullDisplayList(0, 7),
	gsSPDisplayList(mat_skybox_storm_cloud_f3dlite_material_009_layerTransparent),
	gsSPDisplayList(skybox_storm_cloud_skybox_storm_cloud_mesh_layer_Transparent_tri_0),
	gsSPEndDisplayList(),
};

