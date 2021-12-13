#pragma once
//standard library
#include <vector>

#include "Engine/Entity/Object.hpp"

template <class T>
concept OBJDerived = std::is_base_of<Object, T>::value;

class ObjectManager
{
public:
	void init();
	void update(float dt);
	void close();

	Object* addObject(std::string name);
	Object* addObject();
	void addObject(Object* target);

	template<OBJDerived<> T>
	T* addObjectByTemplate()
	{
		std::string name = "Object" + std::to_string(currentIndex);

		Object* obj = new T(currentIndex++, name);

		obj->init();
		objectList.push_back(obj);

		return dynamic_cast<T*>(obj);
	}

	template<OBJDerived<> T>
	T* addObjectByTemplate(std::string name)
	{
		Object* obj = new T(currentIndex++, name);

		obj->init();
		objectList.push_back(obj);

		return dynamic_cast<T*>(obj);
	}

	void drawGUI();

	Object* getObjectByID(uint32_t id) const;
	Object* getObjectByName(std::string name) const;

private:
	std::vector<Object*> objectList;
	unsigned int currentIndex;
};