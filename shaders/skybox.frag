#version 460 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{    
    FragColor = pow(texture(skybox, TexCoords), vec4(2.2));
}