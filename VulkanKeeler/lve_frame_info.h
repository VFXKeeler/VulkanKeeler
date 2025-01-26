#pragma once

#include "lve_camera.h"
#include "lve_game_object.h"

#include <vulkan/vulkan.h>

namespace lve {
#define MAX_LIGHTS 10
struct PointLight
{
	glm::vec4 position{}; //ignore w
	glm::vec4 color{}; // w instentisty
};
struct GlobalUbo {
	glm::mat4 projection{ 1.f };
	glm::mat4 view{ 1.f };
	glm::mat4 inverseView{ 1.f };
	glm::vec4 ambientLightColor{ 1.f, 1.f, 1.f, .02f };  // w is intensity
	PointLight pointLights[MAX_LIGHTS];
	int numLights;
};
struct FrameInfo
{
	int frameIndex;
	float frameTime;
	VkCommandBuffer commandBuffer;
	LveCamera &camera;
	VkDescriptorSet globalDescriptorSet;
	VkDescriptorSet textureDescriptorSet;
	LveGameObject::Map& gameObjects;

};
}