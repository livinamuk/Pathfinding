#pragma once
#include "RendererCommon.h"
#include "Types/DetachedMesh.hpp"

struct BlitDstCoords {
    unsigned int dstX0 = 0;
    unsigned int dstY0 = 0;
    unsigned int dstX1 = 0;
    unsigned int dstY1 = 0;
};

struct MultiDrawIndirectDrawInfo {
    std::vector<DrawIndexedIndirectCommand> commands;
    std::vector<RenderItem3D> renderItems;
};

struct RenderItems {
    std::vector<RenderItem3D> geometry;
    std::vector<RenderItem3D> decals;
};

struct IndirectDrawInfo {
    uint32_t instanceDataOffests[4];
    std::vector<RenderItem3D> instanceData;
    std::vector<DrawIndexedIndirectCommand> playerDrawCommands[4];
};

struct RenderData {

    std::vector<RenderItem2D> renderItems2D;

    DetachedMesh* debugLinesMesh = nullptr;
    DetachedMesh* debugPointsMesh = nullptr;

    bool renderDebugLines = false;

    BlitDstCoords blitDstCoords;
    BlitDstCoords blitDstCoordsPresent;
    RenderMode renderMode;
};

