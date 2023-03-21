#include "editor/EditorPerspectiveCameraController.h"

#include "dough/input/Input.h"
#include "dough/input/InputCodes.h"

using namespace DOH;

namespace DOH::EDITOR {

	EditorPerspectiveCameraController::EditorPerspectiveCameraController(float aspectRatio, float fov)
	:	mCamera(std::make_unique<PerspectiveCamera>(aspectRatio, fov)),
		mPosition(0.0f, 0.0f, 0.0f),
		mDirectionFacing(0.0f, 0.0f, 0.0f),
		mCursorLastPosUpdate(0.0f, 0.0f),
		mClickAndDragActive(false),
		mAspectRatio(aspectRatio),
		mFov(fov),
		mTranslationSpeed(0.3f)
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
		const auto& innerAppInputLayerQuery = Input::getInputLayer("InnerApp");
		if (!innerAppInputLayerQuery.has_value()) {
			//LOG_ERR("Failed to get input layer: " << "InnerApp");
			return;
		}
		const auto& inputLayer = innerAppInputLayerQuery.value().get();

		const float translationDelta = inputLayer.isKeyPressed(DOH_KEY_LEFT_SHIFT) ?
			mTranslationSpeed * 6.0f * delta : mTranslationSpeed * delta;

		if (inputLayer.isKeyPressed(DOH_KEY_A)) {
			mPosition.x -= translationDelta;
		}
		if (inputLayer.isKeyPressed(DOH_KEY_D)) {
			mPosition.x += translationDelta;
		}
		if (inputLayer.isKeyPressed(DOH_KEY_W)) {
			mPosition.z -= translationDelta;
		}
		if (inputLayer.isKeyPressed(DOH_KEY_S)) {
			mPosition.z += translationDelta;
		}
		if (inputLayer.isKeyPressed(DOH_KEY_SPACE)) {
			mPosition.y += translationDelta;
		}
		if (inputLayer.isKeyPressed(DOH_KEY_C)) {
			mPosition.y -= translationDelta;
		}
		if (inputLayer.isKeyPressed(DOH_KEY_Z)) {
			mDirectionFacing.z += translationDelta;
		}
		if (inputLayer.isKeyPressed(DOH_KEY_X)) {
			mDirectionFacing.z -= translationDelta;
		}
		if (inputLayer.isKeyPressed(DOH_KEY_UP)) {
			mDirectionFacing.y += translationDelta;
		}
		if (inputLayer.isKeyPressed(DOH_KEY_DOWN)) {
			mDirectionFacing.y -= translationDelta;
		}
		if (inputLayer.isKeyPressed(DOH_KEY_RIGHT)) {
			mDirectionFacing.x += translationDelta;
		}
		if (inputLayer.isKeyPressed(DOH_KEY_LEFT)) {
			mDirectionFacing.x -= translationDelta;
		}

		//Click and Drag
		//TODO:: separate mClickAndDragTranslationSpeed ?
		//	Change cursor appearence when dragging?
		glm::vec3 rotation{ 0.0f, 0.0f, 0.0f };
		if (inputLayer.isMouseButtonPressed(DOH_MOUSE_BUTTON_RIGHT)) {
			const glm::vec2 currentMousePos = inputLayer.getCursorPos();

			rotation.x += (mCursorLastPosUpdate.x - currentMousePos.x) * delta;
			rotation.y += (currentMousePos.y - mCursorLastPosUpdate.y) * delta;

			if (glm::dot(rotation, rotation) > std::numeric_limits<float>::epsilon()) {
				mDirectionFacing += glm::normalize(rotation) * delta;
			}
		}

		mCursorLastPosUpdate = inputLayer.getCursorPos();
	}

	void EditorPerspectiveCameraController::updateViewMatrices() {
		mCamera->setView(mPosition, mDirectionFacing, { 0.0f, 1.0f, 0.0f });
		mCamera->updateProjectionViewMatrix();
	}
}
