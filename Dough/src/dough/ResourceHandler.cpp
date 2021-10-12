#include "dough/ResourceHandler.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace DOH {

	ResourceHandler ResourceHandler::INSTANCE = ResourceHandler();

	TextureCreationData ResourceHandler::loadTexture(const char* filepath) {
		return ResourceHandler::INSTANCE.loadTextureImpl(filepath);
	}

	void ResourceHandler::freeImage(void* imageData) {
		ResourceHandler::INSTANCE.freeImageImpl(imageData);
	}

	std::vector<char> ResourceHandler::readFile(const std::string& filename) {
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		TRY(!file.is_open(), "Failed to open file.");

		size_t fileSize = (size_t) file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	TextureCreationData ResourceHandler::loadTextureImpl(const char* filepath) {
		TextureCreationData textureData{};

		stbi_uc* pixels = stbi_load(filepath, &textureData.width, &textureData.height, &textureData.channels, STBI_rgb_alpha);

		TRY(pixels == nullptr || textureData.width < 0 || textureData.height < 0 || textureData.channels < 0, "Failed to load image data");

		textureData.data = static_cast<void*>(pixels);
		return textureData;
	}

	void ResourceHandler::freeImageImpl(void* imageData) {
		stbi_image_free(imageData);
	}
}
