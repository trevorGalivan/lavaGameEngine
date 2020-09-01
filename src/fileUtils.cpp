#include "fileUtils.h"

#include <glad/glad.h>
#include "stb_image.h"

#include <filesystem>
#include <iostream>

std::string fileUtils::getAssetsDirectory()
{
	return std::filesystem::current_path().string() + "/assets/";
}

std::string fileUtils::getShadersDirectory()
{
	return std::filesystem::current_path().string() + "/shaders/";
}

// loads the given texture data into an openGL texture, and returns the texture ID
unsigned int fileUtils::makeGLtexture2D(unsigned char* rawData, int width, int height, int nrComponents, bool isSRGB, bool isBilinear)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    GLenum format;
    GLenum internalFormat;
    if (nrComponents == 1) {
        format = GL_RED;
        internalFormat = GL_RED;
    }
    else if (nrComponents == 3) {
        format = GL_RGB;
        internalFormat = isSRGB ? GL_SRGB8 : GL_RGB;
    }
    else if (nrComponents == 4) {
        format = GL_RGBA;
        internalFormat = isSRGB ? GL_SRGB8_ALPHA8 : GL_RGBA;
    }
    else { // Error
        format = GL_RED;
        internalFormat = GL_RED;
        std::cerr << "Unsupported number of components \'" << nrComponents << "\' in image " <<  std::endl;
    }

    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, rawData);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    if (isBilinear) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, 16.f);
    }
    else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    return textureID;
}

// loads the texture at the given path into an openGL texture, and returns the texture ID
unsigned int fileUtils::makeGLtexture2D(const std::string& filename, bool isSRGB, bool isBilinear)
{
    int width, height, nrchannels;
    unsigned char* data = stbi_load((fileUtils::getAssetsDirectory() + filename).c_str(), &width, &height, &nrchannels, 0);

    if (data == nullptr) {
        std::cerr << "Image at " << (fileUtils::getAssetsDirectory() + filename) << " failed to load" << std::endl;
        std::cerr << "Reason: " << stbi_failure_reason();// << std::endl;
        std::cerr << ' ' << errno << std::endl;
        stbi_image_free(data);
        return 0;
    }
    int ID = fileUtils::makeGLtexture2D(data, width, height, nrchannels, isSRGB, isBilinear);
    stbi_image_free(data);
    return ID;
}

