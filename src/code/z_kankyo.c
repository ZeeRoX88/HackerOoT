#pragma increment_block_number "gc-eu:192 gc-eu-mq:192 gc-jp:128 gc-jp-ce:128 gc-jp-mq:128 gc-us:128 gc-us-mq:128" \
                               "ique-cn:128 ntsc-1.0:192 ntsc-1.1:192 ntsc-1.2:192 pal-1.0:192 pal-1.1:192"

#include "libc64/qrand.h"
#include "libu64/gfxprint.h"
#include "array_count.h"
#include "buffers.h"
#include "gfx.h"
#include "gfx_setupdl.h"
#include "gfxalloc.h"
#include "ultra64.h"
#include "printf.h"
#include "regs.h"
#include "rumble.h"
#include "segment_symbols.h"
#include "segmented_address.h"
#include "seqcmd.h"
#include "sequence.h"
#include "sfx.h"
#include "sys_math.h"
#include "sys_math3d.h"
#include "sys_matrix.h"
#include "terminal.h"
#include "translation.h"
#include "versions.h"
#include "z_lib.h"
#include "audio.h"
#include "cutscene.h"
#include "frame_advance.h"
#include "environment.h"
#include "play_state.h"
#include "player.h"
#include "save.h"
#include "debug.h"
#include "skin_matrix.h"
#include "rand.h"
#include "z_debug.h"

#include "assets/objects/gameplay_keep/gameplay_keep.h"
#include "assets/objects/gameplay_hacker_keep/gameplay_hacker_keep.h"
#include "assets/objects/gameplay_field_keep/gameplay_field_keep.h"

#include "assets/textures/new_skybox_static/new_skybox_static.h"

typedef enum LightningBoltState {
    /* 0x00 */ LIGHTNING_BOLT_START,
    /* 0x01 */ LIGHTNING_BOLT_WAIT,
    /* 0x02 */ LIGHTNING_BOLT_DRAW,
    /* 0xFF */ LIGHTNING_BOLT_INACTIVE = 0xFF
} LightningBoltState;

typedef struct ZBufValConversionEntry {
    /* 0x00 */ s32 mantissaShift; // shift applied to the mantissa of the z buffer value
    /* 0x04 */ s32 base;          // 15.3 fixed-point base value for the exponent
} ZBufValConversionEntry;         // size = 0x8

// This table needs as many values as there are values for the 3-bit exponent
ZBufValConversionEntry sZBufValConversionTable[1 << 3] = {
    { 6, 0x0000 << 3 }, { 5, 0x4000 << 3 }, { 4, 0x6000 << 3 }, { 3, 0x7000 << 3 },
    { 2, 0x7800 << 3 }, { 1, 0x7C00 << 3 }, { 0, 0x7E00 << 3 }, { 0, 0x7F00 << 3 },
};

u8 gWeatherMode = WEATHER_MODE_CLEAR; // "E_wether_flg"

u8 gLightConfigAfterUnderwater = 0;

u8 gInterruptSongOfStorms = false;

// Indicates whether the skybox is changing to a different index of the same config (based on time)
u8 gSkyboxIsChanging = false;

// how many units of time that pass every update
u16 gTimeSpeed = 0;

u16 sSunScreenDepth = GPACK_ZDZ(G_MAXFBZ, 0);

typedef struct TimeBasedLightEntry {
    /* 0x00 */ u16 startTime;
    /* 0x02 */ u16 endTime;
    /* 0x04 */ u8 lightSetting;
    /* 0x05 */ u8 nextLightSetting;
} TimeBasedLightEntry; // size = 0x6

TimeBasedLightEntry sTimeBasedLightConfigs[][7] = {
    {
        { CLOCK_TIME(0, 0), CLOCK_TIME(4, 0) + 1, 3, 3 },
        { CLOCK_TIME(4, 0) + 1, CLOCK_TIME(6, 0), 3, 0 },
        { CLOCK_TIME(6, 0), CLOCK_TIME(8, 0) + 1, 0, 1 },
        { CLOCK_TIME(8, 0) + 1, CLOCK_TIME(16, 0), 1, 1 },
        { CLOCK_TIME(16, 0), CLOCK_TIME(17, 0) + 1, 1, 2 },
        { CLOCK_TIME(17, 0) + 1, CLOCK_TIME(19, 0) + 1, 2, 3 },
        { CLOCK_TIME(19, 0) + 1, CLOCK_TIME(24, 0) - 1, 3, 3 },
    },
    {
        { CLOCK_TIME(0, 0), CLOCK_TIME(4, 0) + 1, 7, 7 },
        { CLOCK_TIME(4, 0) + 1, CLOCK_TIME(6, 0), 7, 4 },
        { CLOCK_TIME(6, 0), CLOCK_TIME(8, 0) + 1, 4, 5 },
        { CLOCK_TIME(8, 0) + 1, CLOCK_TIME(16, 0), 5, 5 },
        { CLOCK_TIME(16, 0), CLOCK_TIME(17, 0) + 1, 5, 6 },
        { CLOCK_TIME(17, 0) + 1, CLOCK_TIME(19, 0) + 1, 6, 7 },
        { CLOCK_TIME(19, 0) + 1, CLOCK_TIME(24, 0) - 1, 7, 7 },
    },
    {
        { CLOCK_TIME(0, 0), CLOCK_TIME(4, 0) + 1, 11, 11 },
        { CLOCK_TIME(4, 0) + 1, CLOCK_TIME(6, 0), 11, 8 },
        { CLOCK_TIME(6, 0), CLOCK_TIME(8, 0) + 1, 8, 9 },
        { CLOCK_TIME(8, 0) + 1, CLOCK_TIME(16, 0), 9, 9 },
        { CLOCK_TIME(16, 0), CLOCK_TIME(17, 0) + 1, 9, 10 },
        { CLOCK_TIME(17, 0) + 1, CLOCK_TIME(19, 0) + 1, 10, 11 },
        { CLOCK_TIME(19, 0) + 1, CLOCK_TIME(24, 0) - 1, 11, 11 },
    },
    {
        { CLOCK_TIME(0, 0), CLOCK_TIME(4, 0) + 1, 15, 15 },
        { CLOCK_TIME(4, 0) + 1, CLOCK_TIME(6, 0), 15, 12 },
        { CLOCK_TIME(6, 0), CLOCK_TIME(8, 0) + 1, 12, 13 },
        { CLOCK_TIME(8, 0) + 1, CLOCK_TIME(16, 0), 13, 13 },
        { CLOCK_TIME(16, 0), CLOCK_TIME(17, 0) + 1, 13, 14 },
        { CLOCK_TIME(17, 0) + 1, CLOCK_TIME(19, 0) + 1, 14, 15 },
        { CLOCK_TIME(19, 0) + 1, CLOCK_TIME(24, 0) - 1, 15, 15 },
    },
    {
        { CLOCK_TIME(0, 0), CLOCK_TIME(4, 0) + 1, 23, 23 },
        { CLOCK_TIME(4, 0) + 1, CLOCK_TIME(6, 0), 23, 20 },
        { CLOCK_TIME(6, 0), CLOCK_TIME(8, 0) + 1, 20, 21 },
        { CLOCK_TIME(8, 0) + 1, CLOCK_TIME(16, 0), 21, 21 },
        { CLOCK_TIME(16, 0), CLOCK_TIME(17, 0) + 1, 21, 22 },
        { CLOCK_TIME(17, 0) + 1, CLOCK_TIME(19, 0) + 1, 22, 23 },
        { CLOCK_TIME(19, 0) + 1, CLOCK_TIME(24, 0) - 1, 23, 23 },
    },
};

TimeBasedSkyboxEntry gTimeBasedSkyboxConfigs[][9] = {
    {
        { CLOCK_TIME(0, 0), CLOCK_TIME(4, 0) + 1, false, 3, 3 },
        { CLOCK_TIME(4, 0) + 1, CLOCK_TIME(5, 0) + 1, true, 3, 0 },
        { CLOCK_TIME(5, 0) + 1, CLOCK_TIME(6, 0), false, 0, 0 },
        { CLOCK_TIME(6, 0), CLOCK_TIME(8, 0) + 1, true, 0, 1 },
        { CLOCK_TIME(8, 0) + 1, CLOCK_TIME(16, 0), false, 1, 1 },
        { CLOCK_TIME(16, 0), CLOCK_TIME(17, 0) + 1, true, 1, 2 },
        { CLOCK_TIME(17, 0) + 1, CLOCK_TIME(18, 0) + 1, false, 2, 2 },
        { CLOCK_TIME(18, 0) + 1, CLOCK_TIME(19, 0) + 1, true, 2, 3 },
        { CLOCK_TIME(19, 0) + 1, CLOCK_TIME(24, 0) - 1, false, 3, 3 },
    },
    {
        { CLOCK_TIME(0, 0), CLOCK_TIME(4, 0) + 1, false, 7, 7 },
        { CLOCK_TIME(4, 0) + 1, CLOCK_TIME(5, 0) + 1, true, 7, 4 },
        { CLOCK_TIME(5, 0) + 1, CLOCK_TIME(6, 0), false, 4, 4 },
        { CLOCK_TIME(6, 0), CLOCK_TIME(8, 0) + 1, true, 4, 5 },
        { CLOCK_TIME(8, 0) + 1, CLOCK_TIME(16, 0), false, 5, 5 },
        { CLOCK_TIME(16, 0), CLOCK_TIME(17, 0) + 1, true, 5, 6 },
        { CLOCK_TIME(17, 0) + 1, CLOCK_TIME(18, 0) + 1, false, 6, 6 },
        { CLOCK_TIME(18, 0) + 1, CLOCK_TIME(19, 0) + 1, true, 6, 7 },
        { CLOCK_TIME(19, 0) + 1, CLOCK_TIME(24, 0) - 1, false, 7, 7 },
    },
    {
        { CLOCK_TIME(0, 0), CLOCK_TIME(2, 0) + 1, false, 3, 3 },
        { CLOCK_TIME(2, 0) + 1, CLOCK_TIME(4, 0) + 1, true, 3, 0 },
        { CLOCK_TIME(4, 0) + 1, CLOCK_TIME(8, 0) + 1, false, 0, 0 },
        { CLOCK_TIME(8, 0) + 1, CLOCK_TIME(10, 0), true, 0, 1 },
        { CLOCK_TIME(10, 0), CLOCK_TIME(14, 0) + 1, false, 1, 1 },
        { CLOCK_TIME(14, 0) + 1, CLOCK_TIME(16, 0), true, 1, 2 },
        { CLOCK_TIME(16, 0), CLOCK_TIME(20, 0) + 1, false, 2, 2 },
        { CLOCK_TIME(20, 0) + 1, CLOCK_TIME(22, 0), true, 2, 3 },
        { CLOCK_TIME(22, 0), CLOCK_TIME(24, 0) - 1, false, 3, 3 },
    },
    {
        { CLOCK_TIME(0, 0), CLOCK_TIME(5, 0) + 1, false, 11, 11 },
        { CLOCK_TIME(5, 0) + 1, CLOCK_TIME(6, 0), true, 11, 8 },
        { CLOCK_TIME(6, 0), CLOCK_TIME(7, 0), false, 8, 8 },
        { CLOCK_TIME(7, 0), CLOCK_TIME(8, 0) + 1, true, 8, 9 },
        { CLOCK_TIME(8, 0) + 1, CLOCK_TIME(16, 0), false, 9, 9 },
        { CLOCK_TIME(16, 0), CLOCK_TIME(17, 0) + 1, true, 9, 10 },
        { CLOCK_TIME(17, 0) + 1, CLOCK_TIME(18, 0) + 1, false, 10, 10 },
        { CLOCK_TIME(18, 0) + 1, CLOCK_TIME(19, 0) + 1, true, 10, 11 },
        { CLOCK_TIME(19, 0) + 1, CLOCK_TIME(24, 0) - 1, false, 11, 11 },
    },
};

SkyboxFile gNormalSkyFiles[] = {
    {
        ROM_FILE(vr_fine0_static),
        ROM_FILE(vr_fine0_pal_static),
    },
    {
        ROM_FILE(vr_fine1_static),
        ROM_FILE(vr_fine1_pal_static),
    },
    {
        ROM_FILE(vr_fine2_static),
        ROM_FILE(vr_fine2_pal_static),
    },
    {
        ROM_FILE(vr_fine3_static),
        ROM_FILE(vr_fine3_pal_static),
    },
    {
        ROM_FILE(vr_cloud0_static),
        ROM_FILE(vr_cloud0_pal_static),
    },
    {
        ROM_FILE(vr_cloud1_static),
        ROM_FILE(vr_cloud1_pal_static),
    },
    {
        ROM_FILE(vr_cloud2_static),
        ROM_FILE(vr_cloud2_pal_static),
    },
    {
        ROM_FILE(vr_cloud3_static),
        ROM_FILE(vr_cloud3_pal_static),
    },
    {
        ROM_FILE(vr_holy0_static),
        ROM_FILE(vr_holy0_pal_static),
    },
};

u8 skyboxColors[][2][3] = {
    // dawn
    {
        {112, 104, 144}, 
        {192, 152, 136}
    },
    // day
    {
        {57, 73, 192}, 
        {180, 214, 255}
    },
    // dusk
    {
        {180, 110, 120}, 
        {255, 180, 90}
    },
    // night
    {
        {0, 8, 32}, 
        {0, 56, 168}
    },
    // cloudy dawn
    {
        {24, 24, 32}, 
        {96, 82, 70}
    },
    // cloudy day
    {
        {56, 56, 64}, 
        {112, 112, 96}
    },
    // cloudy dusk
    {
        {72, 64, 48}, 
        {32, 24, 16}
    },
    // cloudy night
    {
        {48, 48, 48}, 
        {0, 0, 0}
    },
    // holy
    {
        {255, 255, 255}, 
        {255, 255, 255}
    },
};

u8 sSandstormColorIndex = 0;
u8 sNextSandstormColorIndex = 0;
f32 sSandstormLerpScale = 0.0f;

u8 gCustomLensFlareOn;
Vec3f gCustomLensFlarePos;
s16 sLensFlareUnused;
s16 gLensFlareScale;
f32 gLensFlareColorIntensity;
s16 gLensFlareGlareStrength;

typedef struct LightningBolt {
    /* 0x00 */ u8 state;
    /* 0x04 */ Vec3f offset;
    /* 0x10 */ Vec3f pos;
    /* 0x1C */ s8 pitch;
    /* 0x1D */ s8 roll;
    /* 0x1E */ u8 textureIndex;
    /* 0x1F */ u8 delayTimer;
} LightningBolt; // size = 0x20

LightningBolt sLightningBolts[3];

LightningStrike gLightningStrike;

s16 sLightningFlashAlpha;

s16 sSunDepthTestX;
s16 sSunDepthTestY;

#pragma increment_block_number "gc-eu:128 gc-eu-mq:128 gc-jp:128 gc-jp-ce:128 gc-jp-mq:128 gc-us:128 gc-us-mq:128" \
                               "ique-cn:128 ntsc-1.0:128 ntsc-1.1:128 ntsc-1.2:128 pal-1.0:128 pal-1.1:128"

LightNode* sNGameOverLightNode;
LightInfo sNGameOverLightInfo;
LightNode* sSGameOverLightNode;
LightInfo sSGameOverLightInfo;
u8 sGameOverLightsIntensity;
u16 sSandstormScroll;

u16 gSkyboxNumStars;
Gfx* sSkyboxStarsDList;
s32 sEnvSkyboxNumStars = 0;
f32 sStarAlpha;
u8 sCloudDensity = 16;
static u8 weatherModeTest;

enum {
    WEATHER_EVENT_SUNNY,
    WEATHER_EVENT_CLOUDY,
    WEATHER_EVENT_RAIN,
    WEATHER_EVENT_THUNDER
};

#define ZBUFVAL_EXPONENT(v) (((v) >> 15) & 7)
#define ZBUFVAL_MANTISSA(v) (((v) >> 4) & 0x7FF)

/**
 * Convert an 18-bits Z buffer value to a fixed point 15.3 value
 *
 * zBufferVal is 18 bits:
 *   3: Exponent of z value
 *  11: Mantissa of z value
 *   4: dz value (unused)
 */
s32 Environment_ZBufValToFixedPoint(s32 zBufferVal) {
    // base[exp] + (mantissa << shift[exp])
    s32 ret = (ZBUFVAL_MANTISSA(zBufferVal) << sZBufValConversionTable[ZBUFVAL_EXPONENT(zBufferVal)].mantissaShift) +
              sZBufValConversionTable[ZBUFVAL_EXPONENT(zBufferVal)].base;

    return ret;
}

u16 Environment_GetPixelDepth(s32 x, s32 y) {
    s32 pixelDepth = gZBuffer[y][x];

    return pixelDepth;
}

void Environment_GraphCallback(GraphicsContext* gfxCtx, void* param) {
    PlayState* play = (PlayState*)param;

    sSunScreenDepth = Environment_GetPixelDepth(sSunDepthTestX, sSunDepthTestY);
    Lights_GlowCheck(play);
}

void Environment_WeatherInitTest(PlayState* play) {
    // test weather init, rewrite this later
    // problem: gweathermode and storm request act independently and block the other one depending which system acts first
    switch (weatherModeTest) {
        case WEATHER_EVENT_SUNNY:
            play->envCtx.precipitation[PRECIP_SOS_MAX] = 0;
            if (play->csCtx.state == CS_STATE_IDLE) {
                Environment_StopStormNatureAmbience(play);
            } else if (Audio_GetActiveSeqId(SEQ_PLAYER_BGM_MAIN) == NA_BGM_NATURE_AMBIENCE) {
                Audio_SetNatureAmbienceChannelIO(NATURE_CHANNEL_LIGHTNING, CHANNEL_IO_PORT_1, 0);
                Audio_SetNatureAmbienceChannelIO(NATURE_CHANNEL_RAIN, CHANNEL_IO_PORT_1, 0);
            }
            if (gWeatherMode == WEATHER_MODE_CLEAR && (play->envCtx.stormRequest == STORM_REQUEST_START)) {
                play->envCtx.stormRequest = STORM_REQUEST_STOP;
            } else {
                play->envCtx.stormRequest = STORM_REQUEST_NONE;
                play->envCtx.stormState = STORM_STATE_OFF;
            }
            play->envCtx.lightningState = LIGHTNING_OFF;
        break;
        case WEATHER_EVENT_CLOUDY:
            play->envCtx.precipitation[PRECIP_SOS_MAX] = 0;
            play->envCtx.stormRequest = STORM_REQUEST_START;
            if ((gWeatherMode != WEATHER_MODE_CLEAR) || play->envCtx.skyboxConfig != 0) {
                play->envCtx.stormState = STORM_STATE_ON;
            }
            play->envCtx.lightningState = LIGHTNING_OFF;
        break;
        case WEATHER_EVENT_RAIN:
            play->envCtx.precipitation[PRECIP_SOS_MAX] = 20;
            play->envCtx.stormRequest = STORM_REQUEST_START;
            if ((gWeatherMode != WEATHER_MODE_CLEAR) || play->envCtx.skyboxConfig != 0) {
                play->envCtx.stormState = STORM_STATE_ON;
            }
            Environment_PlayStormNatureAmbience(play);
        break;
        case WEATHER_EVENT_THUNDER:
            play->envCtx.precipitation[PRECIP_SOS_MAX] = 20;
            play->envCtx.stormRequest = STORM_REQUEST_START;
            if ((gWeatherMode != WEATHER_MODE_CLEAR) || play->envCtx.skyboxConfig != 0) {
                play->envCtx.stormState = STORM_STATE_ON;
            }
            play->envCtx.lightningState = LIGHTNING_ON;
            Environment_PlayStormNatureAmbience(play);
        break;
    }

    if (play->envCtx.stormRequest != STORM_REQUEST_NONE) {
        switch (play->envCtx.stormState) {
            case STORM_STATE_OFF:
                if ((play->envCtx.stormRequest == STORM_REQUEST_START) && !gSkyboxIsChanging) {
                    play->envCtx.changeSkyboxState = CHANGE_SKYBOX_REQUESTED;
                    play->envCtx.skyboxConfig = 0;
                    play->envCtx.changeSkyboxNextConfig = 1;
                    play->envCtx.changeSkyboxTimer = 1;
                    play->envCtx.changeLightEnabled = true;
                    play->envCtx.lightConfig = 0;
                    play->envCtx.changeLightNextConfig = 2;
                    gLightConfigAfterUnderwater = 2;
                    play->envCtx.changeLightTimer = play->envCtx.changeDuration = 1;
                    play->envCtx.stormState++;
                }
                break;

            case STORM_STATE_ON:
                if (!gSkyboxIsChanging && (play->envCtx.stormRequest == STORM_REQUEST_STOP)) {
                    gWeatherMode = WEATHER_MODE_CLEAR;
                    play->envCtx.changeSkyboxState = CHANGE_SKYBOX_REQUESTED;
                    play->envCtx.skyboxConfig = 1;
                    play->envCtx.changeSkyboxNextConfig = 0;
                    play->envCtx.changeSkyboxTimer = 1;
                    play->envCtx.changeLightEnabled = true;
                    play->envCtx.lightConfig = 2;
                    play->envCtx.changeLightNextConfig = 0;
                    gLightConfigAfterUnderwater = 0;
                    play->envCtx.changeLightTimer = play->envCtx.changeDuration = 1;
                    play->envCtx.precipitation[PRECIP_RAIN_MAX] = 0;
                    play->envCtx.stormRequest = STORM_REQUEST_NONE;
                    play->envCtx.stormState = STORM_STATE_OFF;
                }
                break;
        }
    }
}

