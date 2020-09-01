#include "mesh.h"

#include <glad/glad.h>

#include <iostream>
Mesh::Mesh(std::vector<Vertex> vertices_, std::vector<unsigned int> indices_, std::vector<Texture> textures_) : vertices(vertices_), indices(indices_), textures(textures_)
{
    setupMesh();
}

void Mesh::setupMesh()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
        &indices[0], GL_STATIC_DRAW);

    // vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    // vertex tangents
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
    // vertex texture coords
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));

    glBindVertexArray(0);
}

void Mesh::Draw(ShaderProgram& shader)
{
    unsigned int diffuseNr = 0;
    unsigned int MRANr = 0;
    unsigned int normalNr = 0;
    for (unsigned int i = 0; i < textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i); // activate proper texture unit before binding
        // retrieve texture number (the N in diffuse_textureN)
        std::string number;
        std::string name = textures[i].type;
        if (name == "texture_diffuse")
            number = std::to_string(diffuseNr++);
        else if (name == "texture_MRA")
            number = std::to_string(MRANr++);
        else if (name == "texture_normal")
            number = std::to_string(normalNr++);
        shader.setInt(("material." + name + number).c_str(), i);
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }
    glActiveTexture(GL_TEXTURE0);

    // draw mesh
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Mesh::Bind()
{
    glBindVertexArray(VAO);
}
std::unique_ptr<Mesh> makeGrid(glm::vec2 size, glm::uvec2 faces)
{
    std::unique_ptr<Mesh> mesh;
    std::vector<Vertex> verts;
    verts.reserve((faces.x + 1) * (faces.x + 1));
    std::vector<unsigned int> indices;
    indices.reserve((faces.x) * (faces.y) * 6);
    glm::vec2 faceSize = 2.f * size / glm::vec2(faces);
    
    for (unsigned int row = 0; row <= faces.y; ++row) {
        for (unsigned int col = 0; col <= faces.x; ++col) {
            verts.push_back(Vertex{ glm::vec3(col * faceSize.x - size.x, 0, row * faceSize.y - size.y), glm::vec3(0,1,0), glm::vec3(1,0,0), glm::vec2(col, row) / glm::vec2(faces) });

        }
    }
    for (unsigned int i = 0; i < faces.x; ++i) {
        for (unsigned int j = 0; j < faces.y; ++j) {
            indices.push_back(i  + j    * (faces.x + 1));
            indices.push_back(i  + (j+1)* (faces.x + 1));
            indices.push_back(i + 1 + j * (faces.x + 1));
            

            indices.push_back(i + 1 + (j + 1) * (faces.x + 1));
            indices.push_back(i + 1 + j * (faces.x + 1));
            indices.push_back(i   + (j+1) * (faces.x + 1));
        }
    }
    mesh = std::make_unique<Mesh>(std::move(verts), std::move(indices), std::vector<Texture>());
    return mesh;
}
