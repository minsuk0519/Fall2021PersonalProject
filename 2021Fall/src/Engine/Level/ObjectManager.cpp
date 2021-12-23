#include "ObjectManager.hpp"
#include "Engine/Common/Application.hpp"
#include "Engine/Graphic/Graphic.hpp"
#include "Engine/Entity/Light.hpp"
#include "Engine/Entity/Camera.hpp"

ObjectManager::ObjectManager(Level* level) : ownerLevel(level)
{
}

void ObjectManager::init()
{
}

void ObjectManager::postinit()
{
	Graphic* graphic = Application::APP()->GetSystem<Graphic>();

	graphic->BeginCmdBuffer(CMD_INDEX::CMD_BASE);
	graphic->BeginRenderPass(CMD_INDEX::CMD_BASE, RENDERPASS_INDEX::RENDERPASS_PRE);

	for (auto obj : objectList)
	{
		obj->postinit();
	}

	graphic->EndRenderPass(CMD_INDEX::CMD_BASE);
	graphic->EndCmdBuffer(CMD_INDEX::CMD_BASE);

	graphic->BeginCmdBuffer(CMD_INDEX::CMD_SHADOW);
	for (uint32_t i = 0; i < MAX_LIGHT; ++i)
	{
		graphic->BeginRenderPass(CMD_INDEX::CMD_SHADOW, RENDERPASS_INDEX::RENDERPASS_DEPTHCUBEMAP, i);

		uint32_t index = 0;
		for (auto obj : objectList)
		{
			if (dynamic_cast<Light*>(obj) != nullptr) continue;
			if (dynamic_cast<Camera*>(obj) != nullptr) continue;
			graphic->RegisterObject(DESCRIPTORSET_INDEX::DESCRIPTORSET_ID_SHADOWMAP, PROGRAM_ID::PROGRAM_ID_SHADOWMAP, DRAWTARGET_INDEX::DRAWTARGET_CUBE, { index++, i });
		}

		graphic->EndRenderPass(CMD_INDEX::CMD_SHADOW);
	}
	graphic->EndCmdBuffer(CMD_INDEX::CMD_SHADOW);
}

void ObjectManager::update(float dt)
{
	for (auto& obj : objectList)
	{
		obj->update(dt);
	}
}

void ObjectManager::close()
{
	for (auto& obj : objectList)
	{
		obj->close();
		delete obj;
	}

	objectList.clear();
}

Object* ObjectManager::addObject(std::string name)
{
	Object* obj = new Object(ownerLevel, currentIndex++, name);

	obj->init();

	objectList.push_back(obj);

	return obj;
}

Object* ObjectManager::addObject()
{
	std::string name = "Object" + std::to_string(currentIndex);
	Object* obj = new Object(ownerLevel, currentIndex++, name);

	obj->init();

	objectList.push_back(obj);

	return obj;
}

void ObjectManager::drawGUI()
{
	ImGui::BeginListBox("Objects");

	static uint32_t selectedid = 0;

	for (auto obj : objectList)
	{
		if (ImGui::Selectable((obj->name + "##" + std::to_string(obj->id)).c_str(), selectedid == obj->id))
		{
			selectedid = obj->id;
		}
	}

	ImGui::EndListBox();

	getObjectByID(selectedid)->drawGUI();
}

Object* ObjectManager::getObjectByID(uint32_t id) const
{
	for (auto obj : objectList)
	{
		if (obj->id == id)
		{
			return obj;
		}
	}

	return nullptr;
}

Object* ObjectManager::getObjectByName(std::string name) const
{
	for (auto obj : objectList)
	{
		if (obj->name == name)
		{
			return obj;
		}
	}

	return nullptr;
}

void ObjectManager::addObject(Object* target)
{
	target->id = currentIndex++;

	target->init();

	objectList.push_back(target);
}
