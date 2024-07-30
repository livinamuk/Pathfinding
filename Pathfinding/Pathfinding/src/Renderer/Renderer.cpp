#include "Renderer.h"
#include <vector>
#include <map>
#include "../API/OpenGL/GL_renderer.h"
#include "../BackEnd/BackEnd.h"
#include "../Core/Game.h"
#include "../Core/Input.h"
#include "../Core/Pathfinding.h"
#include "../Renderer/RenderData.h"
#include "../Renderer/TextBlitter.h"
#include "../Renderer/RendererUtil.hpp"
#include "../Renderer/RenderData.h"
#include "../Util.hpp"

std::vector<RenderItem2D> CreateRenderItems2D(ivec2 presentSize);
BlitDstCoords GetBlitDstCoords();
void UpdateDebugLinesMesh();
void UpdateDebugPointsMesh();
void RenderGame(RenderData& renderData);
void PresentFinalImage();

DetachedMesh _debugLinesMesh;
DetachedMesh _debugPointsMesh;
DebugLineRenderMode _debugLineRenderMode;

/*

██████╗ ███████╗███╗   ██╗██████╗ ███████╗██████╗     ██████╗  █████╗ ████████╗ █████╗
██╔══██╗██╔════╝████╗  ██║██╔══██╗██╔════╝██╔══██╗    ██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗
██████╔╝█████╗  ██╔██╗ ██║██║  ██║█████╗  ██████╔╝    ██║  ██║███████║   ██║   ███████║
██╔══██╗██╔══╝  ██║╚██╗██║██║  ██║██╔══╝  ██╔══██╗    ██║  ██║██╔══██║   ██║   ██╔══██║
██║  ██║███████╗██║ ╚████║██████╔╝███████╗██║  ██║    ██████╔╝██║  ██║   ██║   ██║  ██║
╚═╝  ╚═╝╚══════╝╚═╝  ╚═══╝╚═════╝ ╚══════╝╚═╝  ╚═╝    ╚═════╝ ╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝ */


std::vector<RenderItem2D> CreateLoadingScreenRenderItems() {

    std::string text = "";
    int maxLinesDisplayed = 40;
    int endIndex = AssetManager::GetLoadLog().size();
    int beginIndex = std::max(0, endIndex - maxLinesDisplayed);
    for (int i = beginIndex; i < endIndex; i++) {
        text += AssetManager::GetLoadLog()[i] + "\n";
    }
    ivec2 location = ivec2(0.0f, (float)PRESENT_HEIGHT);
    ivec2 viewportSize = ivec2(PRESENT_WIDTH, PRESENT_HEIGHT);
    return TextBlitter::CreateText(text, location, viewportSize, Alignment::TOP_LEFT, BitmapFontType::STANDARD);
}

RenderItem2D CreateColoredTile(int x, int y, glm::vec3 color) {
    ivec2 viewportSize = ivec2(PRESENT_WIDTH, PRESENT_HEIGHT);
    return RendererUtil::CreateRenderItem2D("tile_transparent", { x * CELL_SIZE, PRESENT_HEIGHT - y * CELL_SIZE }, viewportSize, TOP_LEFT, color);
}

