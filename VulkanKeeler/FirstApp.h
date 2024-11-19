#pragma once



#include "ive_pipeline.h"
#include "lve_device.h"
#include "lve_swap_chain.h"
#include "lve_window.h"
#include "lve_model.h"

// std
#include <memory>
#include <vector>

namespace lve {
  class FirstApp {
  public:
    static constexpr int WIDTH = 800;
    static constexpr int HEIGHT = 600;

    FirstApp();
    ~FirstApp();


    FirstApp(const FirstApp&) = delete;
    FirstApp& operator =(const FirstApp&) = delete;
    void run();
  private:
    void loadModel();
    void createPipelineLayout();
    void createPipeline();
    void createCommandBuffers();
    void drawFrame();

    LveWindow lveWindow{ WIDTH, HEIGHT, "Hello Vulkan!" };
    LveDevice lveDevice{lveWindow};
    LveSwapChain lveSwapChain{lveDevice, lveWindow.getExtent()};
    std::unique_ptr<LvePipeline> lvePipeline;
    VkPipelineLayout pipelineLayout;
    std::vector<VkCommandBuffer> commandBuffer;
    std::unique_ptr<LveModel> lveModel;

    std::vector<LveModel::Vertex> fractal(int n, int Max, std::vector<LveModel::Vertex> lastInstance);

  };
}