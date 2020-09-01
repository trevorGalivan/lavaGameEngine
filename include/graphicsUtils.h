#pragma once
#include <glm/glm.hpp>

#include <vector>
#include <iostream>

inline float luma(glm::vec3 linearRGB)
{
	return 0.2126f * linearRGB.r + 0.7152f * linearRGB.g + 0.0722f * linearRGB.b;
}

// returns a normal distribution centered at width/2. 
std::vector<float> normalDistribution(unsigned int kernelWidth, unsigned int gaussianWidth);


unsigned int getSMAAareaTex();

unsigned int getSMAAsearchTex();