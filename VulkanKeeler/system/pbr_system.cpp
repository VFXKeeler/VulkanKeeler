#include "pbr_system.h"

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

  struct PBRSystemPushConstantData {
    glm::mat4 modeltransform{ 1.f };
    glm::mat4 normalMatrix{ 1.f };
  };

  PBRSystem::PBRSystem(LveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout textureSetLayout) : lveDevice{ device }
  {
    
    createPipelineLayout(globalSetLayout,textureSetLayout);
    createPipeline(renderPass);

  }
  PBRSystem::~PBRSystem()
  {
    vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);

  }


  void PBRSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout textureSetLayout)
  {
    VkPushConstantRange pushconstantRange{};
    pushconstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushconstantRange.offset = 0;
    pushconstantRange.size = sizeof(PBRSystemPushConstantData);

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout };
    descriptorSetLayouts.push_back(textureSetLayout);
    
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushconstantRange;
    if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
      throw std::runtime_error("failed to create pipeline layout!");
    }
  

  }
  void PBRSystem::createPipeline(VkRenderPass renderPass)
  {

    assert(pipelineLayout != nullptr && "cannont create pipline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};
    LvePipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = pipelineLayout;
    lvePipeline = std::make_unique<LvePipeline>(
      lveDevice,
      "shaders/pbr_shader.vert.spv",
      "shaders/pbr_shader.frag.spv",
      pipelineConfig);
  }


  void PBRSystem::renderGameObjects(FrameInfo& frameInfo) {
    lvePipeline->bind(frameInfo.commandBuffer);

    vkCmdBindDescriptorSets(
      frameInfo.commandBuffer,
      VK_PIPELINE_BIND_POINT_GRAPHICS,
      pipelineLayout,
      0, 1,
      &frameInfo.globalDescriptorSet,
      0,
      nullptr
    );
    assert(frameInfo.commandBuffer != VK_NULL_HANDLE && "DUMB commandBuffer");
    assert(pipelineLayout != VK_NULL_HANDLE && "DUMB pipelineLayout");
    assert(frameInfo.textureDescriptorSet != VK_NULL_HANDLE && "DUMB textureDescriptorSet");

    vkCmdBindDescriptorSets(
      frameInfo.commandBuffer,
      VK_PIPELINE_BIND_POINT_GRAPHICS,
      pipelineLayout,
      1, 1,
      &frameInfo.textureDescriptorSet,
      0,
      nullptr
    );


   

    for (auto& kv : frameInfo.gameObjects) {
      auto& obj = kv.second;
      if (obj.model == nullptr) continue;
      PBRSystemPushConstantData push{};

      push.modeltransform = obj.transform.mat4();
      push.normalMatrix = obj.transform.normalMatrix();

      vkCmdPushConstants(
        frameInfo.commandBuffer,
        pipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        0,
        sizeof(PBRSystemPushConstantData),
        &push);
      obj.model->bind(frameInfo.commandBuffer);
      obj.model->draw(frameInfo.commandBuffer);
    }
  }
}