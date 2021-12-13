#include "Light.hpp"
#include "Engine/Memory/Buffer.hpp"

#include "Engine/Graphic/Graphic.hpp"

Light::Light(unsigned int objid, std::string objname) : Object(objid, objname) {}

PointLight::PointLight(unsigned int objid, std::string objname) : Light(objid, objname) {}

void PointLight::init()
{
	lightdata.ambient = glm::vec3(1, 1, 1);
	lightdata.diffuse = glm::vec3(1, 1, 1);
	lightdata.specular = glm::vec3(1, 1, 1);
	lightdata.attenuationC1 = 1.0f;
	lightdata.attenuationC2 = 0.5f;
	lightdata.attenuationC3 = 0.5f;
	transform.SetPosition(glm::vec3(0, 5, 0));
	lightdata.position = glm::vec3(0, 5, 0);
	lightdata.direction = glm::vec3(0, 1, 0);
	lightdata.theta = 0.0f;
	lightdata.phi = 0.0f;
	lightdata.falloff = 0.0f;
	lightdata.type = 0;
}

void PointLight::update(float dt)
{
	uint32_t size = static_cast<uint32_t>(1);
	for (uint32_t i = 0; i < size; ++i)
	{
		//VulkanMemoryManager::MapMemory(UNIFORM_LIGHTDATA, lightEntities[i]->GetLightDataPointer(camera->GetWorldToCamera()), sizeof(LightData), i * LIGHTDATA_ALLIGNMENT);
		VulkanMemoryManager::MapMemory(UNIFORM_LIGHTDATA, &lightdata, sizeof(LightData), i * LIGHTDATA_ALLIGNMENT);
	}
	int data = size;
	VulkanMemoryManager::MapMemory(UNIFORM_LIGHTDATA, &data, sizeof(int), MAX_LIGHT * LIGHTDATA_ALLIGNMENT);
}

void PointLight::close()
{
}

void* PointLight::GetLightDataPointer(glm::mat4 viewMat)
{
	lightdata.position = glm::vec3(viewMat * glm::vec4(transform.GetPosition(), 1.0f));

	return reinterpret_cast<void*>(&lightdata);
}
