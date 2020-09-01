#include "sprite.h"

#include "fileUtils.h"

#include "stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glad/glad.h>

#include <iostream>
#include <fstream>
#include <array>


void makeMeshFromThicknessMap(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices, unsigned char* data, int width, int height, int nrChannels, double fudge = 0.03);

Sprite::Sprite(const char* filename)
{
    try {
        std::string path = fileUtils::getAssetsDirectory() + filename;
        std::ifstream myfile;
        myfile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        myfile.open(path);

        myfile >> numFrames;
        myfile >> animLength;

        std::string imageType; // diffuse, MRA, normal
        std::string imagePath;

        while (!myfile.eof() && myfile >> imageType >> imagePath) {

            bool isStatic = false; // this texture stays the same thoughout the animation
            bool isBilinear = false;
            bool isSRGB = false;
            {
                int pos;
                while ((pos = imagePath.find_first_of("~!?")) != std::string::npos) {
                    char modifier = imagePath[pos];
                    imagePath.erase(pos, 1); // gets rid of the modifier
                    switch (modifier) {
                    case '~': isStatic = true; break;
                    case '!': isBilinear = true; break;
                    case '?': isSRGB = true; break;
                    }
                }
            }

            int width, totalHeight, nrComponents;

            unsigned char* rawImage = stbi_load((fileUtils::getAssetsDirectory() + imagePath).c_str(), &width, &totalHeight, &nrComponents, 0);

            int height = isStatic ? totalHeight : (totalHeight / numFrames); // height of one frame of the image

            std::vector<unsigned int>* destType = nullptr;

            if (imageType == "diffuse") {
                destType = &diffuse;
            }
            else if (imageType == "MREA") {
                destType = &MREA;
            }
            else if (imageType == "normal") {
                destType = &normal;
            }
            else if (imageType == "thickness") {
                destType = nullptr; // make sure image isnt loaded as texture
                for (int frameNumber = 0; frameNumber < (isStatic ? 1 : numFrames); frameNumber++) {
                    std::vector<Vertex> vertices;
                    std::vector<unsigned int> indices;
                    double fudge = isBilinear ? 0. : 0.08;
                    makeMeshFromThicknessMap(vertices, indices, rawImage + width * height * nrComponents * frameNumber, width, height, nrComponents, fudge);
                    unsigned int tempVAO, tempVBO, tempEBO;
                    glGenVertexArrays(1, &tempVAO);
                    glGenBuffers(1, &tempVBO);
                    glGenBuffers(1, &tempEBO);

                    glBindVertexArray(tempVAO);
                    glBindBuffer(GL_ARRAY_BUFFER, tempVBO);

                    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tempEBO);
                    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

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

                    numIndices.push_back(indices.size());
                    VBOs.push_back(tempVBO);
                    EBOs.push_back(tempEBO);
                    VAOs.push_back(tempVAO);
                    isFlat = false;
                }
            }

            if (destType) {
                if (rawImage)
                {
                    for (int frameNumber = 0; frameNumber < (isStatic ? 1 : numFrames); frameNumber++) {
                        unsigned int frameOffset = width * height * nrComponents * frameNumber;
                        unsigned int textureID = fileUtils::makeGLtexture2D(rawImage + frameOffset, width, height, nrComponents, isSRGB, isBilinear);

                        destType->push_back(textureID);
                    }
                }
                else
                {
                    std::cout << "Texture failed to load at path: " << imagePath << std::endl;
                }
            }
            else {
                if (imageType != "thickness")
                    std::cerr << "Unsupported texture type \'" << imageType << "\' in sprite file " << path << std::endl;
            }
            stbi_image_free(rawImage);
        }
        if (diffuse.size() != numFrames) {
            std::cout << "Warning, animation " << path << " is " << numFrames << " long, but only " << diffuse.size() << " diffuse frames loaded" << std::endl;
        }
        if (diffuse.size() == 0 || MREA.size() == 0 || normal.size() == 0) {
            std::cout << "Warning, sprite at path  " << path << " is incomplete!" << std::endl;
        }
    }catch(std::exception& e){
        std::cerr << e.what() << std::endl;
        std::cerr << "reading " << filename << std::endl;
    }
}

