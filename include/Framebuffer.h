#pragma once
class BasicFramebuffer
{
public:
	unsigned int FboID;
	BasicFramebuffer();
	~BasicFramebuffer();

	BasicFramebuffer(const BasicFramebuffer&) = delete;
	BasicFramebuffer(BasicFramebuffer&& other) noexcept;

	BasicFramebuffer& operator=(BasicFramebuffer&& other) noexcept;

	void bind();
	void bindRead();
	void bindWrite();
};

// has a single (16 bit float) colour attachment, with a depth/stencil renderbuffer
class simpleColourFramebuffer : public BasicFramebuffer
{
public:
	unsigned int colorTexID;

	simpleColourFramebuffer(unsigned int hRes, unsigned int yRes);
	simpleColourFramebuffer(simpleColourFramebuffer&& other) noexcept;

	simpleColourFramebuffer& operator=(simpleColourFramebuffer&& other) noexcept;
	~simpleColourFramebuffer();

	void resize(unsigned int x, unsigned int y);
};

// 32 bit SRGB_Alpha colour buffer
class LDRcolourbuffer : public BasicFramebuffer
{
public:
	unsigned int colorTexID;

	LDRcolourbuffer(unsigned int hRes, unsigned int yRes);
	LDRcolourbuffer(LDRcolourbuffer&& other) noexcept;

	LDRcolourbuffer& operator=(LDRcolourbuffer&& other) noexcept;
	~LDRcolourbuffer();

	void resize(unsigned int x, unsigned int y);
};

// stores all the data needed for the lighting calculations
class Gbuffer : public BasicFramebuffer
{
public:
	unsigned int colorTexID;
	unsigned int depthTexID;
	unsigned int normalTexID;
	unsigned int materialTexID;

	Gbuffer(unsigned int hRes, unsigned int yRes);
	Gbuffer(Gbuffer&& other) noexcept;

	Gbuffer& operator=(Gbuffer&& other) noexcept;
	~Gbuffer();

	void resize(unsigned int x, unsigned int y);
};

