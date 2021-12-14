#pragma once
#include "Engine/Common/Interface.hpp"
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

class Object : public Interface
{
public:
	friend class ObjectManager;

	virtual void init() override;
	virtual void postinit() override;

	virtual void update(float dt) override;
	virtual void close() override;

	virtual ~Object() override;

	virtual void GuiSetting();

public:
	Transform& GetTransform();

	void SetUniform(ObjectUniform objuniform);
	void* GetUniformPointer();
	ObjectUniform& GetUniform();

protected:
	Object(unsigned int objid, std::string objname = "");

	unsigned int id = -1;
	std::string name = "";

	Transform transform;

	ObjectUniform uniform;
};