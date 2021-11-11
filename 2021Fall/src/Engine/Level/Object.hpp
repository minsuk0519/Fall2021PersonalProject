#pragma once
#include "Engine/Common/Transform.hpp"

//standard library
#include <string>

class Object
{
public:
	Object(unsigned int objid, std::string objname = "");

	void init();
	void update(float dt);
	void close();

public:
	Transform& GetTransform();

protected:
	unsigned int id = -1;
	std::string name = "";

	Transform transform;
};