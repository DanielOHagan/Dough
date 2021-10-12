#include "dough/rendering/TextureVulkan.h"

#include "dough/ResourceHandler.h"
#include "dough/rendering/buffer/BufferVulkan.h"
#include "dough/application/Application.h"

namespace DOH {

	TextureVulkan::TextureVulkan(
		int width,
		int height,
		int channels,
		VkImage image,
		VkDeviceMemory imageMemory,
		VkImageView imageView
	) : mWidth(width),
		mHeight(height),
		mChannels(channels),
		mImage(image),
		mImageMemory(imageMemory),
		mImageView(imageView),
		mSampler(VK_NULL_HANDLE)
	{
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

	TextureVulkan TextureVulkan::create(
		VkDevice logicDevice,
		VkPhysicalDevice physicalDevice,
		VkCommandPool cmdPool,
		VkQueue graphicsQueue,
		std::string& filePath
	) {
		TextureCreationData textureData = ResourceHandler::loadTexture(filePath.c_str());

		VkDeviceSize imageSize = textureData.width * textureData.height * 4;

		BufferVulkan imageStagingBuffer = BufferVulkan::createStagedBuffer(
			logicDevice,
			physicalDevice,
			cmdPool,
			graphicsQueue,
			textureData.data,
			imageSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);

		ResourceHandler::freeImage(textureData.data);

		TextureVulkan texture = TextureVulkan(
			textureData.width,
			textureData.height,
			textureData.channels,
			VK_NULL_HANDLE,
			VK_NULL_HANDLE,
			VK_NULL_HANDLE
		);

		TextureVulkan::createImage(
			logicDevice,
			physicalDevice,
			static_cast<uint32_t>(textureData.width),
			static_cast<uint32_t>(textureData.height),
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			texture.mImage,
			texture.mImageMemory
		);

		TextureVulkan::transitionImageLayout(
			logicDevice,
			texture.mImage,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
		);
		Application::get().getRenderer().getContext().copyBufferToImage(
			imageStagingBuffer.getBuffer(),
			texture.mImage,
			static_cast<uint32_t>(textureData.width),
			static_cast<uint32_t>(textureData.height)
		);
		TextureVulkan::transitionImageLayout(
			logicDevice,
			texture.mImage,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		);
		texture.createImageView();
		texture.createSampler();

		imageStagingBuffer.close(logicDevice);

		return texture;
	}

	void TextureVulkan::createImage(
		VkDevice logicDevice,
		VkPhysicalDevice physicalDevice,
		uint32_t width,
		uint32_t height,
		VkFormat format,
		VkImageTiling tiling,
		VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkImage& image,
		VkDeviceMemory& imageMemory
	) {
		VkImageCreateInfo imageCreateInfo{};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.extent.width = width;
		imageCreateInfo.extent.height = height;
		imageCreateInfo.extent.depth = 1;
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.format = format;
		imageCreateInfo.tiling = tiling;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCreateInfo.usage = usage;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.flags = 0; //Optional

		TRY(
			vkCreateImage(logicDevice, &imageCreateInfo, nullptr, &image) != VK_SUCCESS,
			"Failed to create image"
		);

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(logicDevice, image, &memRequirements);

		VkMemoryAllocateInfo allocation{};
		allocation.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocation.allocationSize = memRequirements.size;
		allocation.memoryTypeIndex = RendererVulkan::findPhysicalDeviceMemoryType(
			physicalDevice,
			memRequirements.memoryTypeBits,
			properties
		);

		TRY(
			vkAllocateMemory(logicDevice, &allocation, nullptr, &imageMemory) != VK_SUCCESS,
			"Failed to allocate image memory"
		);

		vkBindImageMemory(logicDevice, image, imageMemory, 0);
	}

	void TextureVulkan::transitionImageLayout(
		VkDevice logicDevice,
		VkImage image,
		VkFormat format,
		VkImageLayout oldLayout,
		VkImageLayout newLayout
	) {
		VkCommandBuffer cmdBuffer = Application::get().getRenderer().getContext().beginSingleTimeCommands();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags srcStage;
		VkPipelineStageFlags dstStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		} else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		} else {
			THROW("Layout transition not supported");
		}

		vkCmdPipelineBarrier(
			cmdBuffer,
			srcStage,
			dstStage,
			0,
			0,
			nullptr,
			0,
			nullptr,
			1,
			&barrier
		);

		Application::get().getRenderer().getContext().endSingleTimeCommands(cmdBuffer);
	}
}
