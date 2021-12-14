#include "ObjectManager.hpp"

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
	Object* obj = new Object(currentIndex++, name);

	obj->init();

	objectList.push_back(obj);

	return obj;
}

Object* ObjectManager::addObject()
{
	std::string name = "Object" + std::to_string(currentIndex);
	Object* obj = new Object(currentIndex++, name);

	obj->init();

	objectList.push_back(obj);

	return obj;
}

void ObjectManager::drawGUI()
{
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
