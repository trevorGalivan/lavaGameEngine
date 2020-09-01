#version 460 core
out vec4 FragColor;
 
in vec2 TexCoords;

struct Camera {
    vec3 camPos;
};

uniform sampler2D gColor;
uniform sampler2D gNormal;
uniform sampler2D gMaterial;
uniform sampler2D gDepth;
uniform Camera camera;
uniform float maxEmmisive = 20.;

uniform vec3 sunDir = vec3(0, -1, 0);
uniform vec3 sunIntensity = vec3(1, 1, 1);
uniform vec3 ambientIntensity = vec3(0.04, 0.04, 0.04);

uniform mat4x4 viewMatrixInv;
uniform mat4x4 projMatrixInv;

vec3 fresnelSchlick(float cosTheta, vec3 F0);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float DistributionGGX(vec3 N, vec3 H, float roughness);
const float PI = 3.141592654;

// this is supposed to get the world position from the depth buffer
vec3 WorldPosFromDepth(float depth) {
    float z = depth * 2.0 - 1.0;

    vec4 clipSpacePosition = vec4(TexCoords * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = projMatrixInv * clipSpacePosition;

    // Perspective division
    viewSpacePosition /= viewSpacePosition.w;

    vec4 worldSpacePosition = viewMatrixInv * viewSpacePosition;

    return worldSpacePosition.xyz;
}

void main()
{
    vec4 MREAtex = texture(gMaterial, TexCoords).rgba;
    float metalness =  MREAtex.r;
    float roughness =  MREAtex.g;
    float emmesivity =  MREAtex.b;
    float AOmap =  MREAtex.a;

    vec3 colorTex = texture(gColor,TexCoords).rgb;

    vec3 normal = texture(gNormal, TexCoords).rgb;
    vec3 FragPos = WorldPosFromDepth(texture(gDepth, TexCoords).r);

    vec3 diffuseSpecular = colorTex.rgb * ambientIntensity * AOmap; // ambient light
    diffuseSpecular += colorTex.rgb * pow(emmesivity, 2.2) * maxEmmisive; // emmisiveness

    vec3 viewDir = normalize(camera.camPos - FragPos);
    vec3 F0 = mix(vec3(0.04), colorTex.rgb, metalness);


    // lighting from the sun
    vec3 halfwayDir = normalize(sunDir + viewDir);

        
    vec3 Fresnel = fresnelSchlick(max(dot(halfwayDir, viewDir), 0.), F0);
    float Dist = DistributionGGX(normal, halfwayDir, roughness);       
    float Geometry   = GeometrySmith(normal, viewDir, sunDir, roughness);      
    
    vec3 numerator    = Dist * Geometry * Fresnel;
    float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, sunDir), 0.0);
    vec3 specular     = numerator / max(denominator, 0.001);  

    vec3 kS = Fresnel;
    vec3 kD = vec3(1.0) - kS;
  
    kD *= 1.0 - metalness;

    float NdotL = max(dot(normal, sunDir), 0.0);        

    diffuseSpecular += (kD * colorTex.rgb / PI + specular) * vec3(3) * NdotL * sunIntensity;

    
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