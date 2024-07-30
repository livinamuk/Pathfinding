#define TINYOBJLOADER_IMPLEMENTATION
#include "AssetManager.h"
#include <future>
#include <thread>
#include <stb_image.h>
#include "../API/OpenGL/GL_backEnd.h"
#include "../API/OpenGL/GL_renderer.h"
#include "../BackEnd/BackEnd.h"
#include "../Util.hpp"

namespace AssetManager {

    std::vector<std::string> _texturePaths;
    std::vector<std::string> _loadLog;
    bool _hardCodedModelsCreated = false;
    bool _finalInitComplete = false;

    std::vector<Vertex> _vertices;
    std::vector<uint32_t> _indices;

    std::vector<Mesh> _meshes;
    std::vector<Model> _models;
    std::vector<Texture> _textures;

    // Used to new data insert into the vectors above
    int _nextVertexInsert = 0;
    int _nextIndexInsert = 0;

    int _quadMeshIndex = 0;

    // Async
    std::mutex _texturesMutex;
    std::mutex _consoleMutex;
    std::vector<std::future<void>> _futures;
}

//                         //
//      Asset Loading      //
//                         //

void AssetManager::FindAssetPaths() {

    auto texturePaths = std::filesystem::directory_iterator("res/textures/");
    for (const auto& entry : texturePaths) {
        FileInfo info = Util::GetFileInfo(entry);
        if (info.filetype == "png" || info.filetype == "jpg" || info.filetype == "tga") {
            _texturePaths.push_back(info.fullpath.c_str());
        }
    }
}

void AssetManager::AddItemToLoadLog(std::string item) {
    _loadLog.push_back(item);
}

std::vector<std::string>& AssetManager::GetLoadLog() {
    return _loadLog;
}

bool AssetManager::LoadingComplete() {
    return (
        _texturePaths.empty() &&
        _hardCodedModelsCreated &&
        _finalInitComplete
        );
}

void AssetManager::LoadNextItem() {

    if (!_hardCodedModelsCreated) {
        CreateHardcodedModels();
        AddItemToLoadLog("Building Hardcoded Mesh");
        _hardCodedModelsCreated = true;
        return;
    }
    if (_texturePaths.size()) {
        if (BackEnd::GetAPI() == API::VULKAN && Util::GetFileInfo(_texturePaths[0]).filetype == "exr") {
            _texturePaths.erase(_texturePaths.begin());
            return;
        }
        Texture& texture = _textures.emplace_back();
        texture.Load(_texturePaths[0].c_str());
        AddItemToLoadLog(_texturePaths[0]);
        _texturePaths.erase(_texturePaths.begin());
        return;
    }
    if (!_finalInitComplete) {
        if (BackEnd::GetAPI() == API::OPENGL) {
            OpenGLRenderer::BindBindlessTextures();
        }
        _finalInitComplete = true;
    }
}

void AssetManager::LoadFont() {
    static auto paths = std::filesystem::directory_iterator("res/textures/font/");
    for (const auto& entry : paths) {
        FileInfo info = Util::GetFileInfo(entry);
        if (info.filetype == "png" || info.filetype == "jpg" || info.filetype == "tga") {
            Texture& texture = _textures.emplace_back();
            texture.Load(info.fullpath);
        }
    }
    if (BackEnd::GetAPI() == API::OPENGL) {
        OpenGLRenderer::BindBindlessTextures();
    }
}

std::vector<Vertex>& AssetManager::GetVertices() {
    return _vertices;
}

std::vector<uint32_t>& AssetManager::GetIndices() {
    return _indices;
}

void AssetManager::UploadVertexData() {
    if (BackEnd::GetAPI() == API::OPENGL) {
        OpenGLBackEnd::UploadVertexData(_vertices, _indices);
    }
}

Model* AssetManager::GetModelByIndex(int index) {
    if (index >= 0 && index < _models.size()) {
        return &_models[index];
    }
    else {
        std::cout << "AssetManager::GetModelByIndex() failed because index '" << index << "' is out of range. Size is " << _models.size() << "!\n";
        return nullptr;
    }
}

int AssetManager::GetModelIndexByName(const std::string& name) {
    for (int i = 0; i < _models.size(); i++) {
        if (_models[i].GetName() == name) {
            return i;
        }
    }
    std::cout << "AssetManager::GetModelIndexByName() failed because name '" << name << "' was not found in _models!\n";
    return -1;
}

bool AssetManager::ModelExists(const std::string& filename) {
    for (Model& texture : _models)
        if (texture.GetName() == filename)
            return true;
    return false;
}

int AssetManager::CreateMesh(std::string name, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, glm::vec3 aabbMin, glm::vec3 aabbMax) {

    Mesh& mesh = _meshes.emplace_back();
    mesh.baseVertex = _nextVertexInsert;
    mesh.baseIndex = _nextIndexInsert;
    mesh.vertexCount = (uint32_t)vertices.size();
    mesh.indexCount = (uint32_t)indices.size();
    mesh.name = name;
    mesh.aabbMin = aabbMin;
    mesh.aabbMax = aabbMax;

    _vertices.reserve(_vertices.size() + vertices.size());
    _vertices.insert(std::end(_vertices), std::begin(vertices), std::end(vertices));

    _indices.reserve(_indices.size() + indices.size());
    _indices.insert(std::end(_indices), std::begin(indices), std::end(indices));

    _nextVertexInsert += mesh.vertexCount;
    _nextIndexInsert += mesh.indexCount;

    return _meshes.size() - 1;
}

