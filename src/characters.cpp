#include "characters.h"

#include "entities.h"
#include "engine.h"

#include <random>

class Revolver : public IWeapon
{
	handle<Entity> ID;
	const double bulletSpawnDist = 0.35;
	double coolDown = 0.;
public:
	Revolver(Engine& engine, Entity& player) {
		ID = makeProp(engine, player.ID, glm::vec2(0.3, 0.2), true, true, 0, "revolver.sprt").ID;
	}

	unsigned int getWeaponID() {
		return 1;
	}
	void attack(Engine& engine, Entity& player) {
		if (coolDown < 0) {
			engine.soundSystem.playSound("Revolver_Shoot.wav");
			FighterComponent& parent = *engine.getComponent(player.fighterHandle);
			PhysicsComponent& parentPhys = *engine.getComponent(player.physicsHandle);
			Entity& self = *engine.getEntity(ID);
			PositionComponent& pos = *engine.getComponent(self.positionHandle);

			auto bulletPos = glm::dvec2(pos.position.x, pos.position.y + 0.1) + bulletSpawnDist * parent.lookDir;

			auto bulletDir = parent.lookDir * 40.;

			if (parent.lookDir.y > 0) {
				bulletDir.x = 0;
			}

			makeBullet(engine, player.catagory, bulletPos, parentPhys.velocity + bulletDir, 2., glm::dvec2(0.2), "bullet.sprt");
			coolDown = 1;
		}
	}
	void special(Engine& engine, Entity& player) {
		return;
	}
	void update(Engine& engine, Entity& player, double dt) {
		coolDown -= dt;

		FighterComponent& parent = *engine.getComponent(player.fighterHandle);
		Entity& self = *engine.getEntity(ID);
		SpriteComponent& sprite = *engine.getComponent(self.spriteHandle);
		//PositionComponent& parentPos = *engine.getComponent(player.positionHandle);
		//PositionComponent& pos = *engine.getComponent(self.positionHandle);
		glm::dvec2 playerHandLocation = glm::dvec2(parent.costume.handPosition.x * parent.lookDir.x, parent.costume.handPosition.y);

		if (parent.lookDir.x < 0) {
			sprite.flipX = true;
		}
		else {
			sprite.flipX = false;
		}

		if (parent.lookDir.y > 0) {
			engine.getComponent(self.positionHandle)->rotation = glm::rotate(glm::identity<glm::dquat>(),3.14159 / 2 * (1-2*sprite.flipX), glm::dvec3(0, 0, 1));
		}
		else if (parent.lookDir.y < 0) {
			engine.getComponent(self.positionHandle)->rotation = glm::rotate(glm::identity<glm::dquat>(), -3.14159 / 4 * (1 - 2 * sprite.flipX), glm::dvec3(0, 0, 1));
		}
		else {
			engine.getComponent(self.positionHandle)->rotation = glm::identity<glm::dquat>();
		}

		engine.getComponent(self.constraintHandle)->anchorPoint = playerHandLocation + glm::dvec2(std::copysign(0.3, parent.lookDir.x),0);
	}
};

class Spacegun : public IWeapon
{
	handle<Entity> ID;
	const double bulletSpawnDist = 0.35;
	double coolDown = 0.;
public:
	Spacegun(Engine& engine, Entity& player) {
		ID = makeProp(engine, player.ID, glm::vec2(0.3, 0.2), true, true, 0, "spacegun.sprt").ID;
	}

