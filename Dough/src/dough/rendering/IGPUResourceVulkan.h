#pragma once

#include <vulkan/vulkan_core.h>

#include "dough/Logging.h"

namespace DOH {

	class RenderingContextVulkan;

	class IGPUResourceVulkan {
	protected:
		IGPUResourceVulkan()
		:	mUsingGpuResource(false)
		{}
		IGPUResourceVulkan(bool usingResource) 
		:	mUsingGpuResource(usingResource)
		{}
		virtual ~IGPUResourceVulkan() {
			if (mUsingGpuResource) {
				LOG_ERR("GPU resource of UNKNOWN type not closed");
			}
		}

		//NOTE:: Some CPU objects use heap allocated objects as well as storing GPU resource handles,
		// therfore, this alone shouldn't be responsible for the "closing" of an entire object.
		//TODO:: Separate "close" into e.g. "removeFromGpu" & "closeEntireObject" so the object can
		// be removed from the GPU without CPU data being removed
		bool mUsingGpuResource;

		//TODO::
		//To be called internally by this object when GPU resources are closed.
		//virtual void onClosed() = 0;

	public:
		virtual void close(VkDevice logicDevice) = 0;
		virtual bool isUsingGpuResource() const { return mUsingGpuResource; }
	};

	//A class interface for classes that store, own and areresponsible for the lifetime of IGPUResource(s) but is not
	//an IGPUResource itself. This is so a class doesn't have to derive from IGPUResource but the class can still
	//interact with the IGUResource API and the deletion queue (addGpuResourceToClose()) function.
	class IGPUResourceOwnerVulkan {
	protected:
		IGPUResourceOwnerVulkan() = default;

	public:
		virtual void addOwnedResourcesToClose(RenderingContextVulkan& context) = 0;
	};
}