void Environment_Init(PlayState* play2, EnvironmentContext* envCtx, s32 unused) {
    u8 i;
    PlayState* play = play2;

    sEnvSkyboxNumStars = 0;
    gSkyboxNumStars = 200; // maybe you could slowly increase and decrease

    // initialize skybox vertex colors
    for (i = 0; i < 3; i++) {
        play->skyboxCtx.skyboxTopColor[i] = skyboxColors[envCtx->skybox1Index][0][i];
        play->skyboxCtx.skyboxBottomColor[i] = skyboxColors[envCtx->skybox1Index][1][i];
    }

    gSaveContext.sunsSongState = SUNSSONG_INACTIVE;

    //! FAKE: (void)0 on CLOCK_TIME(18, 0)
    if (((void)0, gSaveContext.save.dayTime) > ((void)0, CLOCK_TIME(18, 0)) ||
        ((void)0, gSaveContext.save.dayTime) < CLOCK_TIME(6, 30)) {
        ((void)0, gSaveContext.save.nightFlag = 1);
    } else {
        ((void)0, gSaveContext.save.nightFlag = 0);
    }

    play->state.gfxCtx->callback = Environment_GraphCallback;
    play->state.gfxCtx->callbackParam = play;

    Lights_DirectionalSetInfo(&envCtx->dirLight1, 80, 80, 80, 80, 80, 80);
    LightContext_InsertLight(play, &play->lightCtx, &envCtx->dirLight1);

    Lights_DirectionalSetInfo(&envCtx->dirLight2, 80, 80, 80, 80, 80, 80);
    LightContext_InsertLight(play, &play->lightCtx, &envCtx->dirLight2);

    envCtx->skybox1Index = 99;
    envCtx->skybox2Index = 99;

    envCtx->changeSkyboxState = CHANGE_SKYBOX_INACTIVE;
    envCtx->changeSkyboxTimer = 0;
    envCtx->changeLightEnabled = false;
    envCtx->changeLightTimer = 0;

    // envCtx->skyboxDmaState = SKYBOX_DMA_INACTIVE;
    envCtx->lightConfig = 0;
    envCtx->changeLightNextConfig = 0;

    envCtx->glareAlpha = 0.0f;
    envCtx->lensFlareAlphaScale = 0.0f;

    envCtx->lightSetting = 0;
    envCtx->prevLightSetting = 0;
    envCtx->lightBlend = 1.0f;
    envCtx->lightBlendOverride = LIGHT_BLEND_OVERRIDE_NONE;

    envCtx->stormRequest = STORM_REQUEST_NONE;
    envCtx->stormState = STORM_STATE_OFF;
    envCtx->lightningState = LIGHTNING_OFF;
    envCtx->timeSeqState = TIMESEQ_DAY_BGM;
    envCtx->fillScreen = false;

    envCtx->screenFillColor[0] = 0;
    envCtx->screenFillColor[1] = 0;
    envCtx->screenFillColor[2] = 0;
    envCtx->screenFillColor[3] = 0;

    envCtx->customSkyboxFilter = false;

    envCtx->skyboxFilterColor[0] = 0;
    envCtx->skyboxFilterColor[1] = 0;
    envCtx->skyboxFilterColor[2] = 0;
    envCtx->skyboxFilterColor[3] = 0;

    envCtx->sandstormState = SANDSTORM_OFF;
    envCtx->sandstormPrimA = 0;
    envCtx->sandstormEnvA = 0;

    gLightningStrike.state = LIGHTNING_STRIKE_WAIT;
    gLightningStrike.flashRed = 0;
    gLightningStrike.flashGreen = 0;
    gLightningStrike.flashBlue = 0;

    sLightningFlashAlpha = 0;

    gSaveContext.cutsceneTransitionControl = 0;

    envCtx->adjAmbientColor[0] = envCtx->adjAmbientColor[1] = envCtx->adjAmbientColor[2] = envCtx->adjLight1Color[0] =
        envCtx->adjLight1Color[1] = envCtx->adjLight1Color[2] = envCtx->adjFogColor[0] = envCtx->adjFogColor[1] =
            envCtx->adjFogColor[2] = envCtx->adjFogNear = envCtx->adjZFar = 0;

    envCtx->sunPos.x = -(Math_SinS(((void)0, gSaveContext.save.dayTime) - CLOCK_TIME(12, 0)) * 120.0f) * 25.0f;
    envCtx->sunPos.y = +(Math_CosS(((void)0, gSaveContext.save.dayTime) - CLOCK_TIME(12, 0)) * 120.0f) * 25.0f;
    envCtx->sunPos.z = +(Math_CosS(((void)0, gSaveContext.save.dayTime) - CLOCK_TIME(12, 0)) * 20.0f) * 25.0f;

    envCtx->windDirection.x = 80;
    envCtx->windDirection.y = 80;
    envCtx->windDirection.z = 80;

    envCtx->lightBlendEnabled = false;
    envCtx->lightSettingOverride = LIGHT_SETTING_OVERRIDE_NONE;
    envCtx->lightBlendRateOverride = LIGHT_BLENDRATE_OVERRIDE_NONE;

    envCtx->sceneTimeSpeed = 0;
    gTimeSpeed = envCtx->sceneTimeSpeed;

#if DEBUG_FEATURES
    R_ENV_TIME_SPEED_OLD = gTimeSpeed;
    R_ENV_DISABLE_DBG = true;

    if (CREG(3) != 0) {
        gSaveContext.chamberCutsceneNum = CREG(3) - 1;
    }
#endif

    play->envCtx.precipitation[PRECIP_RAIN_MAX] = 0;
    play->envCtx.precipitation[PRECIP_RAIN_CUR] = 0;
    play->envCtx.precipitation[PRECIP_SNOW_CUR] = 0;
    play->envCtx.precipitation[PRECIP_SNOW_MAX] = 0;
    play->envCtx.precipitation[PRECIP_SOS_MAX] = 0;

    if (gSaveContext.retainWeatherMode) {
        if (!IS_CUTSCENE_LAYER) {
            switch (gWeatherMode) {
                case WEATHER_MODE_CLOUDY_CONFIG3:
                    envCtx->skyboxConfig = 1;
                    envCtx->changeSkyboxNextConfig = 1;
                    envCtx->lightConfig = 3;
                    envCtx->changeLightNextConfig = 3;
                    play->envCtx.precipitation[PRECIP_SNOW_MAX] = 0;
                    play->envCtx.precipitation[PRECIP_SNOW_CUR] = 0;
                    break;

                case WEATHER_MODE_CLOUDY_CONFIG2:
                case WEATHER_MODE_SNOW:
                case WEATHER_MODE_RAIN:
                    envCtx->skyboxConfig = 1;
                    envCtx->changeSkyboxNextConfig = 1;
                    envCtx->lightConfig = 2;
                    envCtx->changeLightNextConfig = 2;
                    play->envCtx.precipitation[PRECIP_SNOW_MAX] = 0;
                    play->envCtx.precipitation[PRECIP_SNOW_CUR] = 0;
                    break;

                case WEATHER_MODE_HEAVY_RAIN:
                    envCtx->skyboxConfig = 1;
                    envCtx->changeSkyboxNextConfig = 1;
                    envCtx->lightConfig = 4;
                    envCtx->changeLightNextConfig = 4;
                    play->envCtx.precipitation[PRECIP_SNOW_MAX] = 0;
                    play->envCtx.precipitation[PRECIP_SNOW_CUR] = 0;
                    break;

                default:
                    break;
            }

            if (play->skyboxId == SKYBOX_NORMAL_SKY) {
                if (gWeatherMode == WEATHER_MODE_SNOW) {
                    play->envCtx.precipitation[PRECIP_SNOW_CUR] = play->envCtx.precipitation[PRECIP_SNOW_MAX] = 64;
                } else if (gWeatherMode == WEATHER_MODE_RAIN) {
                    play->envCtx.precipitation[PRECIP_RAIN_MAX] = 20;
                    play->envCtx.precipitation[PRECIP_RAIN_CUR] = 20;
                } else if (gWeatherMode == WEATHER_MODE_HEAVY_RAIN) {
                    play->envCtx.precipitation[PRECIP_RAIN_MAX] = 30;
                    play->envCtx.precipitation[PRECIP_RAIN_CUR] = 30;
                }
            }
        }
    } else {
        gWeatherMode = WEATHER_MODE_CLEAR;
    }

    gInterruptSongOfStorms = false;
    gLightConfigAfterUnderwater = 0;
    gSkyboxIsChanging = false;
    gSaveContext.retainWeatherMode = false;

#if DEBUG_FEATURES
    R_ENV_LIGHT1_DIR(0) = 80;
    R_ENV_LIGHT1_DIR(1) = 80;
    R_ENV_LIGHT1_DIR(2) = 80;

    R_ENV_LIGHT2_DIR(0) = -80;
    R_ENV_LIGHT2_DIR(1) = -80;
    R_ENV_LIGHT2_DIR(2) = -80;

    cREG(9) = 10;
    cREG(10) = 0;
    cREG(11) = 0;
    cREG(12) = 0;
    cREG(13) = 0;
    cREG(14) = 0;
#endif

    gUseCutsceneCam = true;

    for (i = 0; i < ARRAY_COUNT(sLightningBolts); i++) {
        sLightningBolts[i].state = LIGHTNING_BOLT_INACTIVE;
    }

    for (i = 0; i < ARRAY_COUNT(play->csCtx.actorCues); i++) {
        play->csCtx.actorCues[i] = NULL;
    }

    if (Object_GetSlot(&play->objectCtx, OBJECT_GAMEPLAY_FIELD_KEEP) < 0 && !play->envCtx.sunMoonDisabled) {
        play->envCtx.sunMoonDisabled = true;
        PRINTF(VT_COL(YELLOW, BLACK) T("\n\nフィールド常駐以外、太陽設定！よって強制解除！\n",
                                       "\n\nSun setting other than field keep! So forced release!\n") VT_RST);
    }

    gCustomLensFlareOn = false;
    Rumble_Reset();

    if (play->skyboxCtx.drawType == SKYBOX_DRAW_128) {
        Environment_WeatherInitTest(play);
    }
}

u8 Environment_SmoothStepToU8(u8* pvalue, u8 target, u8 scale, u8 step, u8 minStep) {
    s16 stepSize = 0;
    s16 diff = target - *pvalue;

    if (target != *pvalue) {
        stepSize = diff / scale;
        if ((stepSize >= (s16)minStep) || ((s16)-minStep >= stepSize)) {
            if ((s16)step < stepSize) {
                stepSize = step;
            }
            if ((s16)-step > stepSize) {
                stepSize = -step;
            }
            *pvalue += (u8)stepSize;
        } else {
            if (stepSize < (s16)minStep) {
                stepSize = minStep;
                *pvalue += (u8)stepSize;
                if (target < *pvalue) {
                    *pvalue = target;
                }
            }
            if ((s16)-minStep < stepSize) {
                stepSize = -minStep;
                *pvalue += (u8)stepSize;
                if (*pvalue < target) {
                    *pvalue = target;
                }
            }
        }
    }
    return diff;
}

u8 Environment_SmoothStepToS8(s8* pvalue, s8 target, u8 scale, u8 step, u8 minStep) {
    s16 stepSize = 0;
    s16 diff = target - *pvalue;

    if (target != *pvalue) {
        stepSize = diff / scale;
        if ((stepSize >= (s16)minStep) || ((s16)-minStep >= stepSize)) {
            if ((s16)step < stepSize) {
                stepSize = step;
            }
            if ((s16)-step > stepSize) {
                stepSize = -step;
            }
            *pvalue += (s8)stepSize;
        } else {
            if (stepSize < (s16)minStep) {
                stepSize = minStep;
                *pvalue += (s8)stepSize;
                if (target < *pvalue) {
                    *pvalue = target;
                }
            }
            if ((s16)-minStep < stepSize) {
                stepSize = -minStep;
                *pvalue += (s8)stepSize;
                if (*pvalue < target) {
                    *pvalue = target;
                }
            }
        }
    }
    return diff;
}

f32 Environment_LerpWeight(u16 max, u16 min, u16 val) {
    f32 diff = max - min;

    if (diff != 0.0f) {
        f32 ret = 1.0f - (max - val) / diff;

        if (!(ret >= 1.0f)) {
            return ret;
        }
    }

    return 1.0f;
}

f32 Environment_LerpWeightAccelDecel(u16 endFrame, u16 startFrame, u16 curFrame, u16 accelDuration, u16 decelDuration) {
    f32 endFrameF;
    f32 startFrameF;
    f32 curFrameF;
    f32 accelDurationF;
    f32 decelDurationF;
    f32 totalFrames;
    f32 temp;
    f32 framesElapsed;
    f32 ret;

    if (curFrame <= startFrame) {
        return 0.0f;
    }

    if (curFrame >= endFrame) {
        return 1.0f;
    }

    endFrameF = (s32)endFrame;
    startFrameF = (s32)startFrame;
    curFrameF = (s32)curFrame;
    totalFrames = endFrameF - startFrameF;
    framesElapsed = curFrameF - startFrameF;
    accelDurationF = (s32)accelDuration;
    decelDurationF = (s32)decelDuration;

    if ((startFrameF >= endFrameF) || (accelDurationF + decelDurationF > totalFrames)) {
        PRINTF(VT_COL(RED, WHITE) T("\nend_frameとstart_frameのフレーム関係がおかしい!!!",
                                    "\nThe frame relation between end_frame and start_frame is wrong!!!") VT_RST);
        PRINTF(VT_COL(RED, WHITE) "\nby get_parcent_forAccelBrake!!!!!!!!!" VT_RST);

        return 0.0f;
    }

    temp = 1.0f / ((totalFrames * 2.0f) - accelDurationF - decelDurationF);

    if (accelDurationF != 0.0f) {
        if (framesElapsed <= accelDurationF) {
            return temp * framesElapsed * framesElapsed / accelDurationF;
        }
        ret = temp * accelDurationF;
    } else {
        ret = 0.0f;
    }

    if (framesElapsed <= totalFrames - decelDurationF) {
        ret += 2.0f * temp * (framesElapsed - accelDurationF);
        return ret;
    }

    ret += 2.0f * temp * (totalFrames - accelDurationF - decelDurationF);

    if (decelDurationF != 0.0f) {
        ret += temp * decelDurationF;
        if (framesElapsed < totalFrames) {
            ret -= temp * (totalFrames - framesElapsed) * (totalFrames - framesElapsed) / decelDurationF;
        }
    }

    return ret;
}

void Environment_UpdateStorm(EnvironmentContext* envCtx, u8 unused) {
    if (envCtx->stormRequest != STORM_REQUEST_NONE) {
        switch (envCtx->stormState) {
            case STORM_STATE_OFF:
                if ((envCtx->stormRequest == STORM_REQUEST_START) && !gSkyboxIsChanging) {
                    envCtx->changeSkyboxState = CHANGE_SKYBOX_REQUESTED;
                    envCtx->skyboxConfig = 0;
                    envCtx->changeSkyboxNextConfig = 1;
                    envCtx->changeSkyboxTimer = 100;
                    envCtx->changeLightEnabled = true;
                    envCtx->lightConfig = 0;
                    envCtx->changeLightNextConfig = 2;
                    gLightConfigAfterUnderwater = 2;
                    envCtx->changeLightTimer = envCtx->changeDuration = 100;
                    envCtx->stormState++;
                }
                break;

            case STORM_STATE_ON:
                if (!gSkyboxIsChanging && (envCtx->stormRequest == STORM_REQUEST_STOP)) {
                    gWeatherMode = WEATHER_MODE_CLEAR;
                    envCtx->changeSkyboxState = CHANGE_SKYBOX_REQUESTED;
                    envCtx->skyboxConfig = 1;
                    envCtx->changeSkyboxNextConfig = 0;
                    envCtx->changeSkyboxTimer = 100;
                    envCtx->changeLightEnabled = true;
                    envCtx->lightConfig = 2;
                    envCtx->changeLightNextConfig = 0;
                    gLightConfigAfterUnderwater = 0;
                    envCtx->changeLightTimer = envCtx->changeDuration = 100;
                    envCtx->precipitation[PRECIP_RAIN_MAX] = 0;
                    envCtx->stormRequest = STORM_REQUEST_NONE;
                    envCtx->stormState = STORM_STATE_OFF;
                }
                break;
        }
    }
}

void Environment_UpdateSkybox(u8 skyboxId, EnvironmentContext* envCtx, SkyboxContext* skyboxCtx) {
    u32 size;
    u8 i;
    u8 newSkybox1Index = 0xFF;
    u8 newSkybox2Index = 0xFF;
    u8 skyboxBlend = 0;

    if (skyboxId == SKYBOX_CUTSCENE_MAP) {
        envCtx->skyboxConfig = 3;

        for (i = 0; i < ARRAY_COUNT(gTimeBasedSkyboxConfigs[envCtx->skyboxConfig]); i++) {
            if (gSaveContext.skyboxTime >= gTimeBasedSkyboxConfigs[envCtx->skyboxConfig][i].startTime &&
                (gSaveContext.skyboxTime < gTimeBasedSkyboxConfigs[envCtx->skyboxConfig][i].endTime ||
                 gTimeBasedSkyboxConfigs[envCtx->skyboxConfig][i].endTime == 0xFFFF)) {
                if (gTimeBasedSkyboxConfigs[envCtx->skyboxConfig][i].changeSkybox) {
                    envCtx->skyboxBlend =
                        Environment_LerpWeight(gTimeBasedSkyboxConfigs[envCtx->skyboxConfig][i].endTime,
                                               gTimeBasedSkyboxConfigs[envCtx->skyboxConfig][i].startTime,
                                               ((void)0, gSaveContext.skyboxTime)) *
                        255;
                } else {
                    envCtx->skyboxBlend = 0;
                }
                break;
            }
        }
    } else if (skyboxId == SKYBOX_NORMAL_SKY && !envCtx->skyboxDisabled) {
        for (i = 0; i < ARRAY_COUNT(gTimeBasedSkyboxConfigs[envCtx->skyboxConfig]); i++) {
            if (gSaveContext.skyboxTime >= gTimeBasedSkyboxConfigs[envCtx->skyboxConfig][i].startTime &&
                (gSaveContext.skyboxTime < gTimeBasedSkyboxConfigs[envCtx->skyboxConfig][i].endTime ||
                 gTimeBasedSkyboxConfigs[envCtx->skyboxConfig][i].endTime == 0xFFFF)) {
                newSkybox1Index = gTimeBasedSkyboxConfigs[envCtx->skyboxConfig][i].skybox1Index;
                newSkybox2Index = gTimeBasedSkyboxConfigs[envCtx->skyboxConfig][i].skybox2Index;
                gSkyboxIsChanging = gTimeBasedSkyboxConfigs[envCtx->skyboxConfig][i].changeSkybox;

                if (gSkyboxIsChanging) {
                    skyboxBlend = Environment_LerpWeight(gTimeBasedSkyboxConfigs[envCtx->skyboxConfig][i].endTime,
                                                         gTimeBasedSkyboxConfigs[envCtx->skyboxConfig][i].startTime,
                                                         ((void)0, gSaveContext.skyboxTime)) *
                                  255;
                } else {
                    skyboxBlend = Environment_LerpWeight(gTimeBasedSkyboxConfigs[envCtx->skyboxConfig][i].endTime,
                                                         gTimeBasedSkyboxConfigs[envCtx->skyboxConfig][i].startTime,
                                                         ((void)0, gSaveContext.skyboxTime)) *
                                  255;

                    skyboxBlend = (skyboxBlend < 128) ? 255 : 0;

                    if ((envCtx->changeSkyboxState != CHANGE_SKYBOX_INACTIVE) &&
                        (envCtx->changeSkyboxState < CHANGE_SKYBOX_ACTIVE)) {
                        envCtx->changeSkyboxState++;
                        skyboxBlend = 0;
                    }
                }
                break;
            }
        }

        Environment_UpdateStorm(envCtx, skyboxBlend);

        if (envCtx->changeSkyboxState >= CHANGE_SKYBOX_ACTIVE) {
            /* newSkybox1Index = gTimeBasedSkyboxConfigs[envCtx->skyboxConfig][i].skybox1Index;
            newSkybox2Index = gTimeBasedSkyboxConfigs[envCtx->changeSkyboxNextConfig][i].skybox2Index; */

            skyboxBlend = ((f32)envCtx->changeDuration - envCtx->changeSkyboxTimer) / (f32)envCtx->changeDuration * 255;
            envCtx->changeSkyboxTimer--;

            if (envCtx->changeSkyboxTimer <= 0) {
                envCtx->changeSkyboxState = CHANGE_SKYBOX_INACTIVE;
                envCtx->skyboxConfig = envCtx->changeSkyboxNextConfig;
            }
        }

        f32 timeChangeBlend;
        f32 configBlend;

        timeChangeBlend =
                        Environment_LerpWeight(gTimeBasedSkyboxConfigs[envCtx->skyboxConfig][i].endTime,
                                               gTimeBasedSkyboxConfigs[envCtx->skyboxConfig][i].startTime,
                                               ((void)0, gSaveContext.skyboxTime));

        if (envCtx->changeSkyboxState >= CHANGE_SKYBOX_ACTIVE) { // cloudy
            u8 blend8[2];
            configBlend = ((f32)envCtx->changeDuration - envCtx->changeSkyboxTimer) / (f32)envCtx->changeDuration;

            if (envCtx->skyboxConfig != envCtx->changeSkyboxNextConfig) {
                for (u8 j = 0; j < 3; j++) {
                blend8[0] = LERP(skyboxColors[gTimeBasedSkyboxConfigs[envCtx->skyboxConfig][i].skybox1Index][0][j], skyboxColors[gTimeBasedSkyboxConfigs[envCtx->skyboxConfig][i].skybox2Index][0][j], timeChangeBlend);
                blend8[1] = LERP(skyboxColors[gTimeBasedSkyboxConfigs[envCtx->changeSkyboxNextConfig][i].skybox1Index][0][j], skyboxColors[gTimeBasedSkyboxConfigs[envCtx->changeSkyboxNextConfig][i].skybox2Index][0][j], timeChangeBlend);

                skyboxCtx->skyboxTopColor[j] = LERP(blend8[0], blend8[1], configBlend);

                blend8[0] = LERP(skyboxColors[gTimeBasedSkyboxConfigs[envCtx->skyboxConfig][i].skybox1Index][1][j], skyboxColors[gTimeBasedSkyboxConfigs[envCtx->skyboxConfig][i].skybox2Index][1][j], timeChangeBlend);
                blend8[1] = LERP(skyboxColors[gTimeBasedSkyboxConfigs[envCtx->changeSkyboxNextConfig][i].skybox1Index][1][j], skyboxColors[gTimeBasedSkyboxConfigs[envCtx->changeSkyboxNextConfig][i].skybox2Index][1][j], timeChangeBlend);
                skyboxCtx->skyboxBottomColor[j] = LERP(blend8[0], blend8[1], configBlend);
                }
            }
        } else { // regular daytime
            for (u8 j = 0; j < 3; j++) {
                skyboxCtx->skyboxTopColor[j] = LERP(skyboxColors[gTimeBasedSkyboxConfigs[envCtx->skyboxConfig][i].skybox1Index][0][j], skyboxColors[gTimeBasedSkyboxConfigs[envCtx->skyboxConfig][i].skybox2Index][0][j], timeChangeBlend);
                skyboxCtx->skyboxBottomColor[j] = LERP(skyboxColors[gTimeBasedSkyboxConfigs[envCtx->skyboxConfig][i].skybox1Index][1][j], skyboxColors[gTimeBasedSkyboxConfigs[envCtx->skyboxConfig][i].skybox2Index][1][j], timeChangeBlend);
            }
        }

#if DEBUG_FEATURES
        if (newSkybox1Index == 0xFF) {
            PRINTF(VT_COL(RED, WHITE) T("\n環境ＶＲデータ取得失敗！ ささきまでご報告を！",
                                        "\nEnvironment VR data acquisition failed! Report to Sasaki!") VT_RST);
        }
#endif

        if ((envCtx->skybox1Index != newSkybox1Index)) {
            envCtx->skybox1Index = newSkybox1Index;
        }

        if ((envCtx->skybox2Index != newSkybox2Index)) {
            envCtx->skybox2Index = newSkybox2Index;
        }

        envCtx->skyboxBlend = skyboxBlend;
    }
}

