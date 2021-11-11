#include "Level.hpp"
#include "ObjectManager.hpp"
#include "Engine/Graphic/Camera.hpp"

void Level::init()
{
	objManager = new ObjectManager();

	camera = new Camera();
}

void Level::update(float dt)
{
	objManager->update(dt);
}

void Level::close()
{
	objManager->close();
	delete objManager;

	camera->close();
	delete camera;
}
