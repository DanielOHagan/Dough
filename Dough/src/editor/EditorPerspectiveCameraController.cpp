#include "editor/EditorPerspectiveCameraController.h"

#include "dough/input/Input.h"
#include "dough/input/InputCodes.h"

using namespace DOH;

namespace DOH::EDITOR {

	EditorPerspectiveCameraController::EditorPerspectiveCameraController(std::shared_ptr<AInputLayer> inputLayer, float aspectRatio, float fov)
	:	mCamera(std::make_unique<PerspectiveCamera>(aspectRatio, fov)),
		mPosition(0.0f, 0.0f, 0.0f),
		mDirectionFacing(0.0f, 0.0f, 0.001f),
		mCursorLastPosUpdate(0.0f, 0.0f),
		mClickAndDragEnabled(false),
		mClickAndDragActive(false),
		mAspectRatio(aspectRatio),
		mFov(fov),
		mTranslationSpeed(0.3f),
		mInputLayer(inputLayer)
	{}

	void EditorPerspectiveCameraController::onUpdate(float delta) {
		handleInput(delta);
		updateViewMatrices();
	}

	void EditorPerspectiveCameraController::onViewportResize(float aspectRatio) {
		mAspectRatio = aspectRatio;
		mCamera->setProjection(mFov, mAspectRatio);
		updateViewMatrices();
	}

	void EditorPerspectiveCameraController::handleInput(float delta) {
		const float translationDelta = mInputLayer->isKeyPressed(DOH_KEY_LEFT_SHIFT) ?
			mTranslationSpeed * 6.0f * delta : mTranslationSpeed * delta;

		if (mInputLayer->isKeyPressed(DOH_KEY_A)) {
			mPosition.x -= translationDelta;
		}
		if (mInputLayer->isKeyPressed(DOH_KEY_D)) {
			mPosition.x += translationDelta;
		}
		if (mInputLayer->isKeyPressed(DOH_KEY_W)) {
			mPosition.z -= translationDelta;
		}
		if (mInputLayer->isKeyPressed(DOH_KEY_S)) {
			mPosition.z += translationDelta;
		}
		if (mInputLayer->isKeyPressed(DOH_KEY_SPACE)) {
			mPosition.y += translationDelta;
		}
		if (mInputLayer->isKeyPressed(DOH_KEY_C)) {
			mPosition.y -= translationDelta;
		}
		if (mInputLayer->isKeyPressed(DOH_KEY_Z)) {
			mDirectionFacing.z += translationDelta;
		}
		if (mInputLayer->isKeyPressed(DOH_KEY_X)) {
			mDirectionFacing.z -= translationDelta;
		}
		if (mInputLayer->isKeyPressed(DOH_KEY_UP)) {
			mDirectionFacing.y += translationDelta;
		}
		if (mInputLayer->isKeyPressed(DOH_KEY_DOWN)) {
			mDirectionFacing.y -= translationDelta;
		}
		if (mInputLayer->isKeyPressed(DOH_KEY_RIGHT)) {
			mDirectionFacing.x += translationDelta;
		}
		if (mInputLayer->isKeyPressed(DOH_KEY_LEFT)) {
			mDirectionFacing.x -= translationDelta;
		}

		//Click and Drag
		//TODO:: separate mClickAndDragTranslationSpeed ?
		//	Change cursor appearence when dragging?
		if (mClickAndDragEnabled && mInputLayer->isMouseButtonPressed(DOH_MOUSE_BUTTON_RIGHT)) {
			glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };
			const glm::vec2& currentMousePos = mInputLayer->getCursorPos();
			mClickAndDragActive = true;

			rotation.x += (mCursorLastPosUpdate.x - currentMousePos.x) * delta;
			rotation.y += (currentMousePos.y - mCursorLastPosUpdate.y) * delta;

			if (glm::dot(rotation, rotation) > std::numeric_limits<float>::epsilon()) {
				mDirectionFacing += glm::normalize(rotation) * delta;
			}
		} else {
			mClickAndDragActive = false;
		}

		mCursorLastPosUpdate = mInputLayer->getCursorPos();
	}

	void EditorPerspectiveCameraController::updateViewMatrices() {
		mCamera->setView(mPosition, mDirectionFacing, { 0.0f, 1.0f, 0.0f });
		mCamera->updateProjectionViewMatrix();
	}
}
