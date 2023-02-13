#pragma once

#include "dough/Utils.h"
#include "dough/rendering/IGPUResourceVulkan.h"
#include "dough/rendering/ImageVulkan.h"

namespace DOH {

	class TextureVulkan : public IGPUResourceVulkan {

	protected:
		const std::string mName;
		std::unique_ptr<ImageVulkan> mTextureImage;
		VkSampler mSampler;

		uint32_t mId;
		int mWidth;
		int mHeight;
		int mChannels;

	public:
		//TODO:: Keep this here or place somewhere else in another class?
		static constexpr float COLOUR_MAX_VALUE = 255.0f;

		TextureVulkan() = delete;
		TextureVulkan(const TextureVulkan& copy) = delete;
		TextureVulkan operator=(const TextureVulkan& assignment) = delete;

		TextureVulkan(
			VkDevice logicDevice,
			VkPhysicalDevice physicalDevice,
			const std::string& filePath
		);

		//TODO:: Custom width & height with limits.
		//	bool for use of a staging buffer?
		TextureVulkan(
			VkDevice logicDevice,
			VkPhysicalDevice physicalDevice,
			float r,
			float g,
			float b,
			float a,
			bool rgbaNormalised,
			const char* name = "Un-named Texture"
		);

		virtual void close(VkDevice logicDevice) override;

		inline const std::string& getName() const { return mName; }
		inline uint32_t getId() const { return mId; }
		inline int getWidth() const { return mWidth; }
		inline int getHeight() const { return mHeight; }
		inline int getChannels() const { return mChannels; }
		inline int getSize() const { return mWidth * mHeight * mChannels; }
		inline VkImage getImage() const { return mTextureImage->get(); }
		inline VkDeviceMemory getMemory() const { return mTextureImage->getMemory(); }
		inline VkImageView getImageView() const { return mTextureImage->getImageView(); }
		inline VkSampler getSampler() const { return mSampler; }
	};
}
