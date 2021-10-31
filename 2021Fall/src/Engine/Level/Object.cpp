#include "Object.hpp"

void Object::init()
{
}

void Object::update(float dt)
{
}

void Object::close()
{
}

Object::Object(unsigned int objid, std::string objname) : id(objid), name(objname) {}
