#include "Camera.hpp"

glm::vec3 Global_Up = glm::vec3(0.0f, 1.0f, 0.0f);

Camera::Camera() : Object(0) {}

void Camera::init()
{
}

void Camera::update(float dt)
{
	world_to_camera = glm::lookAt(transform.GetPosition(), transform.GetPosition() + transform.GetDirectionVector(), Global_Up);
}

void Camera::close()
{
}

void Camera::Move(float forward, float right)
{
	transform.MovePosition(transform.GetDirectionVector() * forward);
	transform.MovePosition(transform.GetRightVector() * right);
}

void Camera::LookAround(float roll, float pitch)
{
	static float proll = 0.0f;
	static float ppitch = 0.0f;

	proll += roll;
	ppitch -= pitch;

	ppitch = std::max(glm::radians(-89.5f), ppitch);
	ppitch = std::min(glm::radians(89.5f), ppitch);

	glm::quat qt = glm::angleAxis(proll, glm::vec3(0, 1, 0));
	glm::quat qt2 = glm::angleAxis(ppitch, glm::vec3(1, 0, 0));

	transform.SetRotation(qt2 * qt);
}

glm::mat4 Camera::GetWorldToCamera() const
{
	return world_to_camera;
}
