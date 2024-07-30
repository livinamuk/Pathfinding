#pragma once
#include "../../Renderer/RenderData.h"
#include "../../Renderer/RendererCommon.h"

namespace OpenGLRenderer {

    void InitMinimum();
    void RenderLoadingScreen(std::vector<RenderItem2D>& renderItems);
    void RenderFrame(RenderData& renderData);
    void HotloadShaders();
    void BindBindlessTextures();
    void CreateRenderTargets(int presentWidth, int presentHeight);
    void PresentFinalImage();
}