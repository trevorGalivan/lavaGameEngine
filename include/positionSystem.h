#pragma once
#include "system.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class Engine;
class PositionSystem;
// General purpose rotation / position data store

class PositionComponent : public Component<PositionComponent>
{
public:
	typedef PositionSystem owningSystem;
	glm::dvec3 position; // (+x, +y, +z) -> (right, up, into screen)
	glm::dquat rotation;

	void setPos(glm::dvec3 pos) { position = pos; }
	void setPos(glm::dvec2 pos) { position.x = pos.x; position.y = pos.y; }
	void setAngle(double angle) { rotation = glm::rotate(glm::dquat(1, 0, 0, 0), angle, glm::dvec3(0, 0, 1)); }
};

class PositionSystem : System
{
	ComponentManager<PositionComponent> locations;
public:
	PositionComponent* getComponent(handle<PositionComponent> ID);
	void makePositionComponent(Entity &parent, glm::dvec3 position = glm::dvec3(0.), glm::dquat rotation = glm::dquat(1., 0., 0., 0.));
	void destroyComponent(handle<PositionComponent> ID);
};
