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
		context.transitionImageLayout(
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
		context.transitionImageLayout(
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
		bool colourRgbaNormalised
	) : mWidth(1),
		mHeight(1),
		mChannels(4)
	{
		if (colourRgbaNormalised) {
			r *= 255;
			g *= 255;
			b *= 255;
			a *= 255;
		}

		std::array<float, 4> colourData = { a, r, g, b };
		std::shared_ptr<BufferVulkan> imageStagingBuffer = ObjInit::stagedBuffer(
			colourData.data(),
			sizeof(colourData),
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
		context.transitionImageLayout(
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
		context.transitionImageLayout(
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
