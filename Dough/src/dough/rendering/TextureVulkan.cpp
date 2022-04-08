#include "dough/rendering/TextureVulkan.h"

#include "dough/application/Application.h"
#include "dough/rendering/ObjInit.h"

namespace DOH {

	TextureVulkan::TextureVulkan(
		VkDevice logicDevice,
		VkPhysicalDevice physicalDevice,
		const std::string& filePath
	) {
		TextureCreationData textureData = ResourceHandler::loadTexture(filePath.c_str());
		
		mWidth = textureData.width;
		mHeight = textureData.height;
		mChannels = textureData.channels;

		VkDeviceSize imageSize = textureData.width * textureData.height * 4;

		std::shared_ptr<BufferVulkan> imageStagingBuffer = ObjInit::stagedBuffer(
			textureData.data,
			imageSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);

		ResourceHandler::freeImage(textureData.data);

		mImage = RenderingContextVulkan::createImage(
			logicDevice,
			physicalDevice,
			static_cast<uint32_t>(mWidth),
			static_cast<uint32_t>(mHeight),
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
		);
		mImageMemory = RenderingContextVulkan::createImageMemory(logicDevice, physicalDevice, mImage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		Application::get().getRenderer().getContext().transitionImageLayout(
			mImage,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_ASPECT_COLOR_BIT
		);
		Application::get().getRenderer().getContext().copyBufferToImage(
			imageStagingBuffer->getBuffer(),
			mImage,
			static_cast<uint32_t>(mWidth),
			static_cast<uint32_t>(mHeight)
		);
		Application::get().getRenderer().getContext().transitionImageLayout(
			mImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_IMAGE_ASPECT_COLOR_BIT
		);
		createImageView();
		createSampler();

		imageStagingBuffer->close(logicDevice);

		mId = ResourceHandler::getNextUniqueTextureId();
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
		mImage = RenderingContextVulkan::createImage(
			logicDevice,
			physicalDevice,
			static_cast<uint32_t>(mWidth),
			static_cast<uint32_t>(mHeight),
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
		);
		mImageMemory = RenderingContextVulkan::createImageMemory(logicDevice, physicalDevice, mImage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		Application::get().getRenderer().getContext().transitionImageLayout(
			mImage,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_ASPECT_COLOR_BIT
		);
		Application::get().getRenderer().getContext().copyBufferToImage(
			imageStagingBuffer->getBuffer(),
			mImage,
			static_cast<uint32_t>(mWidth),
			static_cast<uint32_t>(mHeight)
		);
		Application::get().getRenderer().getContext().transitionImageLayout(
			mImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_IMAGE_ASPECT_COLOR_BIT
		);
		createImageView();
		createSampler();

		imageStagingBuffer->close(logicDevice);

		mId = ResourceHandler::getNextUniqueTextureId();
	}

	void TextureVulkan::close(VkDevice logicDevice) {
		vkDestroySampler(logicDevice, mSampler, nullptr);
		vkDestroyImageView(logicDevice, mImageView, nullptr);
		vkDestroyImage(logicDevice, mImage, nullptr);
		vkFreeMemory(logicDevice, mImageMemory, nullptr);
	}

	void TextureVulkan::createImageView() {
		mImageView = Application::get().getRenderer().getContext().createImageView(mImage, VK_FORMAT_R8G8B8A8_SRGB);
	}

	void TextureVulkan::createSampler() {
		mSampler = Application::get().getRenderer().getContext().createSampler();
	}
}
