#version 460 core
out vec4 FragColor;
 
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
uniform float maxEmmisive = 1.;
uniform int mapper = 2; 

const vec3 lightPos = vec3(0, 100000, 100000);

vec3 reinhardModified (vec3 linearRGB);
vec3 reinhard(vec3 linearRGB);
vec3 filmic(vec3 linearRGB);
vec3 fresnelSchlick(float cosTheta, vec3 F0);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float DistributionGGX(vec3 N, vec3 H, float roughness);
const float PI = 3.141592654;

void main()
{
    vec4 MREAtex = texture(material.texture_MREA0, TexCoord).rgba;
    float metalness =  MREAtex.r;
    float roughness =  MREAtex.g;
    float emmesivity =  MREAtex.b;
    float AOmap =  MREAtex.a;

    vec4 colorTex = texture(material.texture_diffuse0, TexCoord).rgba;

    if(colorTex.a < 0.3){
        discard;
    }

    vec3 normal = normalize(  TBN  * (texture(material.texture_normal0, TexCoord).rgb * 2. - 1. ) );


    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(camera.camPos - FragPos);
    //vec3 reflectDir = reflect(lightPos, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    vec3 F0 = mix(vec3(0.04), colorTex.rgb, metalness);
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

    vec3 diffuseSpecular = (kD * colorTex.rgb / PI + specular) * vec3(3) * NdotL;

    diffuseSpecular += colorTex.rgb * 0.04 * AOmap;

    diffuseSpecular += colorTex.rgb * pow(emmesivity, 2.2) * maxEmmisive;
    
    if (mapper == 0){
        FragColor = vec4(pow(diffuseSpecular, vec3(1/2.2)), 1.);
    } else if (mapper == 1) {
        FragColor = vec4(reinhard(diffuseSpecular), 1.);
    } else if (mapper == 2){
        FragColor = vec4(filmic(diffuseSpecular), 1.);
    } else if (mapper == 3){
        FragColor = vec4((reinhard(diffuseSpecular) + filmic(diffuseSpecular))/2, 1.);
    } else if (mapper == 4){
        FragColor = vec4((reinhardModified(diffuseSpecular)), 1);
    }
}

vec3 reinhardModified (vec3 linearRGB)
{
    vec3 reinhard = linearRGB / ((linearRGB.x+linearRGB.y+linearRGB.z)/3 + 1);
    return pow(reinhard, vec3(1/2.2));
}

vec3 reinhard (vec3 linearRGB)
{
    vec3 reinhard = linearRGB / (linearRGB + 1);
    return pow(reinhard, vec3(1/2.2));
}

vec3 filmic(vec3 linearRGB) 
{
    const float a = 2.51f;
    const float b = 0.03f;
    const float c = 2.43f;
    const float d = 0.59f;
    const float e = 0.14f;
    vec3 filmic = clamp((linearRGB*(a*linearRGB+b))/(linearRGB*(c*linearRGB+d)+e), 0, 1);

    return pow(filmic, vec3(1/2.2));
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