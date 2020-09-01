#pragma once
#include "system.h"
#include "collider.h"

#include <functional>
#include <optional>


struct DamageOverTime
{
	double timeLeft;
	double damagePerSecond; // make negative to apply healing
};

class HealthComponent : public Component<HealthComponent>
{
	unsigned int collisionCatagory; // do not set manually, used to cache data

	double maxHealth;
	double health;
	
	double invincibilityCooldown;
	double maxInvincibilityCooldown;
	
	bool alive;
	friend class HealthSystem;
	friend class Engine;
	std::vector<DamageOverTime> damageOverTime;
public:
	typedef HealthSystem owningSystem;
	Collider hitBox;

	void applyDamageOverTime(double duration, double DPS);
	void applyHealingOverTime(double duration, double HPS) {
		applyDamageOverTime(duration, -HPS);
	}

	bool destroyParentOnDeath;

	bool damage(Engine& engine, double damage) { // returns true if killing blow
		invincibilityCooldown = maxInvincibilityCooldown;
		health -= damage;
		if (alive && health < 0.) {
			alive = false;
			//if(deathCallback) deathCallback(engine, *this);
			return true;
		}
		return false;
	}

	void healTo(double newHealth) {// cannot ressurect dead entity. Do not call with negative arguement
		if (alive) {
			health = newHealth;
			health = fmax(health, maxHealth);
		}
	}

	void healBy(double deltaHealth) {
		healTo(health + deltaHealth);
	}

	bool isAlive() {
		return alive;
	}

	double getHealth() {
		return health;
	}

	void kill(Engine& engine) {
		health = -1.;

		alive = false;
	}
};

class DamageComponent : public Component<DamageComponent>
{
public:
	typedef HealthSystem owningSystem;
	bool reflectDamage = false; // if true, assert that parent entity has healthComponent, and apply equal damage to said health component
	double damage;
	Collider hurtBox;

	unsigned int targetCatagory; // for damage to occur, target health component must have a common type, and no common team
};

class HealthSystem : public System
{
	ComponentManager<HealthComponent> healthBoxes;
	ComponentManager<DamageComponent> hurtBoxes;
public:
	void killAll(Engine& engine); // calls the deathCallback of every single healthComponent. You monster.

	void update(Engine& engine, double dt);

	HealthComponent* getComponent(handle<HealthComponent> ID) {
		return healthBoxes.find(ID);
	}

	void makeHealthComponent(Entity& parent, double maxHealth, Collider hitBox, bool destroyOnDeath = false, double maxInvincibilityCooldown = 0.1) { // set initial health to maxHealth
		makeHealthComponent(parent, maxHealth, maxHealth, hitBox, destroyOnDeath, maxInvincibilityCooldown);
	}
	void makeHealthComponent(Entity& parent, double maxHealth, double health, Collider hitBox, bool destroyOnDeath=false, double maxInvincibilityCooldown=0.1);
	

	void destroyComponent(handle<HealthComponent> ID) {
		healthBoxes.remove(ID);
	}

	DamageComponent* getComponent(handle<DamageComponent> ID) {
		return hurtBoxes.find(ID);
	}

	void makeDamageComponent(Entity& parent, double damage, Collider hitBox, unsigned int targetCatagory, bool reflectDamage = false);


	void destroyComponent(handle<DamageComponent> ID) {
		hurtBoxes.remove(ID);
	}
};