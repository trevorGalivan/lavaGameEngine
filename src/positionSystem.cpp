#include "positionSystem.h"

#include "engine.h"

PositionComponent* PositionSystem::getComponent(handle<PositionComponent> ID)
{
	return locations.find(ID);
}

void PositionSystem::makePositionComponent(Entity& parent, glm::dvec3 position /* = glm::vec3(0.) */, glm::dquat rotation /* = glm::quat(0.) */)
{
	PositionComponent component;
	component.position = position;
	component.rotation = rotation;
	component.entity_ID = parent.ID;

	parent.positionHandle = locations.add(std::move(component));
}

void PositionSystem::destroyComponent(handle<PositionComponent> ID)
{
	locations.remove(ID);
}