void Environment_EnableUnderwaterLights(PlayState* play, s32 waterLightsIndex) {
    if (waterLightsIndex == WATERBOX_LIGHT_INDEX_NONE) {
        waterLightsIndex = 0;
        PRINTF(VT_COL(YELLOW, BLACK) T("\n水ポリゴンデータに水中カラーが設定されておりません!",
                                       "\nUnderwater color is not set in the water poly data!") VT_RST);
    }

    if (play->envCtx.lightMode == LIGHT_MODE_TIME) {
        gLightConfigAfterUnderwater = play->envCtx.changeLightNextConfig;

        if (play->envCtx.lightConfig != waterLightsIndex) {
            play->envCtx.lightConfig = waterLightsIndex;
            play->envCtx.changeLightNextConfig = waterLightsIndex;
        }
    } else {
        play->envCtx.lightBlendEnabled = false; // instantly switch to water lights
        play->envCtx.lightSettingOverride = waterLightsIndex;
    }
}

void Environment_DisableUnderwaterLights(PlayState* play) {
    if (play->envCtx.lightMode == LIGHT_MODE_TIME) {
        play->envCtx.lightConfig = gLightConfigAfterUnderwater;
        play->envCtx.changeLightNextConfig = gLightConfigAfterUnderwater;
    } else {
        play->envCtx.lightBlendEnabled = false; // instantly switch to previous lights
        play->envCtx.lightSettingOverride = LIGHT_SETTING_OVERRIDE_NONE;
        play->envCtx.lightBlend = 1.0f;
    }
}

#if DEBUG_FEATURES
void Environment_PrintDebugInfo(PlayState* play, Gfx** gfx) {
    if (CAN_SHOW_TIME_INFOS) {
        GfxPrint printer;
        s32 pad[2];

        GfxPrint_Init(&printer);
        GfxPrint_Open(&printer, *gfx);

        GfxPrint_SetPos(&printer, 22, 7);
        GfxPrint_SetColor(&printer, 155, 155, 255, 64);
        GfxPrint_Printf(&printer, "T%03d ", ((void)0, gSaveContext.save.totalDays));
        GfxPrint_Printf(&printer, "E%03d", ((void)0, gSaveContext.save.bgsDayCount));

        GfxPrint_SetColor(&printer, 255, 255, 55, 64);
        GfxPrint_SetPos(&printer, 22, 8);
        GfxPrint_Printf(&printer, "%s", "ZELDATIME ");

        GfxPrint_SetColor(&printer, 255, 255, 255, 64);
        GfxPrint_Printf(&printer, "%02d", (u8)(24 * 60 / (f32)0x10000 * ((void)0, gSaveContext.save.dayTime) / 60.0f));

        if ((gSaveContext.save.dayTime & 0x1F) >= 0x10 || gTimeSpeed >= 6) {
            GfxPrint_Printf(&printer, "%s", ":");
        } else {
            GfxPrint_Printf(&printer, "%s", " ");
        }

        GfxPrint_Printf(&printer, "%02d", (s16)(24 * 60 / (f32)0x10000 * ((void)0, gSaveContext.save.dayTime)) % 60);

        GfxPrint_SetColor(&printer, 255, 255, 55, 64);
        GfxPrint_SetPos(&printer, 22, 9);
        GfxPrint_Printf(&printer, "%s", "VRBOXTIME ");

        GfxPrint_SetColor(&printer, 255, 255, 255, 64);
        GfxPrint_Printf(&printer, "%02d", (u8)(24 * 60 / (f32)0x10000 * ((void)0, gSaveContext.skyboxTime) / 60.0f));

        if ((((void)0, gSaveContext.skyboxTime) & 0x1F) >= 0x10 || gTimeSpeed >= 6) {
            GfxPrint_Printf(&printer, "%s", ":");
        } else {
            GfxPrint_Printf(&printer, "%s", " ");
        }

        GfxPrint_Printf(&printer, "%02d", (s16)(24 * 60 / (f32)0x10000 * ((void)0, gSaveContext.skyboxTime)) % 60);

        GfxPrint_SetColor(&printer, 55, 255, 255, 64);
        GfxPrint_SetPos(&printer, 22, 6);

        if (!IS_DAY) {
            GfxPrint_Printf(&printer, "%s", T("YORU", "NIGHT"));
        } else {
            GfxPrint_Printf(&printer, "%s", T("HIRU", "DAY"));
        }

        *gfx = GfxPrint_Close(&printer);
        GfxPrint_Destroy(&printer);
    }
}
#endif

void Environment_PlayTimeBasedSequence(PlayState* play);
void Environment_UpdateRain(PlayState* play);

void Environment_Update(PlayState* play, EnvironmentContext* envCtx, LightContext* lightCtx, PauseContext* pauseCtx,
                        MessageContext* msgCtx, GameOverContext* gameOverCtx, GraphicsContext* gfxCtx) {
    f32 timeChangeBlend;
    f32 configChangeBlend = 0.0f;
    u16 i;
    u16 j;
    u16 time;
    EnvLightSettings* lightSettingsList = play->envCtx.lightSettingsList;
    u8 blendRate;

    if ((((void)0, gSaveContext.gameMode) != GAMEMODE_NORMAL) &&
        (((void)0, gSaveContext.gameMode) != GAMEMODE_END_CREDITS)) {
        Rumble_ClearRequests();
    }

    if (pauseCtx->state == PAUSE_STATE_OFF) {
        if (!IS_PAUSED(&play->pauseCtx)) {
            if (play->skyboxId == SKYBOX_NORMAL_SKY) {
                play->skyboxCtx.rot.y -= 0.0005f;
            } else if (play->skyboxId == SKYBOX_CUTSCENE_MAP) {
                play->skyboxCtx.rot.y -= 0.005f;
            }
        }

        Environment_UpdateRain(play);
        Environment_PlayTimeBasedSequence(play);
        Environment_UpdateClouds(play);

        if (((void)0, gSaveContext.nextDayTime) >= 0xFF00 && ((void)0, gSaveContext.nextDayTime) != NEXT_TIME_NONE) {
            gSaveContext.nextDayTime -= 0x10;
            PRINTF("\nnext_zelda_time=[%x]", ((void)0, gSaveContext.nextDayTime));

            // nextDayTime is used as both a time of day value and a timer to delay sfx when changing days.
            // When Sun's Song is played, nextDayTime is set to 0x8001 or 0 for day and night respectively.
            // These values will actually get used as a time of day value.
            // After this, nextDayTime is assigned magic values of 0xFFFE or 0xFFFD for day and night respectively.
            // From here, 0x10 is decremented from nextDayTime until it reaches either 0xFF0E or 0xFF0D, effectively
            // delaying the chicken crow or dog howl sfx by 15 frames when loading the new area.

            if (((void)0, gSaveContext.nextDayTime) == (NEXT_TIME_DAY_SET - (15 * 0x10))) {
                Sfx_PlaySfxCentered(NA_SE_EV_CHICKEN_CRY_M);
                gSaveContext.nextDayTime = NEXT_TIME_NONE;
            } else if (((void)0, gSaveContext.nextDayTime) == (NEXT_TIME_NIGHT_SET - (15 * 0x10))) {
                Sfx_PlaySfxCentered2(NA_SE_EV_DOG_CRY_EVENING);
                gSaveContext.nextDayTime = NEXT_TIME_NONE;
            }
        }

        if ((pauseCtx->state == PAUSE_STATE_OFF) && (gameOverCtx->state == GAMEOVER_INACTIVE)) {
            if (((msgCtx->msgLength == 0) && (msgCtx->msgMode == MSGMODE_NONE)) ||
                (((void)0, gSaveContext.gameMode) == GAMEMODE_END_CREDITS)) {

                if ((envCtx->changeSkyboxTimer == 0) && !FrameAdvance_IsEnabled(play) &&

                    (play->transitionMode == TRANS_MODE_OFF || ((void)0, gSaveContext.gameMode) != GAMEMODE_NORMAL)) {

                    if (IS_DAY || gTimeSpeed >= 400) {
                        gSaveContext.save.dayTime += gTimeSpeed;
                    } else {
                        gSaveContext.save.dayTime += gTimeSpeed * 2; // time moves twice as fast at night
                    }
                }
            }
        }

        //! @bug `gTimeSpeed` is unsigned, it can't be negative
#if OOT_VERSION < PAL_1_0
        if ((((void)0, gSaveContext.save.dayTime) > ((void)0, gSaveContext.skyboxTime)) ||
            (((void)0, gSaveContext.save.dayTime) < CLOCK_TIME(1, 0) || gTimeSpeed < 0))
#else
        if (((((void)0, gSaveContext.sceneLayer) >= 5 || gTimeSpeed != 0) &&
             ((void)0, gSaveContext.save.dayTime) > ((void)0, gSaveContext.skyboxTime)) ||
            (((void)0, gSaveContext.save.dayTime) < CLOCK_TIME(1, 0) || gTimeSpeed < 0))
#endif
        {

            gSaveContext.skyboxTime = ((void)0, gSaveContext.save.dayTime);
        }

        time = gSaveContext.save.dayTime;

        if (time > CLOCK_TIME(18, 0) || time < CLOCK_TIME(6, 30)) {
            gSaveContext.save.nightFlag = 1;
        } else {
            gSaveContext.save.nightFlag = 0;
        }

#if CAN_SHOW_TIME_INFOS
        Gfx* displayList;
        Gfx* prevDisplayList;

        OPEN_DISPS(play->state.gfxCtx, "../z_kankyo.c", 1682);

        prevDisplayList = POLY_OPA_DISP;
        displayList = Gfx_Open(POLY_OPA_DISP);
        gSPDisplayList(OVERLAY_DISP++, displayList);
        Environment_PrintDebugInfo(play, &displayList);
        gSPEndDisplayList(displayList++);
        Gfx_Close(prevDisplayList, displayList);
        POLY_OPA_DISP = displayList;

        CLOSE_DISPS(play->state.gfxCtx, "../z_kankyo.c", 1690);
#endif

        if ((envCtx->lightSettingOverride != LIGHT_SETTING_OVERRIDE_NONE) &&
            (envCtx->lightBlendOverride != LIGHT_BLEND_OVERRIDE_FULL_CONTROL) &&
            (envCtx->lightSetting != envCtx->lightSettingOverride) && (envCtx->lightBlend >= 1.0f) &&
            (envCtx->lightSettingOverride <= LIGHT_SETTING_MAX)) {

            envCtx->lightBlend = 0.0f;
            envCtx->prevLightSetting = envCtx->lightSetting;
            envCtx->lightSetting = envCtx->lightSettingOverride;
        }

        if (envCtx->lightSettingOverride == LIGHT_SETTING_OVERRIDE_FULL_CONTROL) {
            // Do nothing; Skip updating lights based on time or light settings
        } else if ((envCtx->lightMode == LIGHT_MODE_TIME) &&
                   (envCtx->lightSettingOverride == LIGHT_SETTING_OVERRIDE_NONE)) {
            for (i = 0; i < ARRAY_COUNT(sTimeBasedLightConfigs[envCtx->lightConfig]); i++) {
                if ((gSaveContext.skyboxTime >= sTimeBasedLightConfigs[envCtx->lightConfig][i].startTime) &&
                    ((gSaveContext.skyboxTime < sTimeBasedLightConfigs[envCtx->lightConfig][i].endTime) ||
                     sTimeBasedLightConfigs[envCtx->lightConfig][i].endTime == 0xFFFF)) {
                    u8 blend8[2];
                    s16 blend16[2];

                    timeChangeBlend = Environment_LerpWeight(sTimeBasedLightConfigs[envCtx->lightConfig][i].endTime,
                                                             sTimeBasedLightConfigs[envCtx->lightConfig][i].startTime,
                                                             ((void)0, gSaveContext.skyboxTime));

                    sSandstormColorIndex = sTimeBasedLightConfigs[envCtx->lightConfig][i].lightSetting & 3;
                    sNextSandstormColorIndex = sTimeBasedLightConfigs[envCtx->lightConfig][i].nextLightSetting & 3;
                    sSandstormLerpScale = timeChangeBlend;

                    if (envCtx->changeLightEnabled) {
                        configChangeBlend =
                            ((f32)envCtx->changeDuration - envCtx->changeLightTimer) / envCtx->changeDuration;
                        envCtx->changeLightTimer--;

                        if (envCtx->changeLightTimer <= 0) {
                            envCtx->changeLightEnabled = false;
                            envCtx->lightConfig = envCtx->changeLightNextConfig;
                        }
                    }

                    for (j = 0; j < 3; j++) {
                        // blend ambient color
                        blend8[0] =
                            LERP(lightSettingsList[sTimeBasedLightConfigs[envCtx->lightConfig][i].lightSetting]
                                     .ambientColor[j],
                                 lightSettingsList[sTimeBasedLightConfigs[envCtx->lightConfig][i].nextLightSetting]
                                     .ambientColor[j],
                                 timeChangeBlend);
                        blend8[1] = LERP(
                            lightSettingsList[sTimeBasedLightConfigs[envCtx->changeLightNextConfig][i].lightSetting]
                                .ambientColor[j],
                            lightSettingsList[sTimeBasedLightConfigs[envCtx->changeLightNextConfig][i].nextLightSetting]
                                .ambientColor[j],
                            timeChangeBlend);
                        envCtx->lightSettings.ambientColor[j] = LERP(blend8[0], blend8[1], configChangeBlend);
                    }

                    // set light1 direction for the sun
                    envCtx->lightSettings.light1Dir[0] =
                        -(Math_SinS(((void)0, gSaveContext.save.dayTime) - CLOCK_TIME(12, 0)) * 120.0f);
                    envCtx->lightSettings.light1Dir[1] =
                        Math_CosS(((void)0, gSaveContext.save.dayTime) - CLOCK_TIME(12, 0)) * 120.0f;
                    envCtx->lightSettings.light1Dir[2] =
                        Math_CosS(((void)0, gSaveContext.save.dayTime) - CLOCK_TIME(12, 0)) * 20.0f;

                    // set light2 direction for the moon
                    envCtx->lightSettings.light2Dir[0] = -envCtx->lightSettings.light1Dir[0];
                    envCtx->lightSettings.light2Dir[1] = -envCtx->lightSettings.light1Dir[1];
                    envCtx->lightSettings.light2Dir[2] = -envCtx->lightSettings.light1Dir[2];

                    for (j = 0; j < 3; j++) {
                        // blend light1Color
                        blend8[0] =
                            LERP(lightSettingsList[sTimeBasedLightConfigs[envCtx->lightConfig][i].lightSetting]
                                     .light1Color[j],
                                 lightSettingsList[sTimeBasedLightConfigs[envCtx->lightConfig][i].nextLightSetting]
                                     .light1Color[j],
                                 timeChangeBlend);
                        blend8[1] = LERP(
                            lightSettingsList[sTimeBasedLightConfigs[envCtx->changeLightNextConfig][i].lightSetting]
                                .light1Color[j],
                            lightSettingsList[sTimeBasedLightConfigs[envCtx->changeLightNextConfig][i].nextLightSetting]
                                .light1Color[j],
                            timeChangeBlend);
                        envCtx->lightSettings.light1Color[j] = LERP(blend8[0], blend8[1], configChangeBlend);

                        // blend light2Color
                        blend8[0] =
                            LERP(lightSettingsList[sTimeBasedLightConfigs[envCtx->lightConfig][i].lightSetting]
                                     .light2Color[j],
                                 lightSettingsList[sTimeBasedLightConfigs[envCtx->lightConfig][i].nextLightSetting]
                                     .light2Color[j],
                                 timeChangeBlend);
                        blend8[1] = LERP(
                            lightSettingsList[sTimeBasedLightConfigs[envCtx->changeLightNextConfig][i].lightSetting]
                                .light2Color[j],
                            lightSettingsList[sTimeBasedLightConfigs[envCtx->changeLightNextConfig][i].nextLightSetting]
                                .light2Color[j],
                            timeChangeBlend);
                        envCtx->lightSettings.light2Color[j] = LERP(blend8[0], blend8[1], configChangeBlend);
                    }

                    // blend fogColor
                    for (j = 0; j < 3; j++) {
                        blend8[0] = LERP(
                            lightSettingsList[sTimeBasedLightConfigs[envCtx->lightConfig][i].lightSetting].fogColor[j],
                            lightSettingsList[sTimeBasedLightConfigs[envCtx->lightConfig][i].nextLightSetting]
                                .fogColor[j],
                            timeChangeBlend);
                        blend8[1] = LERP(
                            lightSettingsList[sTimeBasedLightConfigs[envCtx->changeLightNextConfig][i].lightSetting]
                                .fogColor[j],
                            lightSettingsList[sTimeBasedLightConfigs[envCtx->changeLightNextConfig][i].nextLightSetting]
                                .fogColor[j],
                            timeChangeBlend);
                        envCtx->lightSettings.fogColor[j] = LERP(blend8[0], blend8[1], configChangeBlend);
                    }

                    blend16[0] =
                        LERP16(ENV_LIGHT_SETTINGS_FOG_NEAR(
                                   lightSettingsList[sTimeBasedLightConfigs[envCtx->lightConfig][i].lightSetting]
                                       .blendRateAndFogNear),
                               ENV_LIGHT_SETTINGS_FOG_NEAR(
                                   lightSettingsList[sTimeBasedLightConfigs[envCtx->lightConfig][i].nextLightSetting]
                                       .blendRateAndFogNear),
                               timeChangeBlend);
                    blend16[1] = LERP16(
                        ENV_LIGHT_SETTINGS_FOG_NEAR(
                            lightSettingsList[sTimeBasedLightConfigs[envCtx->changeLightNextConfig][i].lightSetting]
                                .blendRateAndFogNear),
                        ENV_LIGHT_SETTINGS_FOG_NEAR(
                            lightSettingsList[sTimeBasedLightConfigs[envCtx->changeLightNextConfig][i].nextLightSetting]
                                .blendRateAndFogNear),
                        timeChangeBlend);

                    envCtx->lightSettings.fogNear = LERP16(blend16[0], blend16[1], configChangeBlend);

                    blend16[0] =
                        LERP16(lightSettingsList[sTimeBasedLightConfigs[envCtx->lightConfig][i].lightSetting].zFar,
                               lightSettingsList[sTimeBasedLightConfigs[envCtx->lightConfig][i].nextLightSetting].zFar,
                               timeChangeBlend);
                    blend16[1] = LERP16(
                        lightSettingsList[sTimeBasedLightConfigs[envCtx->changeLightNextConfig][i].lightSetting].zFar,
                        lightSettingsList[sTimeBasedLightConfigs[envCtx->changeLightNextConfig][i].nextLightSetting]
                            .zFar,
                        timeChangeBlend);

                    envCtx->lightSettings.zFar = LERP16(blend16[0], blend16[1], configChangeBlend);

#if DEBUG_FEATURES
                    if (sTimeBasedLightConfigs[envCtx->changeLightNextConfig][i].nextLightSetting >=
                        envCtx->numLightSettings) {
                        PRINTF(VT_COL(RED, WHITE) T("\nカラーパレットの設定がおかしいようです！",
                                                    "\nThe color palette setting seems to be wrong!") VT_RST);

                        PRINTF(VT_COL(RED, WHITE) T("\n設定パレット＝[%d] 最後パレット番号＝[%d]\n",
                                                    "\nPalette setting = [%d] Last palette number = [%d]\n") VT_RST,
                               sTimeBasedLightConfigs[envCtx->changeLightNextConfig][i].nextLightSetting,
                               envCtx->numLightSettings - 1);
                    }
#endif

                    break;
                }
            }
        } else {
            if (!envCtx->lightBlendEnabled) {
                for (i = 0; i < 3; i++) {
                    envCtx->lightSettings.ambientColor[i] = lightSettingsList[envCtx->lightSetting].ambientColor[i];
                    envCtx->lightSettings.light1Dir[i] = lightSettingsList[envCtx->lightSetting].light1Dir[i];
                    envCtx->lightSettings.light1Color[i] = lightSettingsList[envCtx->lightSetting].light1Color[i];
                    envCtx->lightSettings.light2Dir[i] = lightSettingsList[envCtx->lightSetting].light2Dir[i];
                    envCtx->lightSettings.light2Color[i] = lightSettingsList[envCtx->lightSetting].light2Color[i];
                    envCtx->lightSettings.fogColor[i] = lightSettingsList[envCtx->lightSetting].fogColor[i];
                }

                envCtx->lightSettings.fogNear =
                    ENV_LIGHT_SETTINGS_FOG_NEAR(lightSettingsList[envCtx->lightSetting].blendRateAndFogNear);
                envCtx->lightSettings.zFar = lightSettingsList[envCtx->lightSetting].zFar;
                envCtx->lightBlend = 1.0f;
            } else {
                blendRate =
                    ENV_LIGHT_SETTINGS_BLEND_RATE_U8(lightSettingsList[envCtx->lightSetting].blendRateAndFogNear);

                if (blendRate == 0) {
                    blendRate++;
                }

                if (envCtx->lightBlendRateOverride != LIGHT_BLENDRATE_OVERRIDE_NONE) {
                    blendRate = envCtx->lightBlendRateOverride;
                }

                if (envCtx->lightBlendOverride == LIGHT_BLEND_OVERRIDE_NONE) {
                    envCtx->lightBlend += blendRate / 255.0f;
                }

                if (envCtx->lightBlend > 1.0f) {
                    envCtx->lightBlend = 1.0f;
                }

                for (i = 0; i < 3; i++) {
                    envCtx->lightSettings.ambientColor[i] =
                        LERP(lightSettingsList[envCtx->prevLightSetting].ambientColor[i],
                             lightSettingsList[envCtx->lightSetting].ambientColor[i], envCtx->lightBlend);
                    envCtx->lightSettings.light1Dir[i] =
                        LERP16(lightSettingsList[envCtx->prevLightSetting].light1Dir[i],
                               lightSettingsList[envCtx->lightSetting].light1Dir[i], envCtx->lightBlend);
                    envCtx->lightSettings.light1Color[i] =
                        LERP(lightSettingsList[envCtx->prevLightSetting].light1Color[i],
                             lightSettingsList[envCtx->lightSetting].light1Color[i], envCtx->lightBlend);
                    envCtx->lightSettings.light2Dir[i] =
                        LERP16(lightSettingsList[envCtx->prevLightSetting].light2Dir[i],
                               lightSettingsList[envCtx->lightSetting].light2Dir[i], envCtx->lightBlend);
                    envCtx->lightSettings.light2Color[i] =
                        LERP(lightSettingsList[envCtx->prevLightSetting].light2Color[i],
                             lightSettingsList[envCtx->lightSetting].light2Color[i], envCtx->lightBlend);
                    envCtx->lightSettings.fogColor[i] =
                        LERP(lightSettingsList[envCtx->prevLightSetting].fogColor[i],
                             lightSettingsList[envCtx->lightSetting].fogColor[i], envCtx->lightBlend);
                }

                envCtx->lightSettings.fogNear =
                    LERP16(ENV_LIGHT_SETTINGS_FOG_NEAR(lightSettingsList[envCtx->prevLightSetting].blendRateAndFogNear),
                           ENV_LIGHT_SETTINGS_FOG_NEAR(lightSettingsList[envCtx->lightSetting].blendRateAndFogNear),
                           envCtx->lightBlend);
                envCtx->lightSettings.zFar = LERP16(lightSettingsList[envCtx->prevLightSetting].zFar,
                                                    lightSettingsList[envCtx->lightSetting].zFar, envCtx->lightBlend);
            }

#if DEBUG_FEATURES
            if (envCtx->lightSetting >= envCtx->numLightSettings) {
                PRINTF("\n" VT_FGCOL(RED)
                           T("カラーパレットがおかしいようです！", "The color palette seems to be wrong!"));

                PRINTF("\n" VT_FGCOL(YELLOW) T("設定パレット＝[%d] パレット数＝[%d]\n",
                                               "Palette setting = [%d] Last palette number = [%d]\n") VT_RST,
                       envCtx->lightSetting, envCtx->numLightSettings);
            }
#endif
        }

        envCtx->lightBlendEnabled = true;

        // Apply lighting adjustments
        for (i = 0; i < 3; i++) {
            if ((s16)(envCtx->lightSettings.ambientColor[i] + envCtx->adjAmbientColor[i]) > 255) {
                lightCtx->ambientColor[i] = 255;
            } else if ((s16)(envCtx->lightSettings.ambientColor[i] + envCtx->adjAmbientColor[i]) < 0) {
                lightCtx->ambientColor[i] = 0;
            } else {
                lightCtx->ambientColor[i] = (s16)(envCtx->lightSettings.ambientColor[i] + envCtx->adjAmbientColor[i]);
            }

            if ((s16)(envCtx->lightSettings.light1Color[i] + envCtx->adjLight1Color[i]) > 255) {
                envCtx->dirLight1.params.dir.color[i] = 255;
            } else if ((s16)(envCtx->lightSettings.light1Color[i] + envCtx->adjLight1Color[i]) < 0) {
                envCtx->dirLight1.params.dir.color[i] = 0;
            } else {
                envCtx->dirLight1.params.dir.color[i] =
                    (s16)(envCtx->lightSettings.light1Color[i] + envCtx->adjLight1Color[i]);
            }

            if ((s16)(envCtx->lightSettings.light2Color[i] + envCtx->adjLight1Color[i]) > 255) {
                envCtx->dirLight2.params.dir.color[i] = 255;
            } else if ((s16)(envCtx->lightSettings.light2Color[i] + envCtx->adjLight1Color[i]) < 0) {
                envCtx->dirLight2.params.dir.color[i] = 0;
            } else {
                envCtx->dirLight2.params.dir.color[i] =
                    (s16)(envCtx->lightSettings.light2Color[i] + envCtx->adjLight1Color[i]);
            }

            if ((s16)(envCtx->lightSettings.fogColor[i] + envCtx->adjFogColor[i]) > 255) {
                lightCtx->fogColor[i] = 255;
            } else if ((s16)(envCtx->lightSettings.fogColor[i] + envCtx->adjFogColor[i]) < 0) {
                lightCtx->fogColor[i] = 0;
            } else {
                lightCtx->fogColor[i] = (s16)(envCtx->lightSettings.fogColor[i] + envCtx->adjFogColor[i]);
            }
        }

        // Set both directional light directions
        envCtx->dirLight1.params.dir.x = envCtx->lightSettings.light1Dir[0];
        envCtx->dirLight1.params.dir.y = envCtx->lightSettings.light1Dir[1];
        envCtx->dirLight1.params.dir.z = envCtx->lightSettings.light1Dir[2];

        envCtx->dirLight2.params.dir.x = envCtx->lightSettings.light2Dir[0];
        envCtx->dirLight2.params.dir.y = envCtx->lightSettings.light2Dir[1];
        envCtx->dirLight2.params.dir.z = envCtx->lightSettings.light2Dir[2];

        // Adjust fog near and far if necessary

        if ((envCtx->lightSettings.fogNear + envCtx->adjFogNear) <= ENV_FOGNEAR_MAX) {
            lightCtx->fogNear = envCtx->lightSettings.fogNear + envCtx->adjFogNear;
        } else {
            lightCtx->fogNear = ENV_FOGNEAR_MAX;
        }

        if ((envCtx->lightSettings.zFar + envCtx->adjZFar) <= ENV_ZFAR_MAX) {
            lightCtx->zFar = envCtx->lightSettings.zFar + envCtx->adjZFar;
        } else {
            lightCtx->zFar = ENV_ZFAR_MAX;
        }

#if DEBUG_FEATURES
        // When environment debug is enabled, various environment related variables can be configured via the reg editor
        if (R_ENV_DISABLE_DBG) {
            R_ENV_AMBIENT_COLOR(0) = lightCtx->ambientColor[0];
            R_ENV_AMBIENT_COLOR(1) = lightCtx->ambientColor[1];
            R_ENV_AMBIENT_COLOR(2) = lightCtx->ambientColor[2];

            R_ENV_LIGHT1_COLOR(0) = envCtx->dirLight1.params.dir.color[0];
            R_ENV_LIGHT1_COLOR(1) = envCtx->dirLight1.params.dir.color[1];
            R_ENV_LIGHT1_COLOR(2) = envCtx->dirLight1.params.dir.color[2];

            R_ENV_LIGHT2_COLOR(0) = envCtx->dirLight2.params.dir.color[0];
            R_ENV_LIGHT2_COLOR(1) = envCtx->dirLight2.params.dir.color[1];
            R_ENV_LIGHT2_COLOR(2) = envCtx->dirLight2.params.dir.color[2];

            R_ENV_FOG_COLOR(0) = lightCtx->fogColor[0];
            R_ENV_FOG_COLOR(1) = lightCtx->fogColor[1];
            R_ENV_FOG_COLOR(2) = lightCtx->fogColor[2];

            R_ENV_Z_FAR = lightCtx->zFar;
            R_ENV_FOG_NEAR = lightCtx->fogNear;

            R_ENV_LIGHT1_DIR(0) = envCtx->dirLight1.params.dir.x;
            R_ENV_LIGHT1_DIR(1) = envCtx->dirLight1.params.dir.y;
            R_ENV_LIGHT1_DIR(2) = envCtx->dirLight1.params.dir.z;

            R_ENV_LIGHT2_DIR(0) = envCtx->dirLight2.params.dir.x;
            R_ENV_LIGHT2_DIR(1) = envCtx->dirLight2.params.dir.y;
            R_ENV_LIGHT2_DIR(2) = envCtx->dirLight2.params.dir.z;

            R_ENV_WIND_DIR(0) = envCtx->windDirection.x;
            R_ENV_WIND_DIR(1) = envCtx->windDirection.y;
            R_ENV_WIND_DIR(2) = envCtx->windDirection.z;
            R_ENV_WIND_SPEED = envCtx->windSpeed;
        } else {
            lightCtx->ambientColor[0] = R_ENV_AMBIENT_COLOR(0);
            lightCtx->ambientColor[1] = R_ENV_AMBIENT_COLOR(1);
            lightCtx->ambientColor[2] = R_ENV_AMBIENT_COLOR(2);

            envCtx->dirLight1.params.dir.color[0] = R_ENV_LIGHT1_COLOR(0);
            envCtx->dirLight1.params.dir.color[1] = R_ENV_LIGHT1_COLOR(1);
            envCtx->dirLight1.params.dir.color[2] = R_ENV_LIGHT1_COLOR(2);

            envCtx->dirLight2.params.dir.color[0] = R_ENV_LIGHT2_COLOR(0);
            envCtx->dirLight2.params.dir.color[1] = R_ENV_LIGHT2_COLOR(1);
            envCtx->dirLight2.params.dir.color[2] = R_ENV_LIGHT2_COLOR(2);
            lightCtx->fogColor[0] = R_ENV_FOG_COLOR(0);
            lightCtx->fogColor[1] = R_ENV_FOG_COLOR(1);
            lightCtx->fogColor[2] = R_ENV_FOG_COLOR(2);
            lightCtx->fogNear = R_ENV_FOG_NEAR;
            lightCtx->zFar = R_ENV_Z_FAR;

            if (cREG(14)) {
                R_ENV_LIGHT1_DIR(0) = Math_CosS(cREG(10)) * Math_CosS(cREG(11)) * 120.0f;
                envCtx->dirLight1.params.dir.x = R_ENV_LIGHT1_DIR(0);
                R_ENV_LIGHT1_DIR(1) = Math_SinS(cREG(10)) * Math_CosS(cREG(11)) * 120.0f;
                envCtx->dirLight1.params.dir.y = R_ENV_LIGHT1_DIR(1);
                R_ENV_LIGHT1_DIR(2) = Math_SinS(cREG(11)) * 120.0f;
                envCtx->dirLight1.params.dir.z = R_ENV_LIGHT1_DIR(2);

                R_ENV_LIGHT2_DIR(0) = Math_CosS(cREG(12)) * Math_CosS(cREG(13)) * 120.0f;
                envCtx->dirLight2.params.dir.x = R_ENV_LIGHT2_DIR(0);
                R_ENV_LIGHT2_DIR(1) = Math_SinS(cREG(12)) * Math_CosS(cREG(13)) * 120.0f;
                envCtx->dirLight2.params.dir.y = R_ENV_LIGHT2_DIR(1);
                R_ENV_LIGHT2_DIR(2) = Math_SinS(cREG(13)) * 120.0f;
                envCtx->dirLight2.params.dir.z = R_ENV_LIGHT2_DIR(2);
            } else {
                envCtx->dirLight1.params.dir.x = R_ENV_LIGHT1_DIR(0);
                envCtx->dirLight1.params.dir.y = R_ENV_LIGHT1_DIR(1);
                envCtx->dirLight1.params.dir.z = R_ENV_LIGHT1_DIR(2);

                envCtx->dirLight2.params.dir.x = R_ENV_LIGHT2_DIR(0);
                envCtx->dirLight2.params.dir.y = R_ENV_LIGHT2_DIR(1);
                envCtx->dirLight2.params.dir.z = R_ENV_LIGHT2_DIR(2);
            }

            envCtx->windDirection.x = R_ENV_WIND_DIR(0);
            envCtx->windDirection.y = R_ENV_WIND_DIR(1);
            envCtx->windDirection.z = R_ENV_WIND_DIR(2);
            envCtx->windSpeed = R_ENV_WIND_SPEED;
        }
#endif

        if ((envCtx->dirLight1.params.dir.x == 0) && (envCtx->dirLight1.params.dir.y == 0) &&
            (envCtx->dirLight1.params.dir.z == 0)) {
            envCtx->dirLight1.params.dir.x = 1;
        }

        if ((envCtx->dirLight2.params.dir.x == 0) && (envCtx->dirLight2.params.dir.y == 0) &&
            (envCtx->dirLight2.params.dir.z == 0)) {
            envCtx->dirLight2.params.dir.x = 1;
        }
    }
}

