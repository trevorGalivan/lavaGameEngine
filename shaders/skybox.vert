#version 460 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 worldToClip;


void main()
{
    TexCoords = aPos;
    vec4 pos = worldToClip * vec4(aPos, 1.0);
    gl_Position = vec4(pos.xyz, pos.z*1.0000001);
} 