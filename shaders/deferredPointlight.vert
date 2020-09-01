#version 460 core
layout (location = 0) in vec3 aPos;

uniform mat4x4 modelToClip;

void main()
{
    gl_Position = modelToClip * vec4(aPos, 1.0);
}