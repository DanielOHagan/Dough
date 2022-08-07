#pragma once

#include <vulkan/vulkan_core.h>

namespace DOH {

	class IGPUResourceVulkan {
	public:
		virtual void close(VkDevice logicDevice) = 0;
		virtual bool isUsingGpuResource() const { return mUsingGpuResource; }

		//TODO:: virtual destructor? use for debugging?

	protected:
		IGPUResourceVulkan()
		:	mUsingGpuResource(false)
		{}
		IGPUResourceVulkan(bool usingResource) 
		:	mUsingGpuResource(usingResource)
		{}

		//NOTE:: Some CPU objects use heap allocated objects as well as storing GPU resource handles,
		// therfore, this alone shouldn't be responsible for the "closing" of an entire object.
		//TODO:: Separate "close" into e.g. "removeFromGpu" & "closeEntireObject" so the object can
		// be removed from the GPU without CPU data being removed
		bool mUsingGpuResource;

		//TODO::
		//To be called internally by this object when GPU resources are closed.
		//virtual void onClosed() = 0;

	};
}
