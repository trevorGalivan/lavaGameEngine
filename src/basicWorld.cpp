#include "basicWorld.h"

#include "entities.h"
#include "characters.h"

PhysicsSettings terrain;
const std::array<std::string, 6> nightskyboxnames = { "night_right.png", "night_left.png", "night_top.png", "night_bottom.png", "night_front.png", "night_back.png" };
const std::array<std::string, 6> dayskyboxnames = { "Sky3.png", "Sky2.png", "SkyTop.png", "SkyBottom.png", "SkyBack.png", "Sky1.png" };
void makeDesert(Engine& engine) {
	Entity& desert = engine.makeEntity();
	engine.positionSystem.makePositionComponent(desert);
	engine.graphicsSystem.makeModelComponent(desert, "desertScene.obj", glm::translate(glm::vec3(0, -14, 0)) * glm::rotate(glm::identity<glm::mat4x4>(), 1.570796f, glm::vec3(0, 1, 0))  );
}

void makeAsteroids(Engine& engine) {
	Entity& asteroids = engine.makeEntity();
	
	engine.positionSystem.makePositionComponent(asteroids, glm::dvec3(0), glm::rotate(3.14159/4., glm::dvec3(0,0,1)));
	engine.graphicsSystem.makeModelComponent(asteroids, "asteroids_LQ.obj", glm::identity<glm::mat4x4>(), glm::rotate(-0.1, glm::dvec3(1, 1, 0)));
}

void makeVolcano(Engine& engine) {
	Entity& volcano = engine.makeEntity();

	engine.positionSystem.makePositionComponent(volcano);
	engine.graphicsSystem.makeModelComponent(volcano, "volcano.obj");
}

Entity& makeDamageZone(Engine& engine, double damage, glm::vec2 position, glm::vec2 halfExtents)
{
	Entity& wall = engine.makeEntity();
	wall.catagory = catagories::worldType;
	engine.positionSystem.makePositionComponent(wall, glm::vec3(position, 0));
	engine.healthSystem.makeDamageComponent(wall, damage, Collider(aRect(halfExtents)), catagories::playerType);

	return wall;
}

void makeWall(Engine& engine, glm::vec2 position, glm::vec2 size, std::string path) {
	Entity& wall = engine.makeEntity();
	wall.catagory = catagories::worldType;
	engine.positionSystem.makePositionComponent(wall, glm::vec3(position, 0));
	engine.physicsSystem.makePhysicsComponent(wall, std::make_optional(Collider(aRect(size))), glm::vec2(0), 0);

	engine.graphicsSystem.makeSpriteComponent(wall, path, size);
}

Entity& makeSlidingPlatform(Engine& engine, glm::vec2 position, glm::vec2 size, float mass, float range, float spring, std::string path)
{
	Entity& wall = engine.makeEntity();
	wall.catagory = catagories::worldType;
	engine.positionSystem.makePositionComponent(wall, glm::vec3(position, 0));
	auto settings = defaultPhysicsSettings;
	settings.restitution = sqrt(settings.restitution);
	settings.dampingFactor = 0.f;
	settings.quadraticDampingFactor *= 0.1f;
	engine.physicsSystem.makePhysicsComponent(wall, std::make_optional(Collider(aRect(size))), glm::vec2(0), mass, settings);
	engine.graphicsSystem.makeModelComponent(wall, path);

	engine.physicsSystem.makeConstraintComponent(wall, position, glm::vec2(0.05f), glm::vec2(range, 0.1f), spring);
	
	return wall;
}

void make3DFloor(Engine& engine, glm::vec2 position, glm::vec2 size, std::string path) {
	Entity& wall = engine.makeEntity();
	wall.catagory = catagories::worldType;
	engine.positionSystem.makePositionComponent(wall, glm::vec3(position, 0));
	auto settings = defaultPhysicsSettings;
	engine.physicsSystem.makePhysicsComponent(wall, std::make_optional(Collider(aRect(size))), glm::vec2(0), 0, settings);
	engine.graphicsSystem.makeModelComponent(wall, path, glm::mat4x4(glm::vec4(0,-1,0,0), glm::vec4(1,0,0,0), glm::vec4(0,0,1,0), glm::vec4(0,0,0,1)));
}

Entity& makeInvisibleWall(Engine& engine, glm::vec2 position, glm::vec2 size)
{
	Entity& wall = engine.makeEntity();
	wall.catagory = catagories::worldType;
	engine.positionSystem.makePositionComponent(wall, glm::vec3(position, 0));
	auto settings = defaultPhysicsSettings;
	engine.physicsSystem.makePhysicsComponent(wall, std::make_optional(Collider(aRect(size))), glm::vec2(0), 0, settings);

	return wall;
}

