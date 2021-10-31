//third party library
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <imgui/imgui_impl_glfw.h>

//standard library
#include <iostream>
#include <stdexcept>
#include <set>
#include <algorithm>

#include "Application.hpp"
#include "Misc/settings.hpp"
#include "System.hpp"

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

Application* Application::applicationPtr = nullptr;

VkExtent2D guiWindowSize = { 800, 800 };

Application::Application() {}

void Application::init()
{
    //initialize glfw
    if (!glfwInit())
    {
        throw std::runtime_error("failed to initialize glfw");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    
    //create main window
    window = glfwCreateWindow(Settings::windowWidth, Settings::windowHeight, "Vulkan window", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

    if (window == nullptr)
    {
        throw std::runtime_error("failed to create glfw window");
    }

    initVulkan();

    {
        //create window for gui
        guiWindow = glfwCreateWindow(guiWindowSize.width, guiWindowSize.height, "GUI Window", NULL, NULL);

        //set current context as created gui window
        glfwMakeContextCurrent(guiWindow);
        glfwSetWindowUserPointer(guiWindow, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

        InitGui();
    }

    glfwMakeContextCurrent(window);
}

void Application::postinit()
{
    for (auto sys : engineSystems)
    {
        sys->init();
    }
}

void Application::update()
{
    while (!glfwWindowShouldClose(window) && !glfwWindowShouldClose(guiWindow)) 
    {
        //main window
        {
            glfwMakeContextCurrent(window);

            glfwPollEvents();

            for (auto sys : engineSystems)
            {
                sys->update(0.016f); //should be changed
            }
        }

        //gui window
        {
            glfwMakeContextCurrent(guiWindow);

            glfwPollEvents();
            glfwSwapBuffers(guiWindow);

            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            {
                if (ImGui::Begin("Hello"))
                {
                    ImGui::End();
                }
            }

            RenderGui();
        }
    }

    vkDeviceWaitIdle(vulkanDevice);
}

void Application::close()
{
    for (auto sys : engineSystems)
    {
        sys->close();
        delete sys;
    }

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    closeVulkan();

    glfwDestroyWindow(guiWindow);

    glfwDestroyWindow(window);

    glfwTerminate();

    delete applicationPtr;
}

Application* Application::APP()
{
    if (applicationPtr == nullptr)
    {
        applicationPtr = new Application();
    }

    return applicationPtr;
}

void Application::initVulkan()
{
    //create vulkan instance
    CreateVulkanInstance();

    //set vulkan debug
    setVulkandebug();

    //surface
    CreateSurface(window, vulkanSurface);

    //pick physical
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(vulkanInstance, &deviceCount, nullptr);
        if (deviceCount == 0)
        {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(vulkanInstance, &deviceCount, devices.data());

        for (const auto& device : devices)
        {
            if (isDeviceSuitable(device))
            {
                vulkanPhysicalDevice = device;
                break;
            }
        }

        if (vulkanPhysicalDevice == VK_NULL_HANDLE)
        {
            throw std::runtime_error("failed to find a suitable GPU!");
        }

        vkGetPhysicalDeviceProperties(vulkanPhysicalDevice, &vulkanDeviceProperties);
    }

    //logical device
    {
        QueueFamilyIndices indices = findQueueFamilies(vulkanPhysicalDevice, vulkanSurface);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) 
        {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            //one queue is for imgui
            queueCreateInfo.queueCount = 2;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};
        //enable sample shading
        deviceFeatures.sampleRateShading = VK_TRUE;

        VkDeviceCreateInfo deviceCreateInfo{};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
        deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
        deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

        if (enableValidationLayers)
        {
            deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else
        {
            deviceCreateInfo.enabledLayerCount = 0;
        }

        if (vkCreateDevice(vulkanPhysicalDevice, &deviceCreateInfo, nullptr, &vulkanDevice) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create logical device!");
        }

        vkGetDeviceQueue(vulkanDevice, indices.graphicsFamily.value(), 0, &vulkanGraphicsQueue);
        //for imgui note : assume this queue has same family index with gui queue
        vkGetDeviceQueue(vulkanDevice, indices.graphicsFamily.value(), 1, &guiQueue);
        vkGetDeviceQueue(vulkanDevice, indices.presentFamily.value(), 0, &vulkanPresentQueue);
    }

    //command pool
    {
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(vulkanPhysicalDevice, vulkanSurface);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
        poolInfo.flags = 0;

        if (vkCreateCommandPool(vulkanDevice, &poolInfo, nullptr, &vulkanCommandPool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create command pool!");
        }
    }
}

void Application::setVulkandebug()
{
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(vulkanInstance, &createInfo, nullptr, &vulkanDebugMessenger) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

void Application::closeVulkan()
{
    ImGui_ImplVulkanH_DestroyWindow(vulkanInstance, vulkanDevice, &guivulkanWindow, nullptr);

    vkDestroyDescriptorPool(vulkanDevice, guiDescriptorPool, nullptr);

    vkDestroyCommandPool(vulkanDevice, vulkanCommandPool, nullptr);

    vkDestroyDevice(vulkanDevice, nullptr);

    vkDestroySurfaceKHR(vulkanInstance, vulkanSurface, nullptr);

    if (enableValidationLayers)
    {
        DestroyDebugUtilsMessengerEXT(vulkanInstance, vulkanDebugMessenger, nullptr);
    }

    vkDestroyInstance(vulkanInstance, nullptr);
}

bool Application::checkValidationLayerSupport()
{
    uint32_t layerCount;

    //get size of layerproperties
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers) 
    {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) 
        {
            if (strcmp(layerName, layerProperties.layerName) == 0) 
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) 
        {
            return false;
        }
    }

    return true;
}

std::vector<const char*> Application::getRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) 
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT /*messageSeverity*/,
    VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* /*pUserData*/)
{

    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, 
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
    const VkAllocationCallbacks* pAllocator, 
    VkDebugUtilsMessengerEXT* pDebugMessenger) 
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) 
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else 
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}

void framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    Application* application = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));

    application->framebufferSizeUpdate = true;

    Settings::windowWidth = width;
    Settings::windowHeight = height;
}

void guiWindowResizeCallback(GLFWwindow* window, int width, int height)
{
    Application* application = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));

    application->guirecreateswapchain = true;

    guiWindowSize.width = width;
    guiWindowSize.height = height;
}

void Application::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | 
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | 
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | 
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | 
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

bool Application::isDeviceSuitable(VkPhysicalDevice device)
{
    QueueFamilyIndices indices = findQueueFamilies(device, vulkanSurface);

    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported) 
    {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

bool Application::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

bool QueueFamilyIndices::isComplete()
{
    return graphicsFamily.has_value() && presentFamily.has_value();
}

QueueFamilyIndices Application::findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies)
    {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if (presentSupport)
        {
            indices.presentFamily = i;
        }

        if (indices.isComplete())
        {
            break;
        }

        i++;
    }

    return indices;
}

SwapChainSupportDetails Application::querySwapChainSupport(VkPhysicalDevice device)
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, vulkanSurface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, vulkanSurface, &formatCount, nullptr);

    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, vulkanSurface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, vulkanSurface, &presentModeCount, nullptr);

    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, vulkanSurface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

VkSurfaceFormatKHR Application::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR Application::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Application::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != UINT32_MAX)
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent =
        {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

void Application::CreateSurface(GLFWwindow* windowptr, VkSurfaceKHR& surface)
{
    if (surface != nullptr)
    {
        std::cout << "target surface is not nullptr!" << std::endl;
    }

    if (glfwCreateWindowSurface(vulkanInstance, windowptr, nullptr, &surface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to creaete window surface!");
    }
}

void Application::CreateVulkanInstance()
{
    if (enableValidationLayers && !checkValidationLayerSupport())
    {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Minsuk API";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 2, 0);
    appInfo.pEngineName = "Minsuk Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 2, 0);
    appInfo.apiVersion = VK_MAKE_API_VERSION(0, 1, 2, 0);

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    std::vector<const char*> extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    if (enableValidationLayers)
    {
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)(&debugCreateInfo);
    }
    else
    {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &vulkanInstance) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create instance!");
    }
}

VkSwapchainKHR Application::CreateSwapChain(uint32_t& imageCount, VkFormat& swapChainImageFormat, VkExtent2D& swapChainExtent)
{
    VkSwapchainKHR swapChain;

    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(vulkanPhysicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    if (extent.width != Settings::windowWidth ||
        extent.height != Settings::windowHeight)
    {
        throw std::runtime_error("Error creating swapchain : window size is incorrect!");
    }

    imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 &&
        imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = vulkanSurface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(vulkanPhysicalDevice, vulkanSurface);
    uint32_t queueFamilyIndices[] = {
        indices.graphicsFamily.value(),
        indices.presentFamily.value()
    };

    if (indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    if (vkCreateSwapchainKHR(vulkanDevice, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create swapchain!");
    }

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;

    return swapChain;
}

VkCommandPool Application::GetCommandPool() const
{
    return vulkanCommandPool;
}


VkQueue Application::GetGraphicQueue() const
{
    return vulkanGraphicsQueue;
}

VkQueue Application::GetPresentQueue() const
{
    return vulkanPresentQueue;
}

VkPhysicalDevice Application::GetPhysicalDevice() const
{
    return vulkanPhysicalDevice;
}

VkPhysicalDeviceProperties Application::GetDeviceProperties() const
{
    return vulkanDeviceProperties;
}

void Application::InitGui()
{
    //create surfac
    CreateSurface(guiWindow, guiSurface);

    guivulkanWindow.Surface = guiSurface;

    QueueFamilyIndices indices = findQueueFamilies(vulkanPhysicalDevice, guiSurface);

    VkBool32 presentSupport = false;

    guiQueueFamilyIndex = indices.graphicsFamily.value();

    const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
    const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    guivulkanWindow.SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(vulkanPhysicalDevice, guiSurface, requestSurfaceImageFormat, (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);

    VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR };
    guivulkanWindow.PresentMode = ImGui_ImplVulkanH_SelectPresentMode(vulkanPhysicalDevice, guiSurface, &present_modes[0], IM_ARRAYSIZE(present_modes));

    ImGui_ImplVulkanH_CreateOrResizeWindow(vulkanInstance, vulkanPhysicalDevice, vulkanDevice, &guivulkanWindow,
        guiQueueFamilyIndex, nullptr, guiWindowSize.width, guiWindowSize.height, guiminImage);

    {
        VkDescriptorPoolSize pool_sizes[] =
        {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
        };
        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
        pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;
        VkResult result = vkCreateDescriptorPool(vulkanDevice, &pool_info, nullptr, &guiDescriptorPool);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create decriptorpool for imgui!");
        }
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForVulkan(guiWindow, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = vulkanInstance;
    init_info.PhysicalDevice = vulkanPhysicalDevice;
    init_info.Device = vulkanDevice;
    init_info.QueueFamily = guiQueueFamilyIndex;
    init_info.Queue = guiQueue;
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = guiDescriptorPool;
    init_info.Allocator = VK_NULL_HANDLE;
    init_info.MinImageCount = guiminImage;
    init_info.ImageCount = guivulkanWindow.ImageCount;
    ImGui_ImplVulkan_Init(&init_info, guivulkanWindow.RenderPass);

    {
        // Use any command queue
        VkCommandPool command_pool = guivulkanWindow.Frames[guivulkanWindow.FrameIndex].CommandPool;
        VkCommandBuffer command_buffer = guivulkanWindow.Frames[guivulkanWindow.FrameIndex].CommandBuffer;

        vkResetCommandPool(vulkanDevice, command_pool, 0);
        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(command_buffer, &begin_info);

        ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

        VkSubmitInfo end_info = {};
        end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        end_info.commandBufferCount = 1;
        end_info.pCommandBuffers = &command_buffer;
        vkEndCommandBuffer(command_buffer);
        vkQueueSubmit(guiQueue, 1, &end_info, VK_NULL_HANDLE);

        vkDeviceWaitIdle(vulkanDevice);
        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }
}

void Application::RenderGui()
{
    if (guirecreateswapchain)
    {
        ImGui_ImplVulkan_SetMinImageCount(guiminImage);
        ImGui_ImplVulkanH_CreateOrResizeWindow(vulkanInstance, vulkanPhysicalDevice, vulkanDevice, 
            &guivulkanWindow, guiQueueFamilyIndex, nullptr, guiWindowSize.width, guiWindowSize.height, guiminImage);
        guivulkanWindow.FrameIndex = 0;

        guirecreateswapchain = false;
    }

    // Rendering
    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();
    const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
    if (!is_minimized)
    {
        guivulkanWindow.ClearValue.color.float32[0] = 0.3f;
        guivulkanWindow.ClearValue.color.float32[1] = 0.3f;
        guivulkanWindow.ClearValue.color.float32[2] = 0.3f;
        guivulkanWindow.ClearValue.color.float32[3] = 1.0f;

        VkSemaphore image_acquired_semaphore = guivulkanWindow.FrameSemaphores[guivulkanWindow.SemaphoreIndex].ImageAcquiredSemaphore;
        VkSemaphore render_complete_semaphore = guivulkanWindow.FrameSemaphores[guivulkanWindow.SemaphoreIndex].RenderCompleteSemaphore;
        VkResult err = vkAcquireNextImageKHR(vulkanDevice, guivulkanWindow.Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &guivulkanWindow.FrameIndex);
        if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
        {
            guirecreateswapchain = true;
            return;
        }

        ImGui_ImplVulkanH_Frame* fd = &guivulkanWindow.Frames[guivulkanWindow.FrameIndex];
        vkWaitForFences(vulkanDevice, 1, &fd->Fence, VK_TRUE, UINT64_MAX);
        vkResetFences(vulkanDevice, 1, &fd->Fence);
        vkResetCommandPool(vulkanDevice, fd->CommandPool, 0);
        VkCommandBufferBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
        VkRenderPassBeginInfo renderpassinfo = {};
        renderpassinfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderpassinfo.renderPass = guivulkanWindow.RenderPass;
        renderpassinfo.framebuffer = fd->Framebuffer;
        renderpassinfo.renderArea.extent.width = guivulkanWindow.Width;
        renderpassinfo.renderArea.extent.height = guivulkanWindow.Height;
        renderpassinfo.clearValueCount = 1;
        renderpassinfo.pClearValues = &guivulkanWindow.ClearValue;
        vkCmdBeginRenderPass(fd->CommandBuffer, &renderpassinfo, VK_SUBPASS_CONTENTS_INLINE);

        ImGui_ImplVulkan_RenderDrawData(draw_data, fd->CommandBuffer);

        // Submit command buffer
        vkCmdEndRenderPass(fd->CommandBuffer);
        VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo submitinfo = {};
        submitinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitinfo.waitSemaphoreCount = 1;
        submitinfo.pWaitSemaphores = &image_acquired_semaphore;
        submitinfo.pWaitDstStageMask = &wait_stage;
        submitinfo.commandBufferCount = 1;
        submitinfo.pCommandBuffers = &fd->CommandBuffer;
        submitinfo.signalSemaphoreCount = 1;
        submitinfo.pSignalSemaphores = &render_complete_semaphore;

        vkEndCommandBuffer(fd->CommandBuffer);
        vkQueueSubmit(guiQueue, 1, &submitinfo, fd->Fence);

        if (guirecreateswapchain) return;

        VkSemaphore render_complete_semaphore = guivulkanWindow.FrameSemaphores[guivulkanWindow.SemaphoreIndex].RenderCompleteSemaphore;
        VkPresentInfoKHR presentinfo = {};
        presentinfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentinfo.waitSemaphoreCount = 1;
        presentinfo.pWaitSemaphores = &render_complete_semaphore;
        presentinfo.swapchainCount = 1;
        presentinfo.pSwapchains = &guivulkanWindow.Swapchain;
        presentinfo.pImageIndices = &guivulkanWindow.FrameIndex;
        err = vkQueuePresentKHR(guiQueue, &presentinfo);
        if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
        {
            guirecreateswapchain = true;
            return;
        }
        guivulkanWindow.SemaphoreIndex = (guivulkanWindow.SemaphoreIndex + 1) % guivulkanWindow.ImageCount;
    }
}

