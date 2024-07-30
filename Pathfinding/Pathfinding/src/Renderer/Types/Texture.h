#pragma once
#include <Compressonator.h>
#include <string>
#include <memory>
#include "../../API/OpenGL/Types/GL_texture.h"

class Texture {

public:

	Texture() = default;

    Texture(std::string filename, int width, int height, int channelCount) {
        this->width = width;
        this->height = height;
        this->channelCount = channelCount;
        this->filename = filename;
    }

	void Load(const std::string filepath);
	int GetWidth();
	int GetHeight();
	std::string& GetFilename();
	std::string& GetFiletype();
    OpenGLTexture& GetGLTexture();

    void SetFilename(std::string name) {
        filename = name;
    }

private:
    OpenGLTexture glTexture;
    std::string filename;
    int width = 0;
    int height = 0;
    int channelCount = 0;
};
