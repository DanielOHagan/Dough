#pragma once

#include <vulkan/vulkan_core.h>

namespace DOH {

	class IGPUResourceVulkan {
	public:
		virtual void close(VkDevice logicDevice) = 0;
		virtual bool isUsingGpuResource() const { return mUsingGpuResource; }

	protected:
		IGPUResourceVulkan()
		:	mUsingGpuResource(false)
		{}

		//NOTE:: Some CPU objects use heap allocated objects as well as storing GPU resource handles,
		// therfore, this alone shouldn't be responsible for the "closing" of an entire object.
		//TODO:: Separate "close" into e.g. "removeFromGpu" & "closeEntireObject" so the object can
		// be removed from the GPU without 
		bool mUsingGpuResource;

		//TODO::
		//To be called internally by this object when GPU resources are closed.
		//virtual void onClosed() = 0;

	};
}
