#pragma once
#include "system.h"

#include <glm/glm.hpp>

#include <string>

class Engine;

// constructs various entities used in the game

// creates a damaging projectile
Entity& makeBullet(Engine& engine, unsigned int team, glm::dvec2 position, glm::dvec2 velocity, double damage, glm::dvec2 halfExtents, std::string sprite, double lifespan = 10.);

// creates a prop, which will home in on its anchor point with perfect damping. Can be constrained fo follow anchor point exactly on given axis.
Entity& makeProp(Engine& engine, handle<Entity> parent, glm::dvec2 halfExtents, bool constrainX, bool constrainY, double springConstant, std::string sprite);