float thickness(unsigned char pixel) { // converts pixel from heightmap into normalized height. Returns negative value if pixel is zero (indicating no geometry present)
    return (pixel-1)/(254.f) / 10.f;
}

// Uses thickness map to make the mesh. Only considers the red channel if there are multiple channels
void makeMeshFromThicknessMap(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices, unsigned char* data, int width, int height, int nrChannels, double fudge) {
    double pixHeight = 2.f / height;
    double pixWidth = 2.f / width;
    // makes front-facing faces
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            unsigned char pixel = data[i * nrChannels + j * width * nrChannels];
            float pixDepth = thickness(pixel);
            if (pixDepth < 0) {
                continue;
            };
            glm::dvec3 pos = glm::vec3(( (i - width / 2.) * pixWidth  ), ((j - height / 2.) * pixHeight ), pixDepth);

            glm::dvec2 TLtex(pos.x, pos.y );
            TLtex = TLtex / 2. + 0.5;
            glm::dvec2 BRtex(pos.x + pixWidth, pos.y - pixHeight);
            BRtex = BRtex / 2. + 0.5;
            TLtex.y += pixHeight/2;
            BRtex.y += pixHeight/2;


            // fudge the texture coords slightly, to avoid shimmering artifacts around sharp corners with MSAA
            glm::dvec2 fudgeVec = glm::dvec2(fudge, -fudge) * glm::dvec2(pixWidth, pixHeight);
            TLtex += fudgeVec;
            BRtex -= fudgeVec;

            std::array<Vertex, 4> unitsquare =
            {//  position                                       normal coords       tangent coords      texture coords
                Vertex{pos + glm::dvec3(0, pixHeight, 0)       , glm::vec3(0, 0, 1), glm::vec3(1, 0, 0), glm::vec2(TLtex)},
                Vertex{pos                                    , glm::vec3(0, 0, 1), glm::vec3(1, 0, 0), glm::vec2(TLtex.x, BRtex.y)},
                Vertex{pos + glm::dvec3(pixWidth, pixHeight, 0), glm::vec3(0, 0, 1), glm::vec3(1, 0, 0), glm::vec2(BRtex.x, TLtex.y)},
                Vertex{pos + glm::dvec3(pixWidth, 0, 0)        , glm::vec3(0, 0, 1), glm::vec3(1, 0, 0), glm::vec2(BRtex)},
            };

            auto firstIndex = vertices.size();
            vertices.push_back(unitsquare[0]);
            vertices.push_back(unitsquare[1]);
            vertices.push_back(unitsquare[2]);
            vertices.push_back(unitsquare[3]);

            indices.push_back(firstIndex);
            indices.push_back(firstIndex+1);
            indices.push_back(firstIndex+2);
            indices.push_back(firstIndex+2);
            indices.push_back(firstIndex+1);
            indices.push_back(firstIndex+3);

            if (pixel > 1) { // generate sides
                if (i == 0 || data[(i - 1) * nrChannels + j * width * nrChannels] < pixel) { // checks if we need to gen left side

                    std::array<Vertex, 4> leftSide =
                    {//  position                                          normal coords       tangent coords      texture coords
                        Vertex{pos + glm::dvec3(0, pixHeight, 0)          , glm::vec3(-1, 0, 0), glm::vec3(0, 0, 1), glm::vec2(TLtex)},
                        Vertex{pos                                       , glm::vec3(-1, 0, 0), glm::vec3(0, 0, 1), glm::vec2(TLtex.x, BRtex.y)},
                        Vertex{pos + glm::dvec3(0, pixHeight, -pixDepth*2), glm::vec3(-1, 0, 0), glm::vec3(0, 0, 1), glm::vec2(BRtex.x, TLtex.y)},
                        Vertex{pos + glm::dvec3(0, 0, -pixDepth*2)        , glm::vec3(-1, 0, 0), glm::vec3(0, 0, 1), glm::vec2(BRtex)},
                    };
                    firstIndex = vertices.size();
                    vertices.push_back(leftSide[0]);
                    vertices.push_back(leftSide[1]);
                    vertices.push_back(leftSide[2]);
                    vertices.push_back(leftSide[3]);

                    indices.push_back(firstIndex);
                    indices.push_back(firstIndex + 2);
                    indices.push_back(firstIndex + 1);
                    indices.push_back(firstIndex + 2);
                    indices.push_back(firstIndex + 3);
                    indices.push_back(firstIndex + 1);
                }

                if (i == width-1 || data[(i + 1) * nrChannels + j * width * nrChannels] < pixel) { // checks if we need to gen right side

                    std::array<Vertex, 4> rightSide =
                    {//  position                                            normal coords       tangent coords      texture coords
                        Vertex{pos + glm::dvec3(pixWidth, pixHeight, -pixDepth * 2), glm::vec3(1, 0, 0), glm::vec3(0, 0, -1), glm::vec2(TLtex)},
                        Vertex{pos + glm::dvec3(pixWidth, 0, -pixDepth * 2)        , glm::vec3(1, 0, 0), glm::vec3(0, 0, -1), glm::vec2(TLtex.x, BRtex.y)},
                        Vertex{pos + glm::dvec3(pixWidth, pixHeight, 0)            , glm::vec3(1, 0, 0), glm::vec3(0, 0, -1), glm::vec2(BRtex.x, TLtex.y)},
                        Vertex{pos + glm::dvec3(pixWidth, 0, 0)                    , glm::vec3(1, 0, 0), glm::vec3(0, 0, -1), glm::vec2(BRtex)},
                    };
                    firstIndex = vertices.size();
                    vertices.push_back(rightSide[0]);
                    vertices.push_back(rightSide[1]);
                    vertices.push_back(rightSide[2]);
                    vertices.push_back(rightSide[3]);

                    indices.push_back(firstIndex);
                    indices.push_back(firstIndex + 2);
                    indices.push_back(firstIndex + 1);
                    indices.push_back(firstIndex + 2);
                    indices.push_back(firstIndex + 3);
                    indices.push_back(firstIndex + 1);
                }

                if (j == height - 1 || data[i * nrChannels + (j+1) * width * nrChannels] < pixel) { // checks if we need to gen top side

                    std::array<Vertex, 4> topSide =
                    {//  position                                                   normal coords       tangent coords      texture coords
                        Vertex{pos + glm::dvec3(0, pixHeight, 0)                   , glm::vec3(0, 1, 0), glm::vec3(1, 0, 0), glm::vec2(TLtex)},
                        Vertex{pos + glm::dvec3(0, pixHeight, -pixDepth*2)         , glm::vec3(0, 1, 0), glm::vec3(1, 0, 0), glm::vec2(TLtex.x, BRtex.y)},
                        Vertex{pos + glm::dvec3(pixWidth, pixHeight, 0)            , glm::vec3(0, 1, 0), glm::vec3(1, 0, 0), glm::vec2(BRtex.x, TLtex.y)},
                        Vertex{pos + glm::dvec3(pixWidth, pixHeight, -pixDepth * 2), glm::vec3(0, 1, 0), glm::vec3(1, 0, 0), glm::vec2(BRtex)},
                    };
                    firstIndex = vertices.size();
                    vertices.push_back(topSide[0]);
                    vertices.push_back(topSide[1]);
                    vertices.push_back(topSide[2]);
                    vertices.push_back(topSide[3]);

                    indices.push_back(firstIndex);
                    indices.push_back(firstIndex + 2);
                    indices.push_back(firstIndex + 1);
                    indices.push_back(firstIndex + 2);
                    indices.push_back(firstIndex + 3);
                    indices.push_back(firstIndex + 1);
                }

                if (j == 0 || data[i * nrChannels + (j-1) * width * nrChannels] < pixel) { // checks if we need to gen bottom side

                    std::array<Vertex, 4> bottomSide =
                    {//  position                                                   normal coords       tangent coords      texture coords
                        Vertex{pos + glm::dvec3(0, 0, -pixDepth * 2)       , glm::vec3(0, -1, 0), glm::vec3(1, 0, 0), glm::vec2(TLtex)},
                        Vertex{pos                                        , glm::vec3(0, -1, 0), glm::vec3(1, 0, 0), glm::vec2(TLtex.x, BRtex.y)},
                        Vertex{pos + glm::dvec3(pixWidth, 0, -pixDepth * 2), glm::vec3(0, -1, 0), glm::vec3(1, 0, 0), glm::vec2(BRtex.x, TLtex.y)},
                        Vertex{pos + glm::dvec3(pixWidth, 0, 0)            , glm::vec3(0, -1, 0), glm::vec3(1, 0, 0), glm::vec2(BRtex)},
                    };
                    firstIndex = vertices.size();
                    vertices.push_back(bottomSide[0]);
                    vertices.push_back(bottomSide[1]);
                    vertices.push_back(bottomSide[2]);
                    vertices.push_back(bottomSide[3]);

                    indices.push_back(firstIndex);
                    indices.push_back(firstIndex + 2);
                    indices.push_back(firstIndex + 1);
                    indices.push_back(firstIndex + 2);
                    indices.push_back(firstIndex + 3);
                    indices.push_back(firstIndex + 1);
                }
            }
        } 
    }
    // TODO: optimize mesh by combining triangles where possible
}

