#version 450
#extension GL_KHR_vulkan_glsl : enable
const vec2 OFFSETS[6] = vec2[](
  vec2(-1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, -1.0),
  vec2(1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, 1.0)
);

layout (location = 0) out vec2 fragOffset;

struct PointLight {
	vec4 position; // ignore w
	vec4 color; // w is intensity

};

layout(set = 0, binding = 0) uniform GlobalUbo{
mat4 projection;
mat4 view;
mat4 invView;
vec4 ambientLightColor;
PointLight pointLights[10];
int numLights;
}ubo;

layout(push_constant) uniform Push{
	vec4 position;
	vec4 color;
	float radius;
} push;

void main(){
fragOffset = OFFSETS[gl_VertexIndex];
vec4 lightInCameraSpace = ubo.view * vec4(push.position.xyz, 1.0);
vec4 positionInCameraSpace = lightInCameraSpace + push.radius * vec4(fragOffset,0.0,0.0);
gl_Position = ubo.projection * positionInCameraSpace;

}