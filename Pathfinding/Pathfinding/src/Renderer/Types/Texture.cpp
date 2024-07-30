#include "Texture.h"
#include "../../BackEnd/BackEnd.h"

void Texture::Load(const std::string filepath) {
    if (BackEnd::GetAPI() == API::OPENGL) {
        glTexture.Load(filepath);
        glTexture.Bake();
    }
}

int Texture::GetWidth() {
    if (BackEnd::GetAPI() == API::OPENGL) {
        return glTexture.GetWidth();
    }
}

int Texture::GetHeight() {
    if (BackEnd::GetAPI() == API::OPENGL) {
        return glTexture.GetHeight();
    }
}

std::string& Texture::GetFilename() {
    if (BackEnd::GetAPI() == API::OPENGL) {
        return glTexture.GetFilename();
    }
}

std::string& Texture::GetFiletype() {
    if (BackEnd::GetAPI() == API::OPENGL) {
        return glTexture.GetFiletype();
    }
}

OpenGLTexture& Texture::GetGLTexture() {
    return glTexture;
}