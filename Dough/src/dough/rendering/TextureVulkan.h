#pragma once

#include "dough/Utils.h"

#include <vulkan/vulkan_core.h>

namespace DOH {

	class TextureVulkan {

	private:
		int mWidth;
		int mHeight;
		int mChannels;

		VkImage mImage;
		VkDeviceMemory mImageMemory;
		VkImageView mImageView;
		VkSampler mSampler;

	public:

		void close(VkDevice logicDevice);

		inline int getWidth() const { return mWidth; }
		inline int getHeight() const { return mHeight; }
		inline int getChannels() const { return mChannels; }
		inline int getSize() const { return mWidth * mHeight * 4; }
		inline VkImage getImage() const { return mImage; }
		inline VkImageView getImageView() const { return mImageView; }
		inline VkSampler getSampler() const { return mSampler; }

	private:
		TextureVulkan(
			int width,
			int height,
			int channels,
			VkImage image,
			VkDeviceMemory imageMemory,
			VkImageView imageView
		);

		void createImageView();
		void createSampler();

	public:
		static TextureVulkan createTexture(
			VkDevice logicDevice,
			VkPhysicalDevice physicalDevice,
			VkCommandPool cmdPool,
			VkQueue graphicsQueue,
			std::string& filePath
		);

		static void transitionImageLayout(
			VkDevice logicDevice,
			VkImage image,
			VkFormat format,
			VkImageLayout oldLayout,
			VkImageLayout newLayout
		);

	private:
		static void createImage(
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
		);

	};
}
