#pragma once

//3rd party library
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>

class Transform
{
public:
	glm::vec3 GetPosition() const;
	glm::quat GetQuaternion() const;
	glm::vec3 GetEulerAngle() const;
	glm::vec3 GetScale() const;
	void SetPosition(glm::vec3 pos);
	void SetRotation(glm::quat qt);
	void SetRotation(glm::vec3 rot);
	void SetScale(glm::vec3 s);

	void MovePosition(glm::vec3 pos);
	void ScaleTransform(glm::vec3 s);

	glm::vec3 GetRightVector() const;
	glm::vec3 GetUpVector() const;
	glm::vec3 GetDirectionVector() const;

	void Rotate(float pitch, float yaw, float roll);
	void Rotate(glm::quat qt);

private:
	glm::vec3 position;
	glm::quat rotation = glm::quat(glm::vec3(0,0,0));
	glm::vec3 scale;
};