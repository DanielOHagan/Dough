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
			const std::string& filePath
		);

		TextureVulkan(
			VkDevice logicDevice,
			VkPhysicalDevice physicalDevice,
			float r,
			float g,
			float b,
			float a,
			bool colourRgbaNormalised = false
		);

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
	};
}
