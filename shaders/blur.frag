#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;

uniform bool horizontal;
const float weight[7] = float[] (
0.209498,
0.183311,
0.122207,
0.0611036,
0.0222195,
0.00555488,
0.000854596);

void main()
{             
    vec2 tex_offset = 1.0 / textureSize(screenTexture, 0); // gets size of single texel
    vec3 result = texture(screenTexture, TexCoords).rgb * weight[0]; // current fragment's contribution
    if(horizontal)
    {
        for(int i = 1; i < 7; ++i)
        {
            result += texture(screenTexture, TexCoords + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
            result += texture(screenTexture, TexCoords - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
        }
    }
    else
    {
        for(int i = 1; i < 7; ++i)
        {
            result += texture(screenTexture, TexCoords + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
            result += texture(screenTexture, TexCoords - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
        }
    }
    FragColor = vec4(result, 1.0);
}