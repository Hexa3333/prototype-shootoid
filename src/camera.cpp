#include "camera.hpp"

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera() {
    position = glm::vec3(0, 0, -3.0f);
}

glm::mat4 Camera::update() {
    direction = glm::vec3(0, 0, 1.0f);
    static const glm::vec3 global_up = glm::vec3(0, 1.0f, 0);
    right = glm::normalize(glm::cross(global_up, -direction));
    up = glm::normalize(glm::cross(right, direction));

    return glm::lookAt(position, position + direction, up);
}

glm::vec3 Camera::get_position() {
    return position;
}

void Camera::set_position(glm::vec3 set) {
    position = set;
}