unsigned int AssetManager::GetQuadMeshIndex() {
    return _quadMeshIndex;
}

Mesh* AssetManager::GetQuadMesh() {
    return &_meshes[_quadMeshIndex];
}

Mesh* AssetManager::GetMeshByIndex(int index) {
    if (index >= 0 && index < _meshes.size()) {
        return &_meshes[index];
    }
    else {
        std::cout << "AssetManager::GetMeshByIndex() failed because index '" << index << "' is out of range. Size is " << _meshes.size() << "!\n";
        return nullptr;
    }
}

int AssetManager::GetMeshIndexByName(const std::string& name) {
    for (int i = 0; i < _meshes.size(); i++) {
        if (_meshes[i].name == name) {
            return i;
        }
    }
    std::cout << "AssetManager::GetMeshIndexByName() failed because name '" << name << "' was not found in _meshes!\n";
    return -1;
}

void AssetManager::LoadTexture(const std::string filepath) {

    int textureIndex = -1;
    int width = 0;
    int height = 0;
    int channelCount = 0;
    unsigned char* data = nullptr;
    std::string filename = Util::GetFilename(filepath);

    if (BackEnd::GetAPI() == API::OPENGL) {

        //CMP_Texture cmpTexture;
        CMP_Texture destTexture;
        bool compressed = true;

        if (!Util::FileExists(filepath)) {
            std::cout << filepath << " does not exist.\n";
        }

        // Check if compressed version exists. If not, create one.
        std::string suffix = filename.substr(filename.length() - 3);
        std::string compressedPath = "res/assets/" + filename + ".dds";

        stbi_set_flip_vertically_on_load(false);
        data = stbi_load(filepath.data(), &width, &height, &channelCount, 0);
        std::lock_guard<std::mutex> lock(_texturesMutex);
        textureIndex = _textures.size();
        Texture& texture = _textures.emplace_back(Texture(filename, width, height, channelCount));
        texture.GetGLTexture().UploadToGPU(data, width, height, channelCount);

        return;
    }
}

int AssetManager::GetTextureCount() {
    return _textures.size();
}

int AssetManager::GetTextureIndexByName(const std::string& name, bool ignoreWarning) {
    for (int i = 0; i < _textures.size(); i++) {
        if (_textures[i].GetFilename() == name) {
            return i;
        }
    }
    if (!ignoreWarning) {
        std::cout << "AssetManager::GetTextureIndex() failed because '" << name << "' does not exist\n";
;   }
    return -1;
}

Texture* AssetManager::GetTextureByIndex(const int index) {
    if (index >= 0 && index < _textures.size()) {
        return &_textures[index];
    }
    std::cout << "AssetManager::GetTextureByIndex() failed because index '" << index << "' is out of range. Size is " << _textures.size() << "!\n";
    return nullptr;
}

Texture* AssetManager::GetTextureByName(const std::string& name) {
    for (Texture& texture : _textures) {
        if (texture.GetFilename() == name)
            return &texture;
    }
    std::cout << "AssetManager::GetTextureByName() failed because '" << name << "' does not exist\n";
    return nullptr;
}

bool AssetManager::TextureExists(const std::string& filename) {
    for (Texture& texture : _textures)
        if (texture.GetFilename() == filename)
            return true;
    return false;
}

std::vector<Texture>& AssetManager::GetTextures() {
    return _textures;
}


//////////////////////////
//                      //
//      Textures        //

ivec2 AssetManager::GetTextureSizeByName(const char* textureName) {

    static std::unordered_map<const char*, int> textureIndices;
    if (textureIndices.find(textureName) == textureIndices.end()) {
        textureIndices[textureName] = AssetManager::GetTextureIndexByName(textureName);
    }
    Texture* texture = AssetManager::GetTextureByIndex(textureIndices[textureName]);
    if (texture) {
        return ivec2(texture->GetWidth(), texture->GetHeight());
    }
    else {
        return ivec2(0, 0);
    }
}

void AssetManager::CreateHardcodedModels() {

    /* Quad */ {
        Vertex vertA, vertB, vertC, vertD;
        vertA.position = { -1.0f, -1.0f, 0.0f };
        vertB.position = { -1.0f, 1.0f, 0.0f };
        vertC.position = { 1.0f,  1.0f, 0.0f };
        vertD.position = { 1.0f,  -1.0f, 0.0f };
        vertA.uv = { 0.0f, 0.0f };
        vertB.uv = { 0.0f, 1.0f };
        vertC.uv = { 1.0f, 1.0f };
        vertD.uv = { 1.0f, 0.0f };
        vertA.normal = glm::vec3(0, 0, 1);
        vertB.normal = glm::vec3(0, 0, 1);
        vertC.normal = glm::vec3(0, 0, 1);
        vertD.normal = glm::vec3(0, 0, 1);
        vertA.tangent = glm::vec3(1, 0, 0);
        vertB.tangent = glm::vec3(1, 0, 0);
        vertC.tangent = glm::vec3(1, 0, 0);
        vertD.tangent = glm::vec3(1, 0, 0);
        std::vector<Vertex> vertices;
        vertices.push_back(vertA);
        vertices.push_back(vertB);
        vertices.push_back(vertC);
        vertices.push_back(vertD);
        std::vector<uint32_t> indices = { 2, 1, 0, 3, 2, 0 };
        std::string name = "Quad";

        Model& model = _models.emplace_back();
        model.SetName(name);
        model.AddMeshIndex(AssetManager::CreateMesh("Quad", vertices, indices));
    }

    UploadVertexData();
}