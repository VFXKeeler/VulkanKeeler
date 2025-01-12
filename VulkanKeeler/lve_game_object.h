#pragma once

#include "lve_model.h"

//std
#include <memory>

namespace lve {

  struct Transfomr2dComponent {
    glm::vec2 translation{}; // position offset3
    glm::vec2 scale{ 1.f, 1.f };
    float rotation;
    
    glm::mat2 mat2() {
      const float s = glm::sin(rotation);
      const float c = glm::cos(rotation);
      glm::mat2 rotMatrix{ {c,s}, {-s,c} };

      glm::mat2 scaleMat{ {scale.x, .0f}, {.0f, scale.y} };
      return  rotMatrix * scaleMat;
    }
  };
  struct RigidBody2dComponent {
    glm::vec2 velocity;
    float mass{1.0f};
  };

  class LveGameObject {
  public:
    using id_t = unsigned int;

    static LveGameObject createGameObject() {
      static id_t currentId = 0;
      return LveGameObject{ currentId++};
    }

    LveGameObject(const LveGameObject&) = delete;
    LveGameObject& operator= (const LveGameObject&) = delete;
    LveGameObject(LveGameObject&&) = default; 
    LveGameObject& operator=(LveGameObject&&) = default;

    id_t getId() { return id; }

    std::shared_ptr<LveModel> model{};
    glm::vec3 color;
    Transfomr2dComponent transform2d{};
    RigidBody2dComponent rigidBody2d{};

  private:
    LveGameObject(id_t objId) : id{ objId } {}


    id_t id;
  };
}