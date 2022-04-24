#include "testGame/TG_PerspectiveCameraController.h"

#include "dough/input/Input.h"
#include "dough/input/InputCodes.h"

using namespace DOH;

namespace TG {

	TG_PerspectiveCameraController::TG_PerspectiveCameraController(float aspectRatio, float fov)
	:	mCamera(std::make_unique<PerspectiveCamera>(aspectRatio, fov)),
		mPosition(0.0f, 0.0f, 1.0f),
		mDirectionFacing(0.0f, 0.0f, 0.0f),
		mCursorLastPosUpdate(0.0f, 0.0f),
		mClickAndDragActive(false),
		mAspectRatio(aspectRatio),
		mFov(fov)
	{}

	void TG_PerspectiveCameraController::onUpdate(float delta) {
		handleInput(delta);
		updateViewMatrices();
	}

	void TG_PerspectiveCameraController::onViewportResize(float aspectRatio) {
		mAspectRatio = aspectRatio;
		mCamera->setProjection(mFov, mAspectRatio);
		updateViewMatrices();
	}

	void TG_PerspectiveCameraController::handleInput(float delta) {
		if (Input::isKeyPressed(DOH_KEY_A)) {
			mPosition.x += -delta;
		}
		if (Input::isKeyPressed(DOH_KEY_D)) {
			mPosition.x += delta;
		}
		if (Input::isKeyPressed(DOH_KEY_W)) {
			mPosition.z += -delta;
		}
		if (Input::isKeyPressed(DOH_KEY_S)) {
			mPosition.z += delta;
		}
		if (Input::isKeyPressed(DOH_KEY_SPACE)) {
			mPosition.y += delta;
		}
		if (Input::isKeyPressed(DOH_KEY_C)) {
			mPosition.y += -delta;
		}
		if (Input::isKeyPressed(DOH_KEY_Z)) {
			mDirectionFacing.z += delta;
		}
		if (Input::isKeyPressed(DOH_KEY_X)) {
			mDirectionFacing.z -= delta;
		}

		//Click and Drag
		//TODO:: separate mClickAndDragTranslationSpeed ?
		//	Change cursor appearence when dragging?
		glm::vec3 rotation{ 0.0f, 0.0f, 0.0f };
		if (Input::isMouseButtonPressed(DOH_MOUSE_BUTTON_RIGHT)) {
			const glm::vec2 currentMousePos = Input::getCursorPos();

			rotation.x += (mCursorLastPosUpdate.x - currentMousePos.x) * delta;
			rotation.y += (currentMousePos.y - mCursorLastPosUpdate.y) * delta;

			if (glm::dot(rotation, rotation) > std::numeric_limits<float>::epsilon()) {
				mDirectionFacing += glm::normalize(rotation) * delta;
			}
		}

		mCursorLastPosUpdate = Input::getCursorPos();
	}

	void TG_PerspectiveCameraController::updateViewMatrices() {
		mCamera->setView(mPosition, mDirectionFacing, { 0.0f, 1.0f, 0.0f });
		mCamera->updateProjectionViewMatrix();
	}
}
