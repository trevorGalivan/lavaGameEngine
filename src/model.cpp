#include "model.h"

#include "fileUtils.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <glad/glad.h>

#include <iostream>

bool isSRGB(std::string typeName) {
    if (typeName == "texture_diffuse") {
        return true;
    }
    return false;
}

glm::mat4x4 Model::getModelMatrix() const
{
    return modelMatrix;
}
void Model::setModelMatrix(glm::mat4x4 modelMatrix_)
{
    modelMatrix = modelMatrix_;
    modelMatrixNormals = glm::transpose(glm::inverse(glm::mat3x3(modelMatrix_)));
}


void Model::Draw(ShaderProgram& shaderProg)
{
    shaderProg.setMat4x4("toWorld", modelMatrix);
    shaderProg.setMat4x4("toWorld_normal", modelMatrixNormals);
    for (unsigned int i = 0; i < meshes.size(); i++)
        meshes[i].Draw(shaderProg);
}

Model::Model(const char* filename)
{
    loadModel(fileUtils::getAssetsDirectory()+filename);
}

void Model::loadModel(std::string path)
{
    stbi_set_flip_vertically_on_load(true);
    Assimp::Importer import;
    const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate |  aiProcess_CalcTangentSpace |  aiProcess_GenSmoothNormals);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
        return;
    }

    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
    // process all the node's meshes (if any)
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }


    // then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }

}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        // process vertex positions, normals and texture coordinates
        glm::vec3 vector;
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.position = vector;

        vector.x = mesh->mNormals[i].x;
        vector.y = mesh->mNormals[i].y;
        vector.z = mesh->mNormals[i].z;
        vertex.normal = vector;

        vector.x = mesh->mTangents[i].x;
        vector.y = mesh->mTangents[i].y;
        vector.z = mesh->mTangents[i].z;
        vertex.tangent = vector;

        if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.texCoord = vec;
        } else {
            vertex.texCoord = glm::vec2(0.0f, 0.0f);
        }
        vertices.push_back(vertex);
    }
    // process indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }
    // process material
    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        std::vector<Texture> diffuseMaps = loadMaterialTextures(material,
                                                                aiTextureType_DIFFUSE,
                                                                "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        std::vector<Texture> MREAMaps = loadMaterialTextures(material,
                                                                 aiTextureType_SPECULAR, 
                                                                 "texture_MREA");
        textures.insert(textures.end(), MREAMaps.begin(), MREAMaps.end());

        std::vector<Texture> normalMaps = loadMaterialTextures(material,
                                                               aiTextureType_HEIGHT,
                                                               "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
    }

    return Mesh(vertices, indices, textures);
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
{
    std::vector<Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        bool skip = false;
        for (unsigned int j = 0; j < texturesLoaded.size(); j++)
        {
            if (std::strcmp(texturesLoaded[j].path.data(), str.C_Str()) == 0)
            {
                textures.push_back(texturesLoaded[j]);
                skip = true;
                break;
            }
        }
        if (!skip)
        {   // if texture hasn't been loaded already, load it
            Texture texture;
            texture.id = fileUtils::makeGLtexture2D(str.C_Str(), isSRGB(typeName));
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            texturesLoaded.push_back(texture); // add to loaded textures
        }
    }
    return textures;
}
