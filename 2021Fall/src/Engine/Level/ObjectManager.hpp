#pragma once
//standard library
#include <vector>

#include "Engine/Entity/Object.hpp"

class Light;

template <class T>
concept OBJDerived = std::is_base_of<Object, T>::value;

class ObjectManager : public Interface
{
public:
	ObjectManager(Level* level);

	virtual void init() override;
	virtual void postinit() override;

	virtual void update(float dt) override;
	virtual void close() override;

	Object* addObject(std::string name);
	Object* addObject();
	void addObject(Object* target);

	template<OBJDerived<> T>
	T* addObjectByTemplate()
	{
		std::string name = "Object" + std::to_string(currentIndex);

		Object* obj = new T(ownerLevel, currentIndex++, name);

		obj->init();
		objectList.push_back(obj);

		if (Light* lit = dynamic_cast<Light*>(obj); lit != nullptr)
		{
			lightList.push_back(lit);
		}

		return dynamic_cast<T*>(obj);
	}

	template<OBJDerived<> T>
	T* addObjectByTemplate(std::string name)
	{
		Object* obj = new T(currentIndex++, name);

		obj->init();
		objectList.push_back(obj);

		if (Light* lit = dynamic_cast<Light*>(obj); lit != nullptr)
		{
			lightList.push_back(lit);
		}

		return dynamic_cast<T*>(obj);
	}

	void drawGUI();

	Object* getObjectByID(uint32_t id) const;
	Object* getObjectByName(std::string name) const;

	template<OBJDerived<> T>
	T* getObjectByTemplate() const
	{
		for (auto obj : objectList)
		{
			if (T* cast = dynamic_cast<T*>(obj); cast != nullptr)
			{
				return cast;
			}
		}

		return nullptr;
	}

private:
	std::vector<Object*> objectList;
	std::vector<Light*> lightList;

	unsigned int currentIndex;

	Level* ownerLevel = nullptr;
};