void Environment_SetupSkyboxStars(PlayState* play) {
    f32 phi_f0;

    if ((play->envCtx.changeSkyboxNextConfig == 0 || (play->envCtx.skyboxConfig == 0 && play->envCtx.changeSkyboxNextConfig == 0)) && (play->skyboxId == SKYBOX_NORMAL_SKY)) {
        if ((gSaveContext.save.dayTime >= CLOCK_TIME(20, 30)) || (gSaveContext.save.dayTime < CLOCK_TIME(3, 0))) {
            phi_f0 = 1.0f;
        } else if (gSaveContext.save.dayTime > CLOCK_TIME(18, 30)) {
            phi_f0 = 1.0f - ((CLOCK_TIME(20, 30) - gSaveContext.save.dayTime) * (1.0f / (CLOCK_TIME(2, 0) + 1)));
        } else if (gSaveContext.save.dayTime < CLOCK_TIME(4, 0)) {
            phi_f0 = (CLOCK_TIME(4, 0) - gSaveContext.save.dayTime) * (1.0f / (CLOCK_TIME(1, 0) + 1));
        } else {
            phi_f0 = 0.0f;
        }

        sStarAlpha = phi_f0;
        sEnvSkyboxNumStars = gSkyboxNumStars;
    } else {
        sStarAlpha = 0.0f;
        sEnvSkyboxNumStars = 0;
    }

    if ((sEnvSkyboxNumStars != 0) && (sStarAlpha != 0.0f)) {
        OPEN_DISPS(play->state.gfxCtx, __FILE__, __LINE__);

        sSkyboxStarsDList = POLY_OPA_DISP;

        gSPNoOp(POLY_OPA_DISP++);

        CLOSE_DISPS(play->state.gfxCtx, __FILE__, __LINE__);
    } else {
        sSkyboxStarsDList = NULL;
    }
}

void Environment_DrawSkyboxStar(Gfx** gfxP, f32 x, f32 y, s32 width, s32 height) {
    Gfx* gfx = *gfxP;
    u32 xl = x * 4.0f;
    u32 yl = y * 4.0f;
    u32 xd = width;
    u32 yd = height;

    gSPTextureRectangle(gfx++, xl, yl, xl + xd, yl + yd, 0, 0, 0, 0, 0);

    *gfxP = gfx;
}

void Environment_DrawSkyboxStarsImpl(PlayState* play, Gfx** gfxP) {
    static const Vec3s D_801DD880[] = {
        { 0x0384, 0x2328, 0xD508 }, { 0x09C4, 0x2328, 0xDA1C }, { 0x0E74, 0x22D8, 0xDA1C }, { 0x1450, 0x2468, 0xD8F0 },
        { 0x1C84, 0x28A0, 0xCBA8 }, { 0x1F40, 0x2134, 0xD8F0 }, { 0x1F40, 0x28A0, 0xDAE4 }, { 0xE4A8, 0x4A38, 0x4A38 },
        { 0xD058, 0x4C2C, 0x3A98 }, { 0xD8F0, 0x36B0, 0x47E0 }, { 0xD954, 0x3264, 0x3E1C }, { 0xD8F0, 0x3070, 0x37DC },
        { 0xD8F0, 0x1F40, 0x5208 }, { 0xD760, 0x1838, 0x27D8 }, { 0x0000, 0x4E20, 0x4A38 }, { 0x076C, 0x2328, 0xDCD8 },
    };
    static const Color_RGBA8_u32 D_801DD8E0[] = {
        { 65, 164, 255, 255 },  { 131, 164, 230, 255 }, { 98, 205, 255, 255 }, { 82, 82, 255, 255 },
        { 123, 164, 164, 255 }, { 98, 205, 255, 255 },  { 98, 164, 230, 255 }, { 255, 90, 0, 255 },
    };
    static const Color_RGBA8_u32 D_801DD900[] = {
        { 64, 80, 112, 255 },   { 96, 96, 128, 255 },   { 128, 112, 144, 255 }, { 160, 128, 160, 255 },
        { 192, 144, 168, 255 }, { 224, 160, 176, 255 }, { 224, 160, 176, 255 }, { 104, 104, 136, 255 },
        { 136, 120, 152, 255 }, { 168, 136, 168, 255 }, { 200, 152, 184, 255 }, { 232, 168, 184, 255 },
        { 224, 176, 184, 255 }, { 240, 192, 192, 255 }, { 232, 184, 192, 255 }, { 248, 200, 192, 255 },
    };
    Vec3f pos;
    f32 temp;
    f32 imgY;
    f32 imgX;
    Gfx* gfx;
    s32 phi_v1;
    s32 negateY;
    f32 invScale;
    f32 temp_f20;
    Gfx* gfxTemp;
    f32 scale;
    s32 i;
    u32 randInt;
    u32 imgWidth;
    f32* imgXPtr;
    f32* imgYPtr;
    Vec3f* posPtr;
    s32 pad[2];
    f32(*viewProjectionMtxF)[4];

    gfx = *gfxP;

    Matrix_MtxToMtxF(play->view.viewingPtr, &play->billboardMtxF);
    Matrix_MtxToMtxF(&play->view.projection, &play->viewProjectionMtxF);
    SkinMatrix_MtxFMtxFMult(&play->viewProjectionMtxF, &play->billboardMtxF, &play->viewProjectionMtxF);

    phi_v1 = 0;

    gDPPipeSync(gfx++);
    gDPSetEnvColor(gfx++, 255, 255, 255, 255.0f * sStarAlpha);
    gDPSetCombineLERP(gfx++, PRIMITIVE, 0, ENVIRONMENT, 0, PRIMITIVE, 0, ENVIRONMENT, 0, PRIMITIVE, 0, ENVIRONMENT, 0,
                      PRIMITIVE, 0, ENVIRONMENT, 0);
    gDPSetOtherMode(gfx++,
                    G_AD_DISABLE | G_CD_DISABLE | G_CK_NONE | G_TC_FILT | G_TF_POINT | G_TT_NONE | G_TL_TILE |
                        G_TD_CLAMP | G_TP_NONE | G_CYC_1CYCLE | G_PM_NPRIMITIVE,
                    G_AC_NONE | G_ZS_PRIM | G_RM_AA_XLU_LINE | G_RM_AA_XLU_LINE2);

    randInt = ((u32)gSaveContext.save.info.playerData.playerName[0] << 0x18) ^
              ((u32)gSaveContext.save.info.playerData.playerName[1] << 0x14) ^
              ((u32)gSaveContext.save.info.playerData.playerName[2] << 0x10) ^
              ((u32)gSaveContext.save.info.playerData.playerName[3] << 0xC) ^
              ((u32)gSaveContext.save.info.playerData.playerName[4] << 8) ^
              ((u32)gSaveContext.save.info.playerData.playerName[5] << 4) ^
              ((u32)gSaveContext.save.info.playerData.playerName[6] << 0) ^
              ((u32)gSaveContext.save.info.playerData.playerName[7] >> 4) ^
              ((u32)gSaveContext.save.info.playerData.playerName[7] << 0x1C);

    for (i = 0; i < sEnvSkyboxNumStars; i++) {
        if (i < 16) {
            pos.x = play->view.eye.x + (s32)D_801DD880[i].x;
            pos.y = play->view.eye.y + (s32)D_801DD880[i].y;
            pos.z = play->view.eye.z + (s32)D_801DD880[i].z;
            imgWidth = 8;
        } else {
            f32 temp_f22;
            // f32 temp_f4;
            f32 temp_f2;

            temp_f20 = Rand_ZeroOne_Variable(&randInt);

            Rand_Next_Variable(&randInt);

            // Set random position
            pos.y = play->view.eye.y + (SQ(temp_f20) * SQ(128.0f)) - 1000.0f;
            pos.x = play->view.eye.x + (Math_SinS(randInt) * (1.2f - temp_f20) * SQ(128.0f));
            pos.z = play->view.eye.z + (Math_CosS(randInt) * (1.2f - temp_f20) * SQ(128.0f));

            temp_f2 = Rand_ZeroOne_Variable(&randInt);

            // Set random width
            imgWidth = (u32)((SQ(temp_f2) * 8.0f) + 2.0f);
        }

        if ((i < 15) || ((i == 15) && ((((void)0, gSaveContext.save.totalDays) % 7) == 0))) {
            gDPSetColor(gfx++, G_SETPRIMCOLOR, D_801DD8E0[i % ARRAY_COUNTU(D_801DD8E0)].rgba);
        } else if (((i & 0x3F) == 0) || (i == 16)) {
            gDPSetColor(gfx++, G_SETPRIMCOLOR, D_801DD900[phi_v1 % ARRAY_COUNTU(D_801DD900)].rgba);
            phi_v1++;
        }

        // posPtr = &pos;
        // imgXPtr = &imgX;
        // imgYPtr = &imgY;
        viewProjectionMtxF = play->viewProjectionMtxF.mf;

        if (imgWidth >= 2) {
            // w component
            scale = pos.x * play->viewProjectionMtxF.mf[0][3] + pos.y * play->viewProjectionMtxF.mf[1][3] +
                    pos.z * play->viewProjectionMtxF.mf[2][3] + play->viewProjectionMtxF.mf[3][3];
            if (scale >= 1.0f) {
                invScale = 1.0f / scale;
                // x component
                imgX = (pos.x * viewProjectionMtxF[0][0] + pos.y * viewProjectionMtxF[1][0] +
                        pos.z * viewProjectionMtxF[2][0] + viewProjectionMtxF[3][0]) *
                       invScale;
                // y component
                imgY = (((pos.x * viewProjectionMtxF[0][1]) + (pos.y * viewProjectionMtxF[1][1]) +
                         (pos.z * viewProjectionMtxF[2][1])) +
                        viewProjectionMtxF[3][1]) *
                       invScale;
            }

            if ((scale >= 1.0f) && (imgX > -1.0f) && (imgX < 1.0f) && (imgY > -1.0f) && (imgY < 1.0f)) {
                imgX = (imgX * (SCREEN_WIDTH / 2)) + (SCREEN_WIDTH / 2);
                imgY = (imgY * -(SCREEN_HEIGHT / 2)) + (SCREEN_HEIGHT / 2);

                gfxTemp = gfx;
                Environment_DrawSkyboxStar(&gfxTemp, imgX, imgY, imgWidth, 4);
                gfx = gfxTemp;
            }
        }
    }

    gDPPipeSync(gfx++);
    *gfxP = gfx;
}

