#include "healthSystem.h"

#include "engine.h"

#include <algorithm>


void HealthComponent::applyDamageOverTime(double duration, double DPS) {
	damageOverTime.push_back(DamageOverTime{ duration, DPS });
}

void HealthSystem::update(Engine& engine, double dt)
{
	for (auto i = healthBoxes.begin(), iEnd = healthBoxes.end(); i != iEnd; ++i) {
		PositionComponent position = *engine.getComponent(engine.getEntity(i->entity_ID)->positionHandle);
		if (i->destroyParentOnDeath && !i->alive) {
			engine.queueDestroyEntity(i->entity_ID);
		}
		i->collisionCatagory = engine.getEntity(i->entity_ID)->catagory;
		i->hitBox.updatePos(position.position);

		i->invincibilityCooldown -= dt;

		auto j = i->damageOverTime.begin();
		while (j != i->damageOverTime.end()) {
			j->timeLeft -= dt;
			if (j->damagePerSecond > 0) {
				i->damage(engine, j->damagePerSecond * dt);
			}
			else {
				i->healBy(-j->damagePerSecond * dt);
			}

			// only increment iterator if we are not destroying 


			if (j->timeLeft > 0) {
				++j;
			}
			else {
				i->damageOverTime.erase(j);
			}

		}

	}

	for (auto j = hurtBoxes.begin(), jEnd = hurtBoxes.end(); j != jEnd; ++j) {
		PositionComponent position = *engine.getComponent(engine.getEntity(j->entity_ID)->positionHandle);
		j->hurtBox.updatePos(position.position);
	}

	for (auto i = healthBoxes.begin(), iEnd = healthBoxes.end(); i != iEnd; ++i) {
		for (auto j = hurtBoxes.begin(), jEnd = hurtBoxes.end(); j != jEnd; ++j) {
			//auto catagory = engine.getEntity(i->entity_ID)->catagory;
			if (catagories::sameType(i->collisionCatagory, j->targetCatagory) && !catagories::sameTeam(i->collisionCatagory, j->targetCatagory)) {
				Contact unused;
				if (collide(i->hitBox, j->hurtBox, unused) && i->invincibilityCooldown < 0) {
					i->damage(engine, j->damage);
					if (j->reflectDamage) {
						engine.getComponent((engine.getEntity(j->entity_ID)->healthHandle))->damage(engine, j->damage);
					}
				}
			}
		}
	}
}

void HealthSystem::makeHealthComponent(Entity& parent, double maxHealth, double health, Collider hitBox, bool destroyParentOnDeath /*=false*/, double maxInvincibilityCooldown /*= 0.1*/) {
	HealthComponent component;
	component.maxHealth = maxHealth;
	component.health = health;
	component.hitBox = hitBox;
	component.alive = true;
	component.destroyParentOnDeath = destroyParentOnDeath;
	component.entity_ID = parent.ID;
	component.maxInvincibilityCooldown = maxInvincibilityCooldown;
	component.invincibilityCooldown = maxInvincibilityCooldown;
	parent.healthHandle = healthBoxes.add(std::move(component));
}

void HealthSystem::makeDamageComponent(Entity& parent, double damage, Collider hitBox, unsigned int targetCatagory, bool reflectDamage /*= false*/) {
	DamageComponent component;
	component.damage = damage;
	component.hurtBox = hitBox;
	component.reflectDamage = reflectDamage;
	component.entity_ID = parent.ID;
	component.targetCatagory = targetCatagory;
	parent.damageHandle = hurtBoxes.add(std::move(component));
}

void HealthSystem::killAll(Engine& engine) {
	std::for_each(healthBoxes.begin(), healthBoxes.end(),
		[&engine](HealthComponent h) {
			h.kill(engine);
		});
}