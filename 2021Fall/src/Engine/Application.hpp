#pragma once
//third party library
#define GLFW_INCLUDE_VULKAN
#define VK_USE_PLATFORM_WIN32_KHR
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <vulkan/vulkan.h>

//standard library
#include <vector>
#include <optional>

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete();
};

class Application
{
public:
	void init();
	void update();
	void close();

private:
	void initVulkan();
	void setVulkandebug();
	
	void closeVulkan();

	bool checkValidationLayerSupport();

	std::vector<const char*> getRequiredExtensions();

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

	bool isDeviceSuitable(VkPhysicalDevice device);

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
private:
	VkInstance vulkanInstance;
	VkDebugUtilsMessengerEXT vulkanDebugMessenger;

	VkPhysicalDevice vulkanPhysicalDevice = VK_NULL_HANDLE;
	VkDevice vulkanDevice;
	VkQueue vulkanGraphicsQueue;
	VkQueue vulkanPresentQueue;

	VkSurfaceKHR vulkanSurface;

	GLFWwindow* window = nullptr;

private:
	unsigned int windowWidth = 1280;
	unsigned int windowHeight = 720;
};

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, 
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
	const VkAllocationCallbacks* pAllocator, 
	VkDebugUtilsMessengerEXT* pDebugMessenger);

void DestroyDebugUtilsMessengerEXT(VkInstance instance, 
	VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks* pAllocator);
