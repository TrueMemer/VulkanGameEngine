#pragma once

#include "PCH.hpp"

struct Vertex 
{
	glm::vec3 position;
	glm::vec3 color;
	glm::vec2 texCoord;

	static VkVertexInputBindingDescription getBindingDescription() 
	{
		VkVertexInputBindingDescription desc = {};
		desc.binding = 0;
		desc.stride = sizeof(Vertex);
		desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return desc;
	}

	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
	{

		std::array<VkVertexInputAttributeDescription, 3> desc;

		desc[0].binding = 0;
		desc[0].location = 0;
		desc[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		desc[0].offset = offsetof(Vertex, position);

		desc[1].binding = 0;
		desc[1].location = 1;
		desc[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		desc[1].offset = offsetof(Vertex, color);

		desc[2].binding = 0;
        desc[2].location = 2;
        desc[2].format = VK_FORMAT_R32G32_SFLOAT;
        desc[2].offset = offsetof(Vertex, texCoord);

		return desc;

	}

	bool operator==(const Vertex& other) const 
	{
		return position == other.position && color == other.color && texCoord == other.texCoord;
	}
};


namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.position) ^
                   (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
                   (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}