#include "Object.hpp"

//3rd party library
#include <glm/glm.hpp>
#include <imgui/imgui.h>

void Object::init()
{
}

void Object::update(float dt)
{
	uniform.objectMat = glm::translate(glm::mat4(1.0f), transform.position) * glm::toMat4(transform.rotation) * glm::scale(glm::mat4(1.0f), transform.scale);
}

void Object::close()
{
}

void Object::GuiSetting()
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

Object::Object(unsigned int objid, std::string objname) : id(objid), name(objname) {}