std::vector<RenderItem2D> CreateRenderItems2D(ivec2 presentSize) {

    ivec2 location = ivec2(0.0f, (float)PRESENT_HEIGHT);
    ivec2 viewportSize = ivec2(PRESENT_WIDTH, PRESENT_HEIGHT);
    std::vector<RenderItem2D> renderItems;

    int drawX = Util::MapRange(Input::GetMouseX(), 0, BackEnd::GetCurrentWindowWidth(), 0, PRESENT_WIDTH);
    int drawY = Util::MapRange(Input::GetMouseY(), 0, BackEnd::GetCurrentWindowHeight(), 0, PRESENT_HEIGHT);
    drawX /= CELL_SIZE;
    drawY /= CELL_SIZE;
    drawX *= CELL_SIZE;
    drawY *= CELL_SIZE;
    drawY = PRESENT_HEIGHT - drawY;

    int cellX = Util::MapRange(Input::GetMouseX(), 0, BackEnd::GetCurrentWindowWidth(), 0, PRESENT_WIDTH) / CELL_SIZE;
    int cellY = Util::MapRange(Input::GetMouseY(), 0, BackEnd::GetCurrentWindowHeight(), 0, PRESENT_HEIGHT) / CELL_SIZE;

    std::string text = "";
    text += "Cell X: " + std::to_string(Pathfinding::GetMouseCellX()) + "\n";
    text += "Cell Y: " + std::to_string(Pathfinding::GetMouseCellY()) + "\n";
    text += "Mouse X: " + std::to_string(Pathfinding::GetMouseX()) + "\n";
    text += "Mouse Y: " + std::to_string(Pathfinding::GetMouseY()) + "\n";

    if (Pathfinding::SlowModeEnabled()) {
        text += "Slowmode: On\n";
    }
    else {
        text += "Slowmode: Off\n";
    }

    for (int x = 0; x < Pathfinding::GetMapWidth(); x++) {
        for (int y = 0; y < Pathfinding::GetMapHeight(); y++) {
            ivec2 drawLocation = ivec2(x * CELL_SIZE, PRESENT_HEIGHT - y * CELL_SIZE);

            if (Pathfinding::IsObstacle(x, y)) {
                renderItems.push_back(RendererUtil::CreateRenderItem2D("tile_wall", drawLocation, viewportSize, TOP_LEFT));
            }
            else {
                renderItems.push_back(RendererUtil::CreateRenderItem2D("tile_dirt", drawLocation, viewportSize, TOP_LEFT));
            }
        }
    }

    AStar aStar = Pathfinding::GetAStar();

    for (auto& cell : aStar.GetClosedList()) {
        renderItems.push_back(CreateColoredTile(cell->x, cell->y, RED));
    }

    for (int i = 0; i < aStar.GetOpenList().Size(); i++) {
        auto& cell = aStar.GetOpenList().items[i];
        renderItems.push_back(CreateColoredTile(cell->x, cell->y, GREEN));
    }
    for (auto& cell : aStar.GetPath()) {
        renderItems.push_back(CreateColoredTile(cell->x, cell->y, BLUE));
    }

    renderItems.push_back(CreateColoredTile(Pathfinding::GetStartX(), Pathfinding::GetStartY(), glm::vec3(0.164f, 0.605f, 0.765f)));
    renderItems.push_back(CreateColoredTile(Pathfinding::GetTargetX(), Pathfinding::GetTargetY(), glm::vec3(0.164f, 0.605f, 0.765f)));

    ivec2 drawLocation = ivec2(drawX, drawY);
    renderItems.push_back(RendererUtil::CreateRenderItem2D("selector", drawLocation, viewportSize, TOP_LEFT));
    RendererUtil::AddRenderItems(renderItems, TextBlitter::CreateText(text, location, viewportSize, Alignment::TOP_LEFT, BitmapFontType::STANDARD));

    return renderItems;
}



void UpdateDebugLinesMesh() {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    AStar aStar = Pathfinding::GetAStar();

    if (aStar.m_intersectionPoints.size() >= 2) {
        for (int i = 0; i < aStar.m_intersectionPoints.size() - 1; i++) {
            glm::vec2 cell0 = aStar.m_intersectionPoints[i];
            glm::vec2 cell1 = aStar.m_intersectionPoints[i + 1];
            vertices.push_back(Vertex(Util::ScreenToNDC(glm::vec2(cell0.x * CELL_SIZE, cell0.y * CELL_SIZE), glm::vec2(PRESENT_WIDTH, PRESENT_HEIGHT)), WHITE));
            vertices.push_back(Vertex(Util::ScreenToNDC(glm::vec2(cell1.x * CELL_SIZE, cell1.y * CELL_SIZE), glm::vec2(PRESENT_WIDTH, PRESENT_HEIGHT)), WHITE));
        }
    }

    for (int i = 0; i < vertices.size(); i++) {
        indices.push_back(i);
    }
    _debugLinesMesh.UpdateVertexBuffer(vertices, indices);
}

void UpdateDebugPointsMesh() {

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    AStar aStar = Pathfinding::GetAStar();
    for (int i = 0; i < aStar.m_intersectionPoints.size(); i++) {
        glm::vec2 cell0 = aStar.m_intersectionPoints[i];
        vertices.push_back(Vertex(Util::ScreenToNDC(glm::vec2(cell0.x * CELL_SIZE, cell0.y * CELL_SIZE), glm::vec2(PRESENT_WIDTH, PRESENT_HEIGHT)), WHITE));
    }


    for (int i = 0; i < vertices.size(); i++) {
        indices.push_back(i);
    }

    _debugPointsMesh.UpdateVertexBuffer(vertices, indices);

}

BlitDstCoords GetBlitDstCoordsPresent() {
    BlitDstCoords coords;
    coords.dstX0 = 0;
    coords.dstY0 = 0;
    coords.dstX1 = PRESENT_WIDTH;
    coords.dstY1 = PRESENT_HEIGHT;
    return coords;
}

