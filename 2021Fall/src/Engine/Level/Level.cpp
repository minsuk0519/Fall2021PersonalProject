#include "Level.hpp"
#include "ObjectManager.hpp"
#include "Engine/Entity/Camera.hpp"
#include "Engine/Entity/Light.hpp"
#include "Engine/Input/Input.hpp"

void Level::init()
{
	objManager = new ObjectManager();

    camera = objManager->addObjectByTemplate<Camera>();
    camera->GetTransform().SetPosition(glm::vec3(0.0f, 3.0f, -5.0f));

    objManager->addObjectByTemplate<PointLight>();
    objManager->addObjectByTemplate<PointLight>();
    objManager->addObjectByTemplate<PointLight>();
    objManager->addObjectByTemplate<PointLight>();

    Object* newobj = objManager->addObject();
    newobj->GetTransform().SetScale(glm::vec3(1.0f));
    newobj->GetTransform().SetPosition(glm::vec3(-7.5f, 0.0f, 0.0f));
    newobj->SetUniform(ObjectUniform{ glm::mat4(1.0f), glm::vec3(0.955008f, 0.637427f, 0.538163f), 1.0f, 1.0f });

    newobj = objManager->addObject();
    newobj->GetTransform().SetScale(glm::vec3(1.0f));
    newobj->GetTransform().SetPosition(glm::vec3(7.5f, 0.0f, 0.0f));
    newobj->SetUniform(ObjectUniform{ glm::mat4(1.0f), glm::vec3(1.0f, 0.765557f, 0.336057f), 1.0f, 1.0f });

    newobj = objManager->addObject();
    newobj->GetTransform().SetScale(glm::vec3(1.0f));
    newobj->GetTransform().SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
    newobj->SetUniform(ObjectUniform{ glm::mat4(1.0f), glm::vec3(0.659777f, 0.608679f, 0.525649f), 1.0f, 1.0f });
}

void Level::postinit()
{
}

void Level::update(float dt)
{
    float cam_speed = 5.0f;

    if (Input::isPressed(KeyBinding::KEY_SHIFT)) cam_speed = 30.0f;
    if (Input::isPressed(KeyBinding::KEY_UP)) camera->Move(cam_speed * dt, 0.0f);
    if (Input::isPressed(KeyBinding::KEY_DOWN)) camera->Move(-cam_speed * dt, 0.0f);
    if (Input::isPressed(KeyBinding::KEY_RIGHT)) camera->Move(0.0f, cam_speed * dt);
    if (Input::isPressed(KeyBinding::KEY_LEFT)) camera->Move(0.0f, -cam_speed * dt);
    if (Input::isPressed(KeyBinding::MOUSE_RIGHT)) camera->LookAround(Input::GetMouseMove().x * dt, Input::GetMouseMove().y * dt);

	objManager->update(dt);
}

void Level::close()
{
	objManager->close();
	delete objManager;
}
