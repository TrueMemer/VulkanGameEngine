#include "PCH.hpp"
#include "Engine.hpp"
#include "Window.hpp"
#include "Renderer.hpp"

void Engine::start()
{
	LOG_INFO("Starting engine");
	engineStartTime = clock.time();
#ifdef _WIN32

	win32Instance = GetModuleHandle(NULL);

#endif
	
	createVulkanInstance();
	createWindow();
	queryVulkanPhysicalDeviceDetails();

	renderer = new Renderer();
	renderer->init();

	Time initTime = clock.time() - engineStartTime;
	LOG_INFO("Initialisation time: " << initTime.getSecondsf() << " seconds");

	Time frameTime;
	double fpsDisplay = 0.f;
	int frames = 0;

	while (isRunning) 
	{
		frameTime = clock.time();
		while (window->processMessages()) {}

		Event ev;
		while (window->eventQ.pollEvent(ev)) {
			switch (ev.type) {
				case Event::KeyDown: {
					if (ev.eventUnion.keyEvent.key.code == Key::KC_ESCAPE)
						isRunning = false;
					std::cout << "Key down: " << char(ev.eventUnion.keyEvent.key.code) << std::endl;
					break;
				}
				case Event::KeyUp: {
					std::cout << "Key up: " << char(ev.eventUnion.keyEvent.key.code) << std::endl;
					break;
				}
			}
		}

		renderer->updateUniformBuffer();
		renderer->render();
		frameTime = clock.time() - frameTime;

		++frames;
		fpsDisplay += frameTime.getSeconds();
		if (fpsDisplay > 1.f)
		{
			//printf("%f\n", double(frames) / fpsDisplay);
			fpsDisplay = 0.f;
			frames = 0;
		}
	}

	quit();
}

void Engine::createWindow()
{
	LOG_INFO("Creating window");
	WindowCreateInfo c;

#ifdef _WIN32
	c.win32Instance = win32Instance;
#endif

	c.title = "Vulkan Engine";

	c.width = 800;
	c.height = 600;

	window = new Window();
	window->create(&c);
}

void Engine::createVulkanInstance()
{
	LOG_INFO("Creating Vulkan instance");
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.pApplicationName = "Keimo";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	std::vector<const char *> enabledExtensions;
	enabledExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	enabledExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#ifdef ENABLE_VULKAN_VALIDATION
	enabledExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif

	VkInstanceCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	info.pNext = nullptr;
	info.flags = 0;
	info.pApplicationInfo = &appInfo;
#ifdef ENABLE_VULKAN_VALIDATION
	info.enabledLayerCount = 1;
	auto layerName = "VK_LAYER_LUNARG_standard_validation";
	info.ppEnabledLayerNames = &layerName;
#else
	info.enabledLayerCount = 0;
	info.ppEnabledLayerNames = nullptr;
#endif

	info.enabledExtensionCount = (uint32_t)enabledExtensions.size();
	info.ppEnabledExtensionNames = enabledExtensions.data();

	if (vkCreateInstance(&info, nullptr, &vkInstance) != VK_SUCCESS) 
	{
		LOG_FATAL("Failed to create Vulkan instance");
	}
	else 
	{
		LOG_INFO("Vulkan instance created successfully");
	}

#ifdef ENABLE_VULKAN_VALIDATION
	VkDebugReportCallbackCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	createInfo.pfnCallback = debugCallbackFunc;
	
	auto createDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(vkInstance, "vkCreateDebugReportCallbackEXT");
	
	if (createDebugReportCallbackEXT(vkInstance, &createInfo, nullptr, &debugCallbackInfo) != VK_SUCCESS) {
		LOG_WARN("Failed to create debug callback");
	}
	else
	{
		LOG_INFO("Created debug callback");
	}
#endif
}

void Engine::queryVulkanPhysicalDeviceDetails()
{
	LOG_INFO("Picking Vulkan device");
	uint32_t physicalDeviceCount = 0;
	vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, nullptr);
	if (physicalDeviceCount == 0) {
		throw std::runtime_error("No devices found with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(physicalDeviceCount);
	vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, devices.data());

	std::multimap<int, VkPhysicalDevice> rankedDevices;

	for (const auto& device : devices) 
	{
		physicalDevicesDetails.push_back(PhysicalDeviceDetails(device));

		auto& qDevice = physicalDevicesDetails.back();

		qDevice.queryDetails();

		S32 highestSuitabilityScore = 0;
		int i = 0;
		for (auto &device : physicalDevicesDetails)
		{
			if (device.suitabilityScore > highestSuitabilityScore)
			{
				highestSuitabilityScore = device.suitabilityScore;
				vkPhysicalDevice = device.vkPhysicalDevice;
				physicalDeviceIndex = i;
			}
			++i;
		}
	}
	if (vkPhysicalDevice == VK_NULL_HANDLE)
	{
		LOG_FATAL("Failed to find suitable GPU");
	}
}

void Engine::quit()
{
	LOG_INFO("Exiting");

	renderer->cleanup();
	window->destroy();
#ifdef ENABLE_VULKAN_VALIDATION
	PFN_vkDestroyDebugReportCallbackEXT(vkGetInstanceProcAddr(vkInstance, "vkDestroyDebugReportCallbackEXT"))(vkInstance, debugCallbackInfo, 0);
#endif
	vkDestroyInstance(vkInstance, nullptr);
}

#ifdef ENABLE_VULKAN_VALIDATION
VKAPI_ATTR VkBool32 VKAPI_CALL Engine::debugCallbackFunc(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData)
{
	LOG_WARN(msg);
	return VK_FALSE;
}
VkDebugReportCallbackEXT Engine::debugCallbackInfo;
#endif

#ifdef _WIN32
HINSTANCE Engine::win32Instance;
#endif
Clock Engine::clock;
Window *Engine::window;
Renderer *Engine::renderer;
VkInstance Engine::vkInstance;
VkPhysicalDevice Engine::vkPhysicalDevice = VK_NULL_HANDLE;

std::vector<PhysicalDeviceDetails> Engine::physicalDevicesDetails;
int Engine::physicalDeviceIndex;

bool Engine::isRunning = true;
Time Engine::engineStartTime;