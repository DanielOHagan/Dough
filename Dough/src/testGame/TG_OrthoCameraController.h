#pragma once

#include "dough/scene/camera/ICameraController.h"
#include "dough/scene/camera/OrthographicCamera.h"
#include "dough/Core.h"

using namespace DOH;

namespace TG {

	class TG_OrthoCameraController : public ICameraController {

		//TODO:: mMaxTranslation and/or mMaxPosition ?

	private:
		std::unique_ptr<OrthographicCamera> mCamera;
		glm::vec3 mPosition;
		glm::vec2 mCursorLastPosUpdate;
		bool mClickAndDragActive;
		float mAspectRatio;
		float mRotation;
		float mZoomLevel;
		float mZoomSpeed;
		float mZoomMax;
		float mZoomMin;
		float mTranslationSpeed;

	public:
		TG_OrthoCameraController(float aspectRatio);
		TG_OrthoCameraController(const TG_OrthoCameraController& copy) = delete;
		TG_OrthoCameraController operator=(const TG_OrthoCameraController& assignment) = delete;

		virtual void onUpdate(float delta) override;
		virtual void onViewportResize(float aspectRatio) override;
		inline virtual ICamera& getCamera() const override { return *mCamera; }

		void translate(glm::vec3& translation);
		inline void translateX(float translation) { mPosition.x += translation; }
		inline void translateY(float translation) { mPosition.y += translation; }
		inline void translateZ(float translation) { mPosition.z += translation; }
		inline void translateXY(float x, float y) { mPosition.x += x; mPosition.y += y; };
		inline void setTranslationSpeed(float speed) { mTranslationSpeed = speed; };
		inline float getTranslationSpeed() const { return mTranslationSpeed; };
		inline void setPosition(glm::vec3 pos) { mPosition = pos; }
		inline const glm::vec3& getPosition() const { return mPosition; };

		void zoom(float zoomAmount);
		inline float getZoomLevel() const { return mZoomLevel; }
		inline void setZoomLevel(float zoomLevel) { mZoomLevel = zoomLevel; updateProjectionMatrix(); }
		inline float getZoomSpeed() const { return mZoomSpeed; }
		inline void setZoomSpeed(float speed) { mZoomSpeed = speed; }
		inline float getZoomMax() const { return mZoomMax; }
		inline void setZoomMax(float zoomMax) { mZoomMax = zoomMax; }
		inline float getZoomMin() const { return mZoomMin; }
		inline void setZoomMin(float zoomMin) { mZoomMin = zoomMin; }

	private:
		//Convenience functions
		inline void updateProjectionMatrix() {
			mCamera->setProjection(
				-mAspectRatio * mZoomLevel,
				mAspectRatio * mZoomLevel,
				-mZoomLevel,
				mZoomLevel
			);
		}
		void updateViewMatrices();
		void handleInput(float delta);
	};
}
