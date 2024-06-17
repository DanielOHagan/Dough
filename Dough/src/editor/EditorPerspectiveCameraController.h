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

		inline glm::vec3 getPosition() const { return mPosition; }
		inline void setPosition(glm::vec3 pos) { mPosition = pos; }
		inline glm::vec3 getDirection() const { return mDirectionFacing; }
		inline void setDirection(glm::vec3 direction) { mDirectionFacing = direction; }

	private:
		void handleInput(float delta);
		void updateViewMatrices();
	};
}
