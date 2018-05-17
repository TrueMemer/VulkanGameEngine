#include "PCH.hpp"
#include "Renderer.hpp"

void Renderer::init()
{
	initVulkanLogicalDevice();
	initVulkanSwapChain();
	initVulkanImageViews();
	initVulkanRenderPass();
	initVulkanGraphicsPipeline();
	initVulkanFramebuffers();
	initVulkanCommandPool();
	initVulkanIndexBuffer();
	initVulkanVertexBuffer();
	initVulkanCommandBuffers();
	initVulkanSemaphores();
}

void Renderer::render() {

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(vkLogicalDevice, vkSwapChain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

	if (result != VK_SUCCESS)
	{
		cleanupSwapChain();
		recreateVulkanSwapChain();
		return;
	}

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { imageAvailableSemaphore };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &vkCommandBuffers[imageIndex];

	VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(vkGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
		LOG_FATAL("Failed to submit Vulkan draw command buffer");
	}

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { vkSwapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;

	presentInfo.pImageIndices = &imageIndex;

	vkQueuePresentKHR(vkPresentQueue, &presentInfo);

	vkQueueWaitIdle(vkPresentQueue);

}

void Renderer::initVulkanLogicalDevice()
{
	LOG_INFO("Creating logical device");

	VkDeviceQueueCreateInfo qci;
	qci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	qci.pNext = 0;
	qci.flags = 0;
	qci.queueFamilyIndex = 0;
	qci.queueCount = 1;
	float priority = 1.f;
	qci.pQueuePriorities = &priority;

	VkPhysicalDeviceFeatures features = {};

	VkDeviceCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	info.pNext = nullptr;
	info.flags = 0;
	info.queueCreateInfoCount = 1;
	info.pQueueCreateInfos = &qci;

#ifdef ENABLE_VULKAN_VALIDATION
	info.enabledLayerCount = 1;
	auto layerName = "VK_LAYER_LUNARG_standard_validation";
	info.ppEnabledLayerNames = &layerName;
#else
	info.enabledLayerCount = 0;
	info.ppEnabledLayerNames = nullptr;
#endif

	info.enabledExtensionCount = 0;
	info.ppEnabledExtensionNames = 0;
	info.pEnabledFeatures = &features;

	if (vkCreateDevice(Engine::vkPhysicalDevice, &info, nullptr, &vkLogicalDevice) != VK_SUCCESS)
	{
		LOG_FATAL("Failed to create logical device");
	}
	else
	{
		LOG_INFO("Logical device created successfully");
	}

	vkGetDeviceQueue(vkLogicalDevice, 0, 0, &vkGraphicsQueue);
	vkGetDeviceQueue(vkLogicalDevice, 0, 0, &vkPresentQueue); // Может не работать на AMD
}

VkSurfaceFormatKHR Renderer::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
{
	if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
	{
		return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	for (const auto& current : formats)
	{
		if (current.format == VK_FORMAT_B8G8R8A8_UNORM && current.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return current;
		}
	}

	return formats[0];
}

VkPresentModeKHR Renderer::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& modes)
{
	for (const auto& mode : modes)
	{
		if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return mode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Renderer::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}
	else
	{
		VkExtent2D actualExtent = { static_cast<uint32_t>(Engine::window->resX), static_cast<uint32_t>(Engine::window->resY) };
		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
		return actualExtent;
	}
}

void Renderer::initVulkanSwapChain()
{
	LOG_INFO("Creating swapchain");

	PhysicalDeviceDetails& details = Engine::getPhysicalDeviceDetails();

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Engine::vkPhysicalDevice, Engine::window->vkSurface, &details.swapChainDetails.capabilities);

	U32 formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(Engine::vkPhysicalDevice, Engine::window->vkSurface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		details.swapChainDetails.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(Engine::vkPhysicalDevice, Engine::window->vkSurface, &formatCount, details.swapChainDetails.formats.data());
	}
	else
	{
		LOG_FATAL("Device not suitable (swap chain format count is 0)");
	}

	U32 presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(Engine::vkPhysicalDevice, Engine::window->vkSurface, &presentModeCount, nullptr);

	if (presentModeCount != 0)
	{
		details.swapChainDetails.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(Engine::vkPhysicalDevice, Engine::window->vkSurface, &presentModeCount, details.swapChainDetails.presentModes.data());
	}
	else
	{
		LOG_FATAL("Device not suitable (swap chain present modes count is 0)");
	}

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(details.swapChainDetails.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(details.swapChainDetails.presentModes);
	VkExtent2D extent = chooseSwapExtent(details.swapChainDetails.capabilities);

	VkSwapchainCreateInfoKHR info = {};
	info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	info.surface = Engine::window->vkSurface;

	uint32_t imageCount = details.swapChainDetails.capabilities.minImageCount + 1;
	if (details.swapChainDetails.capabilities.maxImageCount > 0 && imageCount > details.swapChainDetails.capabilities.maxImageCount) {
		imageCount = details.swapChainDetails.capabilities.maxImageCount;
	}

	info.minImageCount = imageCount;
	info.imageFormat = surfaceFormat.format;
	info.imageColorSpace = surfaceFormat.colorSpace;
	info.imageExtent = extent;
	info.imageArrayLayers = 1;
	info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

	info.preTransform = details.swapChainDetails.capabilities.currentTransform;
	info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	info.presentMode = presentMode;
	info.clipped = VK_TRUE;
	info.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(vkLogicalDevice, &info, nullptr, &vkSwapChain) != VK_SUCCESS)
	{
		LOG_FATAL("Failed to create swapchain")
	}
	else
	{
		LOG_INFO("Swapchain created successfully");
	}

	vkGetSwapchainImagesKHR(vkLogicalDevice, vkSwapChain, &imageCount, nullptr);
	vkSwapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(vkLogicalDevice, vkSwapChain, &imageCount, vkSwapChainImages.data());

	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;
}

void Renderer::initVulkanImageViews()
{
	LOG_INFO("Creating image views");
	vkSwapChainImageViews.resize(vkSwapChainImages.size());

	for (uint32_t i = 0; i < vkSwapChainImages.size(); i++) {

		VkImageViewCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		info.image = vkSwapChainImages[i];
		info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		info.format = swapChainImageFormat;
		info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		info.subresourceRange.baseMipLevel = 0;
		info.subresourceRange.levelCount = 1;
		info.subresourceRange.baseArrayLayer = 0;
		info.subresourceRange.layerCount = 1;

		if (vkCreateImageView(vkLogicalDevice, &info, VK_NULL_HANDLE, &vkSwapChainImageViews[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create image views!");
		}
	}

	LOG_INFO("Image views created successfully");
}

void Renderer::initVulkanRenderPass()
{
	LOG_INFO("Creating render pass");
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkRenderPassCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	info.attachmentCount = 1;
	info.pAttachments = &colorAttachment;
	info.subpassCount = 1;
	info.pSubpasses = &subpass;

	if (vkCreateRenderPass(vkLogicalDevice, &info, nullptr, &vkRenderPass) != VK_SUCCESS)
	{
		LOG_FATAL("Failed to create render pass");
	}
	else
	{
		LOG_INFO("Render pass created successfully");
	}
}

static std::vector<char> readFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("Failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

void Renderer::initVulkanGraphicsPipeline()
{
	LOG_INFO("Creating Vulkan graphics pipeline");

	auto vertShaderCode = readFile("shaders/vert.spv");
	auto fragShaderCode = readFile("shaders/frag.spv");

	VkShaderModule vertShaderModule;
	VkShaderModule fragShaderModule;

	vertShaderModule = createShaderModule(vertShaderCode);
	fragShaderModule = createShaderModule(fragShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStagesArray[] = { vertShaderStageInfo, fragShaderStageInfo };

	auto bindingDescription = Vertex::getBindingDescription();
	auto attributeDescriptions = Vertex::getAttributeDescription();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
	inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.f;
	viewport.y = 0.f;
	viewport.width = (float)swapChainExtent.width;
	viewport.height = (float)swapChainExtent.height;
	viewport.minDepth = 0.f;
	viewport.maxDepth = 1.f;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = swapChainExtent;

	VkPipelineViewportStateCreateInfo viewportStateInfo = {};
	viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateInfo.viewportCount = 1;
	viewportStateInfo.pViewports = &viewport;
	viewportStateInfo.scissorCount = 1;
	viewportStateInfo.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;


	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pushConstantRangeCount = 0;

	if (vkCreatePipelineLayout(vkLogicalDevice, &pipelineLayoutInfo, nullptr, &vkPipelineLayout) != VK_SUCCESS)
	{
		LOG_FATAL("Failed to create pipeline layout");
	}
	else
	{
		LOG_INFO("Pipeline layout created successfully");
	}

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStagesArray;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
	pipelineInfo.pViewportState = &viewportStateInfo;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.layout = vkPipelineLayout;

	pipelineInfo.renderPass = vkRenderPass;
	pipelineInfo.subpass = 0;

	if (vkCreateGraphicsPipelines(vkLogicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &vkPipeline) != VK_SUCCESS)
	{
		LOG_FATAL("Failed to create graphics pipeline");
	}
	else
	{
		LOG_INFO("Graphics pipeline created successfully");
	}

	vkDestroyShaderModule(vkLogicalDevice, vertShaderModule, VK_NULL_HANDLE);
	vkDestroyShaderModule(vkLogicalDevice, fragShaderModule, VK_NULL_HANDLE);

}

void Renderer::initVulkanFramebuffers()
{
	LOG_INFO("Creating framebuffers");
	vkFramebuffers.resize(vkSwapChainImageViews.size());

	for (size_t i = 0; i < vkSwapChainImageViews.size(); i++) {

		VkImageView attachments[] = { vkSwapChainImageViews[i] };

		VkFramebufferCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		info.renderPass = vkRenderPass;
		info.attachmentCount = 1;
		info.pAttachments = attachments;
		info.width = swapChainExtent.width;
		info.height = swapChainExtent.height;
		info.layers = 1;

		if (vkCreateFramebuffer(vkLogicalDevice, &info, nullptr, &vkFramebuffers[i]) != VK_SUCCESS)
		{
			LOG_FATAL("Failed to create framebuffer");
		}

	}

	LOG_INFO("Framebuffers created successfully");
}

void Renderer::initVulkanCommandPool()
{
	LOG_INFO("Creating command pool");
	VkCommandPoolCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	info.queueFamilyIndex = 0;

	if (vkCreateCommandPool(vkLogicalDevice, &info, nullptr, &vkCommandPool) != VK_SUCCESS)
	{
		LOG_FATAL("Failed to create command pool");
	}
	else {
		LOG_INFO("Command pool created successfully");
	}
}

void Renderer::initVulkanVertexBuffer()
{
	LOG_INFO("Creating vertex buffer");
	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

	createVulkanBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vkStagingBuffer, vkStagingBufferMemory);

	void* data;
	vkMapMemory(vkLogicalDevice, vkStagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(vkLogicalDevice, vkStagingBufferMemory);

	createVulkanBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vkVertexBuffer, vkVertexBufferMemory);

	copyVulkanBuffer(vkStagingBuffer, vkVertexBuffer, bufferSize);

	vkDestroyBuffer(vkLogicalDevice, vkStagingBuffer, 0);
	vkFreeMemory(vkLogicalDevice, vkStagingBufferMemory, 0);

}

void Renderer::initVulkanIndexBuffer()
{
	LOG_INFO("Creating index buffer");
	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

	createVulkanBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vkStagingBuffer, vkStagingBufferMemory);

	void* data;
	vkMapMemory(vkLogicalDevice, vkStagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices.data(), (size_t)bufferSize);
	vkUnmapMemory(vkLogicalDevice, vkStagingBufferMemory);

	createVulkanBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vkIndexBuffer, vkIndexBufferMemory);

	copyVulkanBuffer(vkStagingBuffer, vkIndexBuffer, bufferSize);

	vkDestroyBuffer(vkLogicalDevice, vkStagingBuffer, nullptr);
	vkFreeMemory(vkLogicalDevice, vkStagingBufferMemory, nullptr);
}

void Renderer::initVulkanCommandBuffers()
{
	LOG_INFO("Creating Vulkan command buffers");

	vkCommandBuffers.resize(vkFramebuffers.size());

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = vkCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)vkCommandBuffers.size();

	if (vkAllocateCommandBuffers(vkLogicalDevice, &allocInfo, vkCommandBuffers.data()) != VK_SUCCESS) {
		LOG_FATAL("Failed to allocate Vulkan command buffers");
	}

	for (size_t i = 0; i < vkCommandBuffers.size(); i++) {
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

		vkBeginCommandBuffer(vkCommandBuffers[i], &beginInfo);

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = vkRenderPass;
		renderPassInfo.framebuffer = vkFramebuffers[i];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChainExtent;

		VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(vkCommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(vkCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline);

		VkBuffer vertexBuffers[] = { vkVertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(vkCommandBuffers[i], 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(vkCommandBuffers[i], vkIndexBuffer, 0, VK_INDEX_TYPE_UINT16);
		
		vkCmdDrawIndexed(vkCommandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

		vkCmdEndRenderPass(vkCommandBuffers[i]);

		if (vkEndCommandBuffer(vkCommandBuffers[i]) != VK_SUCCESS) {
			LOG_FATAL("Failed to record Vulkan command buffer");
		}
	}

}

void Renderer::initVulkanSemaphores()
{
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	if (vkCreateSemaphore(vkLogicalDevice, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS
		||
		vkCreateSemaphore(vkLogicalDevice, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS) {
		LOG_FATAL("Failed to create semaphores");
	}
	else {
		LOG_INFO("Semaphores created successfully");
	}

}

VkShaderModule Renderer::createShaderModule(const std::vector<char>& code) {

	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(vkLogicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		LOG_FATAL("Failed to create shader module");

	}

	return shaderModule;

}

void Renderer::createVulkanBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags propertyFlags,
	VkBuffer& buffer, VkDeviceMemory &bufferMemory) 
{

	VkBufferCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	info.size = size;
	info.usage = usage;
	info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(vkLogicalDevice, &info, VK_NULL_HANDLE, &buffer) != VK_SUCCESS) 
	{
		LOG_FATAL("Failed to create buffer!");
	}

	VkMemoryRequirements requirements;

	vkGetBufferMemoryRequirements(vkLogicalDevice, buffer, &requirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = requirements.size;
	allocInfo.memoryTypeIndex = Engine::getPhysicalDeviceDetails().getMemoryType(requirements.memoryTypeBits, propertyFlags);

	if (vkAllocateMemory(vkLogicalDevice, &allocInfo, VK_NULL_HANDLE, &bufferMemory) != VK_SUCCESS) 
	{
		LOG_FATAL("Failed to allocate buffer memory!");
	}

	vkBindBufferMemory(vkLogicalDevice, buffer, bufferMemory, 0);
}

void Renderer::copyVulkanBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size) {

	VkCommandBufferAllocateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	bufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	bufferInfo.commandPool = vkCommandPool;
	bufferInfo.commandBufferCount = 1;
	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(vkLogicalDevice, &bufferInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	VkBufferCopy copyRegion = {};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, src, dst, 1, &copyRegion);

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	vkQueueSubmit(vkGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(vkGraphicsQueue);

	vkFreeCommandBuffers(vkLogicalDevice, vkCommandPool, 1, &commandBuffer);

}

void Renderer::cleanup()
{
	cleanupSwapChain();
	vkDestroyBuffer(vkLogicalDevice, vkIndexBuffer, nullptr);
	vkFreeMemory(vkLogicalDevice, vkIndexBufferMemory, nullptr);
	vkDestroyBuffer(vkLogicalDevice, vkVertexBuffer, nullptr);
	vkFreeMemory(vkLogicalDevice, vkVertexBufferMemory, nullptr);
	vkDestroySemaphore(vkLogicalDevice, renderFinishedSemaphore, 0);
	vkDestroySemaphore(vkLogicalDevice, imageAvailableSemaphore, 0);
	vkDestroyCommandPool(vkLogicalDevice, vkCommandPool, 0);
	vkDestroyDevice(vkLogicalDevice, 0);
}

void Renderer::cleanupSwapChain()
{
	for (auto framebuffer : vkFramebuffers)
	{
		vkDestroyFramebuffer(vkLogicalDevice, framebuffer, nullptr);
	}

	vkFreeCommandBuffers(vkLogicalDevice, vkCommandPool, U32(vkCommandBuffers.size()), vkCommandBuffers.data());
	vkDestroyPipeline(vkLogicalDevice, vkPipeline, nullptr);
	vkDestroyPipelineLayout(vkLogicalDevice, vkPipelineLayout, nullptr);
	vkDestroyRenderPass(vkLogicalDevice, vkRenderPass, nullptr);

	for (auto imageView : vkSwapChainImageViews) {
		vkDestroyImageView(vkLogicalDevice, imageView, nullptr);
	}

	vkDestroySwapchainKHR(vkLogicalDevice, vkSwapChain, nullptr);
}

void Renderer::recreateVulkanSwapChain()
{
	vkDeviceWaitIdle(vkLogicalDevice);
	
	initVulkanSwapChain();
	initVulkanImageViews();
	initVulkanRenderPass();
	initVulkanGraphicsPipeline();
	initVulkanFramebuffers();
	initVulkanCommandBuffers();
}