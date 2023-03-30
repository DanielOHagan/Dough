#pragma once

#include "dough/rendering/IGPUResourceVulkan.h"

namespace DOH {

	class ImageVulkan : public IGPUResourceVulkan {
	private:
		VkImage mImage;
		VkDeviceMemory mImageMemory;
		VkImageView mImageView;

	public:
		//TODO:: Currently just stores image handles, in future have the class be able of creating
		//	images with different settings.
		//	Incorporate or remove the use of RenderingContextVulkan::createImage()
		ImageVulkan(VkImage image, VkDeviceMemory imageMemory, VkImageView imageView);

		virtual ~ImageVulkan() override;
		virtual void close(VkDevice logicDevice) override;

		inline VkImage get() const { return mImage; }
		inline VkDeviceMemory getMemory() const { return mImageMemory; }
		inline VkImageView getImageView() const { return mImageView; }

	};
}
