#version 460 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float exposure = 1;

vec3 filmic(vec3 linearRGB) 
{
    const float a = 2.51f;
    const float b = 0.03f;
    const float c = 2.43f;
    const float d = 0.59f;
    const float e = 0.14f;
    vec3 filmic = (linearRGB*(a*linearRGB+b))/(linearRGB*(c*linearRGB+d)+e);

    return filmic;
}

void main()
{ 
    FragColor = vec4( filmic( texture(screenTexture, TexCoords).rgb * exposure), 1);
}