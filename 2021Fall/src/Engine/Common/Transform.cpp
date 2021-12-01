#include "Transform.hpp"

glm::vec3 Transform::GetPosition() const
{
	return position;
}

glm::vec3& Transform::GetPosition()
{
	return position;
}

glm::quat Transform::GetQuaternion() const
{
	return rotation;
}

glm::vec3 Transform::GetEulerAngle() const
{
	return glm::eulerAngles(rotation);
}

glm::vec3 Transform::GetScale() const
{
	return scale;
}

void Transform::SetPosition(glm::vec3 pos)
{
	position = pos;
}

void Transform::SetRotation(glm::quat qt)
{
	rotation = qt;
}

void Transform::SetRotation(glm::vec3 rot)
{
	rotation = glm::quat(rot);
}

void Transform::SetScale(glm::vec3 s)
{
	scale = s;
}

void Transform::MovePosition(glm::vec3 pos)
{
	position += pos;
}

void Transform::ScaleTransform(glm::vec3 s)
{
	scale.x *= s.x;
	scale.y *= s.y;
	scale.z *= s.z;
}

glm::vec3 Transform::GetRightVector() const
{
	return glm::rotate(glm::inverse(rotation), glm::vec3(1.0, 0.0, 0.0));
}

glm::vec3 Transform::GetUpVector() const
{
	return glm::rotate(glm::inverse(rotation), glm::vec3(0.0, 1.0, 0.0));
}

glm::vec3 Transform::GetDirectionVector() const
{
	return glm::rotate(glm::inverse(rotation), glm::vec3(0.0, 0.0, 1.0));
}

void Transform::Rotate(float pitch, float yaw, float roll)
{
	//rotation = glm::quat(glm::vec3(pitch, yaw, roll));
	
}

void Transform::Rotate(glm::quat qt)
{
	rotation = rotation * qt;
}
