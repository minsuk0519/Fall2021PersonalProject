#include "Input.hpp"
#include "Engine/Common/Application.hpp"
#include "Engine/Misc/settings.hpp"

std::bitset<GLFW_KEY_LAST + GLFW_MOUSE_BUTTON_LAST> Input::triggeredKey;
std::bitset<GLFW_KEY_LAST + GLFW_MOUSE_BUTTON_LAST> Input::pressedKey;

Point2D Input::mousePos;
Point2D Input::premousePos;
Point2D Input::mouseMove;

Input::Input(VkDevice device, Application* app) : System(device, app, "Input") {}

void Input::init()
{
	triggeredKey.reset();
	pressedKey.reset();
	mousePos = { 0, 0 };
	premousePos = { 0, 0 };
	mouseMove = { 0, 0 };

	GLFWwindow* windowptr = Application::APP()->GetWindowPointer();
	glfwSetKeyCallback(windowptr, keyCallback);
	glfwSetMouseButtonCallback(windowptr, mouseCallback);
	glfwSetCursorPosCallback(windowptr, mouseposCallback);
}

void Input::postinit()
{
}

void Input::update(float /*dt*/)
{
	triggeredKey.reset();

	if (pressedKey[GLFW_KEY_ESCAPE])
	{
		Application::APP()->SetShutdown();
	}

	Input::mouseMove = { Input::mousePos.x - Input::premousePos.x, Input::mousePos.y - Input::premousePos.y };

	Input::premousePos = Input::mousePos;
}

void Input::close()
{
	triggeredKey.reset();
	pressedKey.reset();
}

void Input::drawGUI()
{
}

bool Input::isKeyPressed(int keycode)
{
	return pressedKey[keycode];
}

bool Input::isMousePressed(int keycode)
{
	int mousecode = keycode + GLFW_KEY_LAST;

	return pressedKey[mousecode];
}

bool Input::isKeyTriggered(int keycode)
{
	return triggeredKey[keycode];
}

bool Input::isMouseTriggered(int keycode)
{
	int mousecode = keycode + GLFW_KEY_LAST;

	return triggeredKey[mousecode];
}

bool Input::isPressed(KeyBinding key)
{
	int keycode = static_cast<int>(key);

	return isKeyPressed(keycode);
}

bool Input::isTriggered(KeyBinding key)
{
	int keycode = static_cast<int>(key);

	return isKeyTriggered(keycode);
}

Point2D Input::GetMousePos()
{
	return mousePos;
}

Point2D Input::GetMouseMove()
{
	return mouseMove;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		Input::pressedKey[key] = true;
		Input::triggeredKey[key] = true;
	}
	if (action == GLFW_RELEASE)
	{
		Input::pressedKey[key] = false;
	}
}

void mouseCallback(GLFWwindow* window, int button, int action, int mods)
{
	int keycode = static_cast<int>(button + GLFW_KEY_LAST);

	if (action == GLFW_PRESS)
	{
		Input::pressedKey[keycode] = true;
		Input::triggeredKey[keycode] = true;
	}
	if (action == GLFW_RELEASE)
	{
		Input::pressedKey[keycode] = false;
	}
}

void mouseposCallback(GLFWwindow* window, double xpos, double ypos)
{
	float adjustypos = static_cast<float>(Settings::windowHeight - ypos);
	Input::mousePos = { static_cast<float>(xpos),  adjustypos };
}