void Environment_DrawSkybox(PlayState* play) {
    Skybox_DrawNew(&play->skyboxCtx, play->state.gfxCtx, &play->lightCtx, play->skyboxId, play->envCtx.skyboxBlend,
                    play->view.eye.x, play->view.eye.y, play->view.eye.z);

    OPEN_DISPS(play->state.gfxCtx, __FILE__, __LINE__);

    gSPSegment(POLY_XLU_DISP++, 0x7, play->skyboxCtx.skyboxStaticSegment); // setting the correct segment for xlu

    Environment_DrawCloudStorm(play);

    static u8 fogIntensity;
    if (play->envCtx.changeSkyboxNextConfig != 0 || (play->envCtx.skyboxConfig != 0 && play->envCtx.changeSkyboxNextConfig != 0)) { // storm condition
        if (play->lightCtx.fogNear < 980) {
            fogIntensity = CLAMP_MAX(fogIntensity + 35, 255);
        } else {
            fogIntensity = CLAMP_MAX(fogIntensity + 35, 200);
        }
    } else {
        if (play->lightCtx.fogNear < 980) {
            fogIntensity = CLAMP_MAX(fogIntensity + 35, 255);
        } else {
            fogIntensity = CLAMP_MIN(fogIntensity - 35, 0);
        }
    }

    POLY_XLU_DISP = Gfx_SetFog(POLY_XLU_DISP, play->lightCtx.fogColor[0], play->lightCtx.fogColor[1],
                               play->lightCtx.fogColor[2], fogIntensity, play->lightCtx.fogNear, 1000);
    
    Environment_DrawCloudHorizon(play);
    Environment_DrawClouds(play);

    POLY_XLU_DISP = Play_SetFog(play, POLY_XLU_DISP);

    CLOSE_DISPS(play->state.gfxCtx, __FILE__, __LINE__);
}

void Environment_DrawSkyboxStars(PlayState* play) {
    Gfx* gfx;
    Gfx* gfxHead;

    if (sSkyboxStarsDList != NULL) {
        OPEN_DISPS(play->state.gfxCtx, __FILE__, __LINE__);

        gfxHead = POLY_OPA_DISP;
        gfx = Gfx_Open(gfxHead);

        gSPDisplayList(sSkyboxStarsDList, gfx);

        Environment_DrawSkyboxStarsImpl(play, &gfx);

        gSPEndDisplayList(gfx++);

        Gfx_Close(gfxHead, gfx);

        POLY_OPA_DISP = gfx;
        sSkyboxStarsDList = NULL;

        CLOSE_DISPS(play->state.gfxCtx, __FILE__, __LINE__);
    }
}

void Environment_ResetCloud(PlayState* play, u8 i) {
    play->envCtx.clouds[i].scale = Rand_ZeroOne();

    if (i < 8) {
        play->envCtx.clouds[i].scale = Rand_ZeroFloat(0.5f) + 0.1f;
        play->envCtx.clouds[i].targetPitch = (u8)Rand_S16Offset(5, 20); // max 25
    } else {
        play->envCtx.clouds[i].scale = Rand_ZeroFloat(0.5f) + 0.5f;
        play->envCtx.clouds[i].targetPitch = (u8)Rand_S16Offset(20, 50); // max 70
    }

    play->envCtx.clouds[i].rot.y = DEG_TO_RAD((((70 - play->envCtx.clouds[i].targetPitch) * 35) / 70)/*  + 5 */);

    if (Rand_ZeroOne() < 0.5f) {
        play->envCtx.clouds[i].rot.y *= -1;
    }

    play->envCtx.clouds[i].texId = (u8)Rand_S16Offset(0, 3); // 0-2
}

void Environment_InitClouds(PlayState* play) {
    u8 i;

    for (i = 0; i < ARRAY_COUNT(play->envCtx.clouds); i++) {
        play->envCtx.clouds[i].rot.x = 0;

        if (i < sCloudDensity) {
            play->envCtx.clouds[i].alpha = 200;
        } else {
            play->envCtx.clouds[i].alpha = 0;
        }

        Environment_ResetCloud(play, i);

        play->envCtx.clouds[i].rot.y = DEG_TO_RAD(Rand_S16Offset(-180, 360));
    }
}

void Environment_UpdateClouds(PlayState* play) {
    u8 i;

    for (i = 0; i < ARRAY_COUNT(play->envCtx.clouds); i++) {
        if (i < sCloudDensity) {
            if (RAD_TO_DEG(play->envCtx.clouds[i].rot.y) >= 0) {
                play->envCtx.clouds[i].rot.y += 0.0001f + (DEG_TO_RAD(play->envCtx.clouds[i].targetPitch) * 0.0005)/*  + (play->envCtx.windSpeed * 0.000005) */;
            } else {
                play->envCtx.clouds[i].rot.y -= 0.0001f + DEG_TO_RAD(play->envCtx.clouds[i].targetPitch * 0.0005)/*  + (play->envCtx.windSpeed * 0.000005) */;
            }

            play->envCtx.clouds[i].rot.z = sinf(RAD_TO_DEG(fabsf(play->envCtx.clouds[i].rot.y)) * (M_PI / 180)) * DEG_TO_RAD(play->envCtx.clouds[i].targetPitch)/*  + DEG_TO_RAD(5) */;

            // reset clouds
            if (RAD_TO_DEG(fabsf(play->envCtx.clouds[i].rot.y)) >= (180 - (((70 - play->envCtx.clouds[i].targetPitch) * 35) / 70))) {
                play->envCtx.clouds[i].alpha = CLAMP_MIN(play->envCtx.clouds[i].alpha - 2, 0);
                if (play->envCtx.clouds[i].alpha <= 0) {
                    Environment_ResetCloud(play, i);
                }
            } else if (play->envCtx.clouds[i].alpha < 200) {
                play->envCtx.clouds[i].alpha = CLAMP_MAX(play->envCtx.clouds[i].alpha + 2, 200);
            }
        } else if (play->envCtx.clouds[i].alpha > 0) {
            play->envCtx.clouds[i].alpha = CLAMP_MIN(play->envCtx.clouds[i].alpha - 5, 0);
        }
    }
}

// storm cloud
void Environment_DrawCloudStorm(PlayState* play) {
    static u8 stormAlpha;

    if (play->envCtx.changeSkyboxNextConfig != 0 || (play->envCtx.skyboxConfig != 0 && play->envCtx.changeSkyboxNextConfig != 0)) { // storm condition
        stormAlpha = CLAMP_MAX(stormAlpha + 5, 180);
        sCloudDensity = 32;
    } else {
        sCloudDensity = 16;
        stormAlpha = CLAMP_MIN(stormAlpha - 5, 0);
    }

    if (stormAlpha <= 0) {
        return;
    }

    OPEN_DISPS(play->state.gfxCtx, __FILE__, __LINE__);

    gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, play->envCtx.dirLight1.params.dir.color[0], play->envCtx.dirLight1.params.dir.color[1], play->envCtx.dirLight1.params.dir.color[2], stormAlpha);
    gDPSetEnvColor(POLY_XLU_DISP++, play->envCtx.dirLight2.params.dir.color[0], play->envCtx.dirLight2.params.dir.color[1], play->envCtx.dirLight2.params.dir.color[2], 0);

    Matrix_Translate(play->view.eye.x, play->view.eye.y + 100, play->view.eye.z, MTXMODE_NEW);
    Matrix_Scale(1.0f, 1.0f, 1.0f, MTXMODE_APPLY);
    Matrix_RotateY(play->skyboxCtx.rot.y, MTXMODE_APPLY);

    gSPMatrix(POLY_XLU_DISP++, MATRIX_FINALIZE(play->state.gfxCtx, "../z_cheap_proc.c", 216),
              G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    gSPDisplayList(POLY_XLU_DISP++, skybox_storm_cloud);

    CLOSE_DISPS(play->state.gfxCtx, __FILE__, __LINE__);
}

// cloud ring
void Environment_DrawCloudHorizon(PlayState* play) {
    OPEN_DISPS(play->state.gfxCtx, __FILE__, __LINE__);

    gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, play->envCtx.dirLight1.params.dir.color[0], play->envCtx.dirLight1.params.dir.color[1], play->envCtx.dirLight1.params.dir.color[2], 150);
    gDPSetEnvColor(POLY_XLU_DISP++, play->envCtx.dirLight2.params.dir.color[0], play->envCtx.dirLight2.params.dir.color[1], play->envCtx.dirLight2.params.dir.color[2], 0);

    Matrix_Translate(play->view.eye.x, play->view.eye.y + 50, play->view.eye.z, MTXMODE_NEW);
    Matrix_Scale(1.0f, 1.5f, 1.0f, MTXMODE_APPLY);
    Matrix_RotateY(play->skyboxCtx.rot.y, MTXMODE_APPLY);

    gSPMatrix(POLY_XLU_DISP++, MATRIX_FINALIZE(play->state.gfxCtx, "../z_cheap_proc.c", 216),
              G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    gSPDisplayList(POLY_XLU_DISP++, skybox_cloud_horizon);

    CLOSE_DISPS(play->state.gfxCtx, __FILE__, __LINE__);
}

// single clouds
void Environment_DrawClouds(PlayState* play) {
    static void* cloudTex[] = {skybox_cloud_01_tex, skybox_cloud_02_tex, skybox_cloud_03_tex};
    u8 i;
    f32 windRot;

    windRot = Math_Atan2F(play->envCtx.windDirection.y, play->envCtx.windDirection.x) - DEG_TO_RAD(90);

    OPEN_DISPS(play->state.gfxCtx, "../z_cheap_proc.c", 214);

    gDPSetEnvColor(POLY_XLU_DISP++, play->envCtx.dirLight2.params.dir.color[0], play->envCtx.dirLight2.params.dir.color[1], play->envCtx.dirLight2.params.dir.color[2], 0);

    for (i = 0; i < ARRAY_COUNT(play->envCtx.clouds); i++) {
        f32 scale;

        gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, play->envCtx.dirLight1.params.dir.color[0], play->envCtx.dirLight1.params.dir.color[1], play->envCtx.dirLight1.params.dir.color[2], play->envCtx.clouds[i].alpha);

        Matrix_Translate(play->view.eye.x, play->view.eye.y, play->view.eye.z, MTXMODE_NEW);

        // yaw
        Matrix_RotateY(play->envCtx.clouds[i].rot.y + windRot, MTXMODE_APPLY);

        // pitch
        Matrix_RotateZ(play->envCtx.clouds[i].rot.z, MTXMODE_APPLY);
        Matrix_Translate(6000.0f, 0, 0, MTXMODE_APPLY);

        scale = play->envCtx.clouds[i].scale + ((f32)RAD_TO_DEG(play->envCtx.clouds[i].rot.z) * 0.03f);
        Matrix_Scale(scale, scale, scale, MTXMODE_APPLY);

        gSPMatrix(POLY_XLU_DISP++, MATRIX_FINALIZE(play->state.gfxCtx, "../z_cheap_proc.c", 216),
                G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
        
        gSPTexture(POLY_XLU_DISP++, 65535, 65535, 0, 0, 1);
	    gDPSetTextureImage(POLY_XLU_DISP++, G_IM_FMT_IA, G_IM_SIZ_8b_LOAD_BLOCK, 1, cloudTex[play->envCtx.clouds[i].texId]);
	    gDPSetTile(POLY_XLU_DISP++, G_IM_FMT_IA, G_IM_SIZ_8b_LOAD_BLOCK, 0, 0, 7, 0, G_TX_WRAP | G_TX_NOMIRROR, 0, 0, G_TX_WRAP | G_TX_NOMIRROR, 0, 0);
	    gDPLoadBlock(POLY_XLU_DISP++, 7, 0, 0, 1023, 256);
	    gDPSetTile(POLY_XLU_DISP++, G_IM_FMT_IA, G_IM_SIZ_8b, 8, 0, 0, 0, G_TX_CLAMP | G_TX_NOMIRROR, 5, 0, G_TX_CLAMP | G_TX_NOMIRROR, 6, 0);
	    gDPSetTileSize(POLY_XLU_DISP++, 0, 0, 0, 252, 124);
        gSPDisplayList(POLY_XLU_DISP++, skybox_cloud);
    }

    CLOSE_DISPS(play->state.gfxCtx, "../z_cheap_proc.c", 219);
}

typedef struct WeatherEvent {
    u16 startTime;
    u16 endTime;
    u8 state; // 0 sunny, 1 cloudy, 2 rain, 3 thunderstorm
} WeatherEvent;

static WeatherEvent weatherSchedule[] = {
    {0,  CLOCK_TIME(23,0) + 1,  0xFF},
    {0,  CLOCK_TIME(23,0) + 1,  0xFF},
    {0,  CLOCK_TIME(23,0) + 1,  0xFF},
    {0,  CLOCK_TIME(23,0) + 1,  0xFF}, // last entry is a marker to calculate weather again
};

void Environment_CalculateWeather(PlayState* play) {
    u16 prevEndTime = gSaveContext.save.dayTime;

    if (Rand_ZeroOne() <= 0.75f && weatherModeTest == WEATHER_EVENT_SUNNY) { // hit weather event if it is sunny, it will be at least cloudy
        if (Rand_ZeroOne() <= 0.75f) { // rain/thunder, shorter schedule
            u8 randState = (u8)Rand_S16Offset(0, 3); // 0-2

            for (u8 i = 0; i < ARRAY_COUNT(weatherSchedule) - 1; i++) {
                weatherSchedule[i].startTime = prevEndTime;
                weatherSchedule[i].endTime = prevEndTime + CLOCK_TIME(3,0); // 3/6/9
                prevEndTime = weatherSchedule[i].endTime;
                if (i == 1) {
                    weatherSchedule[i].state = WEATHER_EVENT_CLOUDY + randState;
                } else {
                    weatherSchedule[i].state = WEATHER_EVENT_CLOUDY; // cloudy
                }
            }
        } else { // cloudy day, longer schedule
            for (u8 i = 0; i < ARRAY_COUNT(weatherSchedule) - 1; i++) {
                weatherSchedule[i].startTime = prevEndTime;
                weatherSchedule[i].endTime = prevEndTime + CLOCK_TIME(6,0); // 6/12/18
                prevEndTime = weatherSchedule[i].endTime;
                weatherSchedule[i].state = WEATHER_EVENT_CLOUDY; // 1 is cloudy
            }
        }
    } else { // sunny day, longer schedule, you could add some other clear skies events here or determine the amount of clouds
        for (u8 i = 0; i < ARRAY_COUNT(weatherSchedule) - 1; i++) {
            weatherSchedule[i].startTime = prevEndTime;
            weatherSchedule[i].endTime = prevEndTime + CLOCK_TIME(6,0); // 6/12/18
            prevEndTime = weatherSchedule[i].endTime;
            weatherSchedule[i].state = WEATHER_EVENT_SUNNY; // 0 is sunny
        }
    }
    weatherSchedule[3].startTime = prevEndTime;
    weatherSchedule[3].endTime = prevEndTime + CLOCK_TIME(6,0); // for marker
}

/* 
Notes: 
- Nighttime is running faster, maybe change this
- Investigate fog glitch and fix it, does it even happen?
- limit this system to scenes which don't use prerenders
 */

void Environment_DynamicWeather(PlayState* play) {
    u8 i = 0;
    /* s16 testHour = (gSaveContext.save.dayTime * (24.0f * 60.0f / 0x10000)) / 60.0f;
    s16 testMin = (s16)(gSaveContext.save.dayTime * (24.0f * 60.0f / 0x10000)) % 60; */

    /* Debug_Print(0, "ztime:%02d:%02d", testHour, testMin);
    Debug_Print_Draw(0, play); */

    for (i = 0; i < ARRAY_COUNT(weatherSchedule); i++) {
        if (gSaveContext.skyboxTime >= weatherSchedule[i].startTime &&
            (gSaveContext.skyboxTime < weatherSchedule[i].endTime ||
            weatherSchedule[i].endTime == 0xFFFF || (weatherSchedule[i].startTime > weatherSchedule[i].endTime && gSaveContext.skyboxTime >= weatherSchedule[i].endTime))) {
            break;
        }
    }
    // marker hit, randomly calculate weather
    if (weatherSchedule[i].state == 0xFF) {
        Environment_CalculateWeather(play);
        i = 0;
    }

    if (weatherSchedule[i].state != weatherModeTest && weatherSchedule[i].state <= 3) {
        weatherModeTest = weatherSchedule[i].state;

        // weather change test, copy paste of song of storms logic, rewrite this later
        switch (weatherModeTest) {
            case WEATHER_EVENT_SUNNY:
                play->envCtx.precipitation[PRECIP_SOS_MAX] = 0;
                if (play->csCtx.state == CS_STATE_IDLE) {
                    Environment_StopStormNatureAmbience(play);
                } else if (Audio_GetActiveSeqId(SEQ_PLAYER_BGM_MAIN) == NA_BGM_NATURE_AMBIENCE) {
                    Audio_SetNatureAmbienceChannelIO(NATURE_CHANNEL_LIGHTNING, CHANNEL_IO_PORT_1, 0);
                    Audio_SetNatureAmbienceChannelIO(NATURE_CHANNEL_RAIN, CHANNEL_IO_PORT_1, 0);
                }
                if (gWeatherMode == WEATHER_MODE_CLEAR && (play->envCtx.stormRequest == STORM_REQUEST_START)) {
                    play->envCtx.stormRequest = STORM_REQUEST_STOP;
                } else {
                    play->envCtx.stormRequest = STORM_REQUEST_NONE;
                    play->envCtx.stormState = STORM_STATE_OFF;
                }
                play->envCtx.lightningState = LIGHTNING_OFF;
            break;
            case WEATHER_EVENT_CLOUDY:
                play->envCtx.precipitation[PRECIP_SOS_MAX] = 0;
                play->envCtx.stormRequest = STORM_REQUEST_START;
                if ((gWeatherMode != WEATHER_MODE_CLEAR) || play->envCtx.skyboxConfig != 0) {
                    play->envCtx.stormState = STORM_STATE_ON;
                }
                play->envCtx.lightningState = LIGHTNING_OFF;
            break;
            case WEATHER_EVENT_RAIN:
                play->envCtx.precipitation[PRECIP_SOS_MAX] = 20;
                play->envCtx.stormRequest = STORM_REQUEST_START;
                if ((gWeatherMode != WEATHER_MODE_CLEAR) || play->envCtx.skyboxConfig != 0) {
                    play->envCtx.stormState = STORM_STATE_ON;
                }
                Environment_PlayStormNatureAmbience(play);
            break;
            case WEATHER_EVENT_THUNDER:
                play->envCtx.precipitation[PRECIP_SOS_MAX] = 20;
                play->envCtx.stormRequest = STORM_REQUEST_START;
                if ((gWeatherMode != WEATHER_MODE_CLEAR) || play->envCtx.skyboxConfig != 0) {
                    play->envCtx.stormState = STORM_STATE_ON;
                }
                play->envCtx.lightningState = LIGHTNING_ON;
                Environment_PlayStormNatureAmbience(play);
            break;
        }
    }

    /* Debug_Print_Draw(3, play);

    s16 testSchedStartHour = (weatherSchedule[0].startTime * (24.0f * 60.0f / 0x10000)) / 60.0f;
    s16 testSchedStartMin = (s16)(weatherSchedule[0].startTime * (24.0f * 60.0f / 0x10000)) % 60;
    s16 testSchedEndHour = (weatherSchedule[0].endTime * (24.0f * 60.0f / 0x10000)) / 60.0f;
    s16 testSchedEndMin = (s16)(weatherSchedule[0].endTime * (24.0f * 60.0f / 0x10000)) % 60;

    Debug_Print(4, "event0:%02d:%02d, %02d:%02d, s:%d", testSchedStartHour, testSchedStartMin, testSchedEndHour, testSchedEndMin, weatherSchedule[0].state);
    Debug_Print_Draw(4, play);

    testSchedStartHour = (weatherSchedule[1].startTime * (24.0f * 60.0f / 0x10000)) / 60.0f;
    testSchedStartMin = (s16)(weatherSchedule[1].startTime * (24.0f * 60.0f / 0x10000)) % 60;
    testSchedEndHour = (weatherSchedule[1].endTime * (24.0f * 60.0f / 0x10000)) / 60.0f;
    testSchedEndMin = (s16)(weatherSchedule[1].endTime * (24.0f * 60.0f / 0x10000)) % 60;

    Debug_Print(5, "event1:%02d:%02d, %02d:%02d, s:%d", testSchedStartHour, testSchedStartMin, testSchedEndHour, testSchedEndMin, weatherSchedule[1].state);
    Debug_Print_Draw(5, play);

    testSchedStartHour = (weatherSchedule[2].startTime * (24.0f * 60.0f / 0x10000)) / 60.0f;
    testSchedStartMin = (s16)(weatherSchedule[2].startTime * (24.0f * 60.0f / 0x10000)) % 60;
    testSchedEndHour = (weatherSchedule[2].endTime * (24.0f * 60.0f / 0x10000)) / 60.0f;
    testSchedEndMin = (s16)(weatherSchedule[2].endTime * (24.0f * 60.0f / 0x10000)) % 60;

    Debug_Print(6, "event2:%02d:%02d, %02d:%02d, s:%d", testSchedStartHour, testSchedStartMin, testSchedEndHour, testSchedEndMin, weatherSchedule[2].state);
    Debug_Print_Draw(6, play);

    testSchedStartHour = (weatherSchedule[3].startTime * (24.0f * 60.0f / 0x10000)) / 60.0f;
    testSchedStartMin = (s16)(weatherSchedule[3].startTime * (24.0f * 60.0f / 0x10000)) % 60;
    testSchedEndHour = (weatherSchedule[3].endTime * (24.0f * 60.0f / 0x10000)) / 60.0f;
    testSchedEndMin = (s16)(weatherSchedule[3].endTime * (24.0f * 60.0f / 0x10000)) % 60;

    Debug_Print(7, "event3:%02d:%02d, %02d:%02d, s:%d", testSchedStartHour, testSchedStartMin, testSchedEndHour, testSchedEndMin, weatherSchedule[3].state);
    Debug_Print_Draw(7, play); */
}

