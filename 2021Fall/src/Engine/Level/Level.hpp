#pragma once
#include "Engine/Common/Interface.hpp"

class ObjectManager;
class Camera;

class Level : public Interface
{
public:
	friend class LevelManager;

	void init() override;
	void postinit() override;

	void update(float dt) override;
	void close() override;

	ObjectManager* GetObjectManager() const;

private:
	ObjectManager* objManager = nullptr;

	Camera* camera = nullptr;
};