#pragma once

#include "Engine/System.hpp"

//standard library
#include <vector>
#include <bitset>
#include <map>

//3rd party library
#include <GLFW/glfw3.h>

enum class KeyBinding : int
{
	KEY_UP = GLFW_KEY_W,
	KEY_DOWN = GLFW_KEY_S,
	KEY_LEFT = GLFW_KEY_A,
	KEY_RIGHT = GLFW_KEY_D,
	KEY_SHIFT = GLFW_KEY_LEFT_SHIFT,

	MOUSE_LEFT = GLFW_KEY_LAST + GLFW_MOUSE_BUTTON_LEFT,
	MOUSE_RIGHT = GLFW_KEY_LAST + GLFW_MOUSE_BUTTON_RIGHT,
};

struct Point2D
{
	float x;
	float y;
};

class Input : public System
{
public:
	Input(VkDevice device, Application* app);

	virtual void init() override;
	virtual void update(float dt) override;
	virtual void close() override;
	virtual void drawGUI() override;

	static bool isKeyPressed(int keycode);
	static bool isMousePressed(int keycode);
	static bool isKeyTriggered(int keycode);
	static bool isMouseTriggered(int keycode);

	static bool isPressed(KeyBinding key);
	static bool isTriggered(KeyBinding key);

	friend void keyCallback(GLFWwindow*, int, int, int, int);
	friend void mouseCallback(GLFWwindow*, int, int, int);
	friend void mouseposCallback(GLFWwindow*, double, double);

	static Point2D GetMousePos();
	static Point2D GetMouseMove();

private:
	static std::bitset<GLFW_KEY_LAST + GLFW_MOUSE_BUTTON_LAST> triggeredKey;
	static std::bitset<GLFW_KEY_LAST + GLFW_MOUSE_BUTTON_LAST> pressedKey;

	static Point2D mousePos;
	static Point2D premousePos;
	static Point2D mouseMove;
};

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void mouseCallback(GLFWwindow* window, int button, int action, int mods);
static void mouseposCallback(GLFWwindow* window, double xpos, double ypos);
