#include "Level.hpp"
#include "ObjectManager.hpp"

void Level::init()
{
	objManager = new ObjectManager();
}

void Level::update(float dt)
{
	objManager->update(dt);
}

void Level::close()
{
	objManager->close();
	delete objManager;
}
