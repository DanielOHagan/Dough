#include "dough/rendering/ImageVulkan.h"

namespace DOH {

	ImageVulkan::ImageVulkan(VkImage image, VkDeviceMemory imageMemory, VkImageView imageView)
	:	IGPUResourceVulkan(true),
		mImage(image),
		mImageMemory(imageMemory),
		mImageView(imageView)
	{}

	void ImageVulkan::close(VkDevice logicDevice) {
		vkDestroyImageView(logicDevice, mImageView, nullptr);
		vkDestroyImage(logicDevice, mImage, nullptr);
		vkFreeMemory(logicDevice, mImageMemory, nullptr);

		mUsingGpuResource = false;
	}
}
