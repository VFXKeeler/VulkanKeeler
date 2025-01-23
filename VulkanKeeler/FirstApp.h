#pragma once


#include "lve_render.h"
#include "lve_device.h"
#include "lve_window.h"
#include "lve_game_object.h"
#include "lve_descriptors.h"

// std
#include <memory>
#include <vector>

namespace lve {
  class FirstApp {
  public:
    static constexpr int WIDTH = 1920;
    static constexpr int HEIGHT = 1080;

    FirstApp();
    ~FirstApp();


    FirstApp(const FirstApp&) = delete;
    FirstApp& operator =(const FirstApp&) = delete;
    void run();
  private:
    void loadGameObjects();


    LveWindow lveWindow{ WIDTH, HEIGHT, "Hello Vulkan!" };
    LveDevice lveDevice{lveWindow};
    LveRender lveRender{ lveWindow , lveDevice };

    std::unique_ptr<LveDescriptorPool> globalPool{};
    LveGameObject::Map gameObjects;


  };
}