	unsigned int getWeaponID() {
		return 2;
	}
	void attack(Engine& engine, Entity& player) {
		if (coolDown < 0) {
			engine.soundSystem.playSound("Laser_Shoot.wav");
			FighterComponent& parent = *engine.getComponent(player.fighterHandle);
			PhysicsComponent& parentPhys = *engine.getComponent(player.physicsHandle);
			Entity& self = *engine.getEntity(ID);
			PositionComponent& pos = *engine.getComponent(self.positionHandle);

			auto bulletPos = glm::dvec2(pos.position.x, pos.position.y + 0.1) + bulletSpawnDist * parent.lookDir;


			auto bulletDir = parent.lookDir * 60.;

			if (parent.lookDir.y < 0) {
				bulletDir.x = 0;
			}

			Entity& bullet = makeBullet(engine, player.catagory, bulletPos, parentPhys.velocity + bulletDir, 2., glm::dvec2(0.4,0.2), "spacebolt.sprt", 3.);
			engine.graphicsSystem.makeLightComponent(bullet, glm::vec3(4.f, 1.5f, 0.5f), 1.f, 0.f, 0.1f);
			engine.getComponent(bullet.physicsHandle)->settings.gravAcceleration = 0.f;
			engine.getComponent(bullet.physicsHandle)->settings.restitution = 0.f;
			coolDown = 1;
		}
	}
	void special(Engine& engine, Entity& player) {
		return;
	}
	void update(Engine& engine, Entity& player, double dt) {
		coolDown -= dt;

		FighterComponent& parent = *engine.getComponent(player.fighterHandle);
		Entity& self = *engine.getEntity(ID);

		glm::vec2 playerHandLocation = glm::vec2(parent.costume.handPosition.x * parent.lookDir.x, parent.costume.handPosition.y);
		SpriteComponent& sprite = *engine.getComponent(self.spriteHandle);
		if (parent.lookDir.x < 0) {
			sprite.flipX = true;
		}
		else {
			sprite.flipX = false;
		}


		if (parent.lookDir.y > 0) {
			engine.getComponent(self.positionHandle)->rotation = glm::rotate(glm::identity<glm::quat>(), float(3.14159 / 4 * (1 - 2 * sprite.flipX)), glm::vec3(0, 0, 1));
		} else if (parent.lookDir.y < 0) {
			engine.getComponent(self.positionHandle)->rotation = glm::rotate(glm::identity<glm::quat>(), float(-3.14159 / 2 * (1 - 2 * sprite.flipX)), glm::vec3(0, 0, 1));
		}
		else {
			engine.getComponent(self.positionHandle)->rotation = glm::identity<glm::quat>();
		}

		engine.getComponent(self.constraintHandle)->anchorPoint = playerHandLocation + glm::vec2(std::copysign(0.3, parent.lookDir.x), 0);
	}
};

class FlameThrower : public IWeapon
{
	handle<Entity> ID;
	const double bulletSpawnDist = 0.35;
	double coolDown = 0.;

	std::default_random_engine generator;
	std::uniform_real_distribution<double> distribution;
public:
	FlameThrower(Engine& engine, Entity& player) : generator(1000), distribution(-1., 1.) {
		ID = makeProp(engine, player.ID, glm::vec2(0.72, 0.2), true, true, 0, "flamethrower.sprt").ID;
	}

	unsigned int getWeaponID() {
		return 3;
	}

	void attack(Engine& engine, Entity& player) {
		if (coolDown < 0) {
			engine.soundSystem.playSound("fire.wav");
			FighterComponent& parent = *engine.getComponent(player.fighterHandle);
			PhysicsComponent& parentPhys = *engine.getComponent(player.physicsHandle);
			Entity& self = *engine.getEntity(ID);
			PositionComponent& pos = *engine.getComponent(self.positionHandle);

			auto bulletPos = glm::dvec2(pos.position.x, pos.position.y + 0.1) + bulletSpawnDist * parent.lookDir;
			auto y = distribution(generator) * 0.5;


			Entity& bullet = makeBullet(engine, player.catagory, bulletPos, parentPhys.velocity + glm::dvec2(20. * sqrt(1-y*y*0.5) * parent.lookDir.x, 10.*y ), 0.2, glm::dvec2(0.5, 0.5), "spacebolt.sprt", 0.5);
			engine.graphicsSystem.makeLightComponent(bullet, glm::vec3(2.5f, 0.2f, 0.f), 1.f, 0.f, 0.1f);
			engine.getComponent(bullet.physicsHandle)->settings.gravAcceleration = 0;
			engine.getComponent(bullet.physicsHandle)->settings.restitution = 0;
			coolDown = 0.08 + 0.02 * distribution(generator);
		}
	}
	void special(Engine& engine, Entity& player) {
		return;
	}
	void update(Engine& engine, Entity& player, double dt) {
		coolDown -= dt;

		FighterComponent& parent = *engine.getComponent(player.fighterHandle);
		Entity& self = *engine.getEntity(ID);
		//PositionComponent& parentPos = *engine.getComponent(player.positionHandle);
		//PositionComponent& pos = *engine.getComponent(self.positionHandle);
		glm::vec2 playerHandLocation = glm::vec2(parent.costume.handPosition.x * parent.lookDir.x, parent.costume.handPosition.y);
		SpriteComponent& sprite = *engine.getComponent(self.spriteHandle);
		if (parent.lookDir.x < 0) {
			sprite.flipX = true;
		}
		else {
			sprite.flipX = false;
		}
		engine.getComponent(self.constraintHandle)->anchorPoint = playerHandLocation + glm::vec2(std::copysign(0.3, parent.lookDir.x), 0);
	}
};

std::unique_ptr<IWeapon> makeRevolver(Engine& engine, Entity& player)
{
	return std::make_unique<Revolver>(engine, player);
}

std::unique_ptr<IWeapon> makeSpacegun(Engine& engine, Entity& player)
{
	return std::make_unique<Spacegun>(engine, player);
}

std::unique_ptr<IWeapon> makeFlamethrower(Engine& engine, Entity& player)
{
	return std::make_unique<FlameThrower>(engine, player);
}

