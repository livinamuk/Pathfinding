#include "GL_renderer.h"
#include "GL_backEnd.h"
#include "Types/GL_gBuffer.h"
#include "Types/GL_shader.h"
#include "Types/GL_frameBuffer.hpp"
#include "../../BackEnd/BackEnd.h"
#include "../../Core/AssetManager.h"
#include "../../Core/Game.h"
#include "../../Renderer/RendererCommon.h"
#include "../../Renderer/TextBlitter.h"
#include "../../Renderer/RendererUtil.hpp"

namespace OpenGLRenderer {

    struct FrameBuffers {
        GLFrameBuffer present;
    } g_frameBuffers;

    struct Shaders {
        Shader UI;
        Shader debugSolidColor;
    } g_shaders;

    struct SSBOs {
        GLuint samplers = 0;
        GLuint renderItems2D = 0;

    } g_ssbos;

    GLuint g_indirectBuffer = 0;
}

void BlitFrameBuffer(GLFrameBuffer* src, GLFrameBuffer* dst, const char* srcName, const char* dstName, GLbitfield mask, GLenum filter);
void BlitFrameBuffer(GLFrameBuffer* src, GLFrameBuffer* dst, const char* srcName, const char* dstName, ViewportInfo srcRegion, ViewportInfo dstRegion, GLbitfield mask, GLenum filter);
void BlitPlayerPresentTargetToDefaultFrameBuffer(GLFrameBuffer* src, GLFrameBuffer* dst, const char* srcName, const char* dstName, GLbitfield mask, GLenum filter, BlitDstCoords& blitDstCoords);

void ClearRenderTargets();
void RenderUI(std::vector<RenderItem2D>& renderItems, GLFrameBuffer& frameBuffer, bool clearScreen);
void DebugPassLines(RenderData& renderData);
void DebugPassPoints(RenderData& renderData);
void DownScaleGBuffer();

void OpenGLRenderer::HotloadShaders() {

    std::cout << "Hotloading shaders...\n";
    g_shaders.UI.Load("GL_ui.vert", "GL_ui.frag");
    g_shaders.debugSolidColor.Load("GL_debug_solidColor.vert", "GL_debug_solidColor.frag");
}

void OpenGLRenderer::CreateRenderTargets(int presentWidth, int presentHeight) {

    g_frameBuffers.present.Create("Present", presentWidth, presentHeight);
    g_frameBuffers.present.CreateAttachment("Color", GL_RGBA8);
    g_frameBuffers.present.CreateDepthAttachment(GL_DEPTH32F_STENCIL8);
}

void OpenGLRenderer::InitMinimum() {

    HotloadShaders();
    CreateRenderTargets(PRESENT_WIDTH, PRESENT_HEIGHT);
    glCreateBuffers(1, &g_ssbos.renderItems2D);
    glNamedBufferStorage(g_ssbos.renderItems2D, MAX_RENDER_OBJECTS_2D * sizeof(RenderItem2D), NULL, GL_DYNAMIC_STORAGE_BIT);
}

void OpenGLRenderer::BindBindlessTextures() {

    // Create the samplers SSBO if needed
    if (g_ssbos.samplers == 0) {
        glCreateBuffers(1, &g_ssbos.samplers);
        glNamedBufferStorage(g_ssbos.samplers, TEXTURE_ARRAY_SIZE * sizeof(glm::uvec2), NULL, GL_DYNAMIC_STORAGE_BIT);
    }
    // Get the handles and stash em in a vector
    std::vector<GLuint64> samplers;
    samplers.reserve(AssetManager::GetTextureCount());
    for (int i = 0; i < AssetManager::GetTextureCount(); i++) {
        samplers.push_back(AssetManager::GetTextureByIndex(i)->GetGLTexture().GetBindlessID());
    }
    // Send to GPU
    glNamedBufferSubData(g_ssbos.samplers, 0, samplers.size() * sizeof(glm::uvec2), &samplers[0]);
}

