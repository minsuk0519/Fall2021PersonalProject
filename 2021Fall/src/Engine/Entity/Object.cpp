#include "Object.hpp"
#include "Engine/Memory/Buffer.hpp"
#include "Engine/Common/Application.hpp"
#include "Engine/Graphic/Graphic.hpp"

//3rd party library
#include <glm/glm.hpp>
#include <imgui/imgui.h>

void Object::init()
{
}

void Object::update(float dt)
{
	uniform.objectMat = glm::translate(glm::mat4(1.0f), transform.position) * glm::toMat4(transform.rotation) * glm::scale(glm::mat4(1.0f), transform.scale);

	//VulkanMemoryManager::MapMemory(UNIFORM_OBJECT_MATRIX, &uniform, sizeof(ObjectUniform));

	Graphic* graphic = Application::APP()->GetSystem<Graphic>();

	graphic->AddDrawInfo({ &uniform, sizeof(ObjectUniform), 128 });
}

void Object::close()
{
}

Object::~Object()
{
}

void Object::drawGUI()
{
	ImGui::DragFloat3("Position", &transform.position.x, 0.1f);
	ImGui::ColorEdit3("Color", &uniform.color.x);
	ImGui::DragFloat("Metal", &uniform.metal, 0.002f, 0.0f, 1.0f);
	ImGui::DragFloat("Roughness", &uniform.roughness, 0.002f, 0.0f, 1.0f);
}

Transform& Object::GetTransform()
{
	return transform;
}

void Object::SetUniform(ObjectUniform objuniform)
{
	uniform = objuniform;
}

void* Object::GetUniformPointer()
{
	return reinterpret_cast<void*>(&uniform);
}

ObjectUniform& Object::GetUniform()
{
	return uniform;
}

void Object::SetDrawBehavior(PROGRAM_ID programid, DRAWTARGET_INDEX drawtargetindex)
{
	programID = programid;
	drawtargetIndex = drawtargetindex;
}

Object::Object(Level* level, unsigned int objid, std::string objname) : ownerLevel(level), id(objid), name(objname), Interface() {}

void Object::postinit()
{
	Graphic* graphic = Application::APP()->GetSystem<Graphic>();
	
	graphic->RegisterObject(programID, drawtargetIndex);
}