void Environment_DrawSunAndMoon(PlayState* play) {
    // new moon vertices
    static Vtx moonVtx[] = {
        VTX(   -16,    -16,      0,     0x0,     0x0, 0xFF, 0xFF, 0xFF, 0xFF),
        VTX(    16,    -16,      0,  0x800,     0x0, 0xFF, 0xFF, 0xFF, 0xFF),
        VTX(   -16,     16,      0,     0x0,  0x800, 0xFF, 0xFF, 0xFF, 0xFF),
        VTX(    16,     16,      0,  0x800,  0x800, 0xFF, 0xFF, 0xFF, 0xFF),
    };

    // This replace gMoonDL in gameplay_keep. TODO make this gMoonDL once asset replacement is sophisticated enough
    static Gfx sMoonDL[] = {
        gsSPMatrix(0x01000000, G_MTX_NOPUSH | G_MTX_MUL | G_MTX_MODELVIEW),
        gsSPLoadGeometryMode(G_CULL_BACK),
        gsDPSetCombineLERP(PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, PRIMITIVE, 0, TEXEL0, 0, 0, 0, 0, COMBINED, 0,
                           0, 0, COMBINED),
        gsDPSetOtherMode(G_AD_NOTPATTERN | G_CD_MAGICSQ | G_CK_NONE | G_TC_FILT | G_TF_BILERP | G_TT_NONE | G_TL_TILE |
                             G_TD_CLAMP | G_TP_PERSP | G_CYC_2CYCLE | G_PM_NPRIMITIVE,
                         G_AC_THRESHOLD | G_ZS_PIXEL | G_RM_FOG_PRIM_A | G_RM_XLU_SURF2),
        gsSPVertex(&moonVtx[0], 4, 0),
        gsSP2Triangles(0, 1, 2, 0, 1, 3, 2, 0),
        gsSPEndDisplayList(),
    };

    s32 alpha;
    f32 color;
    f32 y;
    f32 scale;
    f32 temp;

    /* Debug_Print(8, "wmode:%d", gWeatherMode);
    Debug_Print_Draw(8, play); */

    OPEN_DISPS(play->state.gfxCtx, "../z_kankyo.c", 2266);

    if (play->csCtx.state != CS_STATE_IDLE) {
        Math_SmoothStepToF(&play->envCtx.sunPos.x,
                           -(Math_SinS(((void)0, gSaveContext.save.dayTime) - CLOCK_TIME(12, 0)) * 120.0f) * 25.0f,
                           1.0f, 0.8f, 0.8f);
        Math_SmoothStepToF(&play->envCtx.sunPos.y,
                           (Math_CosS(((void)0, gSaveContext.save.dayTime) - CLOCK_TIME(12, 0)) * 120.0f) * 25.0f, 1.0f,
                           0.8f, 0.8f);
        Math_SmoothStepToF(&play->envCtx.sunPos.z,
                           (Math_CosS(((void)0, gSaveContext.save.dayTime) - CLOCK_TIME(12, 0)) * 20.0f) * 25.0f, 1.0f,
                           0.8f, 0.8f);
    } else {
        play->envCtx.sunPos.x = -(Math_SinS(((void)0, gSaveContext.save.dayTime) - CLOCK_TIME(12, 0)) * 120.0f) * 25.0f;
        play->envCtx.sunPos.y = +(Math_CosS(((void)0, gSaveContext.save.dayTime) - CLOCK_TIME(12, 0)) * 120.0f) * 25.0f;
        play->envCtx.sunPos.z = +(Math_CosS(((void)0, gSaveContext.save.dayTime) - CLOCK_TIME(12, 0)) * 20.0f) * 25.0f;
    }

    if (gSaveContext.save.entranceIndex != ENTR_HYRULE_FIELD_0 || ((void)0, gSaveContext.sceneLayer) != 5) {
        Matrix_Translate(play->view.eye.x + play->envCtx.sunPos.x, play->view.eye.y + play->envCtx.sunPos.y,
                         play->view.eye.z + play->envCtx.sunPos.z, MTXMODE_NEW);

        y = play->envCtx.sunPos.y / 25.0f;
        temp = y / 80.0f;

        alpha = temp * 255.0f;
        alpha = CLAMP(alpha, 0, 255);
        alpha = 255 - alpha;

        color = temp;
        if (color < 0.0f) {
            color = 0.0f;
        }

        if (color > 1.0f) {
            color = 1.0f;
        }

        Gfx_SetupDL_54Opa(play->state.gfxCtx);
        gDPSetRenderMode(POLY_OPA_DISP++, G_RM_FOG_PRIM_A, G_RM_XLU_SURF2);

        // draw sun only above a certain height
        if (play->envCtx.sunPos.y > -500.0f) {
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, (u8)(color * 75.0f) + 180, (u8)(color * 155.0f) + 100, 255);
            gDPSetEnvColor(POLY_OPA_DISP++, 255, (u8)(color * 255.0f), (u8)(color * 255.0f), alpha);

            scale = (color * 2.0f) + 10.0f;
            Matrix_Scale(scale, scale, scale, MTXMODE_APPLY);
            MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx, "../z_kankyo.c", 2364);
            gSPDisplayList(POLY_OPA_DISP++, gSunDL);
        }

        Matrix_Translate(play->view.eye.x - play->envCtx.sunPos.x + 300.0f, play->view.eye.y - play->envCtx.sunPos.y - 380.0f,
                         play->view.eye.z - play->envCtx.sunPos.z + 300.0f, MTXMODE_NEW);

        color = -y / 120.0f;
        color = CLAMP_MIN(color, 0.0f);

        scale = -15.0f * color + 25.0f;
        Matrix_Scale(scale, scale, scale, MTXMODE_APPLY);

        temp = -y / 80.0f;
        temp = CLAMP_MAX(temp, 1.0f);

        alpha = temp * 255.0f;

        if (alpha > 0 && (gSaveContext.save.totalDays % 8) != 4) {
            static void* moonTexs[] = {gMoonTex, gMoonPhase01Tex, gMoonPhase02Tex, gMoonPhase03Tex, NULL, gMoonPhase03Tex, gMoonPhase02Tex, gMoonPhase01Tex};
            u8 moonPhase = gSaveContext.save.totalDays % 8;
            // moonPhase = 5;

            /* Debug_Print(0, "%.3f", play->envCtx.sunPos.y);
            Debug_Print_Draw(0, play); */

            //if moon is at a certain y height, start to lerp between skybox colors, y 1400 - y 900

            gSPMatrix(POLY_OPA_DISP++, MATRIX_FINALIZE(play->state.gfxCtx, "../z_kankyo.c", 2406), G_MTX_LOAD);
            gDPPipeSync(POLY_OPA_DISP++);
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 240, 255, 180, alpha);
            gDPSetEnvColor(POLY_OPA_DISP++, play->skyboxCtx.skyboxTopColor[0], play->skyboxCtx.skyboxTopColor[1], play->skyboxCtx.skyboxTopColor[2], alpha);
            gSPSegment(POLY_OPA_DISP++, 0x08, SEGMENTED_TO_VIRTUAL(moonTexs[moonPhase]));

            s16 moonUls = ((gSaveContext.save.totalDays % 8) > 4) ? 255 : 0;
            // moonUls = 255;

            gSPTexture(POLY_OPA_DISP++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
            gDPLoadTextureBlock(POLY_OPA_DISP++, 0x08000000, G_IM_FMT_IA, G_IM_SIZ_8b, 64, 64, 0, G_TX_MIRROR | G_TX_WRAP,
                             G_TX_MIRROR | G_TX_WRAP, 6, 6, G_TX_NOLOD, G_TX_NOLOD);
            gDPSetTileSize(POLY_OPA_DISP++, G_TX_RENDERTILE, moonUls, moonUls,
                           ((64)  - 1) << G_TEXTURE_IMAGE_FRAC,
                           ((64) - 1) << G_TEXTURE_IMAGE_FRAC);
            gSPDisplayList(POLY_OPA_DISP++, sMoonDL);
        }
    }

    CLOSE_DISPS(play->state.gfxCtx, "../z_kankyo.c", 2429);
}

void Environment_DrawSunLensFlare(PlayState* play, EnvironmentContext* envCtx, View* view, GraphicsContext* gfxCtx,
                                  Vec3f pos, s32 unused) {
    if ((play->envCtx.precipitation[PRECIP_RAIN_CUR] == 0) && (play->envCtx.skyboxConfig == 0)) {
        Environment_DrawLensFlare(play, &play->envCtx, &play->view, play->state.gfxCtx, pos, 2000, 370,
                                  Math_CosS(((void)0, gSaveContext.save.dayTime) - CLOCK_TIME(12, 0)) * 120.0f, 400,
                                  true);
    }
}

f32 sLensFlareScales[] = { 23.0f, 12.0f, 7.0f, 5.0f, 3.0f, 10.0f, 6.0f, 2.0f, 3.0f, 1.0f };

typedef enum LensFlareType {
    /* 0 */ LENS_FLARE_CIRCLE0,
    /* 1 */ LENS_FLARE_CIRCLE1,
    /* 2 */ LENS_FLARE_RING
} LensFlareType;

void Environment_DrawLensFlare(PlayState* play, EnvironmentContext* envCtx, View* view, GraphicsContext* gfxCtx,
                               Vec3f pos, s32 unused, s16 scale, f32 colorIntensity, s16 glareStrength, u8 isSun) {
    s16 i;
    f32 tempX;
    f32 tempY;
    f32 tempZ;
    f32 lookDirX;
    f32 lookDirY;
    f32 lookDirZ;
    f32 tempX2;
    f32 tempY2;
    f32 tempZ2;
    f32 posDirX;
    f32 posDirY;
    f32 posDirZ;
    f32 length;
    f32 dist;
    f32 halfPosX;
    f32 halfPosY;
    f32 halfPosZ;
    f32 cosAngle;
    s32 pad;
    f32 lensFlareAlphaScaleTarget;
    u32 isOffScreen = false;
    f32 alpha;
    f32 adjScale;
    Vec3f screenPos;
    f32 fogInfluence;
    f32 temp;
    f32 glareAlphaScale;
    Color_RGB8 lensFlareColors[] = {
        { 155, 205, 255 }, // blue
        { 255, 255, 205 }, // yellow
        { 255, 255, 205 }, // yellow
        { 255, 255, 205 }, // yellow
        { 155, 255, 205 }, // green
        { 205, 255, 255 }, // light blue
        { 155, 155, 255 }, // dark blue
        { 205, 175, 255 }, // purple
        { 175, 255, 205 }, // light green
        { 255, 155, 235 }, // pink
    };
    u32 lensFlareAlphas[] = {
        50, 10, 25, 40, 70, 30, 50, 70, 50, 40,
    };
    u32 lensFlareTypes[] = {
        LENS_FLARE_RING,    LENS_FLARE_CIRCLE1, LENS_FLARE_CIRCLE1, LENS_FLARE_CIRCLE1, LENS_FLARE_CIRCLE1,
        LENS_FLARE_CIRCLE1, LENS_FLARE_CIRCLE1, LENS_FLARE_CIRCLE1, LENS_FLARE_CIRCLE1, LENS_FLARE_CIRCLE1,
    };

    OPEN_DISPS(gfxCtx, "../z_kankyo.c", 2516);

    dist = Math3D_Vec3f_DistXYZ(&pos, &view->eye) / 12.0f;

    // compute a unit vector in the look direction
    tempX = view->at.x - view->eye.x;
    tempY = view->at.y - view->eye.y;
    tempZ = view->at.z - view->eye.z;

    length = sqrtf(SQ(tempX) + SQ(tempY) + SQ(tempZ));

    lookDirX = tempX / length;
    lookDirY = tempY / length;
    lookDirZ = tempZ / length;

    // compute a position along the look vector half as far as pos
    halfPosX = view->eye.x + lookDirX * (dist * 6.0f);
    halfPosY = view->eye.y + lookDirY * (dist * 6.0f);
    halfPosZ = view->eye.z + lookDirZ * (dist * 6.0f);

    // compute a unit vector in the direction from halfPos to pos
    tempX2 = pos.x - halfPosX;
    tempY2 = pos.y - halfPosY;
    tempZ2 = pos.z - halfPosZ;

    length = sqrtf(SQ(tempX2) + SQ(tempY2) + SQ(tempZ2));

    posDirX = tempX2 / length;
    posDirY = tempY2 / length;
    posDirZ = tempZ2 / length;

    // compute the cosine of the angle between lookDir and posDir
    cosAngle = (lookDirX * posDirX + lookDirY * posDirY + lookDirZ * posDirZ) /
               sqrtf((SQ(lookDirX) + SQ(lookDirY) + SQ(lookDirZ)) * (SQ(posDirX) + SQ(posDirY) + SQ(posDirZ)));

    lensFlareAlphaScaleTarget = cosAngle * 3.5f;
    if (lensFlareAlphaScaleTarget > 1.0f) {
        lensFlareAlphaScaleTarget = 1.0f;
    }

    if (!isSun) {
        lensFlareAlphaScaleTarget = cosAngle;
    }

    if (cosAngle < 0.0f) {
        // don't draw lens flare
    } else {
        if (isSun) {
            Play_GetScreenPos(play, &pos, &screenPos);
            sSunDepthTestX = (s16)screenPos.x;
            sSunDepthTestY = (s16)screenPos.y - 5.0f;
            if (sSunScreenDepth != GPACK_ZDZ(G_MAXFBZ, 0) || screenPos.x < 0.0f || screenPos.y < 0.0f ||
                screenPos.x > SCREEN_WIDTH || screenPos.y > SCREEN_HEIGHT) {
                isOffScreen = true;
            }
        }

        for (i = 0; i < ARRAY_COUNT(lensFlareTypes); i++) {
            Matrix_Translate(pos.x, pos.y, pos.z, MTXMODE_NEW);

            if (isSun) {
                temp = Environment_LerpWeight(60, 15, play->view.fovy);
            }

            Matrix_Translate(-posDirX * i * dist, -posDirY * i * dist, -posDirZ * i * dist, MTXMODE_APPLY);
            adjScale = sLensFlareScales[i] * cosAngle;

            if (isSun) {
                adjScale *= 0.001 * (scale + 630.0f * temp);
            } else {
                adjScale *= 0.0001f * scale * (2.0f * dist);
            }

            Matrix_Scale(adjScale, adjScale, adjScale, MTXMODE_APPLY);

            alpha = colorIntensity / 10.0f;
            alpha = CLAMP_MAX(alpha, 1.0f);
            alpha = alpha * lensFlareAlphas[i];
            alpha = CLAMP_MIN(alpha, 0.0f);

            fogInfluence = (ENV_FOGNEAR_MAX - play->lightCtx.fogNear) / 50.0f;

            fogInfluence = CLAMP_MAX(fogInfluence, 1.0f);

            alpha *= 1.0f - fogInfluence;

#if !PLATFORM_N64
            if (1) {}
#endif

            if (!(isOffScreen ^ 0)) {
                Math_SmoothStepToF(&envCtx->lensFlareAlphaScale, lensFlareAlphaScaleTarget, 0.5f, 0.05f, 0.001f);
            } else {
                Math_SmoothStepToF(&envCtx->lensFlareAlphaScale, 0.0f, 0.5f, 0.05f, 0.001f);
            }

            POLY_XLU_DISP = func_800947AC(POLY_XLU_DISP++);
            gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, lensFlareColors[i].r, lensFlareColors[i].g, lensFlareColors[i].b,
                            alpha * envCtx->lensFlareAlphaScale);
            MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, gfxCtx, "../z_kankyo.c", 2662);
            gDPSetCombineLERP(POLY_XLU_DISP++, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0, PRIMITIVE, TEXEL0,
                              0, PRIMITIVE, 0);
            gDPSetAlphaDither(POLY_XLU_DISP++, G_AD_DISABLE);
            gDPSetColorDither(POLY_XLU_DISP++, G_CD_DISABLE);
            gSPMatrix(POLY_XLU_DISP++, &D_01000000, G_MTX_NOPUSH | G_MTX_MUL | G_MTX_MODELVIEW);

            switch (lensFlareTypes[i]) {
                case LENS_FLARE_CIRCLE0:
                case LENS_FLARE_CIRCLE1:
                    gSPDisplayList(POLY_XLU_DISP++, gLensFlareCircleDL);
                    break;
                case LENS_FLARE_RING:
                    gSPDisplayList(POLY_XLU_DISP++, gLensFlareRingDL);
                    break;
            }
        }

        glareAlphaScale = cosAngle - (1.5f - cosAngle);

        if (glareStrength != 0) {
            if (glareAlphaScale > 0.0f) {
                POLY_XLU_DISP = Gfx_SetupDL_57(POLY_XLU_DISP);

                alpha = colorIntensity / 10.0f;
                alpha = CLAMP_MAX(alpha, 1.0f);
                alpha = alpha * glareStrength;
                alpha = CLAMP_MIN(alpha, 0.0f);

                fogInfluence = (ENV_FOGNEAR_MAX - play->lightCtx.fogNear) / 50.0f;

                fogInfluence = CLAMP_MAX(fogInfluence, 1.0f);

                alpha *= 1.0f - fogInfluence;

                gDPSetAlphaDither(POLY_XLU_DISP++, G_AD_DISABLE);
                gDPSetColorDither(POLY_XLU_DISP++, G_CD_DISABLE);

                if (!(isOffScreen ^ 0)) {
                    Math_SmoothStepToF(&envCtx->glareAlpha, alpha * glareAlphaScale, 0.5f, 50.0f, 0.1f);
                } else {
                    Math_SmoothStepToF(&envCtx->glareAlpha, 0.0f, 0.5f, 50.0f, 0.1f);
                }

                // The blender only uses the 5 most significant bits of alpha, ensure at least one of these bits is set
                if (envCtx->glareAlpha >= 8.0f) {
                    temp = colorIntensity / 120.0f;
                    temp = CLAMP_MIN(temp, 0.0f);

                    gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 255, (u8)(temp * 75.0f) + 180, (u8)(temp * 155.0f) + 100,
                                    (u8)envCtx->glareAlpha);
                    gDPFillRectangle(POLY_XLU_DISP++, 0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1);
                }
            } else {
                envCtx->glareAlpha = 0.0f;
            }
        }
    }

    CLOSE_DISPS(gfxCtx, "../z_kankyo.c", 2750);
}

f32 Environment_RandCentered(void) {
    return Rand_ZeroOne() - 0.5f;
}

