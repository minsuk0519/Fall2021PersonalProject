#include "LevelManager.hpp"
#include "ObjectManager.hpp"

//3rd party library
#include <imgui/imgui.h>

Level* LevelManager::currentLevel = nullptr;
std::vector<Level*> LevelManager::levelList;

LevelManager::LevelManager(VkDevice device, Application* app) : System(device, app, "Level") {}

void LevelManager::init()
{
	Level* newlevel = new Level();

	newlevel->init();

	levelList.push_back(newlevel);

	SetCurrentLevel(newlevel);
}

void LevelManager::postinit()
{
	currentLevel->postinit();
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
		delete level;
	}
	levelList.clear();
}

void LevelManager::drawGUI()
{
	if (ImGui::CollapsingHeader("Objects##LevelManager"))
	{
		currentLevel->objManager->drawGUI();
	}
}

Level* LevelManager::GetCurrentLevel()
{
	return currentLevel;
}

void LevelManager::SetCurrentLevel(Level* level)
{
	currentLevel = level;
}
