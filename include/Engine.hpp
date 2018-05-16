#pragma once

#include "PCH.hpp"
#include "Window.hpp"
#include "Renderer.hpp"
#include "VulkanWrapper.hpp"
#include "Time.hpp"
#include "Clock.hpp"

class Window;
class Renderer;

class Engine
{
public:
	Engine() {}
	~Engine() {}

	static void start();
	static void createVulkanInstance();
	static void queryVulkanPhysicalDeviceDetails();
	static void createWindow();
	static void quit();

	static VkPhysicalDevice getPhysicalDevice() { return vkPhysicalDevice; }
	static PhysicalDeviceDetails& getPhysicalDeviceDetails() { return physicalDevicesDetails[physicalDeviceIndex]; }

	#ifdef ENABLE_VULKAN_VALIDATION
	static VkDebugReportCallbackEXT debugCallbackInfo;
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallbackFunc(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData);
	#endif

	#ifdef _WIN32
	static HINSTANCE win32Instance;
	#endif
	static Clock clock;
	static Window *window;
	static Renderer *renderer;


	static VkInstance vkInstance;
	static VkPhysicalDevice vkPhysicalDevice;
	static std::vector<PhysicalDeviceDetails> physicalDevicesDetails;
	static int physicalDeviceIndex;
	
	static bool isRunning;
	static Time engineStartTime;
};

