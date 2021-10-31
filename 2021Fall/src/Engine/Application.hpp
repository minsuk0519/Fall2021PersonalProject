#pragma once
//third party library
#define GLFW_INCLUDE_VULKAN
#define VK_USE_PLATFORM_WIN32_KHR
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <vulkan/vulkan.h>
#include <imgui/imgui_impl_vulkan.h>

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

	static Application* APP();

//template method
public:
	template <class T> requires std::derived_from<T, System>
	void AddSystem()
	{
		engineSystems.push_back(new T(vulkanDevice, this));
	}

//vulkan method
public:
	VkSwapchainKHR CreateSwapChain(uint32_t& imageCount, VkFormat& swapChainImageFormat, VkExtent2D& swapChainExtent);
	VkCommandPool GetCommandPool() const;
	VkQueue GetGraphicQueue() const;
	VkQueue GetPresentQueue() const;
	VkPhysicalDevice GetPhysicalDevice() const;

	VkPhysicalDeviceProperties GetDeviceProperties() const;

//member variables
public:
	bool framebufferSizeUpdate = false;
	bool guirecreateswapchain = false;

//vulkan method
private:
	//create only one instance
	Application();

	void initVulkan();
	void setVulkandebug();
	
	void closeVulkan();

	bool checkValidationLayerSupport();

	std::vector<const char*> getRequiredExtensions();

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

	bool isDeviceSuitable(VkPhysicalDevice device);

	bool checkDeviceExtensionSupport(VkPhysicalDevice device);

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	void CreateVulkanInstance();
	void CreateSurface(GLFWwindow* windowptr, VkSurfaceKHR& surface);

//gui method
private:
	void InitGui();
	void RenderGui();

//vulkan variables
private:
	VkInstance vulkanInstance = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT vulkanDebugMessenger = VK_NULL_HANDLE;
	VkPhysicalDevice vulkanPhysicalDevice = VK_NULL_HANDLE;
	VkDevice vulkanDevice = VK_NULL_HANDLE;
	VkQueue vulkanGraphicsQueue = VK_NULL_HANDLE;
	VkQueue vulkanPresentQueue = VK_NULL_HANDLE;
	VkSurfaceKHR vulkanSurface = VK_NULL_HANDLE;
	VkCommandPool vulkanCommandPool = VK_NULL_HANDLE;

	VkPhysicalDeviceProperties vulkanDeviceProperties;

//for Gui window
private:
	ImGui_ImplVulkanH_Window guivulkanWindow;
	VkQueue guiQueue;
	VkSurfaceKHR guiSurface = VK_NULL_HANDLE;
	VkDescriptorPool guiDescriptorPool;
	int guiminImage = 2;

	uint32_t guiQueueFamilyIndex;

//member variables
private:
	GLFWwindow* window = nullptr;
	GLFWwindow* guiWindow = nullptr;

	std::vector<System*> engineSystems;

	static Application* applicationPtr;
};

//vulkan helper function
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, 
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
	const VkAllocationCallbacks* pAllocator, 
	VkDebugUtilsMessengerEXT* pDebugMessenger);

void DestroyDebugUtilsMessengerEXT(VkInstance instance, 
	VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks* pAllocator);

//callback function
static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
static void guiWindowResizeCallback(GLFWwindow* window, int width, int height);