#pragma once
#include "system.h"
#include "Camera.h"
#include "ShaderProgram.h"
#include "skybox.h"
#include "model.h"
#include "sprite.h"
#include "Framebuffer.h"

#include <glm/gtx/transform.hpp>

#include <mutex>

class Engine;

// This component allows objects to be rendered to the screen as 3-D objects
// Entities with this component must also have positionComponent
// For 2D clip-space graphics, use UISystem

class GraphicsSystem;

class ModelComponent : public Component<ModelComponent>
{
public:
	typedef GraphicsSystem owningSystem;
	glm::mat4x4 baseMatrix; // applied before the rotation/translation matrix. MUST BE INVERTIBLE
	Model* model;
	std::optional<glm::dquat> angularVelocity;
};

// A 2D or "2.5D" sprite. Will be fit to the bounding box provided by halfExtents
class SpriteComponent : public Component<SpriteComponent>
{
public:
	typedef GraphicsSystem owningSystem;
	glm::vec2 halfExtents;
	double animationSpeed;
	double animationPosition;
	Sprite* sprite;
	std::string path;

	bool flipX;
};

class LightComponent : public Component<LightComponent>
{
public:
	glm::vec3 color;
	float constant;
	float linear;
	float quadratic;

	typedef GraphicsSystem owningSystem;

};

class GraphicsSystem : System
{
	ComponentManager<SpriteComponent> sprites;
	ComponentManager<ModelComponent> models;
	ComponentManager<LightComponent> lights;
	std::map<std::string, Sprite> rawSprites;
	//std::map<std::string, Texture> rawTextures;
	std::map<std::string, Model> rawModels;

	ShaderProgram renderProg;
	ShaderProgram skyProg;
	ShaderProgram outputProg;
	ShaderProgram sunLightProg;
	ShaderProgram pointLightProg;
	ShaderProgram copyProg;
	ShaderProgram bloomPrepassProg;
	ShaderProgram gaussianBlurProg;
	ShaderProgram lavaProg;

	

	unsigned int screenQuadVAO, screenQuadVBO, screenQuadEBO;
	unsigned int unitSquareVAO, unitSquareVBO, unitSquareEBO;
	unsigned int unitCubeVAO, unitCubeVBO, unitCubeEBO;

	simpleColourFramebuffer outputBuffer;
	Gbuffer gBuffer;

	simpleColourFramebuffer blurbuffer1;
	simpleColourFramebuffer blurbuffer2;

	struct SMAA {
		SMAA(unsigned int xres, unsigned int vres) : edges(xres, vres), blend(xres, vres) {}

		ShaderProgram SMAAedgesProg;
		ShaderProgram SMAAweightsProg;
		ShaderProgram SMAAblendProg;
		unsigned int areaTex;
		unsigned int searchTex;

		simpleColourFramebuffer edges;
		simpleColourFramebuffer blend;
	};

	SMAA smaa;

	LDRcolourbuffer finalOutputBuffer;
	std::optional<Skybox> skybox;

	std::unique_ptr<Mesh> grid = makeGrid(glm::vec2(1.), glm::uvec2( 400));

	unsigned int lava;
public:
	bool doLava;

	glm::vec3 sunlightDir = glm::normalize(glm::vec3(0, 4, 1));
	glm::vec3 sunlightIntensity = glm::vec3(1, 1, 1);
	glm::vec3 ambientIntensity = glm::vec3(0.04, 0.04, 0.04);

	glm::uvec2 resolution = glm::uvec2(400u, 400u);
	std::mutex cameraMutex;
	Camera camera;
	GraphicsSystem();
	void update(Engine& engine, double dt);
	void draw(Engine& engine, double dt); // Does NOT swap buffers

	ModelComponent* getComponent(handle<ModelComponent> ID);
	void makeModelComponent(Entity& parent, std::string path, glm::mat4x4 baseMatrix = glm::identity<glm::mat4x4>() , std::optional<glm::dquat> angularVelocity = std::nullopt);
	void destroyComponent(handle<ModelComponent> ID);

	SpriteComponent* getComponent(handle<SpriteComponent> ID);
	void makeSpriteComponent(Entity& parent, std::string path, glm::vec2 halfExtents, double animationSpeed = 1., bool flipX = false);
	void destroyComponent(handle<SpriteComponent> ID);

	LightComponent* getComponent(handle<LightComponent> ID);
	void makeLightComponent(Entity& parent, glm::vec3 color, float constantAttenuation = 1, float linearAttenuation = 0, float quadraticAttenuation = 0);
	void destroyComponent(handle<LightComponent> ID);

	void switchSprite_SECONDARYTHREAD(handle<SpriteComponent> ID, std::string path);

	void onResizeFramebuffer();

	void makeSkybox(std::array<std::string, 6> imagesnames);
};


