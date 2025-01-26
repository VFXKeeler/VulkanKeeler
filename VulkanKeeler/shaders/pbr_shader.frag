#version 450
#extension GL_KHR_vulkan_glsl : enable

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;
layout(location = 3) in vec2 fragTexCoord;
layout(location = 4) in vec3 ReflectedSurfaceToViewerDirection;

layout (location = 0) out vec4 outColor;

struct PointLight {
    vec4 position; // ignore w
    vec4 color; // w is intensity
};

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    mat4 invView;
    vec4 ambientLightColor;
    PointLight pointLights[10];
    int numLights;
} ubo;

layout(set = 1, binding = 0) uniform sampler2D albedoMap;
layout(set = 1, binding = 1) uniform sampler2D normalMap;
layout(set = 1, binding = 2) uniform sampler2D metallicMap;
layout(set = 1, binding = 3) uniform sampler2D roughnessMap;
layout(set = 1, binding = 4) uniform sampler2D aoMap;

layout(push_constant) uniform Push {
    mat4 modelMatrix;
    mat4 normalMatrix;
} push;

const float PI = 3.14159265359;

vec3 getNormalFromMap() {
    vec3 tangentNormal = texture(normalMap, fragTexCoord).xyz * 2.0 - 1.0;
    vec3 Q1 = dFdx(fragPosWorld);
    vec3 Q2 = dFdy(fragPosWorld);
    vec2 st1 = dFdx(fragTexCoord);
    vec2 st2 = dFdy(fragTexCoord);

    vec3 N = normalize(fragNormalWorld);
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}


void main() {
vec3 Light = vec3(0.0);


vec3 cameraPosWorld = ubo.invView[3].xyz;
vec3 viewDirection = normalize(cameraPosWorld - fragPosWorld);

vec3 albedo = pow(texture(albedoMap, fragTexCoord).rgb, vec3(2.2));
float metallic = texture(metallicMap, fragTexCoord).r;
float roughness = texture(roughnessMap, fragTexCoord).r;
float ao = texture(aoMap, fragTexCoord).r;


vec3 N = getNormalFromMap();
vec3 V = normalize(cameraPosWorld - fragPosWorld);
for (int i=0; i<ubo.numLights; i++){
        PointLight light = ubo.pointLights[i];
        vec3 directionToLight = light.position.xyz - fragPosWorld;
        vec3 L = normalize(directionToLight);
        vec3 H = normalize(V + L);

        vec3 radiance = light.color.xyz * light.color.w;
        vec3 F0 = vec3(0.04);
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 nominator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
        vec3 specular = nominator / max(denominator, 0.001);

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        float NdotL = max(dot(N, L), 0.0);
        vec3 Lo = (kD * albedo / PI + specular) * radiance * NdotL;

        vec3 ambient = vec3(0.003) * albedo * ao;

        vec3 color = ambient + Lo;

        color = color / (color + vec3(1.0));
        color = pow(color, vec3(1.0 / 2.2));
        Light += color;
        }


    outColor = vec4(Light,1.0);
}
