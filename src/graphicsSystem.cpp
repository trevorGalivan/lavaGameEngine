#include "graphicsSystem.h"

#include "engine.h"
#include "model.h"
#include "sprite.h"
#include "skybox.h"
#include "graphicsUtils.h"
#include "SMAA.h"
#include "fileUtils.h"

#include "stb_image.h"
#include <glad/glad.h>
#include <glm/gtx/quaternion.hpp>

#include <iostream>

void GraphicsSystem::makeSkybox(std::array<std::string, 6> imagesnames)
{
	skybox.emplace(imagesnames);
}



Sprite* accessOrMake(std::map<std::string, Sprite>& rawSprites, std::string path) {
	if (rawSprites.find(path) == rawSprites.end()) {
		rawSprites.emplace(path, Sprite(path.c_str()));
	}
	return &rawSprites.at(path);
}



GraphicsSystem::GraphicsSystem() : camera(glm::vec3(0., 5., 0.), glm::vec3(0)), outputBuffer(640, 640), gBuffer(640, 640), blurbuffer1(320, 320), blurbuffer2(320, 320), smaa(640, 640), finalOutputBuffer(640, 640)
{
	// build shader for filling gBuffer
	Shader vertex("doNothing.vert", GL_VERTEX_SHADER);
	Shader fragment("fillGbuffer.frag", GL_FRAGMENT_SHADER);
	renderProg.attach(vertex);
	renderProg.attach(fragment);
	renderProg.link();
	renderProg.use();
	
	Shader skyVertex("skybox.vert", GL_VERTEX_SHADER);
	Shader skyFragment("skybox.frag", GL_FRAGMENT_SHADER);
	skyProg.attach(skyVertex);
	skyProg.attach(skyFragment);
	skyProg.link();
	skyProg.use();

	Shader lavaVert("lava.vert", GL_VERTEX_SHADER);
	Shader lavaFrag("lava.frag", GL_FRAGMENT_SHADER);
	lavaProg.attach(lavaVert);
	lavaProg.attach(lavaFrag);
	lavaProg.link();
	lavaProg.use();

	Shader outputVertex("pasteFramebuffer.vert", GL_VERTEX_SHADER);
	{
		Shader outputFragment("tonemap.frag", GL_FRAGMENT_SHADER);
		outputProg.attach(outputVertex);
		outputProg.attach(outputFragment);
		outputProg.link();
		outputProg.use();
	}
	{
		Shader sunlightFragment("deferredSunlight.frag", GL_FRAGMENT_SHADER);
		sunLightProg.attach(outputVertex);
		sunLightProg.attach(sunlightFragment);
		sunLightProg.link();
		sunLightProg.use();
	}

	{
		Shader pointlightVertex("deferredPointlight.vert", GL_VERTEX_SHADER);
		Shader pointlightFragment("deferredPointlight.frag", GL_FRAGMENT_SHADER);
		pointLightProg.attach(pointlightVertex);
		pointLightProg.attach(pointlightFragment);
		pointLightProg.link();
		pointLightProg.use();
	}
	{
		Shader copyFragment("pasteFramebuffer.frag", GL_FRAGMENT_SHADER);
		copyProg.attach(outputVertex);
		copyProg.attach(copyFragment);
		copyProg.link();
		copyProg.use();
	}
	{
		Shader bloomPrepassFragment("bloomPrepass.frag", GL_FRAGMENT_SHADER);
		bloomPrepassProg.attach(outputVertex);
		bloomPrepassProg.attach(bloomPrepassFragment);
		bloomPrepassProg.link();
		bloomPrepassProg.use();
	}
	{
		Shader blurFragment("blur.frag", GL_FRAGMENT_SHADER);
		gaussianBlurProg.attach(blurFragment);
		gaussianBlurProg.attach(outputVertex);
		gaussianBlurProg.link();
		gaussianBlurProg.use();
	}
	
	{
		Shader SMAAedgefrag("", GL_FRAGMENT_SHADER, getSMAAEdgeFragmentCode());
		Shader SMAAedgevert("", GL_VERTEX_SHADER, getSMAAEdgeVertexCode());

		smaa.SMAAedgesProg.attach(SMAAedgefrag);
		smaa.SMAAedgesProg.attach(SMAAedgevert);
		smaa.SMAAedgesProg.link();
		smaa.SMAAedgesProg.use();
	}

	smaa.areaTex = getSMAAareaTex();
	smaa.searchTex = getSMAAsearchTex();

	{
		Shader SMAAweightsfrag("", GL_FRAGMENT_SHADER, getSMAABlendWeightFragmentCode());
		Shader SMAAweightsvert("", GL_VERTEX_SHADER, getSMAABlendWeightVertexCode());

		smaa.SMAAweightsProg.attach(SMAAweightsfrag);
		smaa.SMAAweightsProg.attach(SMAAweightsvert);
		smaa.SMAAweightsProg.link();
		smaa.SMAAweightsProg.use();
	}

	{
		Shader SMAAblendfrag("", GL_FRAGMENT_SHADER, getSMAABlendFragmentCode());
		Shader SMAAblendvert("", GL_VERTEX_SHADER, getSMAABlendVertexCode());

		smaa.SMAAblendProg.attach(SMAAblendfrag);
		smaa.SMAAblendProg.attach(SMAAblendvert);
		smaa.SMAAblendProg.link();
		smaa.SMAAblendProg.use();
	}

	// create unit square for sprite rendering
	Vertex unitSquare[4] =
	{//  position              normal coords       tangent coords      texture coords
		{glm::vec3(-1, 1, 0) , glm::vec3(0, 0, 1), glm::vec3(1, 0, 0), glm::vec2(0, 1)},
		{glm::vec3(-1, -1, 0), glm::vec3(0, 0, 1), glm::vec3(1, 0, 0), glm::vec2(0, 0)},
		{glm::vec3(1, 1, 0)  , glm::vec3(0, 0, 1), glm::vec3(1, 0, 0), glm::vec2(1, 1)},
		{glm::vec3(1, -1, 0) , glm::vec3(0, 0, 1), glm::vec3(1, 0, 0), glm::vec2(1, 0)},
	};

	unsigned int unitSquareIndices[6] =
	{
		0, 1, 2,
		2, 1, 3,
	};

	glGenVertexArrays(1, &unitSquareVAO);
	glGenBuffers(1, &unitSquareVBO);
	glGenBuffers(1, &unitSquareEBO);

	glBindVertexArray(unitSquareVAO);
	glBindBuffer(GL_ARRAY_BUFFER, unitSquareVBO);

	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(Vertex), unitSquare, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, unitSquareEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), unitSquareIndices, GL_STATIC_DRAW);

	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	// vertex normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	// vertex tangents
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
	// vertex texture coords
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));

	glBindVertexArray(0);

	struct screenQuadVertex
	{
		glm::vec2 pos;
		glm::vec2 texCoord;
	};

	// make screen quad
	screenQuadVertex screenQuad[4] =
	{//  position           texture coords
		{glm::vec2(-1, 1) , glm::vec2(0, 1)},
		{glm::vec2(-1, -1), glm::vec2(0, 0)},
		{glm::vec2(1, 1)  , glm::vec2(1, 1)},
		{glm::vec2(1, -1) , glm::vec2(1, 0)},
	};

	glGenVertexArrays(1, &screenQuadVAO);
	glGenBuffers(1, &screenQuadVBO);
	glGenBuffers(1, &screenQuadEBO);

	glBindVertexArray(screenQuadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, screenQuadVBO);

	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(screenQuadVertex), screenQuad, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, screenQuadEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), unitSquareIndices, GL_STATIC_DRAW);

	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(screenQuadVertex), (void*)0);
	// vertex texture coords
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(screenQuadVertex), (void*)offsetof(screenQuadVertex, texCoord));

	glBindVertexArray(0);

	glm::vec3 unitCube[8] =
	{ // position
		glm::vec3(1, 1, 1),
		glm::vec3(-1, 1, 1),
		glm::vec3(1, -1, 1),
		glm::vec3(-1, -1, 1),
		glm::vec3(1, 1, -1),
		glm::vec3(-1, 1, -1),
		glm::vec3(1, -1, -1),
		glm::vec3(-1, -1, -1),
	};

	unsigned int ucIndices[3 * 2 * 6] =
	{// top face
		4, 1, 0,
		5, 1, 4,
	 //bottom face
		6, 2, 3,
		7, 6, 3,
	 //front face
	    2, 0, 1,
		3, 2, 1,
	 //back face
		6, 5, 4,
		7, 5, 6,
	 //right face
		4, 0, 2,
		6, 4, 2,
	 //left face
	    5, 3, 1,
		7, 3, 5,
	};
	glGenVertexArrays(1, &unitCubeVAO);
	glGenBuffers(1, &unitCubeVBO);
	glGenBuffers(1, &unitCubeEBO);

	glBindVertexArray(unitCubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, unitCubeVBO);

	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(glm::vec3), unitCube, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, unitCubeEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * 2 * 6 * sizeof(unsigned int), ucIndices, GL_STATIC_DRAW);

	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

	glBindVertexArray(0);


	// load 3D noise texture
	{
		unsigned char* buffer = (unsigned char*)malloc(32 * 1028 * 1028);
		int x, y, c;
		for (int i = 0; i < 32; i++) {
			unsigned char* temp = stbi_load((fileUtils::getAssetsDirectory() + "noise/grey_simp2_" + std::to_string(i) + ".png").c_str(), &x, &y, &c, 1);
			memcpy(buffer + i * 1028 * 1028, temp, 1028 * 1028);
			stbi_image_free(temp);
		}
		glGenTextures(1, &lava);
		glBindTexture(GL_TEXTURE_3D, lava);
		

		glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, 1028, 1028, 32, 0, GL_RED, GL_UNSIGNED_BYTE, buffer);

		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);

		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		free(buffer);
	}



}

