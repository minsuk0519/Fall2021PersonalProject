#pragma once
#include "Object.hpp"

//3rd party library
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/mat4x4.hpp>

struct Cameratransform
{
	glm::mat4 worldToCamera;
	glm::mat4 cameraToNDC;
};

class Camera : public Object
{
public:
	Camera(Level* level, unsigned int objid, std::string objname);
	virtual void init() override;
	virtual void postinit() override;

	virtual void update(float dt) override;
	virtual void close() override;

public:
	void Move(float forward, float right, float up = 0.0f);
	void LookAround(float roll, float pitch);

	glm::mat4 GetWorldToCamera() const;

	void* GetDataPointer();
	uint32_t GetDataSize();

private:

	Cameratransform camTransform;
};
