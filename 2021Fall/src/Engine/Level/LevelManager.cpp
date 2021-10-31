#include "LevelManager.hpp"

LevelManager::LevelManager(VkDevice device, Application* app) : System(device, app) {}

void LevelManager::init()
{
	Level* newlevel = new Level();

	newlevel->init();

	levelList.push_back(newlevel);

	currentLevel = newlevel;
}

void LevelManager::update(float dt)
{
	currentLevel->update(dt);
}

void LevelManager::close()
{
	for (auto& level : levelList)
	{
		level->close();
	}
}