void BlitPlayerPresentTargetToDefaultFrameBuffer(GLFrameBuffer* src, GLFrameBuffer* dst, const char* srcName, const char* dstName, GLbitfield mask, GLenum filter, BlitDstCoords& blitDstCoords) {
    GLint srcHandle = 0;
    GLint dstHandle = 0;
    GLint srcWidth = BackEnd::GetCurrentWindowWidth();
    GLint srcHeight = BackEnd::GetCurrentWindowHeight();
    GLint dstWidth = BackEnd::GetCurrentWindowWidth();
    GLint dstHeight = BackEnd::GetCurrentWindowHeight();
    GLenum srcSlot = GL_BACK;
    GLenum dstSlot = GL_BACK;
    if (src) {
        srcHandle = src->GetHandle();
        srcWidth = src->GetWidth();
        srcHeight = src->GetHeight();
        srcSlot = src->GetColorAttachmentSlotByName(srcName);
    }
    GLint srcX0 = 0;
    GLint srcY0 = 0;
    GLint srcX1 = srcWidth;
    GLint srcY1 = srcHeight;
    GLint dstX0 = blitDstCoords.dstX0;
    GLint dstY0 = blitDstCoords.dstY0;
    GLint dstX1 = blitDstCoords.dstX1;
    GLint dstY1 = blitDstCoords.dstY1;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, srcHandle);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dstHandle);
    glReadBuffer(srcSlot);
    glDrawBuffer(dstSlot);;
    glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
}

void BlitFrameBuffer(GLFrameBuffer* src, GLFrameBuffer* dst, const char* srcName, const char* dstName, GLbitfield mask, GLenum filter) {
    GLint srcHandle = 0;
    GLint dstHandle = 0;
    GLint srcWidth = BackEnd::GetCurrentWindowWidth();
    GLint srcHeight = BackEnd::GetCurrentWindowHeight();
    GLint dstWidth = BackEnd::GetCurrentWindowWidth();
    GLint dstHeight = BackEnd::GetCurrentWindowHeight();
    GLenum srcSlot = GL_BACK;
    GLenum dstSlot = GL_BACK;
    if (src) {
        srcHandle = src->GetHandle();
        srcWidth = src->GetWidth();
        srcHeight = src->GetHeight();
        srcSlot = src->GetColorAttachmentSlotByName(srcName);
    }
    if (dst) {
        dstHandle = dst->GetHandle();
        dstWidth = dst->GetWidth();
        dstHeight = dst->GetHeight();
        dstSlot = dst->GetColorAttachmentSlotByName(dstName);
    }
    glBindFramebuffer(GL_READ_FRAMEBUFFER, srcHandle);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dstHandle);
    glReadBuffer(srcSlot);
    glDrawBuffer(dstSlot);;
    glBlitFramebuffer(0, 0, srcWidth, srcHeight, 0, 0, dstWidth, dstHeight, mask, filter);
}


void BlitDepthBuffer(GLFrameBuffer* src, GLFrameBuffer* dst, const char* srcName, const char* dstName, GLbitfield mask, GLenum filter) {
    GLint srcHandle = 0;
    GLint dstHandle = 0;
    GLint srcWidth = BackEnd::GetCurrentWindowWidth();
    GLint srcHeight = BackEnd::GetCurrentWindowHeight();
    GLint dstWidth = BackEnd::GetCurrentWindowWidth();
    GLint dstHeight = BackEnd::GetCurrentWindowHeight();
    if (src) {
        srcHandle = src->GetHandle();
        srcWidth = src->GetWidth();
        srcHeight = src->GetHeight();
    }
    if (dst) {
        dstHandle = dst->GetHandle();
        dstWidth = dst->GetWidth();
        dstHeight = dst->GetHeight();
    }
    glBlitFramebuffer(0, 0, srcWidth, srcHeight, 0, 0, dstWidth, dstHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
}



void BlitFrameBuffer(GLFrameBuffer* src, GLFrameBuffer* dst, const char* srcName, const char* dstName, ViewportInfo srcRegion, ViewportInfo dstRegion, GLbitfield mask, GLenum filter) {
    GLint srcHandle = 0;
    GLint dstHandle = 0;
    GLint srcX0 = srcRegion.xOffset;
    GLint srcX1 = srcRegion.xOffset + srcRegion.width;
    GLint srcY0 = srcRegion.yOffset;
    GLint srcY1 = srcRegion.yOffset + srcRegion.height;
    GLint dstX0 = dstRegion.xOffset;
    GLint dstX1 = dstRegion.xOffset + dstRegion.width;
    GLint dstY0 = dstRegion.yOffset;
    GLint dstY1 = dstRegion.yOffset + dstRegion.height;
    GLenum srcSlot = GL_BACK;
    GLenum dstSlot = GL_BACK;
    if (src) {
        srcHandle = src->GetHandle();
        srcSlot = src->GetColorAttachmentSlotByName(srcName);
    }
    if (dst) {
        dstHandle = dst->GetHandle();
        dstSlot = dst->GetColorAttachmentSlotByName(dstName);
    }
    glBindFramebuffer(GL_READ_FRAMEBUFFER, srcHandle);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dstHandle);
    glReadBuffer(srcSlot);
    glDrawBuffer(dstSlot);;
    glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
}


