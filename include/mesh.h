#pragma once
#include "ShaderProgram.h"

#include <glm/glm.hpp>

#include <string>
#include <vector>

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
    glm::vec3 tangent;
	glm::vec2 texCoord; 
};

struct Texture {
	unsigned int id; // OpenGL object ID
    std::string type;
    std::string path; 
};

class Mesh {
public:
    // mesh data
    std::vector<Vertex>       vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture>      textures;

    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
    void Draw(ShaderProgram& shader);
    void Bind();
private:
    //  render data
    unsigned int VAO, VBO, EBO;

    void setupMesh();
};

std::unique_ptr<Mesh> makeGrid(glm::vec2 size, glm::uvec2 faces);