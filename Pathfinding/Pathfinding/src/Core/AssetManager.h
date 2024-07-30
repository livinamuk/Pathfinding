#pragma once
#include "../Common.h"
#include "../Renderer/Types/Mesh.hpp"
#include "../Renderer/Types/Model.hpp"
#include "../Renderer/Types/Texture.h"

namespace AssetManager {

    // Asset Loading
    void FindAssetPaths();
    void LoadNextItem();
    void AddItemToLoadLog(std::string item);
    bool LoadingComplete();
    std::vector<std::string>& GetLoadLog();

    // Vertex data
    std::vector<Vertex>& GetVertices();
    std::vector<uint32_t>& GetIndices();
    void UploadVertexData();

    // Mesh
    Mesh* GetMeshByIndex(int index);
    Mesh* GetQuadMesh();
    int GetMeshIndexByName(const std::string& name);
    int CreateMesh(std::string name, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, glm::vec3 aabbMin = glm::vec3(0), glm::vec3 aabbMax = glm::vec3(0));

    // Models
    Model* GetModelByIndex(int index);
    int GetModelIndexByName(const std::string& name);
    bool ModelExists(const std::string& name);
    void CreateHardcodedModels();
    unsigned int GetQuadMeshIndex();

    // Textures
    void LoadTexture(const std::string filepath);
    Texture* GetTextureByName(const std::string& name);
    Texture* GetTextureByIndex(const int index);
    int GetTextureCount();
    int GetTextureIndexByName(const std::string& filename, bool ignoreWarning = false);
    bool TextureExists(const std::string& name);
    std::vector<Texture>& GetTextures();
    void LoadFont();
    ivec2 GetTextureSizeByName(const char* textureName);
}