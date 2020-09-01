#version 460 core
out vec4 FragColor;
 
in vec3 FragPos;
in mat3x3 TBN;
in centroid vec2 TexCoord;

struct Material {
    sampler2D texture_diffuse0;
    sampler2D texture_specular0;
    sampler2D texture_normal0;
    float     shininess;
};

struct Camera {
    vec3 camPos;
};

uniform Material material;
uniform Camera camera;

void main()
{
    vec3 normal = normalize(  TBN  * (texture(material.texture_normal0, TexCoord).rgb * 2. - 1. ) );

    float diffuseFactor = 0.8 * max(dot(normal, vec3(0., 1., 0.)), 0.);
    const float ambientFactor = 0.2;

    vec3 diffuseAmbient = (diffuseFactor + ambientFactor) * vec3(texture(material.texture_diffuse0, TexCoord));

    vec3 viewDir = normalize(camera.camPos - FragPos);
    vec3 reflectDir = reflect(-vec3(0., 1., 0), normal);


    float specularFactor = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = vec3( specularFactor * texture(material.texture_specular0, TexCoord).r );

    FragColor = vec4(diffuseAmbient + specular, 1.);

}