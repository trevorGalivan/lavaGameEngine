#version 460 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform vec3 bloomCutoff = vec3(7.2415);

void main()
{ 
    FragColor = max((texture(screenTexture, TexCoords) - vec4(bloomCutoff, 0)), vec4(0,0,0,1));
}