void Sprite::Draw(ShaderProgram& shaderProg, double& animPosition, glm::vec3 position, glm::vec2 halfExtents, glm::quat rotation /*= glm::quat(1, 0, 0, 0)*/, bool flipX /*= false*/) { // Assumes the correct VAO already bound
    
    animPosition = std::fmod(animPosition, animLength);
    unsigned int diffTex = diffuse[size_t(animPosition * diffuse.size() / animLength)];
    unsigned int MREATex = MREA[size_t(animPosition * MREA.size() / animLength)];
    unsigned int normTex = normal[size_t(animPosition * normal.size() / animLength)];

    glActiveTexture(GL_TEXTURE0);
    shaderProg.setInt("material.texture_diffuse0", 0);
    glBindTexture(GL_TEXTURE_2D, diffTex);

    glActiveTexture(GL_TEXTURE1);
    shaderProg.setInt("material.texture_MREA0", 1);
    glBindTexture(GL_TEXTURE_2D, MREATex);

    glActiveTexture(GL_TEXTURE2);
    shaderProg.setInt("material.texture_normal0", 2);
    glBindTexture(GL_TEXTURE_2D, normTex);


    glActiveTexture(GL_TEXTURE0);
    if (flipX)
        halfExtents.x = -halfExtents.x;

    auto modelMatrix = glm::translate(glm::mat4x4(1), position) * glm::toMat4(rotation) * glm::scale(glm::vec3(halfExtents, 1));
    auto modelMatrixNormals = glm::transpose(glm::inverse(glm::mat3x3( modelMatrix )));
    shaderProg.setMat4x4("toWorld", modelMatrix);
    shaderProg.setMat4x4("toWorld_normal", modelMatrixNormals);

    if (isFlat) {
        // draw mesh
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
    else {
        auto modelNum = size_t(animPosition * VAOs.size() / animLength);
        glBindVertexArray(VAOs[modelNum]);
        glDrawElements(GL_TRIANGLES, numIndices[modelNum], GL_UNSIGNED_INT, 0);
    }
}
