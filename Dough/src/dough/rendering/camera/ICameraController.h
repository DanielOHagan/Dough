#pragma once

#include "dough/rendering/camera/ICamera.h"

namespace DOH {
	
	class ICameraController {

	public:
		ICameraController() = default;
		ICameraController(const ICameraController& copy) = delete;
		ICameraController operator=(const ICameraController& assignment) = delete;

		virtual void onUpdate(float delta) = 0;
		virtual void onViewportResize(float aspectRatio) = 0;
		inline virtual ICamera& getCamera() const = 0;

	};
}
