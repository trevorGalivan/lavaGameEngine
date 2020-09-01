#include "physicsSystem.h"

#include "engine.h"

// piecewise function for combining two coefficients of restitution
// see https://www.gamedev.net/tutorials/programming/math-and-physics/combining-material-friction-and-restitution-values-r4227/ for explanation
double restitution(double rest1, double rest2) {
	if (rest1 > 0.5 && rest2 > 0.5) {
		return fmax(rest1, rest2);
	}
	else if (rest1 < 0.5 && rest2 < 0.5) {
		return fmin(rest1, rest2);
	}
	else {
		return rest1 + rest2 - 0.5;
	}
}

// Basic acceleration + velocity calculations
void PhysicsSystem::update(Engine& engine, double dt)
{
	for (auto i = bodies.begin(), end = bodies.end(); i != end; ++i) {
		Entity& entity = *engine.getEntity(i->entity_ID);
		PositionComponent& positionComponent = *engine.getComponent(entity.positionHandle);

		i->collisionCatagory = entity.catagory;

		positionComponent.position += glm::dvec3(i->velocity * dt, 0.);

		if (i->invMass) {
			i->velocity.y += i->settings.gravAcceleration * dt;
		}

		i->velocity -= i->velocity * i->settings.dampingFactor * dt;
		i->velocity -= i->velocity * glm::length(i->velocity) * i->settings.quadraticDampingFactor * dt;
		// Collision system stuff
		if (i->collider) {
			i->collider->updatePos(positionComponent.position); // computes AABB, rotation matrices, etc. to prevent cache misses during O(n^2) collision routine
		}
	}

	// resolve all constraints
	for (auto i = constraints.begin(), end = constraints.end(); i != end; ++i) {
		PositionComponent& positionComponent = *engine.getComponent(engine.getEntity(i->entity_ID)->positionHandle);
		PhysicsComponent& physicsComponent = *engine.getComponent(engine.getEntity(i->entity_ID)->physicsHandle);

		glm::dvec2 toAnchor = i->anchorPoint - glm::dvec2(positionComponent.position);
		if (i->parent) {
			Entity* parent = engine.getEntity(*(i->parent));
			if (parent) {
				toAnchor += glm::dvec2(engine.getComponent(parent->positionHandle)->position);
			}
			else { // Parent has been deleted somehow. This entity must be deleted too
				engine.queueDestroyEntity(i->entity_ID);
			}
			
		}

		// vector to nearest point within constraint tolerance;
		glm::dvec2 toTolerance;
		toTolerance.x = copysign(fmax(abs(toAnchor.x) - i->tolerance.x, 0.), toAnchor.x);
		toTolerance.y = copysign(fmax(abs(toAnchor.y) - i->tolerance.y, 0.), toAnchor.y);

		// apply ideal spring impuse to move body back into tolerance
		physicsComponent.velocity += physicsComponent.invMass * toTolerance * i->springConstant * dt;

		// vector to nearest point within constraint max 
		glm::dvec2 toMax;
		toMax.x = copysign(fmax(abs(toAnchor.x) - i->max.x, 0.), toAnchor.x);
		toMax.y = copysign(fmax(abs(toAnchor.y) - i->max.y, 0.), toAnchor.y);
		
		// cancel out any outwards velocity
		if (toMax.y > 0.f) {
			physicsComponent.velocity.y = fmax(0., physicsComponent.velocity.y);
		}
		else if (toMax.y < 0.f) {
			physicsComponent.velocity.y = fmin(0., physicsComponent.velocity.y);
		}

		if (toMax.x > 0.f) {
			physicsComponent.velocity.x = fmax(0., physicsComponent.velocity.x);
		}
		else if (toMax.x < 0.f) {
			physicsComponent.velocity.x = fmin(0., physicsComponent.velocity.x);
		}
		// Cancel any outwards velocity;
		//float vel = glm::dot(glm::normalize(toMax), physicsComponent.velocity);
		//vel = fmin(0., vel);
		//physicsComponent.velocity -= glm::normalize(toMax) * vel;

		// teleport object back into max cosntraint
		positionComponent.position += glm::vec3(toMax,0);
	}

	// Collision checking / resolution routine
	for (auto i = bodies.begin(), iEnd = --bodies.end(), jEnd = bodies.end(); i != iEnd; ++i) {
		if (i->collider) {
			auto j = i;
			++j;
			for (; j != jEnd; ++j) {
				// set output variable to send into collision solver
				if (j->collider && ( catagories::sameType(i->settings.collisionMask, j->collisionCatagory) ||
					catagories::sameType(j->settings.collisionMask, i->collisionCatagory)) &&
					!(catagories::sameTeam(i->settings.collisionMask, j->collisionCatagory) ||
						catagories::sameTeam(j->settings.collisionMask, i->collisionCatagory))
					) {

					Contact contact;

					double e = restitution(i->settings.restitution, j->settings.restitution);

					if (collide(*(i->collider), *(j->collider), contact)) {
						glm::dvec2 relativeVel = j->velocity - i->velocity;
						double normalVel = glm::dot(relativeVel, contact.normal);

						//if (fabs(normalVel) < 0.001) { e /= 5; } // hack to reduce jittering on resting contact
						if (glm::dot(relativeVel, contact.normal) < 0) {
							double mass_sum = i->invMass + j->invMass;

							double impulseScalar = -(1 + e) * normalVel;
							impulseScalar /= mass_sum;

							glm::dvec2 impulse = impulseScalar * contact.normal;

							i->velocity -= i->invMass * impulse;
							j->velocity += j->invMass * impulse;

							// geometric average of friction values
							double cFriction = sqrt(i->settings.kineticFriction * j->settings.kineticFriction);
							glm::dvec2 tangentDir = (relativeVel - normalVel * contact.normal);
							double fricSpeed = glm::length(tangentDir);

							double fricImpulse = 0;// fricSpeed;
							if (fricSpeed > 0.0001) { 
								tangentDir /= fricSpeed;
								fricImpulse = impulseScalar * cFriction;
								fricImpulse = fmin(fricImpulse, fricSpeed / mass_sum);
							}

							i->velocity += i->invMass * fricImpulse * tangentDir;
							j->velocity -= j->invMass * fricImpulse * tangentDir;

							const double correctionThreshold = 0.02; // Threshhold before position correction occurs
							
							glm::dvec2 posCorrection =  fmax(contact.depth - correctionThreshold, 0.0) / (mass_sum) * 0.8 * contact.normal;

							engine.getComponent(engine.getEntity(i->entity_ID)->positionHandle)->position -= glm::dvec3(i->invMass * posCorrection, 0);
							engine.getComponent(engine.getEntity(j->entity_ID)->positionHandle)->position += glm::dvec3(j->invMass * posCorrection, 0);
						}// TODO: add some sort of ret data
					}
				}
			}
		}
	}
}



