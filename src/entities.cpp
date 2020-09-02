#include "entities.h"

#include "engine.h"

// creates a damaging projectile
Entity& makeBullet(Engine& engine, unsigned int team, glm::dvec2 position, glm::dvec2 velocity, double damage, glm::dvec2 halfExtents, std::string sprite, double lifeSpan)
{
	Entity& bullet = engine.makeEntity();
	bullet.catagory = catagories::projectileType | catagories::getTeamBits(team);
	double angle = std::atan2(-velocity.y, velocity.x);

	engine.positionSystem.makePositionComponent(bullet, glm::dvec3(position, 0), glm::angleAxis(angle, glm::dvec3(0, 0, -1)));

	auto settings = defaultPhysicsSettings;
	settings.dampingFactor = 0.;
	settings.quadraticDampingFactor = 0.01;
	settings.kineticFriction = 0.2;
	settings.restitution = 1.;
	settings.collisionMask = catagories::worldType | catagories::playerType | catagories::getTeamBits(team);
	engine.physicsSystem.makePhysicsComponent(bullet, std::make_optional(Collider(aRect(halfExtents))), velocity, 0.5, settings);
	engine.graphicsSystem.makeSpriteComponent(bullet, sprite, halfExtents);

	double health = 0.2 * damage;
	engine.healthSystem.makeHealthComponent(bullet, health, Collider(aRect(halfExtents)), true);
	engine.getComponent(bullet.healthHandle)->applyDamageOverTime(100000000., health / lifeSpan);
	engine.healthSystem.makeDamageComponent(bullet, damage, Collider(aRect(halfExtents * 1.01)), catagories::playerType | catagories::getTeamBits(team), true);

	return bullet;
}

// creates a prop, which will home in on its anchor point with perfect damping. Can be constrained fo follow anchor point exactly on given axis.
Entity& makeProp(Engine& engine, handle<Entity> parentID, glm::dvec2 halfExtents, bool constrainX, bool constrainY, double springConstant, std::string sprite)
{
	Entity& parent = *engine.getEntity(parentID);
	Entity& prop = engine.makeEntity();
	prop.catagory = catagories::propsType | catagories::getTeamBits(parent.catagory);

	engine.positionSystem.makePositionComponent(prop, engine.getComponent(parent.positionHandle)->position);
	double mass = 1.;
	auto settings = defaultPhysicsSettings;
	settings.dampingFactor = 2. * sqrt(springConstant / mass); // critical damping
	settings.quadraticDampingFactor = 0.;
	settings.kineticFriction = 0.;
	settings.restitution = 1.;
	settings.gravAcceleration = 0.;
	settings.collisionMask = catagories::worldType | catagories::playerType;
	engine.physicsSystem.makePhysicsComponent(prop, std::nullopt, glm::dvec2(0.), mass, settings);
	engine.graphicsSystem.makeSpriteComponent(prop, sprite, halfExtents);
	engine.physicsSystem.makeConstraintComponent(prop, glm::dvec2(0.), glm::dvec2(0.), glm::dvec2(!constrainX, !constrainY) * 1000., springConstant, parentID);
	return prop;
}