#version 460 core
layout (location = 0) out vec3 gColor;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gMaterial;
 
in vec3 FragPos;
in vec2 TexCoord;

uniform sampler3D lavaTex;
uniform float time;
void main()
{
    float colorTex = texture(lavaTex, vec3(TexCoord, time)).r;

    gColor = vec3(1, 0.1, 0.01);
    gNormal = vec3(0,1,0);
    gMaterial = vec4(0, 0.5, colorTex * colorTex * 0.9 + 0.1, 1);
}