BlitDstCoords GetBlitDstCoords() {
    BlitDstCoords coords;
    coords.dstX0 = 0;
    coords.dstY0 = 0;
    coords.dstX1 = BackEnd::GetCurrentWindowWidth();
    coords.dstY1 = BackEnd::GetCurrentWindowHeight();
    return coords;
}

RenderData CreateRenderData() {

    ivec2 viewportSize = { PRESENT_WIDTH, PRESENT_HEIGHT };

    RenderData renderData;
    renderData.renderItems2D = CreateRenderItems2D(viewportSize);
    renderData.debugLinesMesh = &_debugLinesMesh;
    renderData.debugPointsMesh = &_debugPointsMesh;
    renderData.blitDstCoords = GetBlitDstCoords();
    renderData.blitDstCoordsPresent = GetBlitDstCoordsPresent();

    return renderData;
}

/*

██████╗ ███████╗███╗   ██╗██████╗ ███████╗██████╗     ███████╗██████╗  █████╗ ███╗   ███╗███████╗
██╔══██╗██╔════╝████╗  ██║██╔══██╗██╔════╝██╔══██╗    ██╔════╝██╔══██╗██╔══██╗████╗ ████║██╔════╝
██████╔╝█████╗  ██╔██╗ ██║██║  ██║█████╗  ██████╔╝    █████╗  ██████╔╝███████║██╔████╔██║█████╗
██╔══██╗██╔══╝  ██║╚██╗██║██║  ██║██╔══╝  ██╔══██╗    ██╔══╝  ██╔══██╗██╔══██║██║╚██╔╝██║██╔══╝
██║  ██║███████╗██║ ╚████║██████╔╝███████╗██║  ██║    ██║     ██║  ██║██║  ██║██║ ╚═╝ ██║███████╗
╚═╝  ╚═╝╚══════╝╚═╝  ╚═══╝╚═════╝ ╚══════╝╚═╝  ╚═╝    ╚═╝     ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝     ╚═╝╚══════╝ */

void Renderer::RenderFrame() {

    UpdateDebugLinesMesh();
    UpdateDebugPointsMesh();

    RenderData renderData = CreateRenderData();
    if (!BackEnd::WindowIsMinimized()) {
        if (BackEnd::GetAPI() == API::OPENGL) {
            OpenGLRenderer::RenderFrame(renderData);
        }
    }
}

void RenderGame(RenderData& renderData) {

    if (BackEnd::GetAPI() == API::OPENGL) {
        OpenGLRenderer::RenderFrame(renderData);
    }
}


/*

███╗   ███╗██╗███████╗ ██████╗
████╗ ████║██║██╔════╝██╔════╝
██╔████╔██║██║███████╗██║
██║╚██╔╝██║██║╚════██║██║
██║ ╚═╝ ██║██║███████║╚██████╗
╚═╝     ╚═╝╚═╝╚══════╝ ╚═════╝ */

void Renderer::RenderLoadingScreen() {

    std::vector<RenderItem2D> renderItems = CreateLoadingScreenRenderItems();
    if (!BackEnd::WindowIsMinimized()) {
        if (BackEnd::GetAPI() == API::OPENGL) {
            OpenGLRenderer::RenderLoadingScreen(renderItems);
        }
    }
}

void Renderer::HotloadShaders() {

    if (BackEnd::GetAPI() == API::OPENGL) {
        OpenGLRenderer::HotloadShaders();
    }
}

inline std::vector<DebugLineRenderMode> _allowedDebugLineRenderModes = {
    BOUNDING_BOXES,
    RTX_LAND_AABBS,
    PHYSX_COLLISION,
};

void Renderer::NextDebugLineRenderMode() {
    _debugLineRenderMode = (DebugLineRenderMode)(int(_debugLineRenderMode) + 1);
    if (_debugLineRenderMode == DEBUG_LINE_MODE_COUNT) {
        _debugLineRenderMode = (DebugLineRenderMode)0;
    }
    // If mode isn't in available modes list, then go to next
    bool allowed = false;
    for (auto& avaliableMode : _allowedDebugLineRenderModes) {
        if (_debugLineRenderMode == avaliableMode) {
            allowed = true;
            break;
        }
    }
    if (!allowed && _debugLineRenderMode != DebugLineRenderMode::SHOW_NO_LINES) {
        NextDebugLineRenderMode();
    }
}

void PresentFinalImage() {

    if (BackEnd::GetAPI() == API::OPENGL) {
        OpenGLRenderer::PresentFinalImage();
    }
}