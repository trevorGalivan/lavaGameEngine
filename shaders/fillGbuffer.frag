#version 460 core
layout (location = 0) out vec3 gColor;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gMaterial;
 
in vec3 FragPos;
in mat3x3 TBN;
in vec2 TexCoord;

struct Material {
    sampler2D texture_diffuse0;
    sampler2D texture_MREA0;
    sampler2D texture_normal0;
    float     shininess;
};

struct Camera {
    vec3 camPos;
};

uniform Material material;
uniform Camera camera;

const vec3 lightPos = vec3(0, 100000, 100000);

void main()
{
    vec4 colorTex = texture(material.texture_diffuse0, TexCoord).rgba;

    if(colorTex.a < 0.3){
        discard;
    }

    gColor = colorTex.rgb;
    gNormal = normalize(  TBN  * (texture(material.texture_normal0, TexCoord).rgb * 2. - 1. ) );
    gMaterial = texture(material.texture_MREA0, TexCoord).rgba;
}

