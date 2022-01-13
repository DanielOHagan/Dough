#include "TG_OrthoCameraController.h"
#include "dough/input/Input.h"
#include "dough/input/InputCodes.h"

using namespace DOH;

namespace TG {

	TG_OrthoCameraController::TG_OrthoCameraController(float aspectRatio)
	:	mPosition({0.0f, 0.0f, 1.0f}),
		mAspectRatio(aspectRatio),
		mRotation(0.0f),
		mZoomLevel(1.0f),
		mZoomSpeed(0.001f),
		mZoomMax(2.00f),
		mZoomMin(0.25f),
		mTranslationSpeed(0.005f),
		mCamera(std::make_shared<OrthographicCamera>(
			-aspectRatio * mZoomLevel,
			aspectRatio * mZoomLevel,
			-mZoomLevel,
			mZoomLevel
		))
	{}

	void TG_OrthoCameraController::onUpdate(float delta) {
		//TODO::
		//Handle Input
		if (Input::isKeyPressed(DOH_KEY_X)) {
			zoom(-1.0f);
		}
		if (Input::isKeyPressed(DOH_KEY_Z)) {
			zoom(1.0f);
		}

		if (Input::isKeyPressed(DOH_KEY_A)) {
			glm::vec3 translation{ -1.0f, 0.0f, 0.0f };
			translate(translation);
		}
		if (Input::isKeyPressed(DOH_KEY_D)) {
			glm::vec3 translation{ 1.0f, 0.0f, 0.0f };
			translate(translation);
		}
		if (Input::isKeyPressed(DOH_KEY_W)) {
			glm::vec3 translation{ 0.0f, 1.0f, 0.0f };
			translate(translation);
		}
		if (Input::isKeyPressed(DOH_KEY_S)) {
			glm::vec3 translation{ 0.0f, -1.0f, 0.0f };
			translate(translation);
		}



		//Update camera matrices
		updateMatrices();
	}

	void TG_OrthoCameraController::onViewportResize(float aspectRatio) {
		mAspectRatio = aspectRatio;
	}

	void TG_OrthoCameraController::translate(glm::vec3& translation) {
		mPosition.x += translation.x * mTranslationSpeed;
		mPosition.y += translation.y * mTranslationSpeed;
		mPosition.z += translation.z * mTranslationSpeed;
	}

	void TG_OrthoCameraController::zoom(float zoomAmount) {
		mZoomLevel -= zoomAmount * mZoomSpeed;

		if (mZoomLevel > mZoomMax) mZoomLevel = mZoomMax;
		else if (mZoomLevel < mZoomMin) mZoomLevel = mZoomMin;
	}

	void TG_OrthoCameraController::updateMatrices() {
		mCamera->setProjection(
			-mAspectRatio * mZoomLevel,
			mAspectRatio * mZoomLevel,
			-mZoomLevel,
			mZoomLevel
		);
		mCamera->setView(mPosition, mRotation);
		mCamera->updateProjectionViewMatrix();
	}
}
