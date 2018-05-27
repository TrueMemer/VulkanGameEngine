#include "Model.hpp"
#include "Engine.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

void Model::load(std::string path) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, path.c_str())) 
	{
        LOG_FATAL(err);
    }

    modelName = path;

	std::unordered_map<Vertex, uint32_t> uniqueVertices = {};

	for (const auto& shape : shapes) 
	{
		for (const auto& index : shape.mesh.indices) 
		{
			Vertex vertex = {};

			vertex.position = 
			{
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			vertex.texCoord = 
			{
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			};

			vertex.color = {1.0f, 1.0f, 1.0f};

			if (uniqueVertices.count(vertex) == 0) 
			{
				uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
				vertices.push_back(vertex);
			}

			indices.push_back(uniqueVertices[vertex]);
		}
	}

    initVulkanVertexBuffer();
    initVulkanIndexBuffer();
	
}


void Model::initVulkanVertexBuffer()
{
	LOG_INFO("<" << modelName << "> Creating vertex buffer");
	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

	Engine::renderer->createVulkanBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, Engine::renderer->vkStagingBuffer, Engine::renderer->vkStagingBufferMemory);

	void* data;
	vkMapMemory(Engine::renderer->vkLogicalDevice, Engine::renderer->vkStagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(Engine::renderer->vkLogicalDevice, Engine::renderer->vkStagingBufferMemory);

	Engine::renderer->createVulkanBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vkVertexBuffer, vkVertexBufferMemory);

	Engine::renderer->copyVulkanBuffer(Engine::renderer->vkStagingBuffer, vkVertexBuffer, bufferSize);

	vkDestroyBuffer(Engine::renderer->vkLogicalDevice, Engine::renderer->vkStagingBuffer, 0);
	vkFreeMemory(Engine::renderer->vkLogicalDevice, Engine::renderer->vkStagingBufferMemory, 0);

}

void Model::initVulkanIndexBuffer()
{
	LOG_INFO("<" << modelName << "> Creating index buffer");
	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

	Engine::renderer->createVulkanBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, Engine::renderer->vkStagingBuffer, Engine::renderer->vkStagingBufferMemory);

	void* data;
	vkMapMemory(Engine::renderer->vkLogicalDevice, Engine::renderer->vkStagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices.data(), (size_t)bufferSize);
	vkUnmapMemory(Engine::renderer->vkLogicalDevice, Engine::renderer->vkStagingBufferMemory);

	Engine::renderer->createVulkanBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vkIndexBuffer, vkIndexBufferMemory);

	Engine::renderer->copyVulkanBuffer(Engine::renderer->vkStagingBuffer, vkIndexBuffer, bufferSize);

	vkDestroyBuffer(Engine::renderer->vkLogicalDevice, Engine::renderer->vkStagingBuffer, nullptr);
	vkFreeMemory(Engine::renderer->vkLogicalDevice, Engine::renderer->vkStagingBufferMemory, nullptr);
}

void Model::destroy()
{
    vkDestroyBuffer(Engine::renderer->vkLogicalDevice, vkVertexBuffer, nullptr);
	vkFreeMemory(Engine::renderer->vkLogicalDevice, vkVertexBufferMemory, nullptr);

    vkDestroyBuffer(Engine::renderer->vkLogicalDevice, vkIndexBuffer, nullptr);
	vkFreeMemory(Engine::renderer->vkLogicalDevice, vkIndexBufferMemory, nullptr);
}