#pragma once
#include "Engine/Common/Interface.hpp"
#include "Engine/Common/Transform.hpp"
#include "Engine/Graphic/Graphic.hpp"

//standard library
#include <string>

//3rd party library
#include <vulkan/vulkan.h>

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

	void SetDrawBehavior(PROGRAM_ID programid, DRAWTARGET_INDEX drawtargetindex);

protected:
	Object(unsigned int objid, std::string objname = "");

	unsigned int id = -1;
	std::string name = "";

	Transform transform;

	ObjectUniform uniform;

private:
	PROGRAM_ID programID = PROGRAM_ID::PROGRAM_ID_BASERENDER;
	DRAWTARGET_INDEX drawtargetIndex = DRAWTARGET_INDEX::DRAWTARGET_MODEL_INSTANCE;
};