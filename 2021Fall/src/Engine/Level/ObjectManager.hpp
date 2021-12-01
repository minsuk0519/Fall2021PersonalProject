#pragma once
//standard library
#include <vector>

#include "Engine/Entity/Object.hpp"

class ObjectManager
{
public:
	void init();
	void update(float dt);
	void close();

	void addObject();

private:
	std::vector<Object*> objectList;
	unsigned int currentIndex;
};