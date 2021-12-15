#include "Camera.hpp"
#include "Engine/Misc/settings.hpp"
#include "Engine/Memory/Buffer.hpp"
#include "Engine/Graphic/Graphic.hpp"

glm::vec3 Global_Up = glm::vec3(0.0f, 1.0f, 0.0f);

Camera::Camera(Level* level, unsigned int objid, std::string objname) : Object(level, objid, objname) {}

void Camera::init()
{
}

void Camera::postinit()
{
}

void Camera::update(float dt)
{
	camTransform.worldToCamera = glm::lookAtLH(transform.GetPosition(), transform.GetPosition() + transform.GetDirectionVector(), Global_Up);
	camTransform.cameraToNDC = glm::perspectiveLH_NO(glm::radians(45.0f), Settings::GetAspectRatio(), 0.1f, 500.0f);
	camTransform.cameraToNDC[1][1] *= -1;

	VulkanMemoryManager::MapMemory(UNIFORM_CAMERA_TRANSFORM, &camTransform);
}

void Camera::close()
{
}

void Camera::Move(float forward, float right, float up)
{
	transform.MovePosition(transform.GetDirectionVector() * forward);
	transform.MovePosition(transform.GetRightVector() * right);
	transform.MovePosition(transform.GetUpVector() * up);
}

void Camera::LookAround(float roll, float pitch)
{
	static float proll = 0.0f;
	static float ppitch = 0.0f;

	proll -= roll;
	ppitch += pitch;

	ppitch = std::max(glm::radians(-89.5f), ppitch);
	ppitch = std::min(glm::radians(89.5f), ppitch);

	glm::quat qt = glm::angleAxis(proll, glm::vec3(0, 1, 0));
	glm::quat qt2 = glm::angleAxis(ppitch, glm::vec3(1, 0, 0));

	transform.SetRotation(qt2 * qt);
}

glm::mat4 Camera::GetWorldToCamera() const
{
	return camTransform.worldToCamera;
}

void* Camera::GetDataPointer()
{
	return reinterpret_cast<void*>(&camTransform);
}

uint32_t Camera::GetDataSize()
{
	return static_cast<uint32_t>(sizeof(Cameratransform));
}