PhysicsComponent* PhysicsSystem::getComponent(handle<PhysicsComponent> ID)
{
	return bodies.find(ID);
}

void PhysicsSystem::makePhysicsComponent(Entity& parent, std::optional<Collider> collider, glm::dvec2 velocity /*= glm::vec3(0.)*/, double mass, PhysicsSettings settings /*= {9.81, true}*/)
{
	PhysicsComponent component;
	component.collider = collider;
	component.velocity = velocity;
	component.invMass = mass == 0. ? 0. : 1./mass;
	component.entity_ID = parent.ID;
	component.settings = settings;

	parent.physicsHandle = bodies.add(std::move(component));
}

void PhysicsSystem::destroyComponent(handle<PhysicsComponent> ID)
{
	bodies.remove(ID);
}

ConstraintComponent* PhysicsSystem::getComponent(handle<ConstraintComponent> ID)
{
	return constraints.find(ID);
}
void PhysicsSystem::makeConstraintComponent(Entity& parent, glm::dvec2 anchorPoint, glm::dvec2 tolerance, glm::dvec2 max, double springConstant, std::optional<handle<Entity>> anchoredTo)
{
	ConstraintComponent component;

	component.anchorPoint = anchorPoint;
	component.tolerance = tolerance;
	component.max = max;
	component.springConstant = springConstant;
	component.entity_ID = parent.ID;
	component.parent = anchoredTo;

	parent.constraintHandle = constraints.add(std::move(component));
}
void PhysicsSystem::destroyComponent(handle<ConstraintComponent> ID)
{
	constraints.remove(ID);
}

std::vector<std::pair<handle<PhysicsComponent>, Contact>> PhysicsSystem::checkCollisions(Engine& engine, glm::dvec2 pos, glm::dvec2 halfExtents, unsigned int typesMask)
{
	Collider collider;
	collider.shape = vShape_t(aRect(halfExtents));
	collider.updatePos(pos);

	std::vector<std::pair<handle<PhysicsComponent>, Contact>> contacts;
	for (auto j = bodies.begin(), jEnd = bodies.end(); j != jEnd; ++j) {
		if (j->collider && (!catagories::sameTeam(typesMask, engine.getEntity(j->entity_ID)->catagory)) && catagories::sameType(typesMask, engine.getEntity( j->entity_ID)->catagory )) {
			Contact contact;
			if (collide(collider, *(j->collider), contact)) {
				contacts.push_back(std::pair(j->ID, contact));
			}
		}
	}
	return contacts;
}

void PhysicsSystem::impulse(handle<PhysicsComponent> object, glm::dvec2 impulse)
{
	PhysicsComponent& c = *getComponent(object);
	c.velocity += c.invMass * impulse;
}