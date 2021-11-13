#pragma once
#include "Engine/Level/Object.hpp"

//3rd party library
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

class Camera : public Object
{
public:
	Camera();
	void init();
	void update(float dt);
	void close();

public:
	void Move(float forward, float right, float up = 0.0f);
	void LookAround(float roll, float pitch);

	glm::mat4 GetWorldToCamera() const;

private:

	glm::mat4 world_to_camera;
};
