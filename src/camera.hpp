#pragma once

#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"

class Camera {
public:
    Camera();
    glm::mat4 update();
    glm::vec3 get_position();
    void set_position(glm::vec3 set);
private:
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 up;
    glm::vec3 right;
};
