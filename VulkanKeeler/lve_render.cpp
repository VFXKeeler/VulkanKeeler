#include "lve_render.h"

// std
#include <stdexcept>
#include <array>
//temp
#include <iostream>
#include <random>
#include <cmath>

namespace lve {


  LveRender::LveRender(LveWindow& window, LveDevice& device) : lveWindow{ window }, lveDevice{ device }
  {
    recreateSwapChain();
    createCommandBuffers();

  }
  LveRender::~LveRender()
  {
    freecommandBuffers();
  }
  


  void LveRender::recreateSwapChain() {
    auto extent = lveWindow.getExtent();
    while (extent.height == 0 || extent.width == 0) {
      extent = lveWindow.getExtent();
      glfwWaitEvents();
    }

    vkDeviceWaitIdle(lveDevice.device());
    if (lveSwapChain == nullptr) {

      lveSwapChain = std::make_unique<LveSwapChain>(lveDevice, extent);

    }
    else {
      std::shared_ptr<LveSwapChain> oldSwapChain = std::move(lveSwapChain);
      lveSwapChain = std::make_unique<LveSwapChain>(lveDevice, extent, oldSwapChain);

      if (!oldSwapChain->compareSwapFormat(*lveSwapChain.get())) {
        throw std::runtime_error("Swap chain image(or depth) format has changed!");


      }

    }

  }



  void LveRender::createCommandBuffers()
  {

    commandBuffers.resize(LveSwapChain::MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandPool = lveDevice.getCommandPool();
    allocateInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());


    if (vkAllocateCommandBuffers(lveDevice.device(), &allocateInfo, commandBuffers.data()) != VK_SUCCESS)
    {
      throw std::runtime_error("failed to allocate command buffers!");
    }


  }

  void LveRender::freecommandBuffers()
  {
    vkFreeCommandBuffers(lveDevice.device(), lveDevice.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
    commandBuffers.clear();
  }



  VkCommandBuffer lve::LveRender::beginFrame()
  {
    assert(!isFrameStarted && "Can't call beginFrame while already in prgress");
    auto result = lveSwapChain->acquireNextImage(&currentImangeIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
      recreateSwapChain();
      return nullptr;
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
      throw std::runtime_error("failed to aquire swap chain image!");
    }
    isFrameStarted = true;

    auto commandBuffer = getCurrentCommandBuffer();
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
      throw std::runtime_error("failed to begin recording command buffer!");
    }
    return commandBuffer;
  }

  void lve::LveRender::endFrame()
  {
    assert(isFrameStarted && "Can't call endFrame while frame is not in prgress");
    auto commanBuffer = getCurrentCommandBuffer();
    if (vkEndCommandBuffer(commanBuffer) != VK_SUCCESS) {
      throw std::runtime_error("failed to record command buffer!");
    }
    auto result = lveSwapChain->submitCommandBuffers(&commanBuffer, &currentImangeIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || lveWindow.wasWindowResized()) {
      lveWindow.resetWindowResizedFlag();
      recreateSwapChain();
    } else if (result != VK_SUCCESS) {
      throw std::runtime_error("failed to present swap chain image!");
    }

    isFrameStarted = false;
    currentFrameIndex = (currentFrameIndex + 1) % LveSwapChain::MAX_FRAMES_IN_FLIGHT;
  }

  

  void lve::LveRender::beginSwapChainRenderPass(VkCommandBuffer commandBuffer)
  {
    assert(isFrameStarted && "Can't call beginSwapChainRenderPiss if frame is not in progress");
    assert(commandBuffer == getCurrentCommandBuffer() && "Can't begin render pass on comm buffer from a different frame");

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = lveSwapChain->getRenderPass();
    renderPassInfo.framebuffer = lveSwapChain->getFrameBuffer(currentImangeIndex);

    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = lveSwapChain->getSwapChainExtent();

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
    clearValues[1].depthStencil = { 1.0f, 0 };
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);


    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(lveSwapChain->getSwapChainExtent().width);
    viewport.height = static_cast<float>(lveSwapChain->getSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor{ {0,0}, lveSwapChain->getSwapChainExtent() };
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

  }

  void lve::LveRender::endSwapChainRenderPass(VkCommandBuffer commandBuffer)
  {
    assert(isFrameStarted && "Can't call beginSwapChainRenderPiss if frame is not in progress");
    assert(commandBuffer == getCurrentCommandBuffer() && "Can't end render pass on comm buffer from a different frame");
    vkCmdEndRenderPass(commandBuffer);
  }
}