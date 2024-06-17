#include "dough/rendering/ImageVulkan.h"

namespace DOH {

	ImageVulkan::ImageVulkan(VkImage image, VkDeviceMemory imageMemory, VkImageView imageView)
	:	IGPUResourceVulkan((image != VK_NULL_HANDLE) || (imageMemory != VK_NULL_HANDLE) || (imageView != VK_NULL_HANDLE)),
		mImage(image),
		mImageMemory(imageMemory),
		mImageView(imageView)
	{}

	ImageVulkan::~ImageVulkan() {
		if (isUsingGpuResource()) {
			LOG_ERR(
				"Image Vulkan GPU resource NOT released before destructor was called." <<
				"Image: " << mImage << " ImageView: " << mImageView << " Memory: " << mImageMemory
			);

			//NOTE:: This is to stop the IGPUResource::~IGPUReource from logging a misleading error message.
			mUsingGpuResource = false;
		}
	}

	void ImageVulkan::close(VkDevice logicDevice) {
		vkDestroyImageView(logicDevice, mImageView, nullptr);
		vkDestroyImage(logicDevice, mImage, nullptr);
		vkFreeMemory(logicDevice, mImageMemory, nullptr);

		mUsingGpuResource = false;
	}
}
