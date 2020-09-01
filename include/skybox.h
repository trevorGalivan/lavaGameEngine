#pragma once
#include <array>

class ShaderProgram;

class Skybox
{
	unsigned int texID;
	unsigned int VAO, VBO, EBO;
public:
	Skybox(std::array<std::string, 6> filenames); // file names should be of the form basename + <front, left, back, etc>
	~Skybox();

	Skybox(const Skybox&) = delete;
	void Draw(ShaderProgram& shaderProg);
};

