#pragma once
#include "mesh.h"

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <optional>

class Sprite
{
    int numFrames;
    float animLength;

    std::vector<unsigned int> diffuse;
    std::vector<unsigned int> MREA;
    std::vector<unsigned int> normal;
    
    std::vector<unsigned int> VAOs, VBOs, EBOs;
    std::vector<unsigned int> numIndices;
public:
    bool isFlat = true;

    Sprite(const char* filename);
    void Draw(ShaderProgram& shader, double& animPosition, glm::vec3 position, glm::vec2 halfExtents, glm::quat rotation = glm::quat(1,0,0,0), bool flipX = false);
    void update(float dt);
};

