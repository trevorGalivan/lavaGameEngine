#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 3) in vec2 aTexCoord;

out vec3 FragPos;
out vec2 TexCoord;

uniform sampler3D lavaTex;

uniform float time;

uniform mat4x4 toScreen;
uniform mat4x4 toClip;
uniform mat4x4 toWorld = mat4x4(vec2(1, 0).xyyy, vec2(1, 0).yxyy, vec2(1, 0).yyxy, vec2(1, 0).yyyx) ;
void main()
{
    float Tex = texture(lavaTex, vec3(aTexCoord, time)).r - 0.5;
    Tex += texture(lavaTex, vec3(aTexCoord, time * 2)).r * 0.5;
    vec4 pos = toClip * toScreen * toWorld * vec4(aPos + vec3(0, -Tex * 0.04 ,0), 1.0);



    gl_Position = pos;
    TexCoord = aTexCoord;

    FragPos = vec3(toWorld * vec4(aPos, 1.0));
}