void DrawQuad() {
    Mesh* mesh = AssetManager::GetQuadMesh();
    glDrawElementsBaseVertex(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * mesh->baseIndex), mesh->baseVertex);
}

/*
   ▄████████    ▄████████ ███▄▄▄▄   ████████▄     ▄████████    ▄████████         ▄███████▄    ▄████████    ▄████████    ▄████████    ▄████████    ▄████████
  ███    ███   ███    ███ ███▀▀▀██▄ ███   ▀███   ███    ███   ███    ███        ███    ███   ███    ███   ███    ███   ███    ███   ███    ███   ███    ███
  ███    ███   ███    █▀  ███   ███ ███    ███   ███    █▀    ███    ███        ███    ███   ███    ███   ███    █▀    ███    █▀    ███    █▀    ███    █▀
 ▄███▄▄▄▄██▀  ▄███▄▄▄     ███   ███ ███    ███  ▄███▄▄▄      ▄███▄▄▄▄██▀        ███    ███   ███    ███   ███          ███         ▄███▄▄▄       ███
▀▀███▀▀▀▀▀   ▀▀███▀▀▀     ███   ███ ███    ███ ▀▀███▀▀▀     ▀▀███▀▀▀▀▀        ▀█████████▀  ▀███████████ ▀███████████ ▀███████████ ▀▀███▀▀▀     ▀███████████
▀███████████   ███    █▄  ███   ███ ███    ███   ███    █▄  ▀███████████        ███          ███    ███          ███          ███   ███    █▄           ███
  ███    ███   ███    ███ ███   ███ ███   ▄███   ███    ███   ███    ███        ███          ███    ███    ▄█    ███    ▄█    ███   ███    ███    ▄█    ███
  ███    ███   ██████████  ▀█   █▀  ████████▀    ██████████   ███    ███       ▄████▀        ███    █▀   ▄████████▀   ▄████████▀    ██████████  ▄████████▀
  ███    ███                                                  ███    ███                                                                                     */


void BindFrameBuffer(GLFrameBuffer& frameBuffer) {
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer.GetHandle());
}

void SetViewport(ViewportInfo viewportInfo) {
    glViewport(viewportInfo.xOffset, viewportInfo.yOffset, viewportInfo.width, viewportInfo.height);
}

void OpenGLRenderer::RenderLoadingScreen(std::vector<RenderItem2D>& renderItems) {

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, g_ssbos.samplers);
    RenderUI(renderItems, g_frameBuffers.present, true);
    BlitFrameBuffer(&g_frameBuffers.present, 0, "Color", "", GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void OpenGLRenderer::RenderFrame(RenderData& renderData) {

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, g_ssbos.samplers);
    RenderUI(renderData.renderItems2D, g_frameBuffers.present, true);
    DebugPassLines(renderData);
    DebugPassPoints(renderData);
    BlitFrameBuffer(&g_frameBuffers.present, 0, "Color", "", GL_COLOR_BUFFER_BIT, GL_NEAREST);

    /*
    ClearRenderTargets();
    RenderUI(renderData.renderItems2D, g_frameBuffers.present, false);
    DownScaleGBuffer();*/
}


