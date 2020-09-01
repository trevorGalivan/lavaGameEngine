#pragma once
#include "componentManager.h"

class Engine;
class Entity;

class System
{
public:
	void update(Engine& engine, double dt) { ; };
};

template <typename T>
class Component
{
public:
	handle<Entity> entity_ID;
	handle<T> ID; 
};

