#include "FirstApp.h"

// std
#include <stdexcept>
#include <array>

namespace lve {
  FirstApp::FirstApp()
  {
    createPipelineLayout();
    createPipeline();
    createCommandBuffers();

  }
  FirstApp::~FirstApp()
  {
    vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
  }
  void FirstApp::run() {
    while (!lveWindow.shouldClose()) {
      glfwPollEvents();
      drawFrame();
    }
    vkDeviceWaitIdle(lveDevice.device());
  }
  void FirstApp::drawFrame()
  {

    uint32_t imageIndex;
    auto result = lveSwapChain.acquireNextImage(&imageIndex);

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
      throw std::runtime_error("failed to aquire swap chain image!");
    }

    result = lveSwapChain.submitCommandBuffers(&commandBuffer[imageIndex], &imageIndex);
    if (result != VK_SUCCESS) {
      throw std::runtime_error("failed to present swap chain image!");
    }

  }
  void FirstApp::createPipelineLayout()
  {
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;
    if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
      throw std::runtime_error("failed to create pipeline layout!");
    }

  }
  void FirstApp::createPipeline()
  {
    auto pipelineConfig = LvePipeline::defaultPipelineConfigInfo(lveSwapChain.width(), lveSwapChain.height());
    pipelineConfig.renderPass = lveSwapChain.getRenderPass();
    pipelineConfig.pipelineLayout = pipelineLayout;
    lvePipeline = std::make_unique<LvePipeline>(
      lveDevice,
      "simple_shader.vert.spv",
      "simple_shader.frag.spv",
      pipelineConfig);
  }
  void FirstApp::createCommandBuffers()
  {

    commandBuffer.resize(lveSwapChain.imageCount());

    VkCommandBufferAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandPool = lveDevice.getCommandPool();
    allocateInfo.commandBufferCount = static_cast<uint32_t>(commandBuffer.size());
    

    if (vkAllocateCommandBuffers(lveDevice.device(), &allocateInfo, commandBuffer.data()) != VK_SUCCESS)
    {
      throw std::runtime_error("failed to allocate command buffers!");
    }

    for (int i = 0; i < commandBuffer.size(); i++) {
      VkCommandBufferBeginInfo beginInfo{};
      beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

      if (vkBeginCommandBuffer(commandBuffer[i], &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
      }

      VkRenderPassBeginInfo renderpassInfo{};
      renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
      renderpassInfo.renderPass = lveSwapChain.getRenderPass();
      renderpassInfo.framebuffer = lveSwapChain.getFrameBuffer(i);

      renderpassInfo.renderArea.offset = { 0,0 };
      renderpassInfo.renderArea.extent = lveSwapChain.getSwapChainExtent();

      std::array<VkClearValue, 2>clearValues{};
      clearValues[0].color = { 0.1f, 0.1f,0.1f ,1.0f };
      clearValues[1].color = { 1.0f, 0 };
      renderpassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
      renderpassInfo.pClearValues = clearValues.data();

      vkCmdBeginRenderPass(commandBuffer[i], &renderpassInfo, VK_SUBPASS_CONTENTS_INLINE);

      lvePipeline->bind(commandBuffer[i]);
      vkCmdDraw(commandBuffer[i], 3, 1, 0, 0);

      vkCmdEndRenderPass(commandBuffer[i]);
      if (vkEndCommandBuffer(commandBuffer[i]) != VK_SUCCESS) {
        throw std::runtime_error("failed to record  command buffer!");
      }
    }
  }
}