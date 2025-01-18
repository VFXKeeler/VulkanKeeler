#include "simple_render_system.h"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <stdexcept>
#include <array>
//temp
#include <iostream>
#include <random>
#include <cmath>

namespace lve {

  struct SimplePushConstantData {
    glm::mat4 transform{ 1.f };
    alignas(16) glm::vec3 color;
  };

  SimpleRenderSystem::SimpleRenderSystem(LveDevice& device, VkRenderPass renderPass) : lveDevice{ device }
  {

    createPipelineLayout();
    createPipeline(renderPass);

  }
  SimpleRenderSystem::~SimpleRenderSystem()
  {
    vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
  }
  
 
  void SimpleRenderSystem::createPipelineLayout()
  {
    VkPushConstantRange pushconstantRange{};
    pushconstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushconstantRange.offset = 0;
    pushconstantRange.size = sizeof(SimplePushConstantData);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushconstantRange;
    if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
      throw std::runtime_error("failed to create pipeline layout!");
    }

  }
  void SimpleRenderSystem::createPipeline(VkRenderPass renderPass)
  {

    assert(pipelineLayout != nullptr && "cannont creeate pipline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};
    LvePipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = pipelineLayout;
    lvePipeline = std::make_unique<LvePipeline>(
      lveDevice,
      "simple_shader.vert.spv",
      "simple_shader.frag.spv",
      pipelineConfig);
  }


  void SimpleRenderSystem::renderGameObjects(VkCommandBuffer commandBuffer, std::vector<LveGameObject>& gameObjects, const LveCamera& camera) {
      lvePipeline->bind(commandBuffer);

      auto projectionView = camera.getProjection() * camera.getView();

      for (auto& obj : gameObjects) {
        SimplePushConstantData push{};
   
        push.color = obj.color;
        push.transform = projectionView *  obj.transform.mat4();

        vkCmdPushConstants(
          commandBuffer,
          pipelineLayout,
          VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
          0,
          sizeof(SimplePushConstantData),
          &push);
        obj.model->bind(commandBuffer);
        obj.model->draw(commandBuffer);
      }
}
}