#include "../../API/OpenGL/Types/gl_detachedMesh.hpp"
#include "../../BackEnd/BackEnd.h"
#include "../../Renderer/RendererCommon.h"

struct DetachedMesh {

private:
    OpenGLDetachedMesh openglDetachedMesh;

public:
    void UpdateVertexBuffer(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices) {
        if (BackEnd::GetAPI() == API::OPENGL) {
            openglDetachedMesh.UpdateVertexBuffer(vertices, indices);
        }
    }
    OpenGLDetachedMesh& GetGLMesh() {
        return openglDetachedMesh;
    }
};