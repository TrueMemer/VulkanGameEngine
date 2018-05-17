#include "PCH.hpp"
#include "VulkanWrapper.hpp"
#include "Engine.hpp"
#include "Window.hpp"

void PhysicalDeviceDetails::queryDetails()
{
	{
		vkGetPhysicalDeviceProperties(vkPhysicalDevice, &deviceProperties);
		vkGetPhysicalDeviceFeatures(vkPhysicalDevice, &deviceFeatures);

		switch (deviceProperties.deviceType)
		{
		case (VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU):
			suitabilityScore += 1000;
			break;
		case (VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU):
			suitabilityScore += 500;
			break;
		case (VK_PHYSICAL_DEVICE_TYPE_CPU):
			suitabilityScore += 100;
			break;
		default:
			suitabilityScore -= 100000;
			break;
		}
	}

	{
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount, nullptr);
		queueFamilies.resize(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount, queueFamilies.data());
	}

	{
		int i = 0;
		for (const auto& queueFamily : queueFamilies)
		{
			VkBool32 surfaceSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice, i, Engine::window->vkSurface, &surfaceSupport);

			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT && graphicsQueueFamily == -1)
				graphicsQueueFamily = i;

			if (graphicsQueueFamily == i && surfaceSupport && presentQueueFamily == -1)
				presentQueueFamily = i;

			if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT && computeQueueFamily == -1)
				computeQueueFamily = i;

			if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
				transferQueueFamily = i;

			suitabilityScore += queueFamily.queueCount;

			++i;
		}

		if (graphicsQueueFamily == -1 || presentQueueFamily == -1 || computeQueueFamily == -1 || transferQueueFamily == -1)
			suitabilityScore -= 100000;
	}

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkPhysicalDevice, Engine::window->vkSurface, &swapChainDetails.capabilities);

	{
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice, Engine::window->vkSurface, &formatCount, nullptr);

		if (formatCount != 0) {
			swapChainDetails.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice, Engine::window->vkSurface, &formatCount, swapChainDetails.formats.data());
		}
		else
			suitabilityScore -= 100000;
	}

	{
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice, Engine::window->vkSurface, &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			swapChainDetails.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice, Engine::window->vkSurface, &presentModeCount, swapChainDetails.presentModes.data());
		}
		else
			suitabilityScore -= 100000;
	}


	{
		vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice, &memoryProperties);
	}
}

U32 PhysicalDeviceDetails::getMemoryType(U32 typeFilter, VkMemoryPropertyFlags properties)
{
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}
	LOG_FATAL("Suitable memory type not found");
}