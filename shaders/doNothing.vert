#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aTangent;
layout (location = 3) in vec2 aTexCoord;

out vec3 FragPos;

out mat3x3 TBN;

out vec2 TexCoord;


uniform mat4x4 toScreen;
uniform mat4x4 toClip;
uniform mat4x4 toWorld = mat4x4(vec2(1, 0).xyyy, vec2(1, 0).yxyy, vec2(1, 0).yyxy, vec2(1, 0).yyyx) ;
uniform mat4x4 toWorld_normal = mat4x4(vec2(1, 0).xyyy, vec2(1, 0).yxyy, vec2(1, 0).yyxy, vec2(1, 0).yyyx);
void main()
{
    gl_Position = toClip * toScreen * toWorld * vec4(aPos, 1.0);
    TexCoord = aTexCoord;

    vec3 Normal = (toWorld_normal * vec4(aNormal, 0)).xyz;
    
    vec3 Tangent = (toWorld_normal * vec4(aTangent, 1)).xyz;
    Tangent = normalize(Tangent - dot(Tangent, Normal) * Normal);
    TBN = mat3x3(Tangent, cross(Normal, Tangent), Normal);

    FragPos = vec3(toWorld * vec4(aPos, 1.0));
}