#pragma once

#include "dough/Utils.h"
#include "dough/rendering/IGPUResourceVulkan.h"
#include "dough/rendering/ImageVulkan.h"

namespace DOH {

	class TextureVulkan : public IGPUResourceVulkan {

	protected:
		std::string mName;
		std::unique_ptr<ImageVulkan> mTextureImage;
		VkSampler mSampler;

		uint32_t mId;
		int mWidth;
		int mHeight;
		int mChannels;

	public:
		//TODO:: Keep this here or place somewhere else in another class?
		static constexpr float COLOUR_MAX_VALUE = 255.0f;

		TextureVulkan(const TextureVulkan& copy) = delete;
		TextureVulkan operator=(const TextureVulkan& assignment) = delete;

		virtual ~TextureVulkan() override;
		virtual void close(VkDevice logicDevice) override;

		/**
		* Read a texture from filePath and upload onto GPU.
		* 
		* @param logicDevice The logic device needed to create the resource on GPU.
		* @param physicalDevice The physical device needed to create the resource on GPU.
		* @param filePath The file path of the texture. This is used as the texture's name.
		*/
		TextureVulkan(
			VkDevice logicDevice,
			VkPhysicalDevice physicalDevice,
			const std::string& filePath
		);

		//TODO:: Custom width & height with limits.
		//	bool for use of a staging buffer?
		/**
		* Create a single colour texture from the given RGBA values and upload onto GPU.
		* 
		* @param logicDevice The logic device needed to create the resource on GPU.
		* @param physicalDevice The physical device needed to create the resource on GPU.
		* @param r The value for the Red channel.
		* @param g The value for the Green channel.
		* @param b The value for the Blue channel.
		* @param a The value for the Alpha channel.
		* @param rgbaNormalised Whether or the given rgba values have been normalised.
		* @param name A name for the texture.
		*/
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

	protected:
		//Allow for loading outside of contructor
		TextureVulkan();

		void load(void* data, VkDeviceSize size);
	};
}
