#include <iostream>
#include "GL_texture.h"
#include "../../../Util.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

GLuint64 OpenGLTexture::GetBindlessID() {
    return bindlessID;
}

OpenGLTexture::OpenGLTexture(const std::string filepath) {
    Load(filepath);
}

bool OpenGLTexture::Load(const std::string filepath) {

    if (!Util::FileExists(filepath)) {
        std::cout << filepath << " does not exist.\n";
        return false;
    }

    _filename = Util::GetFilename(filepath);
    _filetype = Util::GetFileInfo(filepath).filetype;
    stbi_set_flip_vertically_on_load(false);
    _data = stbi_load(filepath.data(), &_width, &_height, &_NumOfChannels, 0);

    return true;
}


void OpenGLTexture::UploadToGPU(void* data, int width, int height, int channelCount) {

    if (_baked || !_data) {
        return;
    }
    _baked = true;

    if (_filetype == "exr") {
        return;
    }
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    GLint format = GL_RGB;
    if (channelCount == 4)
        format = GL_RGBA;
    if (channelCount == 1)
        format = GL_RED;

    else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }

    bindlessID = glGetTextureHandleARB(ID);
    glMakeTextureHandleResidentARB(bindlessID);
    //glMakeTextureHandleNonResidentARB(bindlessID); to unbind

    _width = width;
    _height = height;
}

bool OpenGLTexture::Bake() {

    if (_baked) {
        return true;
    }
    _baked = true;
    if (_data == nullptr && _floatData == nullptr) {
        return false;
    }
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    GLint format = GL_RGB;
    if (_NumOfChannels == 4)
        format = GL_RGBA;
    if (_NumOfChannels == 1)
        format = GL_RED;

    if (_filetype == "exr") {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16, _width, _height, 0, GL_RGBA, GL_FLOAT, _floatData);
        free(_floatData);
    }
    else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _width, _height, 0, format, GL_UNSIGNED_BYTE, _data);
        stbi_image_free(_data);
    }

    bindlessID = glGetTextureHandleARB(ID);
    glMakeTextureHandleResidentARB(bindlessID);
    //glMakeTextureHandleNonResidentARB(bindlessID); to unbind

    _data = nullptr;
    return true;
}

unsigned int OpenGLTexture::GetID() {
    return ID;
}

void OpenGLTexture::Bind(unsigned int slot) {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, ID);
}

int OpenGLTexture::GetWidth() {
    return _width;
}

int OpenGLTexture::GetHeight() {
    return _height;
}

std::string& OpenGLTexture::GetFilename() {
    return _filename;
}

std::string& OpenGLTexture::GetFiletype() {
    return _filetype;
}

bool OpenGLTexture::IsBaked() {
    return _baked;
}
