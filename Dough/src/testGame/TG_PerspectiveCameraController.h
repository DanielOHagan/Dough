#pragma once

#include "dough/scene/camera/ICameraController.h"
#include "dough/scene/camera/PerspectiveCamera.h"
#include "dough/Core.h"

using namespace DOH;

namespace TG {

	class TG_PerspectiveCameraController : public ICameraController {

	private:
		std::unique_ptr<PerspectiveCamera> mCamera;
		glm::vec3 mPosition;
		glm::vec3 mDirectionFacing;
		glm::vec2 mCursorLastPosUpdate;
		bool mClickAndDragActive;
		float mAspectRatio;
		float mFov;

	public:
		TG_PerspectiveCameraController(float aspectRatio, float fov = 60.0f);
		TG_PerspectiveCameraController(const TG_PerspectiveCameraController& copy) = delete;
		TG_PerspectiveCameraController operator=(const TG_PerspectiveCameraController& assignment) = delete;

		virtual void onUpdate(float delta) override;
		virtual void onViewportResize(float aspectRatio) override;
		inline virtual ICamera& getCamera() const override { return *mCamera; }

		inline glm::vec3 getPosition() const { return mPosition; }
		inline void setPosition(glm::vec3 pos) { mPosition = pos; }
		inline glm::vec3 getDirection() const { return mDirectionFacing; }
		inline void setDirection(glm::vec3 direction) { mDirectionFacing = direction; }

	private:
		void handleInput(float delta);
		void updateViewMatrices();
	};
}
