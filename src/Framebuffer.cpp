#include "Framebuffer.h"

#include <glad/glad.h>

#include <iostream>
BasicFramebuffer::BasicFramebuffer()
{
	glGenFramebuffers(1, &FboID);
}

BasicFramebuffer::BasicFramebuffer(BasicFramebuffer&& other) noexcept {
	this->FboID = other.FboID;
	other.FboID = 0;
}
BasicFramebuffer& BasicFramebuffer::operator=(BasicFramebuffer&& other) noexcept
{
	if (this != &other) {
		if (FboID)
			glDeleteFramebuffers(1, &FboID);

		this->FboID = other.FboID;
		other.FboID = 0;
	}
	return *this;
}

BasicFramebuffer::~BasicFramebuffer()
{
	if(FboID)
	glDeleteFramebuffers(1, &FboID);
}

void BasicFramebuffer::bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, FboID);
}

void BasicFramebuffer::bindRead()
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, FboID);
}
void BasicFramebuffer::bindWrite()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FboID);
}

simpleColourFramebuffer::simpleColourFramebuffer(unsigned int hRes, unsigned int yRes) {
	bind();
	// attach HDR colour texture
	glGenTextures(1, &colorTexID);
	glBindTexture(GL_TEXTURE_2D, colorTexID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, hRes, yRes, 0, GL_RGBA, GL_HALF_FLOAT, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexID, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Colour framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

simpleColourFramebuffer::simpleColourFramebuffer(simpleColourFramebuffer&& other) noexcept : BasicFramebuffer(std::move(other))
{
	this->colorTexID = other.colorTexID;

	other.colorTexID = 0;

}

simpleColourFramebuffer& simpleColourFramebuffer::operator=(simpleColourFramebuffer&& other) noexcept
{
	if (this != &other) {
		BasicFramebuffer::operator=(std::move(other));

		if (colorTexID)
			glDeleteTextures(1, &colorTexID);

		this->colorTexID = other.colorTexID;

		other.colorTexID = 0;

	}
	return *this;
}

simpleColourFramebuffer::~simpleColourFramebuffer()
{
	if (colorTexID)
	glDeleteTextures(1, &colorTexID);

}

void simpleColourFramebuffer::resize(unsigned int x, unsigned int y)
{
	auto temp = simpleColourFramebuffer(x, y);
	*this = std::move(temp);
}



LDRcolourbuffer::LDRcolourbuffer(unsigned int hRes, unsigned int yRes) {
	bind();
	// attach HDR colour texture
	glGenTextures(1, &colorTexID);
	glBindTexture(GL_TEXTURE_2D, colorTexID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, hRes, yRes, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexID, 0);
	// attach depth buffer

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Colour framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

LDRcolourbuffer::LDRcolourbuffer(LDRcolourbuffer&& other) noexcept : BasicFramebuffer(std::move(other))
{
	this->colorTexID = other.colorTexID;

	other.colorTexID = 0;
}

LDRcolourbuffer& LDRcolourbuffer::operator=(LDRcolourbuffer&& other) noexcept
{
	if (this != &other) {
		BasicFramebuffer::operator=(std::move(other));

		if (colorTexID)
			glDeleteTextures(1, &colorTexID);

		this->colorTexID = other.colorTexID;

		other.colorTexID = 0;

	}
	return *this;
}

LDRcolourbuffer::~LDRcolourbuffer()
{
	if (colorTexID)
		glDeleteTextures(1, &colorTexID);
}

void LDRcolourbuffer::resize(unsigned int x, unsigned int y)
{
	auto temp = LDRcolourbuffer(x, y);
	*this = std::move(temp);
}



Gbuffer::Gbuffer(unsigned int hRes, unsigned int yRes)
{
	bind();
	// attach SDR (SRGB_Alpha) colour texture
	glGenTextures(1, &colorTexID);
	glBindTexture(GL_TEXTURE_2D, colorTexID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, hRes, yRes, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexID, 0);

	// attach normal buffer
	glGenTextures(1, &normalTexID);
	glBindTexture(GL_TEXTURE_2D, normalTexID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, hRes, yRes, 0, GL_RGBA, GL_HALF_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normalTexID, 0);

	// attach material buffer
	glGenTextures(1, &materialTexID);
	glBindTexture(GL_TEXTURE_2D, materialTexID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, hRes, yRes, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, materialTexID, 0);

	glGenTextures(1, &depthTexID);
	glBindTexture(GL_TEXTURE_2D, depthTexID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, hRes, yRes, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexID, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Gbuffer is not complete!" << std::endl;

	int encoding = 100;
	glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING, &encoding);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
Gbuffer::Gbuffer(Gbuffer&& other) noexcept
{
	this->colorTexID = other.colorTexID;
	this->materialTexID = other.materialTexID;
	this->normalTexID = other.normalTexID;
	this->depthTexID = other.depthTexID;
	other.colorTexID = 0;
	other.materialTexID = 0;
	other.normalTexID = 0;
	other.depthTexID = 0;
}

Gbuffer& Gbuffer::operator=(Gbuffer&& other) noexcept
{
	if (this != &other) {
		BasicFramebuffer::operator=(std::move(other));

		if (colorTexID)
			glDeleteTextures(1, &colorTexID);
		if (normalTexID)
			glDeleteTextures(1, &normalTexID);
		if (materialTexID)
			glDeleteTextures(1, &materialTexID);
		if (depthTexID)
			glDeleteTextures(1, &depthTexID);

		this->colorTexID = other.colorTexID;
		this->materialTexID = other.materialTexID;
		this->normalTexID = other.normalTexID;
		this->depthTexID = other.depthTexID;
		other.colorTexID = 0;
		other.materialTexID = 0;
		other.normalTexID = 0;
		other.depthTexID = 0;
	}
	return *this;
}
Gbuffer::~Gbuffer()
{
	if (colorTexID)
		glDeleteTextures(1, &colorTexID);
	if (normalTexID)
		glDeleteTextures(1, &normalTexID);
	if (materialTexID)
		glDeleteTextures(1, &materialTexID);
	if (depthTexID)
		glDeleteTextures(1, &depthTexID);
}

void Gbuffer::resize(unsigned int x, unsigned int y)
{
	auto temp = Gbuffer(x, y);
	*this = std::move(temp);
}