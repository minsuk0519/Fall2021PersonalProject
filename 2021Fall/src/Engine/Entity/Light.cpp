#include "Light.hpp"
#include "Engine/Memory/Buffer.hpp"
#include "Engine/Graphic/Graphic.hpp"
#include "Engine/Level/Level.hpp"
#include "Engine/Level/ObjectManager.hpp"
#include "Camera.hpp"

Light::Light(Level* level, unsigned int objid, std::string objname) : Object(level, objid, objname) {}

void Light::setLightIndex(uint32_t index, bool end)
{
	lightIndex = index;
	endIndex = end;
}

PointLight::PointLight(Level* level, unsigned int objid, std::string objname) : Light(level, objid, objname) {}

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

void PointLight::postinit()
{
	Object::postinit();
}

void PointLight::update(float dt)
{
	Object::update(dt);

	VulkanMemoryManager::MapMemory(UNIFORM_LIGHTDATA, 
		GetLightDataPointer(ownerLevel->GetObjectManager()->getObjectByTemplate<Camera>()->GetWorldToCamera()), 
		sizeof(LightData), lightIndex * LIGHTDATA_ALLIGNMENT);

	int data = lightIndex + 1;
	if(endIndex) VulkanMemoryManager::MapMemory(UNIFORM_LIGHTDATA, &data, sizeof(int), MAX_LIGHT * LIGHTDATA_ALLIGNMENT);
}

void PointLight::close()
{
}

void* PointLight::GetLightDataPointer(glm::mat4 viewMat)
{
	lightdata.position = glm::vec3(viewMat * glm::vec4(transform.GetPosition(), 1.0f));

	return reinterpret_cast<void*>(&lightdata);
}
