#pragma once
#include "Engine/Common/Transform.hpp"

//standard library
#include <string>

struct ObjectUniform
{
	glm::mat4 objectMat = glm::mat4(1.0f);

	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
	float metal = 0.5f;
	float roughness = 0.5f;
};

class Object
{
public:
	Object(unsigned int objid, std::string objname = "");

	virtual void init();
	virtual void update(float dt);
	virtual void close();

	virtual void GuiSetting();

public:
	Transform& GetTransform();

	void SetUniform(ObjectUniform objuniform);
	void* GetUniformPointer();
	ObjectUniform& GetUniform();

protected:
	unsigned int id = -1;
	std::string name = "";

	Transform transform;

	ObjectUniform uniform;
};