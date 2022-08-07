#pragma once

#include "dough/Utils.h"
#include "dough/rendering/IGPUResourceVulkan.h"
#include "dough/rendering/ImageVulkan.h"

namespace DOH {

	class TextureVulkan : public IGPUResourceVulkan {

	private:
		uint32_t mId;
		int mWidth;
		int mHeight;
		int mChannels;

		std::unique_ptr<ImageVulkan> mTextureImage;
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

		inline uint32_t getId() const { return mId; }
		inline int getWidth() const { return mWidth; }
		inline int getHeight() const { return mHeight; }
		inline int getChannels() const { return mChannels; }
		inline int getSize() const { return mWidth * mHeight * 4; }
		inline VkImage getImage() const { return mTextureImage->get(); }
		inline VkDeviceMemory getMemory() const { return mTextureImage->getMemory(); }
		inline VkImageView getImageView() const { return mTextureImage->getImageView(); }
		inline VkSampler getSampler() const { return mSampler; }
	};
}
