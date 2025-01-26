#include "FirstApp.h"

#include "keyboard_movement_controller.h"
#include "lve_buffer.h"
#include "lve_camera.h"
#include "system/pbr_system.h"
#include "system/point_light_system.h"
#include "lve_texture.h"
// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <chrono>
#include <stdexcept>

namespace lve {



  FirstApp::FirstApp() {
    globalPool =
      LveDescriptorPool::Builder(lveDevice)
      .setMaxSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT)
      .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, LveSwapChain::MAX_FRAMES_IN_FLIGHT)
      .build();

    ///////// Texture Pool /////////////////
    texturePool =
      LveDescriptorPool::Builder(lveDevice)
      .setMaxSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT)
      .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, LveSwapChain::MAX_FRAMES_IN_FLIGHT)
      .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, LveSwapChain::MAX_FRAMES_IN_FLIGHT)
      .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, LveSwapChain::MAX_FRAMES_IN_FLIGHT)
      .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, LveSwapChain::MAX_FRAMES_IN_FLIGHT)
      .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, LveSwapChain::MAX_FRAMES_IN_FLIGHT)
      .build();
    loadGameObjects();
  }

  FirstApp::~FirstApp() {}

  void FirstApp::run() {
    std::vector<std::unique_ptr<LveBuffer>> uboBuffers(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < uboBuffers.size(); i++) {
      uboBuffers[i] = std::make_unique<LveBuffer>(
        lveDevice,
        sizeof(GlobalUbo),
        1,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
      uboBuffers[i]->map();
    }

    auto globalSetLayout =
      LveDescriptorSetLayout::Builder(lveDevice)
      .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
      .build();

    ///////////////////////////////////////// ADDING INT TEXTURES ///////////////////////////////////
    auto textureSetLayout =
      LveDescriptorSetLayout::Builder(lveDevice)
      .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
      .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
      .addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
      .addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
      .addBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
      .build();

    Texture albedoMap = Texture(lveDevice, "I:\\VulkanProjects\\VulkanKeeler\\VulkanKeeler\\VulkanKeeler\\textures\\Substance_graph_basecolor.tga");
    Texture normalMap = Texture(lveDevice, "I:\\VulkanProjects\\VulkanKeeler\\VulkanKeeler\\VulkanKeeler\\textures\\Substance_graph_normal.tga");
    Texture metallicMap = Texture(lveDevice, "I:\\VulkanProjects\\VulkanKeeler\\VulkanKeeler\\VulkanKeeler\\textures\\Substance_graph_metallic.tga");
    Texture roughnessMap = Texture(lveDevice, "I:\\VulkanProjects\\VulkanKeeler\\VulkanKeeler\\VulkanKeeler\\textures\\Substance_graph_roughness.tga");
    Texture aoMap = Texture(lveDevice, "I:\\VulkanProjects\\VulkanKeeler\\VulkanKeeler\\VulkanKeeler\\textures\\Substance_graph_ambientocclusion.tga");

    VkDescriptorImageInfo imageAlbido;
    imageAlbido.sampler = albedoMap.getSampler();
    imageAlbido.imageView = albedoMap.getImageView();
    imageAlbido.imageLayout = albedoMap.getImageLayout();
    VkDescriptorImageInfo imageNormal;
    imageNormal.sampler = normalMap.getSampler();
    imageNormal.imageView = normalMap.getImageView();
    imageNormal.imageLayout = normalMap.getImageLayout();
    VkDescriptorImageInfo imageMetal;
    imageMetal.sampler = metallicMap.getSampler();
    imageMetal.imageView = metallicMap.getImageView();
    imageMetal.imageLayout = metallicMap.getImageLayout();
    VkDescriptorImageInfo imageRough;
    imageRough.sampler = roughnessMap.getSampler();
    imageRough.imageView = roughnessMap.getImageView();
    imageRough.imageLayout = roughnessMap.getImageLayout();
    VkDescriptorImageInfo imageAO;
    imageAO.sampler = aoMap.getSampler();
    imageAO.imageView = aoMap.getImageView();
    imageAO.imageLayout = aoMap.getImageLayout();


    std::vector<VkDescriptorSet> textureDescriptorSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < textureDescriptorSets.size(); i++) {
      LveDescriptorWriter(*textureSetLayout, *texturePool)
        .writeImage(0, &imageAlbido)
        .writeImage(1, &imageNormal)
        .writeImage(2, &imageMetal)
        .writeImage(3, &imageRough)
        .writeImage(4, &imageAO)
        .build(textureDescriptorSets[i]);
    }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////

      std::vector<VkDescriptorSet> globalDescriptorSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
      for (int i = 0; i < globalDescriptorSets.size(); i++) {
        auto bufferInfo = uboBuffers[i]->descriptorInfo();
        LveDescriptorWriter(*globalSetLayout, *globalPool)
          .writeBuffer(0, &bufferInfo)
          .build(globalDescriptorSets[i]);
      }/////////////////
      PBRSystem pbrRenderSystem{
          lveDevice,
          lveRender.getSwapChainRenderPass(),
          globalSetLayout->getDescriptorSetLayout(),
          textureSetLayout->getDescriptorSetLayout()
          
      };
      PointLightSystem pointLightSystem{
          lveDevice,
          lveRender.getSwapChainRenderPass(),
          globalSetLayout->getDescriptorSetLayout() };
      LveCamera camera{};

      auto viewerObject = LveGameObject::createGameObject();
      viewerObject.transform.translation.z = -2.5f;
      KeyboardMovementController cameraController{};

      auto currentTime = std::chrono::high_resolution_clock::now();
      while (!lveWindow.shouldClose()) {
        glfwPollEvents();

        auto newTime = std::chrono::high_resolution_clock::now();
        float frameTime =
          std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
        currentTime = newTime;

        cameraController.moveInPlaneXZ(lveWindow.getGLFWwindow(), frameTime, viewerObject);
        camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

        float aspect = lveRender.getAspectRatio();
        camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);

        if (auto commandBuffer = lveRender.beginFrame()) {
          int frameIndex = lveRender.getFrameIndex();
          FrameInfo frameInfo{
              frameIndex,
              frameTime,
              commandBuffer,
              camera,
              globalDescriptorSets[frameIndex],
              textureDescriptorSets[frameIndex],
              gameObjects };

          // update
          GlobalUbo ubo{};
          ubo.projection = camera.getProjection();
          ubo.view = camera.getView();
          ubo.inverseView = camera.getInverseView();
          pointLightSystem.update(frameInfo, ubo);
          uboBuffers[frameIndex]->writeToBuffer(&ubo);
          uboBuffers[frameIndex]->flush();

          // render
          lveRender.beginSwapChainRenderPass(commandBuffer);
          pbrRenderSystem.renderGameObjects(frameInfo);
          pointLightSystem.render(frameInfo);
          lveRender.endSwapChainRenderPass(commandBuffer);
          lveRender.endFrame();
        }
      }

      vkDeviceWaitIdle(lveDevice.device());
    }



  void FirstApp::loadGameObjects(){
    std::shared_ptr<LveModel> lveModel =
      LveModel::createModelFromFile(lveDevice, "I:\\VulkanProjects\\VulkanKeeler\\VulkanKeeler\\Models\\flat_vase.obj");
    auto flatVase = LveGameObject::createGameObject();
    flatVase.model = lveModel;
    flatVase.transform.translation = { -.5f, .5f, 0.f };
    flatVase.transform.scale = { 3.f, 1.5f, 3.f };
    gameObjects.emplace(flatVase.getId(), std::move(flatVase));

    lveModel = LveModel::createModelFromFile(lveDevice, "I:\\VulkanProjects\\VulkanKeeler\\VulkanKeeler\\Models\\smooth_vase.obj");
    auto smoothVase = LveGameObject::createGameObject();
    smoothVase.model = lveModel;
    smoothVase.transform.translation = { .5f, .5f, 0.f };
    smoothVase.transform.scale = { 3.f, 1.5f, 3.f };
    gameObjects.emplace(smoothVase.getId(), std::move(smoothVase));

    lveModel = LveModel::createModelFromFile(lveDevice, "I:\\VulkanProjects\\VulkanKeeler\\VulkanKeeler\\Models\\quad.obj");
    auto floor = LveGameObject::createGameObject();
    floor.model = lveModel;
    floor.transform.translation = { 0.f, .5f, 0.f };
    floor.transform.scale = { 3.f, 1.f, 3.f };
    gameObjects.emplace(floor.getId(), std::move(floor));

    

    std::vector<glm::vec3> lightColors{
     {1.f, 1.f, .6f},
     {6.f, 1.f, .1f},
     {1.f, .6f, .6f},

     //
    };
    for (int i = 0; i < lightColors.size(); i++) {
      auto pointLight = LveGameObject::makePointLight(0.1f);
      pointLight.color = lightColors[i];
      auto rotateLight = glm::rotate( glm::mat4(1.f), (i * glm::two_pi<float>()) / lightColors.size(), { 0.f, -1.f, 0.f });
      pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -0.f, -1.f, 1.f));
      gameObjects.emplace(pointLight.getId(), std::move(pointLight));
    }

  }

}  // namespace lve