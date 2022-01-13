#pragma once

#include <vulkan/vulkan_core.h>

namespace DOH {

	class IGPUResourceVulkan {

	public:
		virtual void close(VkDevice logicDevice) = 0;

	protected:
		//TODO::
		//To be called internally by this object when GPU resources are closed.
		//virtual void onClosed() = 0;

	};
}