void ClearRenderTargets() {

    GLFrameBuffer& presentBuffer = OpenGLRenderer::g_frameBuffers.present;
    presentBuffer.Bind();
    presentBuffer.SetViewport();
    unsigned int attachments[1] = {
        presentBuffer.GetColorAttachmentSlotByName("Color")
    };
    glDrawBuffers(1, attachments);
    glClearColor(0.0f, 0.1f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

/*

 █  █ █▀▀ █▀▀ █▀▀█ 　 ▀█▀ █▀▀█ ▀▀█▀▀ █▀▀ █▀▀█ █▀▀ █▀▀█ █▀▀ █▀▀
 █  █ ▀▀█ █▀▀ █▄▄▀ 　  █  █  █   █   █▀▀ █▄▄▀ █▀▀ █▄▄█ █   █▀▀
 █▄▄█ ▀▀▀ ▀▀▀ ▀─▀▀ 　 ▀▀▀ ▀  ▀   ▀   ▀▀▀ ▀ ▀▀ ▀   ▀  ▀ ▀▀▀ ▀▀▀  */

void RenderUI(std::vector<RenderItem2D>& renderItems, GLFrameBuffer& frameBuffer, bool clearScreen) {

    frameBuffer.Bind();
    frameBuffer.SetViewport();

    // GL State
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    if (clearScreen) {
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
    }
    // Feed instance data to GPU
    OpenGLRenderer::g_shaders.UI.Use();
    glNamedBufferSubData(OpenGLRenderer::g_ssbos.renderItems2D, 0, renderItems.size() * sizeof(RenderItem2D), &renderItems[0]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, OpenGLRenderer::g_ssbos.renderItems2D);

    // Draw instanced
    Mesh* mesh = AssetManager::GetQuadMesh();
    glBindVertexArray(OpenGLBackEnd::GetVertexDataVAO());
    glDrawElementsInstancedBaseVertex(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * mesh->baseIndex), renderItems.size(), mesh->baseVertex);
}

/*

    █▀▀▄ █▀▀ █▀▀█ █  █ █▀▀▀ 　 █▀▀█ █▀▀█ █▀▀ █▀▀
    █  █ █▀▀ █▀▀▄ █  █ █ ▀█ 　 █▄▄█ █▄▄█ ▀▀█ ▀▀█
    █▄▄▀ ▀▀▀ ▀▀▀▀ ▀▀▀▀ ▀▀▀▀ 　 ▀    ▀  ▀ ▀▀▀ ▀▀▀  */

void DebugPassLines(RenderData& renderData) {

    OpenGLDetachedMesh& linesMesh = renderData.debugLinesMesh->GetGLMesh();

    // Render target
    GLFrameBuffer& presentBuffer = OpenGLRenderer::g_frameBuffers.present;
    ViewportInfo viewportInfo = RendererUtil::CreateViewportInfo(0, SplitscreenMode::NONE, presentBuffer.GetWidth(), presentBuffer.GetHeight());
    BindFrameBuffer(presentBuffer);
    SetViewport(viewportInfo);

    Shader& shader = OpenGLRenderer::g_shaders.debugSolidColor;
    shader.Use();

    glDisable(GL_DEPTH_TEST);
    if (linesMesh.GetVertexCount() > 0) {
        glBindVertexArray(linesMesh.GetVAO());
        glDrawElements(GL_LINES, linesMesh.GetIndexCount(), GL_UNSIGNED_INT, 0);
    }
}

void DebugPassPoints(RenderData& renderData) {

    OpenGLDetachedMesh& pointsMesh = renderData.debugPointsMesh->GetGLMesh();

    // Render target
    GLFrameBuffer& presentBuffer = OpenGLRenderer::g_frameBuffers.present;
    ViewportInfo viewportInfo = RendererUtil::CreateViewportInfo(0, SplitscreenMode::NONE, presentBuffer.GetWidth(), presentBuffer.GetHeight());
    BindFrameBuffer(presentBuffer);
    SetViewport(viewportInfo);

    Shader& shader = OpenGLRenderer::g_shaders.debugSolidColor;
    shader.Use();

    glDisable(GL_DEPTH_TEST);
    if (pointsMesh.GetVertexCount() > 0) {
        glPointSize(4);
        glBindVertexArray(pointsMesh.GetVAO());
        glDrawElements(GL_POINTS, pointsMesh.GetIndexCount(), GL_UNSIGNED_INT, 0);
    }
}

void OpenGLRenderer::PresentFinalImage() {
    BlitDstCoords blitDstCoords;
    blitDstCoords.dstX0 = 0;
    blitDstCoords.dstY0 = 0;
    blitDstCoords.dstX1 = BackEnd::GetCurrentWindowWidth();
    blitDstCoords.dstY1 = BackEnd::GetCurrentWindowHeight();
    BlitPlayerPresentTargetToDefaultFrameBuffer(&g_frameBuffers.present, 0, "Color", "", GL_COLOR_BUFFER_BIT, GL_NEAREST, blitDstCoords);
}