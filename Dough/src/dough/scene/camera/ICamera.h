#pragma once

#include "dough/Maths.h"

#include <memory>

namespace DOH {

	class CameraGpuData;

	class ICamera {
	public:
		ICamera() = default;
		ICamera(const ICamera& copy) = delete;
		ICamera operator=(const ICamera& assignment) = delete;

		virtual void updateProjectionViewMatrix() = 0;

		inline virtual glm::mat4x4& getViewMatrix() = 0;
		inline virtual glm::mat4x4& getProjectionMatrix() = 0;
		inline virtual glm::mat4x4& getProjectionViewMatrix() = 0;

		inline virtual std::shared_ptr<CameraGpuData> getGpuData() { return nullptr; };
		inline virtual void setGpuData(std::shared_ptr<CameraGpuData> gpuData) = 0;
		inline bool hasGpuData() { return getGpuData() != nullptr; }
	};
}
