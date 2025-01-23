#pragma once



#include "lve_device.h"
#include "lve_swap_chain.h"
#include "lve_window.h"
#include "lve_model.h"

// std
#include <cassert>
#include <memory>
#include <vector>


namespace lve {
  class LveRender {
  public:

    LveRender(LveWindow& lveWindow, LveDevice& lveDevice);
    ~LveRender();


    LveRender(const LveRender&) = delete;
    LveRender& operator =(const LveRender&) = delete;

    VkRenderPass getSwapChainRenderPass() const { return lveSwapChain->getRenderPass(); }
    float getAspectRatio() const { return lveSwapChain->extentAspectRatio(); }
    bool isFrameInProgress() const { return isFrameStarted; }

    VkCommandBuffer getCurrentCommandBuffer() const { 
      assert(isFrameStarted && "Cannot get command buffer when frame not in progess");
      return commandBuffers[currentFrameIndex]; }

    int getFrameIndex() const {
      assert(isFrameStarted && "Cannot get frame index when frame not in progess");
      return currentFrameIndex;
    }

    VkCommandBuffer beginFrame();
    void endFrame();
    void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
    void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

  private:
    void createCommandBuffers();
    void freecommandBuffers();
    void recreateSwapChain();


    LveWindow& lveWindow;
    LveDevice& lveDevice;
    std::unique_ptr<LveSwapChain> lveSwapChain;
    std::vector<VkCommandBuffer> commandBuffers;

    uint32_t currentImangeIndex;
    int currentFrameIndex{0};
    bool isFrameStarted = {false};

  };
}