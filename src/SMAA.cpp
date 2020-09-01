#include "SMAA.h"

#include <fstream>
#include <iostream>
#include <sstream>

const std::string header_vs =
"#version 460 core\n\
 uniform vec4 SMAA_RT_METRICS = vec4(1./640.,1./640.,640.,640.);\n\
 #define SMAA_PRESET_ULTRA 1\n\
 #define SMAA_GLSL_4 1\n\
 #define SMAA_INCLUDE_PS 0\n\
 #define SMAA_INCLUDE_VS 1\n\
";

const std::string header_ps =
"#version 460 core\n\
 uniform vec4 SMAA_RT_METRICS = vec4(1./640.,1./640.,640.,640.);\n\
 #define SMAA_PRESET_ULTRA 1\n\
 #define SMAA_GLSL_4 1\n\
 #define SMAA_INCLUDE_PS 1\n\
 #define SMAA_INCLUDE_VS 0\n\
";

const std::string edge_vs =
"layout(location = 0) in vec2 aPos;\n\
layout(location = 1) in vec2 aTexCoords;\n\
out vec4 offset0;\n\
out vec4 offset1;\n\
out vec4 offset2;\n\
out vec2 TexCoords;\n\
void SMAAEdgeDetectionVS(vec2 texcoord, inout vec4 offset[3]);\n\
void main(){\n\
gl_Position = vec4(aPos.x, aPos.y, 1.0, 1.0);\n\
TexCoords = aTexCoords;\n\
vec4 offset[3];\n\
SMAAEdgeDetectionVS(aTexCoords, offset);\n\
offset0 = offset[0];\n\
offset1 = offset[1];\n\
offset2 = offset[2];\n\
}\n";

const std::string edge_ps =
"uniform sampler2D icolor;\n\
in vec4 offset0;\n\
in vec4 offset1;\n\
in vec4 offset2;\n\
in vec2 TexCoords;\n\
out vec4 FragColor;\n\
vec2 SMAAColorEdgeDetectionPS(vec2 texcoord, vec4 offset[3], in sampler2D color);\n\
void main(){\n\
vec4 offset[3];\n\
offset[0] = offset0;\n\
offset[1] = offset1;\n\
offset[2] = offset2;\n\
FragColor = vec4( SMAAColorEdgeDetectionPS(TexCoords, offset, icolor),0,0 );\n\
}\n";

const std::string weights_vs =
"layout(location = 0) in vec2 aPos;\n\
layout(location = 1) in vec2 aTexCoords;\n\
out vec4 offset0;\n\
out vec4 offset1;\n\
out vec4 offset2;\n\
out vec2 TexCoords;\n\
out vec2 PixCoords;\n\
void SMAABlendingWeightCalculationVS(vec2 texcoord, out vec2 pixcoord, inout vec4 offset[3]);\n\
void main(){\n\
gl_Position = vec4(aPos.x, aPos.y, 1.0, 1.0);\n\
TexCoords = aTexCoords;\n\
vec4 offset[3];\n\
SMAABlendingWeightCalculationVS(aTexCoords, PixCoords, offset);\n\
offset0 = offset[0];\n\
offset1 = offset[1];\n\
offset2 = offset[2];\n\
}\n";

const std::string weights_ps =
"uniform sampler2D iedges;\n\
uniform sampler2D areaTex;\n\
uniform sampler2D searchTex;\n\
in vec4 offset0;\n\
in vec4 offset1;\n\
in vec4 offset2;\n\
in vec2 TexCoords;\n\
in vec2 PixCoords;\n\
out vec4 FragColor;\n\
vec4 SMAABlendingWeightCalculationPS(vec2 texcoord,\
vec2 pixcoord,\
vec4 offset[3],\
sampler2D edgesTex,\
sampler2D areaTex,\
sampler2D searchTex,\
vec4 subsampleIndices);\
void main(){\n\
vec4 offset[3];\n\
offset[0] = offset0;\n\
offset[1] = offset1;\n\
offset[2] = offset2;\n\
FragColor = SMAABlendingWeightCalculationPS(TexCoords, PixCoords, offset, iedges, areaTex, searchTex, vec4(0));\n\
}\n";

const std::string blend_vs =
"layout(location = 0) in vec2 aPos;\n\
layout(location = 1) in vec2 aTexCoords;\n\
out vec4 offset;\n\
out vec2 TexCoords;\n\
void SMAANeighborhoodBlendingVS(vec2 texcoord, out vec4 offset);\n\
void main(){\n\
gl_Position = vec4(aPos.x, aPos.y, 1.0, 1.0);\n\
TexCoords = aTexCoords;\n\
SMAANeighborhoodBlendingVS(aTexCoords, offset);\n\
}\n";

const std::string blend_ps =
"uniform sampler2D icolor;\n\
uniform sampler2D iblendTex;\n\
in vec4 offset;\n\
in vec2 TexCoords;\n\
out vec4 FragColor;\n\
vec4 SMAANeighborhoodBlendingPS(vec2 texcoord, vec4 offset, in sampler2D color, in sampler2D blendTex);\n\
void main(){\n\
FragColor = SMAANeighborhoodBlendingPS(TexCoords, offset, icolor, iblendTex);\n\
}\n";

std::string SMAA_code;

void loadSMAA() {
    if (SMAA_code.empty()) {
        std::ifstream f("SMAA/SMAA.hlsl");
        std::stringstream s;
        s << f.rdbuf();
        SMAA_code = s.str();
    }
}

std::string getSMAAEdgeFragmentCode()
{
    loadSMAA();
    return header_ps + edge_ps + SMAA_code;
}

std::string getSMAAEdgeVertexCode()
{
    loadSMAA();
    return header_vs + edge_vs + SMAA_code;
}

std::string getSMAABlendWeightFragmentCode() {
    loadSMAA();
    return header_ps + weights_ps + SMAA_code;
}
std::string getSMAABlendWeightVertexCode()
{
    loadSMAA();
    return header_vs + weights_vs + SMAA_code;
}

std::string getSMAABlendFragmentCode() {
    loadSMAA();
    return header_ps + blend_ps + SMAA_code;
}
std::string getSMAABlendVertexCode()
{
    loadSMAA();
    return header_vs + blend_vs + SMAA_code;
}
// create source file by doing header_xx + <SMAA code> + <shader code>