void Environment_DrawRain(PlayState* play, View* view, GraphicsContext* gfxCtx) {
    s16 i;
    s32 pad;
    Vec3f vec;
    f32 temp1;
    f32 temp2;
    f32 temp3;
    f32 length;
    f32 rotX;
    f32 rotY;
    f32 x50;
    f32 y50;
    f32 z50;
    f32 x280;
    f32 z280;
    Vec3f unused = { 0.0f, 0.0f, 0.0f };
    Vec3f windDirection = { 0.0f, 0.0f, 0.0f };
    Player* player = GET_PLAYER(play);

#if OOT_VERSION < PAL_1_0
    if (!(play->cameraPtrs[CAM_ID_MAIN]->stateFlags & CAM_STATE_CAMERA_IN_WATER))
#else
    if (!(play->cameraPtrs[CAM_ID_MAIN]->stateFlags & CAM_STATE_CAMERA_IN_WATER) &&
        (play->envCtx.precipitation[PRECIP_SNOW_CUR] == 0))
#endif
    {
        OPEN_DISPS(gfxCtx, "../z_kankyo.c", 2799);

        vec.x = view->at.x - view->eye.x;
        vec.y = view->at.y - view->eye.y;
        vec.z = view->at.z - view->eye.z;

        length = sqrtf(SQXYZ(vec));

        temp1 = vec.x / length;
        temp2 = vec.y / length;
        temp3 = vec.z / length;

        x50 = view->eye.x + temp1 * 50.0f;
        y50 = view->eye.y + temp2 * 50.0f;
        z50 = view->eye.z + temp3 * 50.0f;

        x280 = view->eye.x + temp1 * 280.0f;
        z280 = view->eye.z + temp3 * 280.0f;

        if (play->envCtx.precipitation[PRECIP_RAIN_CUR]) {
            gDPPipeSync(POLY_XLU_DISP++);
            gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 150, 255, 255, 30);
            POLY_XLU_DISP = Gfx_SetupDL(POLY_XLU_DISP, SETUPDL_20);
        }

        // draw rain drops
        for (i = 0; i < play->envCtx.precipitation[PRECIP_RAIN_CUR]; i++) {
            temp2 = Rand_ZeroOne();
            temp1 = Rand_ZeroOne();
            temp3 = Rand_ZeroOne();

            Matrix_Translate((temp2 - 0.7f) * 100.0f + x50, (temp1 - 0.7f) * 100.0f + y50,
                             (temp3 - 0.7f) * 100.0f + z50, MTXMODE_NEW);

            windDirection.x = play->envCtx.windDirection.x;
            windDirection.y = play->envCtx.windDirection.y;
            windDirection.z = play->envCtx.windDirection.z;

            vec.x = windDirection.x;
            vec.y = windDirection.y + 500.0f + Rand_ZeroOne() * 200.0f;
            vec.z = windDirection.z;
            length = sqrtf(SQXZ(vec));

            gSPMatrix(POLY_XLU_DISP++, &D_01000000, G_MTX_NOPUSH | G_MTX_MUL | G_MTX_MODELVIEW);
            rotX = Math_Atan2F(length, -vec.y);
            rotY = Math_Atan2F(vec.z, vec.x);
            Matrix_RotateY(-rotY, MTXMODE_APPLY);
            Matrix_RotateX(M_PI / 2 - rotX, MTXMODE_APPLY);
            Matrix_Scale(0.4f, 1.2f, 0.4f, MTXMODE_APPLY);
            MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, gfxCtx, "../z_kankyo.c", 2887);
            gSPDisplayList(POLY_XLU_DISP++, gRaindropDL);
        }

        // draw droplet rings on the ground
        if (player->actor.world.pos.y < view->eye.y) {
            u8 materialFlag = false;

            for (i = 0; i < play->envCtx.precipitation[PRECIP_RAIN_CUR]; i++) {
                if (!materialFlag) {
                    Gfx_SetupDL_25Xlu(gfxCtx);
                    gDPSetEnvColor(POLY_XLU_DISP++, 155, 155, 155, 0);
                    gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 255, 255, 255, 120);
                    materialFlag++;
                }

                Matrix_Translate(Environment_RandCentered() * 280.0f + x280, player->actor.world.pos.y + 2.0f,
                                 Environment_RandCentered() * 280.0f + z280, MTXMODE_NEW);

                if ((LINK_IS_ADULT && ((player->actor.world.pos.y + 2.0f - view->eye.y) > -48.0f)) ||
                    (!LINK_IS_ADULT && ((player->actor.world.pos.y + 2.0f - view->eye.y) > -30.0f))) {
                    Matrix_Scale(0.02f, 0.02f, 0.02f, MTXMODE_APPLY);
                } else {
                    Matrix_Scale(0.1f, 0.1f, 0.1f, MTXMODE_APPLY);
                }

                MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, gfxCtx, "../z_kankyo.c", 2940);
                gSPDisplayList(POLY_XLU_DISP++, gEffShockwaveDL);
            }
        }

        CLOSE_DISPS(gfxCtx, "../z_kankyo.c", 2946);
    }
}

void Environment_ChangeLightSetting(PlayState* play, u32 lightSetting) {
    if ((play->envCtx.lightSetting != lightSetting) && (play->envCtx.lightBlend >= 1.0f) &&
        (play->envCtx.lightSettingOverride == LIGHT_SETTING_OVERRIDE_NONE)) {
        if (lightSetting >= LIGHT_SETTING_MAX) {
            lightSetting = 0;
        }

        play->envCtx.lightBlend = 0.0f;
        play->envCtx.prevLightSetting = play->envCtx.lightSetting;
        play->envCtx.lightSetting = lightSetting;
    }
}

/**
 * Draw color filters over the skybox. There are two filters.
 * The first uses the global fog color, and an alpha calculated with `fogNear`.
 * This filter draws unconditionally for skybox 29 at full alpha.
 * (note: skybox 29 is unused in the original game)
 * For the rest of the skyboxes it will draw if fogNear is less than 980.
 *
 * The second filter uses a custom color specified in `skyboxFilterColor`
 * and can be enabled with `customSkyboxFilter`.
 *
 * An example usage of a filter is to dim the skybox in cloudy conditions.
 */
void Environment_DrawSkyboxFilters(PlayState* play) {
    if (play->envCtx.customSkyboxFilter) {
        OPEN_DISPS(play->state.gfxCtx, "../z_kankyo.c", 3048);

        Gfx_SetupDL_57Opa(play->state.gfxCtx);
        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, play->envCtx.skyboxFilterColor[0], play->envCtx.skyboxFilterColor[1],
                        play->envCtx.skyboxFilterColor[2], play->envCtx.skyboxFilterColor[3]);
        gDPFillRectangle(POLY_OPA_DISP++, 0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1);

        CLOSE_DISPS(play->state.gfxCtx, "../z_kankyo.c", 3056);
    }
}

void Environment_DrawLightningFlash(PlayState* play, u8 red, u8 green, u8 blue, u8 alpha) {
    OPEN_DISPS(play->state.gfxCtx, "../z_kankyo.c", 3069);

    Gfx_SetupDL_57Opa(play->state.gfxCtx);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, red, green, blue, alpha);
    gDPFillRectangle(POLY_OPA_DISP++, 0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1);

    CLOSE_DISPS(play->state.gfxCtx, "../z_kankyo.c", 3079);
}

void Environment_UpdateLightningStrike(PlayState* play) {
    // if (play->envCtx.lightningState != LIGHTNING_OFF) {
        switch (gLightningStrike.state) {
            case LIGHTNING_STRIKE_WAIT:
                if (play->envCtx.lightningState != LIGHTNING_OFF) {
                    // every frame theres a 10% chance of the timer advancing 50 units
                    if (Rand_ZeroOne() < 0.1f) {
                        gLightningStrike.delayTimer += 50.0f;
                    }

                    gLightningStrike.delayTimer += Rand_ZeroOne();

                    if (gLightningStrike.delayTimer > 500.0f) {
                        gLightningStrike.flashRed = 200;
                        gLightningStrike.flashGreen = 200;
                        gLightningStrike.flashBlue = 255;
                        gLightningStrike.flashAlphaTarget = 200;

                        gLightningStrike.delayTimer = 0.0f;
                        Environment_AddLightningBolts(play,
                                                      (u8)(Rand_ZeroOne() * (ARRAY_COUNT(sLightningBolts) - 0.1f)) + 1);
                        sLightningFlashAlpha = 0;
                        gLightningStrike.state++;
                    }
                }
                break;
            case LIGHTNING_STRIKE_START:
                gLightningStrike.flashRed = 200;
                gLightningStrike.flashGreen = 200;
                gLightningStrike.flashBlue = 255;

                play->envCtx.adjAmbientColor[0] += 80;
                play->envCtx.adjAmbientColor[1] += 80;
                play->envCtx.adjAmbientColor[2] += 100;

                sLightningFlashAlpha += 100;

                if (sLightningFlashAlpha >= gLightningStrike.flashAlphaTarget) {
                    Audio_SetNatureAmbienceChannelIO(NATURE_CHANNEL_LIGHTNING, CHANNEL_IO_PORT_0, 0);
                    gLightningStrike.state++;
                    gLightningStrike.flashAlphaTarget = 0;
                }
                break;
            case LIGHTNING_STRIKE_END:
                if (play->envCtx.adjAmbientColor[0] > 0) {
                    play->envCtx.adjAmbientColor[0] -= 10;
                    play->envCtx.adjAmbientColor[1] -= 10;
                }

                if (play->envCtx.adjAmbientColor[2] > 0) {
                    play->envCtx.adjAmbientColor[2] -= 10;
                }

                sLightningFlashAlpha -= 10;

                if (sLightningFlashAlpha <= gLightningStrike.flashAlphaTarget) {
                    play->envCtx.adjAmbientColor[0] = 0;
                    play->envCtx.adjAmbientColor[1] = 0;
                    play->envCtx.adjAmbientColor[2] = 0;

                    gLightningStrike.state = LIGHTNING_STRIKE_WAIT;

                    if (play->envCtx.lightningState == LIGHTNING_LAST) {
                        play->envCtx.lightningState = LIGHTNING_OFF;
                    }
                }
                break;
        }
    // }

    if (gLightningStrike.state != LIGHTNING_STRIKE_WAIT) {
        Environment_DrawLightningFlash(play, gLightningStrike.flashRed, gLightningStrike.flashGreen,
                                       gLightningStrike.flashBlue, sLightningFlashAlpha);
    }
}

/**
 * Request the number of lightning bolts specified by `num`
 * Note: only 3 lightning bolts can be active at the same time.
 */
void Environment_AddLightningBolts(PlayState* play, u8 num) {
    s16 boltsAdded = 0;
    s16 i;

    for (i = 0; i < ARRAY_COUNT(sLightningBolts); i++) {
        if (sLightningBolts[i].state == LIGHTNING_BOLT_INACTIVE) {
            sLightningBolts[i].state = LIGHTNING_BOLT_START;
            boltsAdded++;

            if (boltsAdded >= num) {
                break;
            }
        }
    }
}

/**
 * Draw any active lightning bolt entries contained in `sLightningBolts`
 */
void Environment_DrawLightning(PlayState* play, s32 unused) {
    static void* lightningTextures[] = {
        gEffLightning1Tex, gEffLightning2Tex, gEffLightning3Tex,
        gEffLightning4Tex, gEffLightning5Tex, gEffLightning6Tex,
        gEffLightning7Tex, gEffLightning8Tex, NULL,
    };
    s16 i;
    f32 dx;
    f32 dz;
    f32 x;
    f32 z;
    s32 pad[2];
    Vec3f unused1 = { 0.0f, 0.0f, 0.0f };
    Vec3f unused2 = { 0.0f, 0.0f, 0.0f };
    IF_F3DEX3_DONT_SKIP_TEX_INIT();

    OPEN_DISPS(play->state.gfxCtx, "../z_kankyo.c", 3253);

    for (i = 0; i < ARRAY_COUNT(sLightningBolts); i++) {
        switch (sLightningBolts[i].state) {
            case LIGHTNING_BOLT_START:
                dx = play->view.at.x - play->view.eye.x;
                dz = play->view.at.z - play->view.eye.z;

                x = dx / sqrtf(SQ(dx) + SQ(dz));
                z = dz / sqrtf(SQ(dx) + SQ(dz));

                sLightningBolts[i].pos.x = play->view.eye.x + x * 9500.0f;
                sLightningBolts[i].pos.y = Rand_ZeroOne() * 1000.0f + 4000.0f;
                sLightningBolts[i].pos.z = play->view.eye.z + z * 9500.0f;

                sLightningBolts[i].offset.x = (Rand_ZeroOne() - 0.5f) * 5000.0f;
                sLightningBolts[i].offset.y = 0.0f;
                sLightningBolts[i].offset.z = (Rand_ZeroOne() - 0.5f) * 5000.0f;

                sLightningBolts[i].textureIndex = 0;
                sLightningBolts[i].pitch = (Rand_ZeroOne() - 0.5f) * 40.0f;
                sLightningBolts[i].roll = (Rand_ZeroOne() - 0.5f) * 40.0f;
                sLightningBolts[i].delayTimer = 3 * (i + 1);
                sLightningBolts[i].state++;
                break;
            case LIGHTNING_BOLT_WAIT:
                sLightningBolts[i].delayTimer--;

                if (sLightningBolts[i].delayTimer <= 0) {
                    sLightningBolts[i].state++;
                }
                break;
            case LIGHTNING_BOLT_DRAW:
                if (sLightningBolts[i].textureIndex < 7) {
                    sLightningBolts[i].textureIndex++;
                } else {
                    sLightningBolts[i].state = LIGHTNING_BOLT_INACTIVE;
                }
                break;
        }

        if (sLightningBolts[i].state == LIGHTNING_BOLT_DRAW) {
            Matrix_Translate(sLightningBolts[i].pos.x + sLightningBolts[i].offset.x,
                             sLightningBolts[i].pos.y + sLightningBolts[i].offset.y,
                             sLightningBolts[i].pos.z + sLightningBolts[i].offset.z, MTXMODE_NEW);
            Matrix_RotateX(DEG_TO_RAD(sLightningBolts[i].pitch), MTXMODE_APPLY);
            Matrix_RotateZ(DEG_TO_RAD(sLightningBolts[i].roll), MTXMODE_APPLY);
            Matrix_Scale(22.0f, 100.0f, 22.0f, MTXMODE_APPLY);
            gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 255, 255, 255, 128);
            gDPSetEnvColor(POLY_XLU_DISP++, 0, 255, 255, 128);
            MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx, "../z_kankyo.c", 3333);
            gSPSegment(POLY_XLU_DISP++, 0x08, SEGMENTED_TO_VIRTUAL(lightningTextures[sLightningBolts[i].textureIndex]));
            IF_F3DEX3_DONT_SKIP_TEX_HERE(POLY_XLU_DISP++, sLightningBolts[i].textureIndex);
            Gfx_SetupDL_61Xlu(play->state.gfxCtx);
            gDPSetAlphaCompare(POLY_XLU_DISP++, G_AC_THRESHOLD);
            gSPMatrix(POLY_XLU_DISP++, &D_01000000, G_MTX_NOPUSH | G_MTX_MUL | G_MTX_MODELVIEW);
            gSPDisplayList(POLY_XLU_DISP++, gEffLightningDL);
        }
    }

    CLOSE_DISPS(play->state.gfxCtx, "../z_kankyo.c", 3353);
}

void Environment_PlaySceneSequence(PlayState* play) {
    play->envCtx.timeSeqState = TIMESEQ_DISABLED;

    // both lost woods exits on the bridge from kokiri to hyrule field
    if (((void)0, gSaveContext.save.entranceIndex) == ENTR_LOST_WOODS_8 ||
        ((void)0, gSaveContext.save.entranceIndex) == ENTR_LOST_WOODS_9) {
        Audio_PlayNatureAmbienceSequence(NATURE_ID_KOKIRI_REGION);
    } else if (((void)0, gSaveContext.forcedSeqId) != NA_BGM_GENERAL_SFX) {
        if (!Environment_IsForcedSequenceDisabled()) {
            SEQCMD_PLAY_SEQUENCE(SEQ_PLAYER_BGM_MAIN, 0, 0, ((void)0, gSaveContext.forcedSeqId));
        }
        gSaveContext.forcedSeqId = NA_BGM_GENERAL_SFX;
    } else if (play->sceneSequences.seqId == NA_BGM_NO_MUSIC) {
        if (play->sceneSequences.natureAmbienceId == NATURE_ID_NONE) {
            return;
        }
        if (((void)0, gSaveContext.natureAmbienceId) != play->sceneSequences.natureAmbienceId) {
            Audio_PlayNatureAmbienceSequence(play->sceneSequences.natureAmbienceId);
        }
    } else if (play->sceneSequences.natureAmbienceId == NATURE_ID_NONE) {
        PRINTF(T("\n\n\nBGM設定game_play->sound_info.BGM=[%d] old_bgm=[%d]\n\n",
                 "\n\n\nBGM Configuration game_play->sound_info.BGM=[%d] old_bgm=[%d]\n\n"),
               play->sceneSequences.seqId, ((void)0, gSaveContext.seqId));
        if (((void)0, gSaveContext.seqId) != play->sceneSequences.seqId) {
            Audio_PlaySceneSequence(play->sceneSequences.seqId);
        }
    } else if (((void)0, gSaveContext.save.dayTime) >= CLOCK_TIME(7, 0) &&
               ((void)0, gSaveContext.save.dayTime) <= CLOCK_TIME(17, 10)) {
        if (((void)0, gSaveContext.seqId) != play->sceneSequences.seqId) {
            Audio_PlaySceneSequence(play->sceneSequences.seqId);
        }

        play->envCtx.timeSeqState = TIMESEQ_FADE_DAY_BGM;
    } else {
        if (((void)0, gSaveContext.natureAmbienceId) != play->sceneSequences.natureAmbienceId) {
            Audio_PlayNatureAmbienceSequence(play->sceneSequences.natureAmbienceId);
        }

        if (((void)0, gSaveContext.save.dayTime) > CLOCK_TIME(17, 10) &&
            ((void)0, gSaveContext.save.dayTime) <= CLOCK_TIME(19, 0)) {
            play->envCtx.timeSeqState = TIMESEQ_EARLY_NIGHT_CRITTERS;
        } else if (((void)0, gSaveContext.save.dayTime) > CLOCK_TIME(19, 0) + 1 ||
                   ((void)0, gSaveContext.save.dayTime) < CLOCK_TIME(6, 30)) {
            play->envCtx.timeSeqState = TIMESEQ_NIGHT_CRITTERS;
        } else {
            play->envCtx.timeSeqState = TIMESEQ_MORNING_CRITTERS;
        }
    }

    PRINTF("\n-----------------\n", ((void)0, gSaveContext.forcedSeqId));
    PRINTF(T("\n 強制ＢＧＭ=[%d]", "\n Forced BGM=[%d]"), ((void)0, gSaveContext.forcedSeqId));
    PRINTF("\n     ＢＧＭ=[%d]", play->sceneSequences.seqId);
    PRINTF(T("\n     エンブ=[%d]", "\n      Embed=[%d]"), play->sceneSequences.natureAmbienceId);
    PRINTF("\n     status=[%d]", play->envCtx.timeSeqState);

    Audio_SetEnvReverb(play->roomCtx.curRoom.echo);
}

void Environment_PlayTimeBasedSequence(PlayState* play) {
    switch (play->envCtx.timeSeqState) {
        case TIMESEQ_DAY_BGM:
            Audio_SetNatureAmbienceChannelIO(NATURE_CHANNEL_CRITTER_4 << 4 | NATURE_CHANNEL_CRITTER_5,
                                             CHANNEL_IO_PORT_1, 0);

            if (play->envCtx.precipitation[PRECIP_RAIN_MAX] == 0 && play->envCtx.precipitation[PRECIP_SOS_MAX] == 0) {
                PRINTF("\n\n\nNa_StartMorinigBgm\n\n");
                Audio_PlayMorningSceneSequence(play->sceneSequences.seqId);
            }

            play->envCtx.timeSeqState++;
            break;

        case TIMESEQ_FADE_DAY_BGM:
            if (gSaveContext.save.dayTime > CLOCK_TIME(17, 10)) {
                if (play->envCtx.precipitation[PRECIP_RAIN_MAX] == 0 &&
                    play->envCtx.precipitation[PRECIP_SOS_MAX] == 0) {
                    SEQCMD_STOP_SEQUENCE(SEQ_PLAYER_BGM_MAIN, 240);
                }

                play->envCtx.timeSeqState++;
            }
            break;

        case TIMESEQ_NIGHT_BEGIN_SFX:
            if (gSaveContext.save.dayTime > CLOCK_TIME(18, 0)) {
                Sfx_PlaySfxCentered2(NA_SE_EV_DOG_CRY_EVENING);
                play->envCtx.timeSeqState++;
            }
            break;

        case TIMESEQ_EARLY_NIGHT_CRITTERS:
            if (play->envCtx.precipitation[PRECIP_RAIN_MAX] == 0 && play->envCtx.precipitation[PRECIP_SOS_MAX] == 0) {
                Audio_PlayNatureAmbienceSequence(play->sceneSequences.natureAmbienceId);
                Audio_SetNatureAmbienceChannelIO(NATURE_CHANNEL_CRITTER_0, CHANNEL_IO_PORT_1, 1);
            }

            play->envCtx.timeSeqState++;
            break;

        case TIMESEQ_NIGHT_DELAY:
            if (gSaveContext.save.dayTime > CLOCK_TIME(19, 0)) {
                play->envCtx.timeSeqState++;
            }
            break;

        case TIMESEQ_NIGHT_CRITTERS:
            Audio_SetNatureAmbienceChannelIO(NATURE_CHANNEL_CRITTER_0, CHANNEL_IO_PORT_1, 0);

            if (play->envCtx.precipitation[PRECIP_RAIN_MAX] == 0 && play->envCtx.precipitation[PRECIP_SOS_MAX] == 0) {
                Audio_SetNatureAmbienceChannelIO(NATURE_CHANNEL_CRITTER_1 << 4 | NATURE_CHANNEL_CRITTER_3,
                                                 CHANNEL_IO_PORT_1, 1);
            }

            play->envCtx.timeSeqState++;
            break;

        case TIMESEQ_DAY_BEGIN_SFX:
            if ((gSaveContext.save.dayTime <= CLOCK_TIME(19, 0)) && (gSaveContext.save.dayTime > CLOCK_TIME(6, 30))) {
                gSaveContext.save.totalDays++;
                gSaveContext.save.bgsDayCount++;
                gSaveContext.dogIsLost = true;
                Sfx_PlaySfxCentered(NA_SE_EV_CHICKEN_CRY_M);

                if ((Inventory_ReplaceItem(play, ITEM_WEIRD_EGG, ITEM_CHICKEN) ||
                     Inventory_ReplaceItem(play, ITEM_POCKET_EGG, ITEM_POCKET_CUCCO)) &&
                    play->csCtx.state == 0 && !Player_InCsMode(play)) {
                    Message_StartTextbox(play, 0x3066, NULL);
                }

                play->envCtx.timeSeqState++;
            }
            break;

        case TIMESEQ_MORNING_CRITTERS:
            Audio_SetNatureAmbienceChannelIO(NATURE_CHANNEL_CRITTER_1 << 4 | NATURE_CHANNEL_CRITTER_3,
                                             CHANNEL_IO_PORT_1, 0);

            if (play->envCtx.precipitation[PRECIP_RAIN_MAX] == 0 && play->envCtx.precipitation[PRECIP_SOS_MAX] == 0) {
                Audio_SetNatureAmbienceChannelIO(NATURE_CHANNEL_CRITTER_4 << 4 | NATURE_CHANNEL_CRITTER_5,
                                                 CHANNEL_IO_PORT_1, 1);
            }

            play->envCtx.timeSeqState++;
            break;

        case TIMESEQ_DAY_DELAY:
            if (gSaveContext.save.dayTime > CLOCK_TIME(7, 0)) {
                play->envCtx.timeSeqState = 0;
            }
            break;
    }
}

