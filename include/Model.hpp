#pragma once

#include "PCH.hpp"
#include "Vertex.hpp"

class Model 
{
public:
    Model() {}
    //Model(std::string path);
    ~Model() {}

    void load(std::string path);

    void destroy();

    void initVulkanIndexBuffer();
	void initVulkanVertexBuffer();

    const VkBuffer& getVertexBuffer() { return vkVertexBuffer; }
    const VkBuffer& getIndexBuffer() { return vkIndexBuffer; }

    const size_t getVerticesSize() { return vertices.size(); }
    const size_t getIndicesSize() { return indices.size(); }
private:
    std::string modelName;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

    VkBuffer vkVertexBuffer;
	VkDeviceMemory vkVertexBufferMemory;

	VkBuffer vkIndexBuffer;
	VkDeviceMemory vkIndexBufferMemory;
};