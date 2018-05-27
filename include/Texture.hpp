#pragma once

#include "PCH.hpp"
#include "Image.hpp"

class Texture
{
public:
    Texture() {}
    ~Texture() {}

    VkImage getVkImage() { return vkImage; }
    VkImageView getVkImageView() { return vkImageView; }
    VkDeviceMemory getVkImageMemory() { return vkMemory; }

    int maxMipLevel;

    void loadFile(std::string path, bool genMipmaps = true);
    void loadImage(Image *image, bool genMipmaps = true);
    void destroy();

private:
    int width, height;
	VkImage vkImage;
	VkDeviceMemory vkMemory;
    VkImageView vkImageView;
};