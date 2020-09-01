#pragma once
#include "system.h"

#include <glm/glm.hpp>

#include <vector>
#include <tuple>
#include <string>
#include <memory>
class FighterSystem;

struct FighterStats
{
	int maxJumps;
	double horizontalSpeed;
	double jumpStrength;
};

const FighterStats defaultStats =
{
	2, 20., 10.
};

class IWeapon
{
public:
	virtual unsigned int getWeaponID() = 0;
	virtual ~IWeapon() {};
	virtual void attack(Engine& engine, Entity& player) = 0;
	virtual void special(Engine& engine, Entity& player) = 0;
	virtual void update(Engine& engine, Entity& player, double dt) = 0;
};

struct Costume
{
	std::string right;
	std::string left;
	std::string propRight;
	std::string propLeft;
	glm::dvec2 handPosition; // position of hand when facing right, in local coords. (May be used by IWeapon derivatives to render correctly in hand)
};

class FighterComponent : public Component<FighterComponent> 
{
public:
	typedef FighterSystem owningSystem;

	glm::dvec2 lookDir; // should be unit vector
	int playerNumber;
	double attackCooldown;
	int numJumps; // reset on touching ground
	FighterStats fighterStats;
	double jumpCooldown;

	struct Prop {
		glm::dvec2 offset;
		handle<Entity> ID;
	};

	Prop prop;

	Costume costume;

	std::unique_ptr<IWeapon> weapon = nullptr;
};


class FighterSystem : public System
{
	ComponentManager<FighterComponent> fighters;
public:
	void update(Engine& engine, double dt);

	FighterComponent* getComponent(handle<FighterComponent> ID);
	void makeFighterComponent(Entity& parent, int number, Costume skin, FighterStats stats = defaultStats);
	void destroyComponent(handle<FighterComponent> ID);
};

