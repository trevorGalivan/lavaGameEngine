#pragma once
#include "system.h"
#include "collider.h"
#include "entityCatagories.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <optional>

class Engine;
class PhysicsSystem;

struct PhysicsSettings {
	double dampingFactor;
	double quadraticDampingFactor;
	double gravAcceleration;
	double restitution;
	unsigned int collisionMask;
	double kineticFriction;
};

const PhysicsSettings defaultPhysicsSettings =
{
	0.05,
	0.02,
	-9.81,
	0.4,
	catagories::worldType,
	0.5
};

// expects PositionComponent
class PhysicsComponent : public Component<PhysicsComponent>
{
	unsigned int collisionCatagory; // do not set manually. Used to cache bitmask to avoid O(n^2) entity lookups during collision
	friend class PhysicsSystem;
public:
	typedef PhysicsSystem owningSystem;
	glm::dvec2 velocity;
	PhysicsSettings settings;
	
	double invMass;

	std::optional<Collider> collider; // could be broken off into seperate system to improve the len(physicsComponents) >>> len(colliders) case, at the cost of reduced locality
};

class ConstraintComponent : public Component<ConstraintComponent>
{
public:
	typedef PhysicsSystem owningSystem;
	glm::dvec2 anchorPoint;
	glm::dvec2 tolerance;
	glm::dvec2 max;
	std::optional<handle<Entity>> parent;
	double springConstant;
};


class PhysicsSystem : System
{
	ComponentManager<PhysicsComponent> bodies;
	ComponentManager<ConstraintComponent> constraints;
public:
	virtual void update(Engine& engine, double dt);
	PhysicsComponent* getComponent(handle<PhysicsComponent> ID);
	void makePhysicsComponent(Entity& parent, std::optional<Collider> collider = std::nullopt, glm::dvec2 velocity = glm::dvec2(0.), double mass = 1.,  PhysicsSettings settings = defaultPhysicsSettings);
	void destroyComponent(handle<PhysicsComponent> ID);

	ConstraintComponent* getComponent(handle<ConstraintComponent> ID);
	void makeConstraintComponent(Entity& parent, glm::dvec2 anchorPoint, glm::dvec2 tolerance, glm::dvec2 max, double springConstant, std::optional<handle<Entity>> anchoredTo = std::nullopt);
	void destroyComponent(handle<ConstraintComponent> ID);
	
	std::vector<std::pair<handle<PhysicsComponent>, Contact>> checkCollisions(Engine& engine, glm::dvec2 pos, glm::dvec2 halfExtents, unsigned int bitmask = catagories::worldType); // Checks against all existing colliders
	void impulse(handle<PhysicsComponent> ID, glm::dvec2 impulse); // applies a linear impulse to the given object
};