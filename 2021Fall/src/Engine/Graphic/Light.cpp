#include "Light.hpp"

Light::Light() : Object(0) {}

PointLight::PointLight() : Light() {}

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
}

void PointLight::close()
{
}

void* PointLight::GetLightDataPointer(glm::mat4 viewMat)
{
	lightdata.position = glm::vec3(viewMat * glm::vec4(transform.GetPosition(), 1.0f));

	return reinterpret_cast<void*>(&lightdata);
}
