#pragma once
#include "ShaderProgram.h"
#include "mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <vector>
#include <string>

class Model
{
    // model data
    glm::mat4x4 modelMatrix = glm::mat4x4(1.f);
    glm::mat4x4 modelMatrixNormals = glm::mat4x4(1.f);
    std::vector<Mesh> meshes;
    std::vector<Texture> texturesLoaded;

    void loadModel(std::string path);
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
public:
    Model(const char* filename);

    void Draw(ShaderProgram& shader);

    glm::mat4x4 getModelMatrix() const;
    void setModelMatrix(glm::mat4x4 modelMatrix_);
};

