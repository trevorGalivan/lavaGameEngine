#pragma once
#include "engine.h"

#include <glm/glm.hpp>


typedef void(*CharacterFunction)(Engine&, glm::vec2 position, int playernum);
typedef void(*WorldFunction)(Engine&, const std::vector<CharacterFunction>&);

void desertWorld(Engine& engine, const std::vector<CharacterFunction>& players);
void spaceWorld(Engine& engine, const std::vector<CharacterFunction>& players);
void volcanoWorld(Engine& engine, const std::vector<CharacterFunction>& players);
