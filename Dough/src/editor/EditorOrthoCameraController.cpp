#include "editor/EditorOrthoCameraController.h"

#include "dough/input/Input.h"
#include "dough/input/InputCodes.h"

#include <algorithm>

using namespace DOH;

namespace DOH::EDITOR {

	EditorOrthoCameraController::EditorOrthoCameraController(float aspectRatio)
	:	mCamera(std::make_unique<OrthographicCamera>(
			-aspectRatio,
			aspectRatio,
			-1.0f,
			1.0f
		)),
		mPosition(0.0f, 0.0f, 1.0f),
		mCursorLastPosUpdate(0.0f, 0.0f),
		mClickAndDragActive(false),
		mAspectRatio(aspectRatio),
		mRotation(0.0f),
		mZoomLevel(1.0f),
		mZoomSpeed(0.1f),
		mZoomMax(50.00f),
		mZoomMin(0.25f),
		mTranslationSpeed(0.3f)
	{}

	void EditorOrthoCameraController::onUpdate(float delta) {
		handleInput(delta);
		updateViewMatrices();
	}

	void EditorOrthoCameraController::onViewportResize(float aspectRatio) {
		mAspectRatio = aspectRatio;
		updateProjectionMatrix();
	}

	void EditorOrthoCameraController::translate(glm::vec3& translation) {
		mPosition.x += translation.x * mTranslationSpeed;
		mPosition.y += translation.y * mTranslationSpeed;
		mPosition.z += translation.z * mTranslationSpeed;
	}

	void EditorOrthoCameraController::zoom(float zoomAmount) {
		mZoomLevel -= zoomAmount;

		mZoomLevel = std::clamp(mZoomLevel, mZoomMin, mZoomMax);

		updateProjectionMatrix();
	}

	void EditorOrthoCameraController::updateViewMatrices() {
		mCamera->setView(mPosition, mRotation);
		mCamera->updateProjectionViewMatrix();
	}

	void EditorOrthoCameraController::handleInput(float delta) {
		const float translationDelta = Input::isKeyPressed(DOH_KEY_LEFT_SHIFT) ?
			mTranslationSpeed * 6.0f * delta : mTranslationSpeed * delta;

		//Zooming
		if (Input::isKeyPressed(DOH_KEY_X)) {
			zoom(-mZoomSpeed * delta);
		}
		if (Input::isKeyPressed(DOH_KEY_Z)) {
			zoom(mZoomSpeed * delta);
		}

		//WASD translation
		if (Input::isKeyPressed(DOH_KEY_A)) {
			translateX(-translationDelta);
		}
		if (Input::isKeyPressed(DOH_KEY_D)) {
			translateX(translationDelta);
		}
		if (Input::isKeyPressed(DOH_KEY_W)) {
			translateY(translationDelta);
		}
		if (Input::isKeyPressed(DOH_KEY_S)) {
			translateY(-translationDelta);
		}

		//Click and Drag
		//TODO:: separate mClickAndDragTranslationSpeed ?
		//	Change cursor appearence when dragging?
		if (Input::isMouseButtonPressed(DOH_MOUSE_BUTTON_RIGHT)) {
			if (!mClickAndDragActive) {
				mClickAndDragActive = true;
			}

			//TODO:: fix differing speeds on differing UPS rates
			//Invert axes for a "drag" effect
			const glm::vec2 currentMousePos = Input::getCursorPos();

			translateXY(
				(mCursorLastPosUpdate.x - currentMousePos.x) * translationDelta,
				(currentMousePos.y - mCursorLastPosUpdate.y) * translationDelta
			);
		} else {
			mClickAndDragActive = false;
		}

		mCursorLastPosUpdate = Input::getCursorPos();
	}
}
