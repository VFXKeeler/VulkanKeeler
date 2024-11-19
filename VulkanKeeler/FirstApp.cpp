#include "FirstApp.h"

// std
#include <stdexcept>
#include <array>
//temp
#include <iostream>
namespace lve {
  FirstApp::FirstApp()
  {
    loadModel();
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
  std::vector<LveModel::Vertex> FirstApp::fractal(int n, int Max, std::vector<LveModel::Vertex> lastInstance)
  {
    n++;
    std::vector<LveModel::Vertex> local;
    // get the points i need
    // loop through the last points
    // get the index mod of last index
    // get the looped forward and backwards triangle for position data
    // make a new triangle that has the original position and the new ones and write it to the local vector 

    for (int i = 0; i < lastInstance.size(); i++) {

      int vertMod = i % 3;
      int nextIndex;
      int lastIndex;
      LveModel::Vertex lastVertex;
      LveModel::Vertex nextVertex;
      switch (vertMod) {
      case 0:
        nextIndex = i + 1;
        lastIndex = i + 2;
        break;
      case 1:
        nextIndex = i + 1;
        lastIndex = i - 1;
        break;
      case 2:
        nextIndex = i - 2;
        lastIndex = i - 1;
        break;
      default:
        nextIndex = i;
        lastIndex = i;
      }

      float lastX = (((float)lastInstance[lastIndex].position.r)/ 2.0f) + ((float)lastInstance[i].position.r/ 2.0f );
      float lastY = (((float)lastInstance[lastIndex].position.g)/ 2.0f) + ((float)lastInstance[i].position.g/ 2.0f );
      float nextX = (((float)lastInstance[nextIndex].position.r)/ 2.0f) + ((float)lastInstance[i].position.r/ 2.0f );
      float nextY = (((float)lastInstance[nextIndex].position.g)/ 2.0f) + ((float)lastInstance[i].position.g/ 2.0f );

      lastVertex.position = glm::vec2( lastX, lastY );
      nextVertex.position = glm::vec2( nextX, nextY);

      
      local.push_back(nextVertex);
      local.push_back(lastVertex);
      local.push_back(lastInstance[i]);



    }
      

    if (n < Max) {
      return fractal(n+1, Max, local);
    }
    else {
      return local;
    }
  }
  void FirstApp::loadModel()
  {
    
    
    std::vector<LveModel::Vertex> vertices{
      {{0.0f, -0.5f}},
      {{0.5f, 0.5f}},
      {{-0.5f, 0.5f}},
      
    };
    vertices =fractal(0, 5, vertices);
    lveModel = std::make_unique<LveModel>(lveDevice, vertices);
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
      lveModel->bind(commandBuffer[i]);
      lveModel->draw(commandBuffer[i]);

      vkCmdEndRenderPass(commandBuffer[i]);
      if (vkEndCommandBuffer(commandBuffer[i]) != VK_SUCCESS) {
        throw std::runtime_error("failed to record  command buffer!");
      }
    }
  }
}