void make3DWall(Engine& engine, glm::vec2 position, glm::vec2 size, std::string path) {
	Entity& wall = makeInvisibleWall(engine, position, size);

	engine.graphicsSystem.makeModelComponent(wall, path);
}

void desertWorld(Engine& engine, const std::vector<CharacterFunction>& players) {
	// floors
	make3DFloor(engine, glm::vec2(7-14, -14), glm::vec2(7, 0.5), "brickwall3.obj");
	make3DFloor(engine, glm::vec2(7 - 14, 14*2), glm::vec2(7, 0.5), "brickwall3.obj");

	make3DFloor(engine, glm::vec2(-7 - 14, -14), glm::vec2(7, 0.5), "brickwall3.obj");
	make3DFloor(engine, glm::vec2(-7 - 14, 14 * 2), glm::vec2(7, 0.5), "brickwall3.obj");

	make3DFloor(engine, glm::vec2(7, -14), glm::vec2(7, 0.5), "brickwall3.obj");
	make3DFloor(engine, glm::vec2(7, 14 * 2), glm::vec2(7, 0.5), "brickwall3.obj");

	make3DFloor(engine, glm::vec2(-7, -14), glm::vec2(7, 0.5), "brickwall3.obj");
	make3DFloor(engine, glm::vec2(-7, 14 * 2), glm::vec2(7, 0.5), "brickwall3.obj");

	make3DFloor(engine, glm::vec2(7 + 14, -14), glm::vec2(7, 0.5), "brickwall3.obj");
	make3DFloor(engine, glm::vec2(7 + 14, 14 * 2), glm::vec2(7, 0.5), "brickwall3.obj");

	make3DFloor(engine, glm::vec2(-7 + 14, -14), glm::vec2(7, 0.5), "brickwall3.obj");
	make3DFloor(engine, glm::vec2(-7 + 14, 14 * 2), glm::vec2(7, 0.5), "brickwall3.obj");

	// walls
	make3DWall(engine, glm::vec2(14*2, 7),  glm::vec2(0.5, 7), "brickwall3.obj");
	make3DWall(engine, glm::vec2(-14 * 2, 7), glm::vec2(0.5, 7), "brickwall3.obj");

	make3DWall(engine, glm::vec2(14 * 2, -7), glm::vec2(0.5, 7), "brickwall3.obj");
	make3DWall(engine, glm::vec2(-14 * 2, -7), glm::vec2(0.5, 7), "brickwall3.obj");

	make3DWall(engine, glm::vec2(14 * 2, 7+14), glm::vec2(0.5, 7), "brickwall3.obj");
	make3DWall(engine, glm::vec2(-14 * 2, 7 + 14), glm::vec2(0.5, 7), "brickwall3.obj");

	make3DWall(engine, glm::vec2(14 * 2, -7 + 14), glm::vec2(0.5, 7), "brickwall3.obj");
	make3DWall(engine, glm::vec2(-14 * 2, -7 + 14), glm::vec2(0.5, 7), "brickwall3.obj");

	const std::array<glm::vec2, 2> positions = { glm::vec2(4, 1.5), glm::vec2(-4, 1.5) };
	for (int i = 0; i < 2 && i < players.size(); ++i) { // this map is designed for two players max
		players[i](engine, positions[i], i+1);
	}

	makeDesert(engine);

	engine.graphicsSystem.makeSkybox(dayskyboxnames);
}

