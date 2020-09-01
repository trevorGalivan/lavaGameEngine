#pragma once
#include <glm/glm.hpp>

#include <memory>

class Engine;

void makeCowboy(Engine& engine, glm::vec2 position, int playerNum);
void makeAstronaut(Engine& engine, glm::vec2 position, int playerNum);
void makeRobot(Engine& engine, glm::vec2 position, int playerNum);