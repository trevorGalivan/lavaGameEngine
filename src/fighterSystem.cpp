#include "fighterSystem.h"

#include "engine.h"
#include "entities.h"
#include "entityCatagories.h"

#include <iostream>

void FighterSystem::update(Engine& engine, double dt) {
	glm::vec2 maxPlayerPos(-10000, -10000); // used to detirmine optimal camera placement to focus on players
	glm::vec2 minPlayerPos(10000, 10000);

	for (auto i = fighters.begin(), end = fighters.end(); i != end; ++i) {
		Entity& entity = *engine.getEntity(i->entity_ID);
		PositionComponent& positionComponent = *engine.getComponent(entity.positionHandle);
		PhysicsComponent& physicsComponent = *engine.getComponent(entity.physicsHandle);
		SpriteComponent& spriteComponent = *engine.getComponent(entity.spriteHandle);

		glm::vec2 size = std::get<aRect>(physicsComponent.collider->shape).halfExtents;

		// camera position calculations
		{
			auto TRcorner = glm::vec2(positionComponent.position) + size;
			auto BLcorner = glm::vec2(positionComponent.position) - size;
			maxPlayerPos.x = std::fmax(TRcorner.x, maxPlayerPos.x);
			maxPlayerPos.y = std::fmax(TRcorner.y, maxPlayerPos.y);
			minPlayerPos.x = std::fmin(BLcorner.x, minPlayerPos.x);
			minPlayerPos.y = std::fmin(BLcorner.y, minPlayerPos.y);
		}

		auto feetContacts = engine.physicsSystem.checkCollisions(engine, glm::dvec2(positionComponent.position) - glm::dvec2(0, size.y), glm::dvec2(size.x, 0.05));
		bool grounded = feetContacts.size();

		bool slidingOnRight = engine.physicsSystem.checkCollisions(engine, glm::dvec2(positionComponent.position) + glm::dvec2(size.x, 0), glm::dvec2(0.05, size.y - 0.05)).size();
		slidingOnRight = slidingOnRight && engine.buttonInputSystem.isPressed(engine, i->playerNumber, Control_MoveRight);
		bool slidingOnLeft = engine.physicsSystem.checkCollisions(engine, glm::dvec2(positionComponent.position) - glm::dvec2(size.x, 0), glm::dvec2(0.05, size.y - 0.05)).size();
		slidingOnLeft = slidingOnLeft && engine.buttonInputSystem.isPressed(engine, i->playerNumber, Control_MoveLeft);

		if ((slidingOnRight || slidingOnLeft) && physicsComponent.velocity.y < -1) {
			physicsComponent.velocity.y = -1;
		}

		if ( grounded || slidingOnRight || slidingOnLeft ) {
			i->numJumps = 0;
		}

		i->jumpCooldown -= dt;
		i->attackCooldown -= dt;

		if (engine.buttonInputSystem.isPressed(engine, i->playerNumber, Control_MoveLeft)) {
			if (grounded && physicsComponent.velocity.x > -5) {// allow quick turning
				physicsComponent.velocity.x -= 3 * float(i->fighterStats.horizontalSpeed * dt);
				for (auto j : feetContacts) { // push off against objects beneath feet
					if (glm::dot(j.second.normal, glm::dvec2(0, 1))) {
						engine.physicsSystem.impulse(j.first, glm::dvec2(3 * i->fighterStats.horizontalSpeed * dt / physicsComponent.invMass, 0) / double(feetContacts.size()));
					}
				}
			}
			physicsComponent.velocity.x -= float(i->fighterStats.horizontalSpeed * dt);

			i->lookDir.x = -1;
		}
		else if (engine.buttonInputSystem.isPressed(engine, i->playerNumber, Control_MoveRight)) {
			if (grounded && physicsComponent.velocity.x < 5) { // allow quick turning
				physicsComponent.velocity.x += 3 * i->fighterStats.horizontalSpeed * dt;
				for (auto j : feetContacts) {// push off against objects beneath feet
					if (glm::dot(j.second.normal, glm::dvec2(0, 1))) {
						engine.physicsSystem.impulse(j.first, glm::dvec2(-3 * i->fighterStats.horizontalSpeed * dt / physicsComponent.invMass, 0) / double(feetContacts.size()));
					}
				}
			}
			physicsComponent.velocity.x += float(i->fighterStats.horizontalSpeed * dt);
			i->lookDir.x = 1;
		}

		if (engine.buttonInputSystem.isPressed(engine, i->playerNumber, Control_Down)) {
			i->lookDir.y = -1;
		}
		else if (engine.buttonInputSystem.isPressed(engine, i->playerNumber, Control_Up)) {
			i->lookDir.y = 1;
		}
		else {
			i->lookDir.y = 0;
		}

		if (engine.buttonInputSystem.isPressed(engine, i->playerNumber, Control_Throw)) {
			auto toBeThrown = engine.physicsSystem.checkCollisions(engine, glm::dvec2(positionComponent.position) + i->costume.handPosition * i->lookDir, glm::dvec2(0.5), entity.catagory);
			for (auto j = toBeThrown.begin(), jEnd = toBeThrown.end(); j != jEnd; ++j) {
				engine.physicsSystem.impulse(j->first, i->lookDir * 20000. * dt);
				engine.physicsSystem.impulse(physicsComponent.ID, i->lookDir * -10000. * dt);
			}
		}

		
		// special case: if player sliding down wall, face away from wall
		if (slidingOnLeft && !slidingOnRight) {
			i->lookDir.x = 1;
		}
		else if (slidingOnRight && !slidingOnLeft) {
			i->lookDir.x = -1;
		}

		// jumping logic
		if (i->numJumps < i->fighterStats.maxJumps - !(grounded || slidingOnLeft || slidingOnRight) && (i->jumpCooldown < 0 || grounded) && engine.buttonInputSystem.justPressed(i->playerNumber, Control_Jump)) {
			physicsComponent.velocity.y = i->fighterStats.jumpStrength;
			if (slidingOnLeft && !slidingOnRight) {
				physicsComponent.velocity.x = i->fighterStats.jumpStrength;
			}
			else if (slidingOnRight && !slidingOnLeft) {
				physicsComponent.velocity.x = -i->fighterStats.jumpStrength;
			}
			i->numJumps++;
			
			i->jumpCooldown = 0.5;
		}
		

		if (i->lookDir.x > 0.1) {
			engine.graphicsSystem.switchSprite_SECONDARYTHREAD(spriteComponent.ID, i->costume.right);
		}
		else if (i->lookDir.x < -0.1){
			engine.graphicsSystem.switchSprite_SECONDARYTHREAD(spriteComponent.ID, i->costume.left);
		}

		if (i->weapon) {
			i->weapon->update(engine, entity, dt);
			if (engine.buttonInputSystem.isPressed(engine, i->playerNumber, Control_Attack)) {
				i->weapon->attack(engine, entity);
			}
			if (engine.buttonInputSystem.isPressed(engine, i->playerNumber, Control_Attack)) {
				i->weapon->attack(engine, entity);
			}
		}

		spriteComponent.animationSpeed = (abs(physicsComponent.velocity.x) / 10 + 0.5);

		if (i->prop.ID != INVALID_HANDLE) {
			Entity& prop = *engine.getEntity(i->prop.ID);
			ConstraintComponent& constraint = *engine.getComponent(prop.constraintHandle);
			SpriteComponent& sprite = *engine.getComponent(prop.spriteHandle);
			if (i->lookDir.x > 0) {
				constraint.anchorPoint = i->prop.offset;
				engine.graphicsSystem.switchSprite_SECONDARYTHREAD(sprite.ID, i->costume.propRight);
			}
			else {
				constraint.anchorPoint = glm::vec2(-(i->prop.offset.x), i->prop.offset.y);
				engine.graphicsSystem.switchSprite_SECONDARYTHREAD(sprite.ID, i->costume.propLeft);
			}
		}
	}
	
	// set optimal camera position
	if(maxPlayerPos.x > minPlayerPos.x && maxPlayerPos.y > minPlayerPos.y)
	{
		std::lock_guard lock(engine.graphicsSystem.cameraMutex);
		auto& camera = engine.graphicsSystem.camera;
		glm::vec2 span = maxPlayerPos - minPlayerPos;
		double z = span.x / (std::tan(camera.horizontalFOV / 2) * 2.);
		z = std::max(z, span.y / (std::tan(camera.horizontalFOV / camera.aspectRatio / 2) * 2.));
		auto newPos = glm::dvec3((maxPlayerPos.x + minPlayerPos.x) / 2, (maxPlayerPos.y + minPlayerPos.y) / 2, z*1.05 + 7);

		camera.setPos(newPos);
	}
}

FighterComponent* FighterSystem::getComponent(handle<FighterComponent> ID) {
	return fighters.find(ID);
}

void FighterSystem::makeFighterComponent(Entity& parent, int number, Costume skin, FighterStats stats /*= defaultStats*/) {
	FighterComponent component;
	component.playerNumber = number;
	component.entity_ID = parent.ID;
	component.numJumps = 0;
	component.jumpCooldown = 0;
	component.attackCooldown = 0;
	component.fighterStats = stats;
	component.costume = skin;

	parent.fighterHandle = fighters.add(std::move(component));
}

void FighterSystem::destroyComponent(handle<FighterComponent> ID) {
	fighters.remove(ID);
}