void spaceWorld(Engine& engine, const std::vector<CharacterFunction>& players)
{
	const std::array<glm::vec2, 2> positions = { glm::vec2(8, 1.5), glm::vec2(-8, 1.5) };
	for (int i = 0; i < 2 && i < players.size(); ++i) { // this map is designed for two players max
		players[i](engine, positions[i], i + 1);
	}

	makeSlidingPlatform(engine, glm::vec2(16, -16), glm::vec2(2, 0.5), 100, 8, 100, "space_platform.obj");
	makeSlidingPlatform(engine, glm::vec2(-16, -16), glm::vec2(2, 0.5), 100, 32, 100, "space_platform.obj");
	makeSlidingPlatform(engine, glm::vec2(0, -16), glm::vec2(2, 0.5), 100, 8, 100, "space_platform.obj");

	makeSlidingPlatform(engine, glm::vec2(8, 0), glm::vec2(2, 0.5), 1000, 12, 50, "space_platform.obj");
	makeSlidingPlatform(engine, glm::vec2(-8, 0), glm::vec2(2, 0.5), 1000, 12, 50, "space_platform.obj");

	makeSlidingPlatform(engine, glm::vec2(0, 16), glm::vec2(2, 0.5), 2000, 48, 400, "space_platform.obj");
	makeSlidingPlatform(engine, glm::vec2(16, 16), glm::vec2(2, 0.5), 2000, 48, 400, "space_platform.obj");
	makeSlidingPlatform(engine, glm::vec2(-16, 16), glm::vec2(2, 0.5), 2000, 48, 400, "space_platform.obj");

	makeSlidingPlatform(engine, glm::vec2(8, 32), glm::vec2(2, 0.5), 1000, 12, 50, "space_platform.obj");
	makeSlidingPlatform(engine, glm::vec2(-8, 32), glm::vec2(2, 0.5), 1000, 12, 50, "space_platform.obj");

	makeDamageZone(engine, 2, glm::vec2(0.f, -100.f), glm::vec2(10000.f, 20.f));


	// walls
	make3DWall(engine, glm::vec2(14 * 2, 7), glm::vec2(1, 20), "greeble_wall.obj");
	make3DWall(engine, glm::vec2(-14 * 2, 7), glm::vec2(1, 20), "greeble_wall.obj");

	make3DWall(engine, glm::vec2(14 * 2, 7+20), glm::vec2(1, 20), "greeble_wall.obj");
	make3DWall(engine, glm::vec2(-14 * 2, 7+20), glm::vec2(1, 20), "greeble_wall.obj");

	engine.graphicsSystem.sunlightDir = glm::normalize(glm::vec3(-1, 0.2, 0.1));

	makeAsteroids(engine);
	engine.graphicsSystem.ambientIntensity.r *= 1.5;
	engine.graphicsSystem.ambientIntensity.b *= 1.5;
	engine.graphicsSystem.ambientIntensity.g *= 0.5;
	engine.graphicsSystem.makeSkybox(nightskyboxnames);
}

Entity& makeFloatingPlatform(Engine& engine, glm::vec2 position, glm::vec2 size, float mass, float range, float spring, std::string path)
{
	Entity& wall = engine.makeEntity();
	wall.catagory = catagories::worldType;
	engine.positionSystem.makePositionComponent(wall, glm::vec3(position, 0));
	auto settings = defaultPhysicsSettings;
	settings.dampingFactor *= 2;
	settings.quadraticDampingFactor = 0.5;
	settings.gravAcceleration = 0;
	engine.physicsSystem.makePhysicsComponent(wall, std::make_optional(Collider(aRect(size))), glm::vec2(0), mass, settings);
	engine.graphicsSystem.makeModelComponent(wall, path);

	engine.physicsSystem.makeConstraintComponent(wall, position, glm::vec2(0.01f), glm::vec2(0.01f, range), spring);

	return wall;
}

void volcanoWorld(Engine& engine, const std::vector<CharacterFunction>& players)
{
	makeVolcano(engine);
	
	makeInvisibleWall(engine, glm::vec2(16 , 45), glm::vec2(1, 45));
	makeInvisibleWall(engine, glm::vec2(-16 , 45), glm::vec2(1, 45));
	makeInvisibleWall(engine, glm::vec2(0, 92), glm::vec2(36, 2));

	makeDamageZone(engine, 2, glm::vec2(0.f, -50.1f), glm::vec2(10000.f, 50.3f));

	makeFloatingPlatform(engine, glm::vec2(8, 0.1), glm::vec2(2, 0.5), 1000, 3, 200, "space_platform.obj");
	makeFloatingPlatform(engine, glm::vec2(-8, 0.1), glm::vec2(2, 0.5), 1000, 3, 200, "space_platform.obj");
	const std::array<glm::vec2, 2> positions = { glm::vec2(8, 1.5), glm::vec2(-8, 1.5) };
	for (int i = 0; i < 2 && i < players.size(); ++i) { // this map is designed for two players max
		players[i](engine, positions[i], i + 1);
	}
	engine.graphicsSystem.sunlightDir = glm::normalize(glm::vec3(0, -1, 0.1));
	engine.graphicsSystem.sunlightIntensity = glm::vec3(2.5, 0.4, 0.1);
	engine.graphicsSystem.doLava = true;

	engine.graphicsSystem.ambientIntensity.r *= 2.;
	engine.graphicsSystem.ambientIntensity.b *= 0.5;

	engine.graphicsSystem.makeSkybox(nightskyboxnames);
}