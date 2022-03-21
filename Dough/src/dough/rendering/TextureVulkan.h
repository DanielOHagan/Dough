#pragma once

#include "dough/Utils.h"
#include "dough/rendering/IGPUResourceVulkan.h"

namespace DOH {

	class TextureVulkan : public IGPUResourceVulkan {

	private:
		int mWidth;
		int mHeight;
		int mChannels;

		VkImage mImage;
		VkDeviceMemory mImageMemory;
		VkImageView mImageView;
		VkSampler mSampler;

	public:
		TextureVulkan() = delete;
		TextureVulkan(const TextureVulkan& copy) = delete;
		TextureVulkan operator=(const TextureVulkan& assignment) = delete;

		TextureVulkan(
			VkDevice logicDevice,
			VkPhysicalDevice physicalDevice,
			VkCommandPool cmdPool,
			VkQueue graphicsQueue,
			std::string& filePath
		);

		//TODO::
		//TextureVulkan(
		//	VkDevice logicDevice,
		//	VkPhysicalDevice physicalDevice,
		//	VkCommandPool cmdPool,
		//	VkQueue graphicsQueue,
		//	glm::vec4& colour
		//);

		virtual void close(VkDevice logicDevice) override;

		inline int getWidth() const { return mWidth; }
		inline int getHeight() const { return mHeight; }
		inline int getChannels() const { return mChannels; }
		inline int getSize() const { return mWidth * mHeight * 4; }
		inline VkImage getImage() const { return mImage; }
		inline VkImageView getImageView() const { return mImageView; }
		inline VkSampler getSampler() const { return mSampler; }

	private:
		void createImageView();
		void createSampler();

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
	public:
		//static void transitionImageLayout(
		//	VkDevice logicDevice,
		//	VkImage image,
		//	VkFormat format,
		//	VkImageLayout oldLayout,
		//	VkImageLayout newLayout
		//);
	};
}
