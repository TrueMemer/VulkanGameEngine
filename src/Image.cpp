#include "Image.hpp"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

void Image::load(std::string path)
{
	int bpp;
	unsigned char* loadedData = stbi_load(path.c_str(), &width, &height, &bpp, 4);
	if (!loadedData) {
		LOG_WARN("Failed to load image: " << path);
		return;
	}
	mipLevels = static_cast<U32>(std::floor(std::log2(std::max(width, height)))) + 1;
	data.resize(width*height);
	memcpy(&data[0], loadedData, width * height * sizeof(Pixel));
	stbi_image_free(loadedData);
}

void Image::save(std::string path)
{
	int result = stbi_write_png(path.c_str(), width, height, 4, &data[0], 0);
}