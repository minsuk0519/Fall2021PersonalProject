#pragma once

#include "Engine/System.hpp"
#include "Level.hpp"

//standard library
#include <vector>

//3rd party library

class LevelManager : public System
{
public:
	LevelManager(VkDevice device, Application* app);

	virtual void init() override;
	virtual void update(float dt) override;
	virtual void close() override;

private:
	Level* currentLevel = nullptr;
	std::vector<Level*> levelList;
};