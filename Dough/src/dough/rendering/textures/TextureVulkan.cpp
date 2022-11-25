#include "dough/rendering/textures/TextureVulkan.h"

#include "dough/application/Application.h"
#include "dough/rendering/ObjInit.h"

namespace DOH {

	TextureVulkan::TextureVulkan(
		VkDevice logicDevice,
		VkPhysicalDevice physicalDevice,
		const std::string& filePath
	) {
		TextureCreationData textureData = ResourceHandler::loadTexture(filePath.c_str());
		
		mWidth = textureData.Width;
		mHeight = textureData.Height;
		mChannels = textureData.Channels;

		//IMPORTANT:: Textures used in the engine are assumed to have 4 channels when used.
		VkDeviceSize imageSize = textureData.Width * textureData.Height * 4;

		std::shared_ptr<BufferVulkan> imageStagingBuffer = ObjInit::stagedBuffer(
			textureData.Data,
			imageSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);

		ResourceHandler::freeImage(textureData.Data);

		auto& context = Application::get().getRenderer().getContext();
		VkImage image = context.createImage(
			static_cast<uint32_t>(mWidth),
			static_cast<uint32_t>(mHeight),
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
		);
		VkDeviceMemory imageMem = context.createImageMemory(
			image,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		context.transitionStagedImageLayout(
			image,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_ASPECT_COLOR_BIT
		);
		context.copyBufferToImage(
			imageStagingBuffer->getBuffer(),
			image,
			static_cast<uint32_t>(mWidth),
			static_cast<uint32_t>(mHeight)
		);
		context.transitionStagedImageLayout(
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_IMAGE_ASPECT_COLOR_BIT
		);
		VkImageView imageView = context.createImageView(image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
		mSampler = context.createSampler();

		mTextureImage = std::make_unique<ImageVulkan>(image, imageMem, imageView);

		imageStagingBuffer->close(logicDevice);

		mId = ResourceHandler::getNextUniqueTextureId();
		mUsingGpuResource = true;
	}

	TextureVulkan::TextureVulkan(
		VkDevice logicDevice,
		VkPhysicalDevice physicalDevice,
		float r,
		float g,
		float b,
		float a,
		bool rgbaNormalised
	) : mWidth(1),
		mHeight(1),
		mChannels(4)
	{
		if (!rgbaNormalised) {
			r = (r / TextureVulkan::COLOUR_MAX_VALUE);
			g = (g / TextureVulkan::COLOUR_MAX_VALUE);
			b = (b / TextureVulkan::COLOUR_MAX_VALUE);
			a = (a / TextureVulkan::COLOUR_MAX_VALUE);
		}

		//Clamp between 0.0f and 1.0f then turn negative to remove signed-bit before uint32_t cast
		r = -1 * std::min(std::max(r, 0.0f), 1.0f);
		g = -1 * std::min(std::max(g, 0.0f), 1.0f);
		b = -1 * std::min(std::max(b, 0.0f), 1.0f);
		a = -1 * std::min(std::max(a, 0.0f), 1.0f);

		const size_t textureSize = mWidth * mHeight * mChannels;
		const size_t length = mWidth * mHeight;

		//TODO::
		// Some kind of conversion system for other Vulkan formats?

		//Lossy compress 4 uint32_t to VK_FORMAT_R8G8B8A8_SRGB and create array to represent texture
		uint32_t colour = 
			(static_cast<uint32_t>(r) << 24) |
			(static_cast<uint32_t>(g) << 16) |
			(static_cast<uint32_t>(b) << 8) |
			static_cast<uint32_t>(r);
		std::vector<uint32_t> colourData = {};
		colourData.resize(length);
		for (size_t i = 0; i < length; i++) {
			colourData[i] = colour;
		}

		std::shared_ptr<BufferVulkan> imageStagingBuffer = ObjInit::stagedBuffer(
			colourData.data(),
			static_cast<VkDeviceSize>(textureSize),
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);

		auto& context = Application::get().getRenderer().getContext();
		VkImage image = context.createImage(
			static_cast<uint32_t>(mWidth),
			static_cast<uint32_t>(mHeight),
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
		);
		VkDeviceMemory imageMem = context.createImageMemory(image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		context.transitionStagedImageLayout(
			image,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_ASPECT_COLOR_BIT
		);
		context.copyBufferToImage(
			imageStagingBuffer->getBuffer(),
			image,
			static_cast<uint32_t>(mWidth),
			static_cast<uint32_t>(mHeight)
		);
		context.transitionStagedImageLayout(
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_IMAGE_ASPECT_COLOR_BIT
		);
		VkImageView imageView = context.createImageView(image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
		mSampler = context.createSampler();
		mTextureImage = std::make_unique<ImageVulkan>(image, imageMem, imageView);

		imageStagingBuffer->close(logicDevice);

		mId = ResourceHandler::getNextUniqueTextureId();
		mUsingGpuResource = true;
	}

	void TextureVulkan::close(VkDevice logicDevice) {
		vkDestroySampler(logicDevice, mSampler, nullptr);
		mTextureImage->close(logicDevice);
		mUsingGpuResource = false;
	}
}
