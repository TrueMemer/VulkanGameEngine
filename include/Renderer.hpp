#pragma once

#include "PCH.hpp"
#include "Engine.hpp"
#include "Vertex.hpp"

struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

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
	VkDescriptorSetLayout vkDescriptorSetLayout;
	VkDescriptorPool vkDescriptorPool;
	VkDescriptorSet vkDescriptorSet;
	VkRenderPass vkRenderPass;
	VkCommandPool vkCommandPool;
	std::vector<VkCommandBuffer> vkCommandBuffers;
	std::vector<VkFramebuffer> vkFramebuffers;
	std::vector<VkImage> vkSwapChainImages;
	std::vector<VkImageView> vkSwapChainImageViews;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;

	VkBuffer vkVertexBuffer;
	VkDeviceMemory vkVertexBufferMemory;

	VkBuffer vkIndexBuffer;
	VkDeviceMemory vkIndexBufferMemory;

	VkBuffer vkUniformBuffer;
	VkDeviceMemory vkUniformBufferMemory;

	VkBuffer vkStagingBuffer;
	VkDeviceMemory vkStagingBufferMemory;

	const std::vector<Vertex> vertices = {
		{ { -0.5f, -0.5f },{ 1.0f, 0.0f, 0.0f } },
		{ { 0.5f, -0.5f },{ 0.0f, 1.0f, 0.0f } },
		{ { 0.5f, 0.5f },{ 0.0f, 0.0f, 1.0f } },
		{ { -0.5f, 0.5f },{ 1.0f, 1.0f, 1.0f } }
	};
	
	const std::vector<uint16_t> indices = {
		0, 1, 2, 2, 3, 0
	};

	UniformBufferObject ubo;

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
	void initVulkanDescriptorSetLayout();
	void initVulkanGraphicsPipeline();
	void initVulkanFramebuffers();
	void initVulkanCommandPool();
	void initVulkanIndexBuffer();
	void initVulkanVertexBuffer();
	void initVulkanUniformBuffer();
	void initVulkanDescriptorPool();
	void initVulkanDescriptorSet();
	void initVulkanCommandBuffers();
	void initVulkanSemaphores();
	VkShaderModule createShaderModule(const std::vector<char>& code);

	void createVulkanBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags propertyFlags, VkBuffer& buffer, VkDeviceMemory &bufferMemory);
	void copyVulkanBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);

	void updateUniformBuffer();

	void cleanupSwapChain();
	void recreateVulkanSwapChain();
};