void makeCowboy(Engine& engine, glm::vec2 position, int playerNum) {
	Entity& player = engine.makeEntity();
	player.catagory = catagories::playerType | catagories::teamBit(playerNum);
	engine.positionSystem.makePositionComponent(player, glm::vec3(position, 0));
	auto settings = defaultPhysicsSettings;
	settings.restitution = 0.05f;
	engine.physicsSystem.makePhysicsComponent(player, std::make_optional(Collider(aRect(glm::vec2(0.5, 1.)))), glm::vec3(0), 80, settings);
	engine.graphicsSystem.makeSpriteComponent(player, "cowboyWalkRight.sprt", glm::vec2(0.5, 1));
	Costume costume = { "cowboyWalkRight.sprt", "cowboyWalkLeft.sprt", "cowboyHat.sprt", "cowboyHat.sprt", glm::vec2(0.48, 0.1)};
	engine.fighterSystem.makeFighterComponent(player, playerNum, costume);
	FighterComponent& fighter = *engine.getComponent(player.fighterHandle);
	engine.healthSystem.makeHealthComponent(player, 10., Collider(aRect(glm::vec2(0.5, 1.))), true);

	//engine.graphicsSystem.makeLightComponent(player, glm::vec3(20, 20, 20), 1, 0.5, 0.2);

	Entity& hat = makeProp(engine, player.ID, glm::vec2(0.5 * 0.75, 0.25 * 0.75), true, false, 2000, "cowboyHat.sprt");
	engine.getComponent(hat.physicsHandle)->settings.gravAcceleration = -1000;
	engine.getComponent(hat.constraintHandle)->max = glm::vec2(0, 0.5f);
	fighter.prop = { glm::vec2(0, 1.5), hat.ID };
	fighter.weapon = makeRevolver(engine, player);
}

void makeAstronaut(Engine& engine, glm::vec2 position, int playerNum) {
	Entity& player = engine.makeEntity();
	player.catagory = catagories::playerType | catagories::teamBit(playerNum);
	engine.positionSystem.makePositionComponent(player, glm::vec3(position, 0));
	auto settings = defaultPhysicsSettings;
	settings.restitution = 0.03f;
	settings.dampingFactor = 0.f;
	settings.gravAcceleration *= 0.5f;
	settings.kineticFriction *= 2.;
	engine.physicsSystem.makePhysicsComponent(player, std::make_optional(Collider(aRect(glm::vec2(0.5, 1.)))), glm::vec3(0), 207, settings);
	engine.graphicsSystem.makeSpriteComponent(player, "astronautWalkRight.sprt", glm::vec2(0.5, 1));
	FighterStats stats = defaultStats;

	stats.horizontalSpeed *= 0.8f;
	stats.maxJumps = 1;
	Costume costume = { "astronautWalkRight.sprt", "astronautWalkLeft.sprt", "astronautWalkRight.sprt", "astronautWalkRight.sprt", glm::vec2(0.48, -0.2) };
	engine.fighterSystem.makeFighterComponent(player, playerNum, costume, stats);
	engine.healthSystem.makeHealthComponent(player, 8., Collider(aRect(glm::vec2(0.5, 1.))), true);
	FighterComponent& fighter = *engine.getComponent(player.fighterHandle);
	fighter.weapon = makeSpacegun(engine, player);
}

void makeRobot(Engine& engine, glm::vec2 position, int playerNum) {
	Entity& player = engine.makeEntity();
	player.catagory = catagories::playerType | catagories::teamBit(playerNum);
	engine.positionSystem.makePositionComponent(player, glm::vec3(position, 0));
	auto settings = defaultPhysicsSettings;
	settings.restitution *= 0.1;
	engine.physicsSystem.makePhysicsComponent(player, std::make_optional(Collider(aRect(glm::vec2(0.7, 0.81667)))), glm::vec3(0), 1000, settings);
	engine.graphicsSystem.makeSpriteComponent(player, "robotWalkRight.sprt", glm::vec2(0.7, 0.81667));
	FighterStats stats = defaultStats;

	stats.horizontalSpeed *= 1.2f;
	stats.maxJumps = 2;
	
	Costume costume = { "robotWalkRight.sprt", "robotWalkLeft.sprt", "robotWalkRight.sprt", "robotWalkRight.sprt", glm::vec2(-0.5, 1) };
	engine.fighterSystem.makeFighterComponent(player, playerNum, costume, stats);
	engine.healthSystem.makeHealthComponent(player, 8., Collider(aRect(glm::vec2(0.7, 0.81667))), true);
	FighterComponent& fighter = *engine.getComponent(player.fighterHandle);
	fighter.weapon = makeFlamethrower(engine, player);
}