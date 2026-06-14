#include "gfx.h"
#include "gfx_setupdl.h"
#include "sys_matrix.h"
#include "light.h"
#include "skybox.h"
#include "assets/textures/new_skybox_static/new_skybox_static.h"
#include "segmented_address.h"

Mtx* sSkyboxDrawMatrix;

Mtx* Skybox_UpdateMatrix(SkyboxContext* skyboxCtx, f32 x, f32 y, f32 z) {
    Matrix_Translate(x, y, z, MTXMODE_NEW);
    Matrix_Scale(1.0f, 1.0f, 1.0f, MTXMODE_APPLY);
    /* Matrix_RotateX(skyboxCtx->rot.x, MTXMODE_APPLY);
    Matrix_RotateY(skyboxCtx->rot.y, MTXMODE_APPLY);
    Matrix_RotateZ(skyboxCtx->rot.z, MTXMODE_APPLY); */
    return MATRIX_TO_MTX(sSkyboxDrawMatrix, "../z_vr_box_draw.c", 42);
}

void Skybox_Draw(SkyboxContext* skyboxCtx, GraphicsContext* gfxCtx, LightContext* lightCtx, s16 skyboxId, s16 blend,
                 f32 x, f32 y, f32 z) {
    OPEN_DISPS(gfxCtx, "../z_vr_box_draw.c", 52);

    Gfx_SetupDL_40Opa(gfxCtx);
    gDPSetRenderMode(POLY_OPA_DISP++, G_RM_FOG_PRIM_A, G_RM_OPA_SURF2);

    gSPSegment(POLY_OPA_DISP++, 0x7, skyboxCtx->staticSegments[0]);
    gSPSegment(POLY_OPA_DISP++, 0x8, skyboxCtx->staticSegments[1]);
    gSPSegment(POLY_OPA_DISP++, 0x9, skyboxCtx->palettes);

    gDPSetPrimColor(POLY_OPA_DISP++, 0x00, 0x00, 0, 0, 0, blend);
    gSPTexture(POLY_OPA_DISP++, 0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON);

    // Prepare matrix
    sSkyboxDrawMatrix = GRAPH_ALLOC(gfxCtx, sizeof(Mtx));
    Matrix_Translate(x, y, z, MTXMODE_NEW);
    Matrix_Scale(1.0f, 1.0f, 1.0f, MTXMODE_APPLY);
    Matrix_RotateX(skyboxCtx->rot.x, MTXMODE_APPLY);
    Matrix_RotateY(skyboxCtx->rot.y, MTXMODE_APPLY);
    Matrix_RotateZ(skyboxCtx->rot.z, MTXMODE_APPLY);
    MATRIX_TO_MTX(sSkyboxDrawMatrix, "../z_vr_box_draw.c", 76);
    gSPMatrix(POLY_OPA_DISP++, sSkyboxDrawMatrix, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

    // Enable magic square RGB dithering and bilinear filtering
    gDPSetColorDither(POLY_OPA_DISP++, G_CD_MAGICSQ);
    gDPSetTextureFilter(POLY_OPA_DISP++, G_TF_BILERP);

    // All skyboxes use CI8 textures with an RGBA16 palette
    gDPLoadTLUT_pal256(POLY_OPA_DISP++, skyboxCtx->palettes[0]);
    gDPSetTextureLUT(POLY_OPA_DISP++, G_TT_RGBA16);

    // Enable texture filtering RDP pipeline stages for bilinear filtering
    gDPSetTextureConvert(POLY_OPA_DISP++, G_TC_FILT);

    if (skyboxCtx->drawType != SKYBOX_DRAW_128) {
        // 256x256 textures, per-face palettes
        // 2, 3 or 4 faces

        gSPDisplayList(POLY_OPA_DISP++, skyboxCtx->dListBuf[0]); // -z face upper
        gSPDisplayList(POLY_OPA_DISP++, skyboxCtx->dListBuf[1]); // -z face lower

        gDPPipeSync(POLY_OPA_DISP++);
        gDPLoadTLUT_pal256(POLY_OPA_DISP++, skyboxCtx->palettes[1]);
        gSPDisplayList(POLY_OPA_DISP++, skyboxCtx->dListBuf[2]); // +x face upper
        gSPDisplayList(POLY_OPA_DISP++, skyboxCtx->dListBuf[3]); // +x face lower

        if (skyboxId != SKYBOX_BAZAAR) {
            if (skyboxId < SKYBOX_KOKIRI_SHOP || skyboxId > SKYBOX_BOMBCHU_SHOP) {
                // Skip remaining faces for most shop skyboxes

                gDPPipeSync(POLY_OPA_DISP++);
                gDPLoadTLUT_pal256(POLY_OPA_DISP++, skyboxCtx->palettes[2]);
                gSPDisplayList(POLY_OPA_DISP++, skyboxCtx->dListBuf[4]); // +z face upper
                gSPDisplayList(POLY_OPA_DISP++, skyboxCtx->dListBuf[5]); // +z face lower

                if (skyboxCtx->drawType != SKYBOX_DRAW_256_3FACE) {
                    gDPPipeSync(POLY_OPA_DISP++);
                    gDPLoadTLUT_pal256(POLY_OPA_DISP++, skyboxCtx->palettes[3]);
                    gSPDisplayList(POLY_OPA_DISP++, skyboxCtx->dListBuf[6]); // -x face upper
                    gSPDisplayList(POLY_OPA_DISP++, skyboxCtx->dListBuf[7]); // -x face lower
                }
            }
        }
    } else {
        // 128x128 and 128x64 textures
        // 5 or 6 faces

        // Draw each face
        gSPDisplayList(POLY_OPA_DISP++, skyboxCtx->dListBuf[0]); // -z face
        gSPDisplayList(POLY_OPA_DISP++, skyboxCtx->dListBuf[2]); // +z face
        gSPDisplayList(POLY_OPA_DISP++, skyboxCtx->dListBuf[4]); // -x face
        gSPDisplayList(POLY_OPA_DISP++, skyboxCtx->dListBuf[6]); // +x face
        gSPDisplayList(POLY_OPA_DISP++, skyboxCtx->dListBuf[8]); // +y face
        if (skyboxId != SKYBOX_CUTSCENE_MAP) {
            // Render the bottom face black except in the cutscene map
            gDPPipeSync(POLY_OPA_DISP++);
            gDPSetCombineLERP(POLY_OPA_DISP++, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, COMBINED, 0, 0, 0, COMBINED);
        }
        gSPDisplayList(POLY_OPA_DISP++, skyboxCtx->dListBuf[10]); // -y face
    }

    gDPPipeSync(POLY_OPA_DISP++);

    CLOSE_DISPS(gfxCtx, "../z_vr_box_draw.c", 125);
}

Vtx skybox_cylinder_vertices[37] = {
	{{ {0, 0, -100}, 0, {1008, 240}, {180, 214, 255, 255} }},
	{{ {70, 35, -70}, 0, {880, -16}, {57, 73, 192, 255} }},
	{{ {0, 35, -100}, 0, {1008, -16}, {57, 73, 192, 255} }},
	{{ {70, 0, -70}, 0, {880, 240}, {180, 214, 255, 255} }},
	{{ {70, -35, -70}, 0, {880, 496}, {57, 73, 192, 255} }},
	{{ {0, -35, -100}, 0, {1008, 496}, {57, 73, 192, 255} }},
	{{ {100, 0, 0}, 0, {752, 240}, {180, 214, 255, 255} }},
	{{ {100, -35, 0}, 0, {752, 496}, {57, 73, 192, 255} }},
	{{ {70, -35, 70}, 0, {624, 496}, {57, 73, 192, 255} }},
	{{ {70, 0, 70}, 0, {624, 240}, {180, 214, 255, 255} }},
	{{ {0, 0, 100}, 0, {496, 240}, {180, 214, 255, 255} }},
	{{ {0, -35, 100}, 0, {496, 496}, {57, 73, 192, 255} }},
	{{ {-70, -35, 70}, 0, {368, 496}, {57, 73, 192, 255} }},
	{{ {-70, 0, 70}, 0, {368, 240}, {180, 214, 255, 255} }},
	{{ {-100, 0, 0}, 0, {240, 240}, {180, 214, 255, 255} }},
	{{ {-100, -35, 0}, 0, {240, 496}, {57, 73, 192, 255} }},
	{{ {-70, -35, -70}, 0, {112, 496}, {57, 73, 192, 255} }},
	{{ {-70, 0, -70}, 0, {112, 240}, {180, 214, 255, 255} }},
	{{ {0, 0, -100}, 0, {-16, 240}, {180, 214, 255, 255} }},
	{{ {0, -35, -100}, 0, {-16, 496}, {57, 73, 192, 255} }},
	{{ {-70, 35, -70}, 0, {112, -16}, {57, 73, 192, 255} }},
	{{ {0, 35, -100}, 0, {-16, -16}, {57, 73, 192, 255} }},
	{{ {-100, 35, 0}, 0, {240, -16}, {57, 73, 192, 255} }},
	{{ {-70, 35, 70}, 0, {368, -16}, {57, 73, 192, 255} }},
	{{ {0, 35, 100}, 0, {496, -16}, {57, 73, 192, 255} }},
	{{ {70, 35, 70}, 0, {624, -16}, {57, 73, 192, 255} }},
	{{ {100, 35, 0}, 0, {752, -16}, {57, 73, 192, 255} }},
	{{ {0, 35, -100}, 0, {240, 506}, {57, 73, 192, 255} }},
	{{ {70, 35, -70}, 0, {414, 578}, {57, 73, 192, 255} }},
	{{ {100, 35, 0}, 0, {486, 752}, {57, 73, 192, 255} }},
	{{ {0, 35, 100}, 0, {240, 998}, {57, 73, 192, 255} }},
	{{ {70, 35, 70}, 0, {414, 926}, {57, 73, 192, 255} }},
	{{ {0, 35, 100}, 0, {240, 998}, {57, 73, 192, 255} }},
	{{ {-100, 35, 0}, 0, {-6, 752}, {57, 73, 192, 255} }},
	{{ {0, 35, -100}, 0, {240, 506}, {57, 73, 192, 255} }},
	{{ {-70, 35, 70}, 0, {66, 926}, {57, 73, 192, 255} }},
	{{ {-70, 35, -70}, 0, {66, 578}, {57, 73, 192, 255} }},
};

void Skybox_DrawNew(SkyboxContext* skyboxCtx, GraphicsContext* gfxCtx, LightContext* lightCtx, s16 skyboxId, s16 blend, f32 x, f32 y, f32 z) {
    Vtx* vtx;
    u8 i;

    vtx = Graph_Alloc(gfxCtx, sizeof(Vtx) * 37);

    if (vtx != NULL) {
        for (i = 0; i < 37; i++) {
            vtx[i].v.ob[0] = skybox_cylinder_vertices[i].v.ob[0];
            vtx[i].v.ob[1] = skybox_cylinder_vertices[i].v.ob[1];
            vtx[i].v.ob[2] = skybox_cylinder_vertices[i].v.ob[2];

            vtx[i].v.flag = 0;
            vtx[i].v.tc[0] = skybox_cylinder_vertices[i].v.tc[0];
            vtx[i].v.tc[1] = skybox_cylinder_vertices[i].v.tc[1];

            vtx[i].v.cn[3] = 255; // vertex color alpha

            switch (i) {
                case 0:
                case 3:
                case 6:
                case 9:
                case 10:
                case 13:
                case 14:
                case 17:
                case 18: // cloud color bottom
                    vtx[i].v.cn[0] = skyboxCtx->skyboxBottomColor[0];
                    vtx[i].v.cn[1] = skyboxCtx->skyboxBottomColor[1];
                    vtx[i].v.cn[2] = skyboxCtx->skyboxBottomColor[2];
                    break;
                default: // cloud color top
                    vtx[i].v.cn[0] = skyboxCtx->skyboxTopColor[0];
                    vtx[i].v.cn[1] = skyboxCtx->skyboxTopColor[1];
                    vtx[i].v.cn[2] = skyboxCtx->skyboxTopColor[2];
                    break;
            }
        }
    }
    
    OPEN_DISPS(gfxCtx, "../z_cheap_proc.c", 214);

    gSPSegment(POLY_OPA_DISP++, 0x7, skyboxCtx->skyboxStaticSegment); // setting the correct segment for opa

    sSkyboxDrawMatrix = Graph_Alloc(gfxCtx, sizeof(Mtx));
    Matrix_Translate(x, y, z, MTXMODE_NEW);
    Matrix_Scale(1.0f, 1.0f, 1.0f, MTXMODE_APPLY);
    MATRIX_TO_MTX(sSkyboxDrawMatrix, "../z_vr_box_draw.c", 76);
    gSPMatrix(POLY_OPA_DISP++, sSkyboxDrawMatrix, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

    gSPDisplayList(POLY_OPA_DISP++, mat_skybox_cylinder_b_f3dlite_material_008_layerOpaque);

    gSPVertex(POLY_OPA_DISP++, vtx + 0, 32, 0);
    gSP2Triangles(POLY_OPA_DISP++, 0, 1, 2, 0, 0, 3, 1, 0);
    gSP2Triangles(POLY_OPA_DISP++, 4, 3, 0, 0, 4, 0, 5, 0);
    gSP2Triangles(POLY_OPA_DISP++, 4, 6, 3, 0, 4, 7, 6, 0);
    gSP2Triangles(POLY_OPA_DISP++, 8, 6, 7, 0, 8, 9, 6, 0);
    gSP2Triangles(POLY_OPA_DISP++, 8, 10, 9, 0, 8, 11, 10, 0);
    gSP2Triangles(POLY_OPA_DISP++, 12, 10, 11, 0, 12, 13, 10, 0);
    gSP2Triangles(POLY_OPA_DISP++, 12, 14, 13, 0, 12, 15, 14, 0);
    gSP2Triangles(POLY_OPA_DISP++, 16, 14, 15, 0, 16, 17, 14, 0);
    gSP2Triangles(POLY_OPA_DISP++, 16, 18, 17, 0, 16, 19, 18, 0);
    gSP2Triangles(POLY_OPA_DISP++, 18, 20, 17, 0, 18, 21, 20, 0);
    gSP2Triangles(POLY_OPA_DISP++, 14, 17, 20, 0, 14, 20, 22, 0);
    gSP2Triangles(POLY_OPA_DISP++, 14, 22, 23, 0, 14, 23, 13, 0);
    gSP2Triangles(POLY_OPA_DISP++, 10, 13, 23, 0, 10, 23, 24, 0);
    gSP2Triangles(POLY_OPA_DISP++, 10, 24, 25, 0, 10, 25, 9, 0);
    gSP2Triangles(POLY_OPA_DISP++, 6, 9, 25, 0, 6, 25, 26, 0);
    gSP2Triangles(POLY_OPA_DISP++, 6, 26, 1, 0, 6, 1, 3, 0);
    gSP2Triangles(POLY_OPA_DISP++, 27, 28, 29, 0, 27, 29, 30, 0);
    gSP1Triangle(POLY_OPA_DISP++, 29, 31, 30, 0);

    gSPVertex(POLY_OPA_DISP++, vtx + 32, 5, 0);
    gSP2Triangles(POLY_OPA_DISP++, 0, 1, 2, 0, 0, 3, 1, 0);
    gSP1Triangle(POLY_OPA_DISP++, 1, 4, 2, 0);

    // add the cloud draw code here

    CLOSE_DISPS(gfxCtx, "../z_cheap_proc.c", 219);
}

void Skybox_Update(SkyboxContext* skyboxCtx) {
}
