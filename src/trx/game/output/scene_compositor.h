#pragma once

#include <trx/game/output/scene_source.h>

typedef enum {
    SCENE_UI_TARGET_OVERLAY = 0,
    SCENE_UI_TARGET_SCENE = 1,
} SCENE_UI_TARGET;

void SceneCompositor_Init(void);
void SceneCompositor_Shutdown(void);
void SceneCompositor_AddSource(const SCENE_SOURCE *source);
void SceneCompositor_BeginScene(void);
void SceneCompositor_EndScene(void);
void SceneCompositor_Flush(void);
void SceneCompositor_AnimateTextures(void);

void SceneCompositor_SetUiTarget(const SCENE_UI_TARGET target);
SCENE_UI_TARGET SceneCompositor_GetUiTarget(void);
