#include "graphicsUtils.h"

#include "SMAA/AreaTex.h"
#include "SMAA/SearchTex.h"

#include <glad/glad.h>

unsigned int getSMAAareaTex()
{
	unsigned int areaTex;
	glGenTextures(1, &areaTex);
	glBindTexture(GL_TEXTURE_2D, areaTex);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, AREATEX_WIDTH, AREATEX_HEIGHT, 0, GL_RG, GL_UNSIGNED_BYTE, areaTexBytes);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	

	return areaTex;
}

unsigned int getSMAAsearchTex()
{
	unsigned int searchTex;

	glGenTextures(1, &searchTex);
	glBindTexture(GL_TEXTURE_2D, searchTex);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, SEARCHTEX_WIDTH, SEARCHTEX_HEIGHT, 0, GL_RED, GL_UNSIGNED_BYTE, searchTexBytes);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	return searchTex;
}

std::vector<float> normalDistribution(unsigned int kernelWidth, unsigned int gaussianWidth)
{
	std::vector<float> distibution;
	distibution.reserve(kernelWidth);

	float n = float(gaussianWidth - 1);
	float x = float(1);
	//float total = pow(2, n);
	float sum = 0;

	unsigned int start = (gaussianWidth - kernelWidth) / 2;
	unsigned int end = gaussianWidth - start;
	for (unsigned int k = 0; k < end; ++k) {
		if (k >= start) {
			distibution.push_back(x);
			sum += x;
		}
		x = x * (n - k) / (k + 1);
	}

	for (auto x : distibution) {
		x /= sum;
	}

	return distibution;
}