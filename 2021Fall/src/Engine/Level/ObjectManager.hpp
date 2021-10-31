#pragma once
//standard library
#include <vector>

#include "Object.hpp"

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