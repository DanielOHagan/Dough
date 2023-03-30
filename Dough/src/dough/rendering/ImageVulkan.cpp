#include "dough/rendering/ImageVulkan.h"

namespace DOH {

	ImageVulkan::ImageVulkan(VkImage image, VkDeviceMemory imageMemory, VkImageView imageView)
	:	IGPUResourceVulkan(true),
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
		}
	}

	void ImageVulkan::close(VkDevice logicDevice) {
		vkDestroyImageView(logicDevice, mImageView, nullptr);
		vkDestroyImage(logicDevice, mImage, nullptr);
		vkFreeMemory(logicDevice, mImageMemory, nullptr);

		mUsingGpuResource = false;
	}
}
