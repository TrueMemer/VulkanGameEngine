#pragma once

#include "PCH.hpp"
#include "Engine.hpp"

class Renderer
{
public:
	Renderer() {}
	~Renderer() {}

	VkDevice vkLogicalDevice;
	VkQueue vkGraphicsQueue;
	VkQueue vkPresentQueue;
	VkSwapchainKHR vkSwapChain;
	VkPipeline vkPipeline;
	VkPipelineLayout vkPipelineLayout;
	VkRenderPass vkRenderPass;
	VkCommandPool vkCommandPool;
	std::vector<VkCommandBuffer> vkCommandBuffers;
	std::vector<VkFramebuffer> vkFramebuffers;
	std::vector<VkImage> vkSwapChainImages;
	std::vector<VkImageView> vkSwapChainImageViews;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& modes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;

	void init();
	void render();
	void cleanup();

	void initVulkanLogicalDevice();
	void initVulkanSwapChain();
	void initVulkanImageViews();
	void initVulkanRenderPass();
	void initVulkanGraphicsPipeline();
	void initVulkanFramebuffers();
	void initVulkanCommandPool();
	void initVulkanCommandBuffers();
	void initVulkanSemaphores();
	VkShaderModule createShaderModule(const std::vector<char>& code);

	void cleanupSwapChain();
	void recreateVulkanSwapChain();
};

