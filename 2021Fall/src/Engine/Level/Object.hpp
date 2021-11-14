#pragma once
#include "Engine/Common/Transform.hpp"

//standard library
#include <string>

class Object
{
public:
	Object(unsigned int objid, std::string objname = "");

	virtual void init();
	virtual void update(float dt);
	virtual void close();

public:
	Transform& GetTransform();

protected:
	unsigned int id = -1;
	std::string name = "";

	Transform transform;
};