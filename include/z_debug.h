#ifndef DEBUG_H
#define DEBUG_H

#include "ultra64.h"
#include "ultra64/ultratypes.h"
#include "z_math.h"

struct PlayState;
struct GraphicsContext;
struct Input;

void Regs_Init(void);
void DebugCamera_ScreenText(u8 x, u8 y, const char* text);
void DebugCamera_ScreenTextColored(u8 x, u8 y, u8 colorIndex, const char* text);
#if DEBUG_FEATURES
void Regs_UpdateEditor(struct Input* input);
#endif
void Debug_DrawText(struct GraphicsContext* gfxCtx);

void Debug_Print_Draw(u8 line, struct PlayState* play);
void Debug_Print(u8 line, const char* fmt, ...);

#endif
