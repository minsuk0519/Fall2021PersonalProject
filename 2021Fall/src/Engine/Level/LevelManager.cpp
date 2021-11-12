#include "LevelManager.hpp"

//3rd party library
#include <imgui/imgui.h>

LevelManager::LevelManager(VkDevice device, Application* app) : System(device, app, "Level") {}

void LevelManager::init()
{
	Level* newlevel = new Level();

	newlevel->init();

	levelList.push_back(newlevel);

	SetCurrentLevel(newlevel);
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
    ImGui::Begin(name.c_str());



    ImGui::End();
}

void LevelManager::SetCurrentLevel(Level* level)
{
	currentLevel = level;
}
