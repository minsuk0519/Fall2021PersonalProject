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
#include <concepts>

class System;

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete();
};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class Application
{
public:
	void init();
	void postinit();
	void update();
	void close();

	template <class T> requires std::derived_from<T, System>
	void AddSystem()
	{
		engineSystems.push_back(new T(vulkanDevice, this));
	}

	//
	VkFormat GetSwapChainImageFormat() const;
	const std::vector<VkImageView>& GetSwapChainImageViews() const;
	VkCommandPool GetCommandPool() const;
private:
	//method
	void initVulkan();
	void setVulkandebug();
	
	void closeVulkan();

	bool checkValidationLayerSupport();

	std::vector<const char*> getRequiredExtensions();

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

	bool isDeviceSuitable(VkPhysicalDevice device);

	bool checkDeviceExtensionSupport(VkPhysicalDevice device);

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
private:
	VkInstance vulkanInstance;
	VkDebugUtilsMessengerEXT vulkanDebugMessenger;

	VkPhysicalDevice vulkanPhysicalDevice = VK_NULL_HANDLE;
	VkDevice vulkanDevice;
	VkQueue vulkanGraphicsQueue;
	VkQueue vulkanPresentQueue;

	VkSurfaceKHR vulkanSurface;

	VkSwapchainKHR vulkanSwapChain;

	std::vector<VkImage> vulkanSwapChainImages;

	VkExtent2D vulkanSwapChainExtent;

	std::vector<VkImageView> vulkanSwapChainImageViews;

	VkFormat vulkanSwapChainImageFormat;

	VkCommandPool vulkanCommandPool;

	GLFWwindow* window = nullptr;

	std::vector<System*> engineSystems;
};

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, 
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
	const VkAllocationCallbacks* pAllocator, 
	VkDebugUtilsMessengerEXT* pDebugMessenger);

void DestroyDebugUtilsMessengerEXT(VkInstance instance, 
	VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks* pAllocator);
