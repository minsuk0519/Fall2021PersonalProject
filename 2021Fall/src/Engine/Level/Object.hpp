#pragma once

//standard library
#include <string>

class Object
{
public:
	void init();
	void update(float dt);
	void close();

	Object(unsigned int objid, std::string objname = "");

private:
	unsigned int id = -1;
	std::string name = "";
};