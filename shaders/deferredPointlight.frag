#version 460 core
out vec4 FragColor;

struct Camera {
    vec3 camPos;
};

uniform sampler2D gColor;
uniform sampler2D gNormal;
uniform sampler2D gMaterial;
uniform sampler2D gDepth;
uniform Camera camera;

struct Light {
   vec3 position;
    vec3 color;
    float constant;
    float linear;
    float quadratic;
};



uniform Light light;
uniform vec2 resolution;
uniform mat4x4 viewMatrixInv;
uniform mat4x4 projMatrixInv;

vec3 fresnelSchlick(float cosTheta, vec3 F0);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float DistributionGGX(vec3 N, vec3 H, float roughness);
const float PI = 3.141592654;

// this is supposed to get the world position from the depth buffer
vec3 WorldPosFromDepth(float depth, vec2 normalizedCoord) {
    float z = depth * 2.0 - 1.0;

    vec4 clipSpacePosition = vec4(normalizedCoord, z, 1.0);
    vec4 viewSpacePosition = projMatrixInv * clipSpacePosition;

    // Perspective division
    viewSpacePosition /= viewSpacePosition.w;

    vec4 worldSpacePosition = viewMatrixInv * viewSpacePosition;

    return worldSpacePosition.xyz;
}

void main()
{
    vec2 TexCoord = gl_FragCoord.xy / resolution;
    vec2 screenPos = 2 *  TexCoord - 1;

    vec4 MREAtex = texture(gMaterial, TexCoord).rgba;
    float metalness =  MREAtex.r;
    float roughness =  MREAtex.g;
    float emmesivity =  MREAtex.b;
    float AOmap =  MREAtex.a;

    vec3 colorTex = texture(gColor,TexCoord).rgb;

    vec3 normal = texture(gNormal, TexCoord).rgb;
    vec3 FragPos = WorldPosFromDepth(texture(gDepth, TexCoord).r, screenPos);

    vec3 viewDir = normalize(camera.camPos - FragPos);
    vec3 F0 = mix(vec3(0.04), colorTex.rgb, metalness);


    vec3 lightDir = light.position - FragPos;
    float lightDist = length(lightDir);
    lightDir /= lightDist;

    float attenuation = 1 / (light.constant + light.linear * lightDist + light.quadratic * (lightDist * lightDist));
         
    vec3 halfwayDir = normalize(lightDir + viewDir);

        
    vec3 Fresnel = fresnelSchlick(max(dot(halfwayDir, viewDir), 0.), F0);
    float Dist = DistributionGGX(normal, halfwayDir, roughness);       
    float Geometry   = GeometrySmith(normal, viewDir, lightDir, roughness);      
    
    vec3 numerator    = Dist * Geometry * Fresnel;
    float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0);
    vec3 specular     = numerator / max(denominator, 0.001);  

    vec3 kS = Fresnel;
    vec3 kD = vec3(1.0) - kS;
  
    kD *= 1.0 - metalness;

    float NdotL = max(dot(normal, lightDir), 0.0);        

    vec3 diffuseSpecular = (kD * colorTex.rgb / PI + specular) * vec3(3) * NdotL * attenuation * light.color;

    
    FragColor = vec4(diffuseSpecular, 1);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - max(cosTheta, 1.), 5.0);
}