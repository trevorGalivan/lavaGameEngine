#pragma once
#include "model.h"
#include "entityCatagories.h"
#include "keyMappings.h"
#include "componentManager.h"
#include "fpsCounter.h"

#include "graphicsSystem.h"
#include "physicsSystem.h"
#include "positionSystem.h"
#include "inputSystem.h"
#include "healthSystem.h"
#include "fighterSystem.h"
#include "soundSystem.h"
#define NUM_SYSTEMS 7

#include <glm/gtx/quaternion.hpp>

#include <array>
#include <atomic>

class Engine;
class Entity;
class System;
template <typename T>
class Component;

struct GLFWwindow;

// Ctor loads openGL function ptrs and initializes window
class WindowWrapper
{
public:
	GLFWwindow* window = nullptr;

	WindowWrapper(unsigned int winWidth = 400, unsigned int winHeight = 400);
};

// Provides an interface to initialize window, recieve input, get the time, and other system-specific functions;
// Is responsible for constructing and calling each system
class Engine
{
	WindowWrapper windowWrapper;
	double lastUpdateTime = 0.;
	double lastDrawTime = 0.;
	glm::dvec2 lastMousePos = glm::dvec2(0.);

	friend void keyActionCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	friend void mouseButtonActionCallback(GLFWwindow* window, int key, int action, int mods);
	friend void framebufferSizeCallback(GLFWwindow* window, int width, int height);
	friend void mouseMovementCallback();

	std::array<std::atomic<bool>, LAST_KEY> keyMap;
	std::array<std::atomic<bool>, LAST_GAMEPAD> gamepad1Map;
	std::queue<handle<Entity>> destructionQueue;
	void updateJoySticks();
	void destroyEntity(handle<Entity> ID);

	FpsCounter fpsCounter;

	std::mutex entityDeletionMutex;
public:
	GraphicsSystem graphicsSystem;
	PhysicsSystem physicsSystem;
	PositionSystem positionSystem;
	ButtonInputSystem buttonInputSystem;
	HealthSystem healthSystem;
	FighterSystem fighterSystem;
	SoundSystem soundSystem;
	template<typename T>
	T& getSystem() { ; }
	template<>
	GraphicsSystem& getSystem<GraphicsSystem>() { return graphicsSystem; }
	template<>
	PhysicsSystem& getSystem<PhysicsSystem>() { return physicsSystem; }
	template<>
	PositionSystem& getSystem<PositionSystem>() { return positionSystem; }
	template<>
	ButtonInputSystem& getSystem<ButtonInputSystem>() { return buttonInputSystem; }
	template<>
	HealthSystem& getSystem<HealthSystem>() { return healthSystem; }
	template<>
	FighterSystem& getSystem<FighterSystem>() { return fighterSystem; }
	template<>
	SoundSystem& getSystem<SoundSystem>() { return soundSystem; }

	template<typename T>
	typename T::owningSystem& getOwningSystem() { return getSystem<T::owningSystem>(); }

	template<typename T>
	T* getComponent(handle<T> key) {
		return getOwningSystem<T>().getComponent(key);
	}

	template<typename T>
	void destroyComponent(handle<T> key) {
		if (key != handle<T>(INVALID_HANDLE)) {
			getOwningSystem<T>().destroyComponent(key);
		}
		
	}

	Entity* getEntity(handle<Entity> key) {
		return entities.find(key);
	}

	void queueDestroyEntity(handle<Entity> key) {
		destructionQueue.push(key);
	}

	Entity& makeEntity(unsigned int catagory = catagories::miscType);

	ComponentManager<Entity> entities;
	double physicsTimestep = 1./240.;



	Engine(unsigned int winWidth, unsigned int winHeight);
	~Engine();

	void update();
	void draw();
	void doLoopForever();

	bool checkButtonState(Button);

	double getTime();
};

// Connects all components within a single in-game object together by storing handles for all of them.
class Entity
{
public:
	handle<Entity> ID;
	unsigned int catagory; // see entityCatagories.h
	handle<ModelComponent> modelHandle;
	handle<SpriteComponent> spriteHandle;
	handle<LightComponent> lightHandle;
	handle<PhysicsComponent> physicsHandle;
	handle<ConstraintComponent> constraintHandle;
	handle<PositionComponent> positionHandle;
	handle<HealthComponent> healthHandle;
	handle<DamageComponent> damageHandle;
	handle<FighterComponent> fighterHandle;
};