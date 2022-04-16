#pragma once

#include "dough/Maths.h"

namespace DOH {

	class ICamera {

	public:
		ICamera() = default;
		ICamera(const ICamera& copy) = delete;
		ICamera operator=(const ICamera& assignment) = delete;

		virtual void updateProjectionViewMatrix() = 0;

		inline virtual glm::mat4x4& getViewMatrix() = 0;
		inline virtual glm::mat4x4& getProjectionMatrix() = 0;
		inline virtual glm::mat4x4& getProjectionViewMatrix() = 0;
	};
}