void GraphicsSystem::update(Engine& engine, double dt)
{
	;
}

void GraphicsSystem::draw(Engine& engine, double dt)
{
	glEnable(GL_FRAMEBUFFER_SRGB);
	gBuffer.bind();
	glClearColor(0.f, 0.f, 0.f, 1.0f);
	glDepthMask(1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glDisable(GL_BLEND);
	//glEnable(GL_MULTISAMPLE);
	
	glViewport(0, 0, resolution.x, resolution.y);

	renderProg.use();	

	for (int i = 0; i < 5; i++) {
		if (engine.checkButtonState(KEY_0 + i)) {
			renderProg.setInt("mapper", i);
		}
	}

	Camera cameraCopy;
	{
		std::lock_guard cameraLock(cameraMutex);
		cameraCopy = camera;
	}

	unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);

	// gBuffer fill pass
	renderProg.setMat4x4("toScreen", cameraCopy.getWorldMatrix());
	renderProg.setMat4x4("toClip", cameraCopy.getProjectionMatrix());
	renderProg.setVec3("camera.camPos", cameraCopy.getPos());
	// render all 3D models
	for (auto i = models.begin(), end = models.end(); i != end; ++i) {
		PositionComponent& positionComponent = *engine.getComponent(engine.getEntity( i->entity_ID)->positionHandle);
		
		if (i->angularVelocity) {
			positionComponent.rotation += dt * 0.5 * *(i->angularVelocity) * positionComponent.rotation;
			positionComponent.rotation = glm::normalize(positionComponent.rotation);
		}

		i->model->setModelMatrix(glm::translate(glm::mat4x4(1), glm::vec3(positionComponent.position)) * glm::toMat4(glm::quat(positionComponent.rotation)) * i->baseMatrix);
		i->model->Draw(renderProg);
	}

	bool reversedFaceCull = false; // if true, glCullFace is set to "front face" instead of back face

	// render all sprites
	for (auto i = sprites.begin(), end = sprites.end(); i != end; ++i) {
		glBindVertexArray(unitSquareVAO);
		const PositionComponent& positionComponent = *engine.getComponent(engine.getEntity(i->entity_ID)->positionHandle);
		i->animationPosition += i->animationSpeed * dt;
		if (i->sprite == nullptr) {
			i->sprite = accessOrMake(rawSprites, i->path);
		}
		if (i->flipX != reversedFaceCull) {
			reversedFaceCull = !reversedFaceCull;
			glCullFace(reversedFaceCull ? GL_FRONT : GL_BACK);
		}

		i->sprite->Draw(renderProg, i->animationPosition, positionComponent.position, i->halfExtents, positionComponent.rotation, i->flipX);
	}
	glCullFace(GL_BACK);

	if (doLava) {
		lavaProg.use();
		lavaProg.setMat4x4("toScreen", cameraCopy.getWorldMatrix());
		lavaProg.setMat4x4("toClip", cameraCopy.getProjectionMatrix());
		grid->Bind();
		lavaProg.setMat4x4("toWorld", glm::translate(glm::vec3(0, 0.5, 0)) * glm::scale(glm::vec3(20)));

		glActiveTexture(GL_TEXTURE0);
		lavaProg.setInt("lavaTex", 0);
		glBindTexture(GL_TEXTURE_3D, lava);
		lavaProg.setFloat("time", float(engine.getTime() * 0.04));

		glDrawElements(GL_TRIANGLES, grid->indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	// lighting
	
	gBuffer.bindRead();
	outputBuffer.bindWrite();
	glDepthMask(0);
	glDepthFunc(GL_NOTEQUAL);
	glClearColor(0.f, 0.f, 0.f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	// sunlight / ambient light pass
	sunLightProg.use();
	sunLightProg.setVec3("camera.camPos", cameraCopy.getPos());
	sunLightProg.setMat4x4("viewMatrixInv", glm::inverse(cameraCopy.getWorldMatrix()));
	sunLightProg.setMat4x4("projMatrixInv", glm::inverse(cameraCopy.getProjectionMatrix()));

	sunLightProg.setVec3("sunDir", sunlightDir);
	sunLightProg.setVec3("sunIntensity", sunlightIntensity);
	sunLightProg.setVec3("ambientIntensity", ambientIntensity);

	glActiveTexture(GL_TEXTURE0);
	sunLightProg.setInt("gColor", 0);
	glBindTexture(GL_TEXTURE_2D, gBuffer.colorTexID);
	glActiveTexture(GL_TEXTURE1);
	sunLightProg.setInt("gNormal", 1);
	glBindTexture(GL_TEXTURE_2D, gBuffer.normalTexID);
	glActiveTexture(GL_TEXTURE2);
	sunLightProg.setInt("gMaterial", 2);
	glBindTexture(GL_TEXTURE_2D, gBuffer.materialTexID);
	glActiveTexture(GL_TEXTURE3);
	sunLightProg.setInt("gDepth", 3);
	glBindTexture(GL_TEXTURE_2D, gBuffer.depthTexID);

	glBindVertexArray(screenQuadVAO);


	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	
	
	// point lights
	pointLightProg.use();
	pointLightProg.setVec2("resolution", resolution);
	pointLightProg.setVec3("camera.camPos", cameraCopy.getPos());
	pointLightProg.setMat4x4("viewMatrixInv", glm::inverse(cameraCopy.getWorldMatrix()));
	pointLightProg.setMat4x4("projMatrixInv", glm::inverse(cameraCopy.getProjectionMatrix()));

	glActiveTexture(GL_TEXTURE0);
	pointLightProg.setInt("gColor", 0);
	glBindTexture(GL_TEXTURE_2D, gBuffer.colorTexID);
	glActiveTexture(GL_TEXTURE1);
	pointLightProg.setInt("gNormal", 1);
	glBindTexture(GL_TEXTURE_2D, gBuffer.normalTexID);
	glActiveTexture(GL_TEXTURE2);
	pointLightProg.setInt("gMaterial", 2);
	glBindTexture(GL_TEXTURE_2D, gBuffer.materialTexID);
	glActiveTexture(GL_TEXTURE3);
	pointLightProg.setInt("gDepth", 3);
	glBindTexture(GL_TEXTURE_2D, gBuffer.depthTexID);

	glm::mat4x4 worldToClip = cameraCopy.getProjectionMatrix() * cameraCopy.getWorldMatrix();

	glBindVertexArray(unitCubeVAO);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glBlendFunc(GL_ONE, GL_ONE);
	glEnable(GL_BLEND);
	glDepthFunc(GL_GREATER);
	for (auto i = lights.begin(), iEnd = lights.end(); i != iEnd; ++i) {
		float brightness = luma(i->color);
		float extent = (-i->linear + sqrt(i->linear * i->linear - 4.f * (i->constant - brightness / (0.01f)) * i->quadratic)) / (2.f * i->quadratic);
		
		glm::vec3 position = engine.getComponent(engine.getEntity(i->entity_ID)->positionHandle)->position;
		pointLightProg.setMat4x4("modelToClip", worldToClip * (glm::translate(position) * glm::scale(glm::vec3(extent))));
		pointLightProg.setVec3("light.position", position);
		pointLightProg.setFloat("light.constant", i->constant);
		pointLightProg.setFloat("light.linear", i->linear);
		pointLightProg.setFloat("light.quadratic", i->quadratic);
		pointLightProg.setVec3("light.color", i->color);
		glDrawElements(GL_TRIANGLES, 3 * 2 * 6, GL_UNSIGNED_INT, 0);
	}


	glDisable(GL_BLEND);
	
	glDisable(GL_CULL_FACE);
	glDepthFunc(GL_LEQUAL);
	 //render skybox
	if (skybox) {
		skyProg.use();
		skyProg.setMat4x4("worldToClip", cameraCopy.getProjectionMatrix()* glm::mat4x4(glm::mat3x3(cameraCopy.getWorldMatrix())));
		skybox->Draw(skyProg);
	}
	
	// post processing
	glDisable(GL_DEPTH_TEST);
	// bloom
	glViewport(0, 0, resolution.x/2, resolution.y/2);
	{
		// prepass (bloom cutoff, downscale)
		blurbuffer1.bind();
		bloomPrepassProg.use();
		glBindVertexArray(screenQuadVAO);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);

		glActiveTexture(GL_TEXTURE0);
		bloomPrepassProg.setInt("screenTexture", 0);
		glBindTexture(GL_TEXTURE_2D, outputBuffer.colorTexID);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		// multi-pass blur
		for (int i = 0; i < 5; ++i) {
			blurbuffer2.bind();
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			gaussianBlurProg.use();
			glActiveTexture(GL_TEXTURE0);
			gaussianBlurProg.setBool("horizontal", true);
			gaussianBlurProg.setInt("screenTexture", 0);

			glBindTexture(GL_TEXTURE_2D, blurbuffer1.colorTexID);

			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

			blurbuffer1.bind();
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glActiveTexture(GL_TEXTURE0);
			gaussianBlurProg.setBool("horizontal", false);
			gaussianBlurProg.setInt("screenTexture", 0);

			glBindTexture(GL_TEXTURE_2D, blurbuffer2.colorTexID);

			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		}
		
		
		// add result to screen buffer
		glViewport(0, 0, resolution.x, resolution.y);
		glBlendFunc(GL_ONE, GL_ONE);
		glEnable(GL_BLEND);
		outputBuffer.bind();
		copyProg.use();
		glActiveTexture(GL_TEXTURE0);
		bloomPrepassProg.setInt("screenTexture", 0);

		glBindTexture(GL_TEXTURE_2D, blurbuffer1.colorTexID);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	// SMAA edge detection pass
	smaa.edges.bind();
	
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	smaa.SMAAedgesProg.use();
	smaa.SMAAedgesProg.setVec4("SMAA_RT_METRICS", glm::vec4(1.f / glm::vec2(resolution), resolution));
	glActiveTexture(GL_TEXTURE0);
	smaa.SMAAedgesProg.setInt("icolor", 0);
	glBindTexture(GL_TEXTURE_2D, outputBuffer.colorTexID);
	glBindVertexArray(screenQuadVAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	// SMAA blend weights pass
	
	smaa.blend.bind();

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	smaa.SMAAweightsProg.use();
	smaa.SMAAweightsProg.setVec4("SMAA_RT_METRICS", glm::vec4(1.f / glm::vec2(resolution), resolution));
	glActiveTexture(GL_TEXTURE0);
	smaa.SMAAweightsProg.setInt("iedges", 0);
	glBindTexture(GL_TEXTURE_2D, smaa.edges.colorTexID);
	glActiveTexture(GL_TEXTURE1);
	smaa.SMAAweightsProg.setInt("areaTex", 1);
	glBindTexture(GL_TEXTURE_2D, smaa.areaTex);
	glActiveTexture(GL_TEXTURE2);
	smaa.SMAAweightsProg.setInt("searchTex", 2);
	glBindTexture(GL_TEXTURE_2D, smaa.searchTex);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	
	// tonemapping, gamma correction
	finalOutputBuffer.bind();
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT );
	
	outputProg.use();
	glBindVertexArray(screenQuadVAO);

	glActiveTexture(GL_TEXTURE0);
	outputProg.setInt("screenTexture", 0);

	glBindTexture(GL_TEXTURE_2D, outputBuffer.colorTexID);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	// SMAA blend pass / final output
	//glDisable(GL_FRAMEBUFFER_SRGB);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	smaa.SMAAblendProg.use();
	smaa.SMAAblendProg.setVec4("SMAA_RT_METRICS", glm::vec4(1.f / glm::vec2(resolution), resolution));
	
	glActiveTexture(GL_TEXTURE0);
	smaa.SMAAblendProg.setInt("icolor", 0);
	glBindTexture(GL_TEXTURE_2D, finalOutputBuffer.colorTexID);
	glActiveTexture(GL_TEXTURE1);
	smaa.SMAAblendProg.setInt("iblendTex", 1);
	glBindTexture(GL_TEXTURE_2D, smaa.blend.colorTexID);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	
}

ModelComponent* GraphicsSystem::getComponent(handle<ModelComponent> ID)
{
	return models.find(ID);
}

void GraphicsSystem::makeModelComponent(Entity& parent, std::string path, glm::mat4x4 baseMatrix /*= glm::identity<glm::mat4x4>()*/, std::optional<glm::dquat> angularVelocity)
{
	ModelComponent component;
	if (rawModels.count(path) == 0) {
		rawModels.emplace(path, Model(path.c_str()));
	}
	component.baseMatrix = baseMatrix;
	component.model = &rawModels.at(path);
	component.entity_ID = parent.ID;
	component.angularVelocity = angularVelocity;
	parent.modelHandle = models.add(std::move(component));
}

void GraphicsSystem::destroyComponent(handle<ModelComponent> ID)
{
	//delete models.find(ID)->model;
	models.remove(ID);
}

SpriteComponent* GraphicsSystem::getComponent(handle<SpriteComponent> ID)
{
	return sprites.find(ID);
}

void GraphicsSystem::makeSpriteComponent(Entity& parent, std::string path, glm::vec2 halfExtents, double animationSpeed /*= 1.*/)
{
	SpriteComponent component;

	component.halfExtents = halfExtents;
	component.path = path;
	component.sprite = nullptr;// accessOrMake(rawSprites, path);
	component.entity_ID = parent.ID;
	component.animationSpeed = animationSpeed;
	component.animationPosition = 0.;
	parent.spriteHandle = sprites.add(std::move(component));
}

void GraphicsSystem::destroyComponent(handle<SpriteComponent> ID)
{
	//delete sprites.find(ID)->sprite;
	sprites.remove(ID);
}

void GraphicsSystem::switchSprite_SECONDARYTHREAD(handle<SpriteComponent> ID, std::string path)
{
	SpriteComponent& component = *sprites.find(ID);
	if (component.path != path) {
		component.path = path;
		component.sprite = nullptr;
	}
}

LightComponent* GraphicsSystem::getComponent(handle<LightComponent> ID)
{
	return lights.find(ID);
}
void GraphicsSystem::makeLightComponent(Entity& parent, glm::vec3 color, float constantAttenuation /*= 1*/, float linearAttenuation /*= 0*/, float quadraticAttenuation /*= 0*/)
{
	LightComponent component;
	component.entity_ID = parent.ID;
	component.color = color;
	component.constant = constantAttenuation;
	component.linear = linearAttenuation;
	component.quadratic = quadraticAttenuation;
	parent.lightHandle = lights.add(std::move(component));
}
void GraphicsSystem::destroyComponent(handle<LightComponent> ID)
{
	lights.remove(ID);
}

void GraphicsSystem::onResizeFramebuffer()
{
	//std::cout << resolution.x << ' ' << resolution.y << std::endl;
	outputBuffer.resize(resolution.x, resolution.y);
	gBuffer.resize(resolution.x, resolution.y);

	outputBuffer.bind();
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gBuffer.depthTexID, 0);

	blurbuffer1.resize(resolution.x / 2, resolution.y / 2);
	blurbuffer2.resize(resolution.x / 2, resolution.y / 2);

	smaa.blend.resize(resolution.x, resolution.y);
	smaa.edges.resize(resolution.x, resolution.y);
	
	finalOutputBuffer.resize(resolution.x, resolution.y);
}