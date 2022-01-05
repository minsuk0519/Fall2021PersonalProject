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

std::vector<Object*>& ObjectManager::getObjList()
{
	return objectList;
}

void ObjectManager::addObject(Object* target)
{
	target->id = currentIndex++;

	target->init();

	objectList.push_back(target);
}
