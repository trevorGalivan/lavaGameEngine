#pragma once
#include <string>

namespace fileUtils {

	std::string getAssetsDirectory();

	std::string getShadersDirectory();

	// loads the texture at the given path into an openGL texture, and returns the texture ID
	unsigned int makeGLtexture2D(const std::string& filename, bool isSRGB, bool isBilinear = true);

	// loads the given texture data into an openGL texture, and returns the texture ID
	unsigned int makeGLtexture2D(unsigned char* rawData, int width, int height, int nrComponents, bool isSRGB, bool isBilinear = true);
}
