#pragma once

#include "dough/scene/camera/ICameraController.h"
#include "dough/scene/camera/PerspectiveCamera.h"
#include "dough/Core.h"
#include "dough/input/AInputLayer.h"

using namespace DOH;

namespace DOH::EDITOR {

	class EditorPerspectiveCameraController : public ICameraController {
	private:
		std::unique_ptr<PerspectiveCamera> mCamera;
		glm::vec3 mPosition;
		glm::vec3 mDirectionFacing;
		glm::vec2 mCursorLastPosUpdate;
		float mAspectRatio;
		float mFov;
		float mTranslationSpeed;
		bool mClickAndDragEnabled;
		bool mClickAndDragActive;

		//NOTE:: using a shared pointer so the input layer isn't invalidated if removed from Input::mInputLayers
		std::shared_ptr<AInputLayer> mInputLayer;

	public:
		EditorPerspectiveCameraController(std::shared_ptr<AInputLayer> inputLayer, float aspectRatio, float fov = 60.0f);
		EditorPerspectiveCameraController(const EditorPerspectiveCameraController& copy) = delete;
		EditorPerspectiveCameraController operator=(const EditorPerspectiveCameraController& assignment) = delete;

		inline void setInputLayer(std::shared_ptr<AInputLayer> inputLayer) { mInputLayer = inputLayer; }
		inline AInputLayer& getInputLayer() const { return *mInputLayer; }

		virtual void onUpdate(float delta) override;
		virtual void onViewportResize(float aspectRatio) override;
		inline virtual ICamera& getCamera() const override { return *mCamera; }

		inline const glm::vec3& getPosition() const { return mPosition; }
		inline void setPosition(glm::vec3& pos) { mPosition = pos; }
		inline void setPositionXYZ(float x, float y, float z) { mPosition.x = x; mPosition.y = y; mPosition.z = z;}
		inline const glm::vec3& getDirection() const { return mDirectionFacing; }
		inline void setDirection(glm::vec3& direction) { mDirectionFacing = direction; }
		inline void setDirectionXYZ(float x, float y, float z) { mDirectionFacing.x = x; mDirectionFacing.y = y; mDirectionFacing.z = z; }
		inline const glm::vec2& getCursorLastPos() const { return mCursorLastPosUpdate; }
		inline void setCursorLastPos(float x, float y) { mCursorLastPosUpdate.x = x; mCursorLastPosUpdate.y = y; }
		inline float getAspectRatio() const { return mAspectRatio; }
		inline void setAspectRatio(float aspectRatio) { mAspectRatio = aspectRatio; }
		inline float getFov() const { return mFov; }
		inline void setFov(float fov) { mFov = fov; }
		inline float getTranslationSpeed() const { return mTranslationSpeed; }
		inline void setTranslationSpeed(float translationSpeed) { mTranslationSpeed = translationSpeed; }
		inline bool isClickAndDragEnabled() const { return mClickAndDragEnabled; }
		inline void setClickAndDragEnabled(bool enabled) { mClickAndDragEnabled = enabled; if (!enabled) mClickAndDragActive = false; }
		inline bool isClickAndDragActive() const { return mClickAndDragActive; }

	private:
		void handleInput(float delta);
		void updateViewMatrices();
	};
}
