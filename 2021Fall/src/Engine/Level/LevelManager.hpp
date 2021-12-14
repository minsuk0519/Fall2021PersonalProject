#pragma once

#include "Engine/Common/System.hpp"
#include "Level.hpp"

//standard library
#include <vector>

//3rd party library

class LevelManager : public System
{
public:
	LevelManager(VkDevice device, Application* app);

	virtual void init() override;
	virtual void postinit() override;

	virtual void update(float dt) override;
	virtual void close() override;
	virtual void drawGUI() override;

public:
	void SetCurrentLevel(Level* level);

private:
	Level* currentLevel = nullptr;
	std::vector<Level*> levelList;
};