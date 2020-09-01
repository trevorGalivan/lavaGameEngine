#include "skybox.h"

#include "fileUtils.h"
#include "ShaderProgram.h"

#include "stb_image.h"
#include <glm/glm.hpp>
#include <glad/glad.h>

#include <iostream>

Skybox::Skybox(std::array<std::string, 6> paths) {
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texID);

    std::string directory = fileUtils::getAssetsDirectory();
    for (int i = 0; i < 6; i++) {
        std::string filename = directory + paths[i];

        int width, height, nrComponents;
        stbi_set_flip_vertically_on_load(false);
        unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
        if (data)
        {
            GLenum format;
            if (nrComponents == 1) {
                format = GL_RED;
            }
            else if (nrComponents == 3) {
                format = GL_RGB;
            }
            else if (nrComponents == 4) {
                format = GL_RGBA;
            }
            else { // Error
                format = GL_RED;
                std::cerr << "Unsupported number of components \'" << nrComponents << "\' in image " << filename << std::endl;
            }

            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, data
            );

            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

            stbi_image_free(data);
        }
        else
        {
            std::cout << "Texture failed to load at path: " << filename << std::endl;
            std::cout << "Reason: " << stbi_failure_reason();// << std::endl;
            std::cout << ' ' << errno << std::endl;
            stbi_image_free(data);
        }
    }

    glm::vec3 cube[8] = {
        {1, 1, 1},
        {-1, 1, 1},
        {-1, -1, 1},
        {1, -1, 1},
        {1, 1, -1},
        {-1, 1, -1},
        {-1, -1, -1},
        {1, -1, -1},
    };

    unsigned int indices[36] = {
        0, 2, 1, 0, 3, 2,
        0, 1, 5, 0, 5, 4,
        0, 7, 3, 0, 4, 7,
        6, 1, 2, 1, 6, 5,
        6, 3, 7, 6, 2, 3,
        5, 7, 4, 6, 7, 5,
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(glm::vec3), cube, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(unsigned int),
        &indices[0], GL_STATIC_DRAW);

    // vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

    glBindVertexArray(0);
    stbi_set_flip_vertically_on_load(true);
}

Skybox::~Skybox()
{
    glDeleteTextures(1, &texID);
    glDeleteBuffers(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Skybox::Draw(ShaderProgram& shaderProg) { // Assumes the correct VAO already bound


    glActiveTexture(GL_TEXTURE0);
    shaderProg.setInt("skybox", 0);
    glBindTexture(GL_TEXTURE_2D, texID);


    glActiveTexture(GL_TEXTURE0);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}