void Environment_DrawCustomLensFlare(PlayState* play) {
    Vec3f pos;

    if (gCustomLensFlareOn) {
        pos.x = gCustomLensFlarePos.x;
        pos.y = gCustomLensFlarePos.y;
        pos.z = gCustomLensFlarePos.z;

        Environment_DrawLensFlare(play, &play->envCtx, &play->view, play->state.gfxCtx, pos, sLensFlareUnused,
                                  gLensFlareScale, gLensFlareColorIntensity, gLensFlareGlareStrength, false);
    }
}

void Environment_InitGameOverLights(PlayState* play) {
    s32 pad;
    Player* player = GET_PLAYER(play);

    sGameOverLightsIntensity = 0;

    Lights_PointNoGlowSetInfo(&sNGameOverLightInfo, (s16)player->actor.world.pos.x - 10.0f,
                              (s16)player->actor.world.pos.y + 10.0f, (s16)player->actor.world.pos.z - 10.0f, 0, 0, 0,
                              255);
    sNGameOverLightNode = LightContext_InsertLight(play, &play->lightCtx, &sNGameOverLightInfo);

    Lights_PointNoGlowSetInfo(&sSGameOverLightInfo, (s16)player->actor.world.pos.x + 10.0f,
                              (s16)player->actor.world.pos.y + 10.0f, (s16)player->actor.world.pos.z + 10.0f, 0, 0, 0,
                              255);
    sSGameOverLightNode = LightContext_InsertLight(play, &play->lightCtx, &sSGameOverLightInfo);
}

void Environment_FadeInGameOverLights(PlayState* play) {
    Player* player = GET_PLAYER(play);
    s16 i;

    Lights_PointNoGlowSetInfo(&sNGameOverLightInfo, (s16)player->actor.world.pos.x - 10.0f,
                              (s16)player->actor.world.pos.y + 10.0f, (s16)player->actor.world.pos.z - 10.0f,
                              sGameOverLightsIntensity, sGameOverLightsIntensity, sGameOverLightsIntensity, 255);
    Lights_PointNoGlowSetInfo(&sSGameOverLightInfo, (s16)player->actor.world.pos.x + 10.0f,
                              (s16)player->actor.world.pos.y + 10.0f, (s16)player->actor.world.pos.z + 10.0f,
                              sGameOverLightsIntensity, sGameOverLightsIntensity, sGameOverLightsIntensity, 255);

    if (sGameOverLightsIntensity < 254) {
        sGameOverLightsIntensity += 2;
    }

    if (Play_CamIsNotFixed(play)) {
        for (i = 0; i < 3; i++) {
            if (play->envCtx.adjAmbientColor[i] > -255) {
                play->envCtx.adjAmbientColor[i] -= 12;
                play->envCtx.adjLight1Color[i] -= 12;
            }
            play->envCtx.adjFogColor[i] = -255;
        }

        if (play->envCtx.lightSettings.zFar + play->envCtx.adjZFar > 900) {
            play->envCtx.adjZFar -= 100;
        }

        if (play->envCtx.lightSettings.fogNear + play->envCtx.adjFogNear > 950) {
            play->envCtx.adjFogNear -= 10;
        }
    } else {
        play->envCtx.fillScreen = true;
        play->envCtx.screenFillColor[0] = 0;
        play->envCtx.screenFillColor[1] = 0;
        play->envCtx.screenFillColor[2] = 0;
        play->envCtx.screenFillColor[3] = sGameOverLightsIntensity;
    }
}

void Environment_FadeOutGameOverLights(PlayState* play) {
    Player* player = GET_PLAYER(play);
    s16 i;

    if (sGameOverLightsIntensity >= 3) {
        sGameOverLightsIntensity -= 3;
    } else {
        sGameOverLightsIntensity = 0;
    }

    if (sGameOverLightsIntensity == 1) {
        LightContext_RemoveLight(play, &play->lightCtx, sNGameOverLightNode);
        LightContext_RemoveLight(play, &play->lightCtx, sSGameOverLightNode);
    } else if (sGameOverLightsIntensity >= 2) {
        Lights_PointNoGlowSetInfo(&sNGameOverLightInfo, (s16)player->actor.world.pos.x - 10.0f,
                                  (s16)player->actor.world.pos.y + 10.0f, (s16)player->actor.world.pos.z - 10.0f,
                                  sGameOverLightsIntensity, sGameOverLightsIntensity, sGameOverLightsIntensity, 255);
        Lights_PointNoGlowSetInfo(&sSGameOverLightInfo, (s16)player->actor.world.pos.x + 10.0f,
                                  (s16)player->actor.world.pos.y + 10.0f, (s16)player->actor.world.pos.z + 10.0f,
                                  sGameOverLightsIntensity, sGameOverLightsIntensity, sGameOverLightsIntensity, 255);
    }

    if (Play_CamIsNotFixed(play)) {
        for (i = 0; i < 3; i++) {
            Math_SmoothStepToS(&play->envCtx.adjAmbientColor[i], 0, 5, 12, 1);
            Math_SmoothStepToS(&play->envCtx.adjLight1Color[i], 0, 5, 12, 1);
            play->envCtx.adjFogColor[i] = 0;
        }
        play->envCtx.adjZFar = 0;
        play->envCtx.adjFogNear = 0;
    } else {
        play->envCtx.fillScreen = true;
        play->envCtx.screenFillColor[0] = 0;
        play->envCtx.screenFillColor[1] = 0;
        play->envCtx.screenFillColor[2] = 0;
        play->envCtx.screenFillColor[3] = sGameOverLightsIntensity;
        if (sGameOverLightsIntensity == 0) {
            play->envCtx.fillScreen = false;
        }
    }
}

void Environment_UpdateRain(PlayState* play) {
    u8 max = MAX(play->envCtx.precipitation[PRECIP_RAIN_MAX], play->envCtx.precipitation[PRECIP_SOS_MAX]);

    if (play->envCtx.precipitation[PRECIP_RAIN_CUR] != max && ((play->state.frames % 8) == 0)) {
        if (play->envCtx.precipitation[PRECIP_RAIN_CUR] < max) {
            play->envCtx.precipitation[PRECIP_RAIN_CUR] += 2;
        } else {
            play->envCtx.precipitation[PRECIP_RAIN_CUR] -= 2;
        }
    }
}

void Environment_FillScreen(GraphicsContext* gfxCtx, u8 red, u8 green, u8 blue, u8 alpha, u8 drawFlags) {
    // The blender operates only with the 5 most significant bits of alpha, so we better have at least
    // one bit set in that range to bother doing this
    if (alpha < 8) {
        return;
    }

    OPEN_DISPS(gfxCtx, "../z_kankyo.c", 3835);

    if (drawFlags & FILL_SCREEN_OPA) {
        POLY_OPA_DISP = Gfx_SetupDL_57(POLY_OPA_DISP);
        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, red, green, blue, alpha);
        gDPSetAlphaDither(POLY_OPA_DISP++, G_AD_DISABLE);
        gDPSetColorDither(POLY_OPA_DISP++, G_CD_DISABLE);
        gDPFillRectangle(POLY_OPA_DISP++, 0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1);
    }

    if (drawFlags & FILL_SCREEN_XLU) {
        POLY_XLU_DISP = Gfx_SetupDL_57(POLY_XLU_DISP);
        gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, red, green, blue, alpha);

        if ((u32)alpha == 255) {
            gDPSetRenderMode(POLY_XLU_DISP++, G_RM_OPA_SURF, G_RM_OPA_SURF2);
        }

        gDPSetAlphaDither(POLY_XLU_DISP++, G_AD_DISABLE);
        gDPSetColorDither(POLY_XLU_DISP++, G_CD_DISABLE);
        gDPFillRectangle(POLY_XLU_DISP++, 0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1);
    }

    CLOSE_DISPS(gfxCtx, "../z_kankyo.c", 3863);
}

Color_RGB8 sSandstormPrimColors[] = {
    { 210, 156, 85 },
    { 255, 200, 100 },
    { 225, 160, 50 },
    { 105, 90, 40 },
};

Color_RGB8 sSandstormEnvColors[] = {
    { 155, 106, 35 },
    { 200, 150, 50 },
    { 170, 110, 0 },
    { 50, 40, 0 },
};

void Environment_DrawSandstorm(PlayState* play, u8 sandstormState) {
    s32 primA1;
    s32 envA1;
    s32 primA = play->envCtx.sandstormPrimA;
    s32 envA = play->envCtx.sandstormEnvA;
    Color_RGBA8 primColor;
    Color_RGBA8 envColor;
    s32 pad;
    f32 sp98;

    switch (sandstormState) {
        case SANDSTORM_ACTIVE:
            if ((play->sceneId == SCENE_HAUNTED_WASTELAND) && (play->roomCtx.curRoom.num == 0)) {
                envA1 = 0;
                primA1 = (play->envCtx.sandstormEnvA > 128) ? 255 : play->envCtx.sandstormEnvA >> 1;
            } else {
                primA1 = play->state.frames % 128;
                if (primA1 > 64) {
                    primA1 = 128 - primA1;
                }
                primA1 += 73;
                envA1 = 128;
            }
            break;

        case SANDSTORM_FILL:
            primA1 = 255;
            envA1 = (play->envCtx.sandstormPrimA >= 255) ? 255 : 128;
            break;

        case SANDSTORM_UNFILL:
            envA1 = 128;
            if (play->envCtx.sandstormEnvA > 128) {
                primA1 = 255;
            } else {
                primA1 = play->state.frames % 128;
                if (primA1 > 64) {
                    primA1 = 128 - primA1;
                }
                primA1 += 73;
            }
            if ((primA1 >= primA) && (primA1 != 255)) {
                play->envCtx.sandstormState = SANDSTORM_ACTIVE;
            }
            break;

        case SANDSTORM_DISSIPATE:
            envA1 = 0;
            primA1 = (play->envCtx.sandstormEnvA > 128) ? 255 : play->envCtx.sandstormEnvA >> 1;

            if (primA == 0) {
                play->envCtx.sandstormState = SANDSTORM_OFF;
            }
            break;
    }

    if (ABS(primA - primA1) < 9) {
        primA = primA1;
    } else if (primA1 < primA) {
        primA -= 9;
    } else {
        primA += 9;
    }

    if (ABS(envA - envA1) < 9) {
        envA = envA1;
    } else if (envA1 < envA) {
        envA -= 9;
    } else {
        envA += 9;
    }

    play->envCtx.sandstormPrimA = primA;
    play->envCtx.sandstormEnvA = envA;

    sp98 = (512.0f - (primA + envA)) * (3.0f / 128.0f);

    if (sp98 > 6.0f) {
        sp98 = 6.0f;
    }

    if ((play->envCtx.lightMode != LIGHT_MODE_TIME) ||
        (play->envCtx.lightSettingOverride != LIGHT_SETTING_OVERRIDE_NONE)) {
        primColor.r = sSandstormPrimColors[1].r;
        primColor.g = sSandstormPrimColors[1].g;
        primColor.b = sSandstormPrimColors[1].b;
        envColor.r = sSandstormEnvColors[1].r;
        envColor.g = sSandstormEnvColors[1].g;
        envColor.b = sSandstormEnvColors[1].b;
    } else if (sSandstormColorIndex == sNextSandstormColorIndex) {
        primColor.r = sSandstormPrimColors[sSandstormColorIndex].r;
        primColor.g = sSandstormPrimColors[sSandstormColorIndex].g;
        primColor.b = sSandstormPrimColors[sSandstormColorIndex].b;
        envColor.r = sSandstormEnvColors[sSandstormColorIndex].r;
        envColor.g = sSandstormEnvColors[sSandstormColorIndex].g;
        envColor.b = sSandstormEnvColors[sSandstormColorIndex].b;
    } else {
        primColor.r = (s32)F32_LERP(sSandstormPrimColors[sSandstormColorIndex].r,
                                    sSandstormPrimColors[sNextSandstormColorIndex].r, sSandstormLerpScale);
        primColor.g = (s32)F32_LERP(sSandstormPrimColors[sSandstormColorIndex].g,
                                    sSandstormPrimColors[sNextSandstormColorIndex].g, sSandstormLerpScale);
        primColor.b = (s32)F32_LERP(sSandstormPrimColors[sSandstormColorIndex].b,
                                    sSandstormPrimColors[sNextSandstormColorIndex].b, sSandstormLerpScale);
        envColor.r = (s32)F32_LERP(sSandstormEnvColors[sSandstormColorIndex].r,
                                   sSandstormEnvColors[sNextSandstormColorIndex].r, sSandstormLerpScale);
        envColor.g = (s32)F32_LERP(sSandstormEnvColors[sSandstormColorIndex].g,
                                   sSandstormEnvColors[sNextSandstormColorIndex].g, sSandstormLerpScale);
        envColor.b = (s32)F32_LERP(sSandstormEnvColors[sSandstormColorIndex].b,
                                   sSandstormEnvColors[sNextSandstormColorIndex].b, sSandstormLerpScale);
    }

    envColor.r = ((envColor.r * sp98) + ((6.0f - sp98) * primColor.r)) * (1.0f / 6.0f);
    envColor.g = ((envColor.g * sp98) + ((6.0f - sp98) * primColor.g)) * (1.0f / 6.0f);
    envColor.b = ((envColor.b * sp98) + ((6.0f - sp98) * primColor.b)) * (1.0f / 6.0f);

    {
        u16 sp96 = (s32)(sSandstormScroll * (11.0f / 6.0f));
        u16 sp94 = (s32)(sSandstormScroll * (9.0f / 6.0f));
        u16 sp92 = (s32)(sSandstormScroll * (6.0f / 6.0f));

        OPEN_DISPS(play->state.gfxCtx, "../z_kankyo.c", 4044);

        POLY_XLU_DISP = Gfx_SetupDL_64(POLY_XLU_DISP);

        gDPSetAlphaDither(POLY_XLU_DISP++, G_AD_NOISE);
        gDPSetColorDither(POLY_XLU_DISP++, G_CD_NOISE);
        gDPSetPrimColor(POLY_XLU_DISP++, 0, 0x80, primColor.r, primColor.g, primColor.b, play->envCtx.sandstormPrimA);
        gDPSetEnvColor(POLY_XLU_DISP++, envColor.r, envColor.g, envColor.b, play->envCtx.sandstormEnvA);
        gSPSegment(POLY_XLU_DISP++, 0x08,
                   Gfx_TwoTexScroll(play->state.gfxCtx, G_TX_RENDERTILE, (u32)sp96 % 4096, 0, 512, 32, 1,
                                    (u32)sp94 % 4096, 4095 - ((u32)sp92 % 4096), 256, 64));
        gDPSetTextureLUT(POLY_XLU_DISP++, G_TT_NONE);
        gSPDisplayList(POLY_XLU_DISP++, gFieldSandstormDL);

        CLOSE_DISPS(play->state.gfxCtx, "../z_kankyo.c", 4068);
    }

    sSandstormScroll += (s32)sp98;
}

void Environment_AdjustLights(PlayState* play, f32 arg1, f32 arg2, f32 arg3, f32 arg4) {
    f32 temp;
    s32 i;

    if (play->roomCtx.curRoom.type != ROOM_TYPE_BOSS && Play_CamIsNotFixed(play)) {
        arg1 = CLAMP_MIN(arg1, 0.0f);
        arg1 = CLAMP_MAX(arg1, 1.0f);

        temp = arg1 - arg3;

        if (arg1 < arg3) {
            temp = 0.0f;
        }

        play->envCtx.adjFogNear = (arg2 - play->envCtx.lightSettings.fogNear) * temp;

        if (arg1 == 0.0f) {
            for (i = 0; i < 3; i++) {
                play->envCtx.adjFogColor[i] = 0;
            }
        } else {
            temp = arg1 * 5.0f;
            temp = CLAMP_MAX(temp, 1.0f);

            for (i = 0; i < 3; i++) {
                play->envCtx.adjFogColor[i] = -(s16)(play->envCtx.lightSettings.fogColor[i] * temp);
            }
        }

        if (arg4 <= 0.0f) {
            return;
        }

        arg1 *= arg4;

        for (i = 0; i < 3; i++) {
            play->envCtx.adjAmbientColor[i] = -(s16)(play->envCtx.lightSettings.ambientColor[i] * arg1);
            play->envCtx.adjLight1Color[i] = -(s16)(play->envCtx.lightSettings.light1Color[i] * arg1);
        }
    }
}

s32 Environment_GetBgsDayCount(void) {
    return gSaveContext.save.bgsDayCount;
}

void Environment_ClearBgsDayCount(void) {
    gSaveContext.save.bgsDayCount = 0;
}

s32 Environment_GetTotalDays(void) {
    return gSaveContext.save.totalDays;
}

void Environment_ForcePlaySequence(u16 seqId) {
    gSaveContext.forcedSeqId = seqId;
}

s32 Environment_IsForcedSequenceDisabled(void) {
    s32 isDisabled = false;

    if (gSaveContext.forcedSeqId == NA_BGM_DISABLED) {
        isDisabled = true;
    }

    return isDisabled;
}

void Environment_PlayStormNatureAmbience(PlayState* play) {
    if (play->sceneSequences.natureAmbienceId == NATURE_ID_NONE) {
        Audio_PlayNatureAmbienceSequence(NATURE_ID_MARKET_NIGHT);
    } else {
        Audio_PlayNatureAmbienceSequence(play->sceneSequences.natureAmbienceId);
    }

    Audio_SetNatureAmbienceChannelIO(NATURE_CHANNEL_RAIN, CHANNEL_IO_PORT_1, 1);
    Audio_SetNatureAmbienceChannelIO(NATURE_CHANNEL_LIGHTNING, CHANNEL_IO_PORT_1, 1);
}

void Environment_StopStormNatureAmbience(PlayState* play) {
    Audio_SetNatureAmbienceChannelIO(NATURE_CHANNEL_RAIN, CHANNEL_IO_PORT_1, 0);
    Audio_SetNatureAmbienceChannelIO(NATURE_CHANNEL_LIGHTNING, CHANNEL_IO_PORT_1, 0);

    if (Audio_GetActiveSeqId(SEQ_PLAYER_BGM_MAIN) == NA_BGM_NATURE_AMBIENCE) {
        gSaveContext.seqId = NA_BGM_NATURE_SFX_RAIN;
        Environment_PlaySceneSequence(play);
    }
}

void Environment_WarpSongLeave(PlayState* play) {
    gWeatherMode = WEATHER_MODE_CLEAR;
    gSaveContext.save.cutsceneIndex = CS_INDEX_NONE;
    gSaveContext.respawnFlag = -3;
    play->nextEntranceIndex = gSaveContext.respawn[RESPAWN_MODE_RETURN].entranceIndex;
    play->transitionTrigger = TRANS_TRIGGER_START;
    play->transitionType = TRANS_TYPE_FADE_WHITE;
    gSaveContext.nextTransitionType = TRANS_TYPE_FADE_WHITE;

    switch (play->nextEntranceIndex) {
        case ENTR_DEATH_MOUNTAIN_CRATER_0:
            Flags_SetEventChkInf(EVENTCHKINF_B9);
            break;

        case ENTR_LAKE_HYLIA_0:
            Flags_SetEventChkInf(EVENTCHKINF_B1);
            break;

        case ENTR_DESERT_COLOSSUS_0:
            Flags_SetEventChkInf(EVENTCHKINF_B8);
            break;

        case ENTR_GRAVEYARD_0:
            Flags_SetEventChkInf(EVENTCHKINF_B6);
            break;

        case ENTR_TEMPLE_OF_TIME_0:
            Flags_SetEventChkInf(EVENTCHKINF_A7);
            break;

        case ENTR_SACRED_FOREST_MEADOW_0:
            break;
    }
}
