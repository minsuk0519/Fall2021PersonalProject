#pragma once
#include "Engine/Common/Interface.hpp"
#include "Engine/Common/Transform.hpp"
#include "Engine/Graphic/Graphic.hpp"
#include "Engine/Memory/Buffer.hpp"

//standard library
#include <string>

//3rd party library
#include <vulkan/vulkan.h>

class Level;

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

	virtual void drawGUI();

public:
	Transform& GetTransform();

	void SetUniform(ObjectUniform objuniform);
	void* GetUniformPointer();
	ObjectUniform& GetUniform();

	void SetDrawBehavior(DESCRIPTORSET_INDEX descriptorsetid, PROGRAM_ID programid, DRAWTARGET_INDEX drawtargetindex, UniformBufferIndex uniformbufferid);
	unsigned int getID() const;

protected:
	Object(Level* level, unsigned int objid, std::string objname = "");

	unsigned int id = -1;
	std::string name = "";

	Transform transform;

	ObjectUniform uniform;

	Level* ownerLevel = nullptr;

private:
	PROGRAM_ID programID = PROGRAM_ID::PROGRAM_ID_BASERENDER;
	DESCRIPTORSET_INDEX descriptorsetID = DESCRIPTORSET_INDEX::DESCRIPTORSET_ID_OBJ;
	DRAWTARGET_INDEX drawtargetIndex = DRAWTARGET_INDEX::DRAWTARGET_MODEL_INSTANCE;
	UniformBufferIndex uniformID = UniformBufferIndex::UNIFORM_OBJECT_MATRIX;
};