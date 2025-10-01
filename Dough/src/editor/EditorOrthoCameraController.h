#pragma once

#include "dough/scene/camera/ICameraController.h"
#include "dough/scene/camera/OrthographicCamera.h"
#include "dough/Core.h"
#include "dough/input/AInputLayer.h"

using namespace DOH;

namespace DOH::EDITOR {

	class EditorOrthoCameraController : public ICameraController {
	private:
		std::reference_wrapper<OrthographicCamera> mCamera;
		glm::vec3 mPosition;
		glm::vec2 mCursorLastPosUpdate;
		bool mClickAndDragEnabled;
		bool mClickAndDragActive;
		float mAspectRatio;
		float mRotation;
		float mZoomLevel;
		float mZoomSpeed;
		float mZoomMax;
		float mZoomMin;
		float mTranslationSpeed;

		//NOTE:: using a shared pointer so the input layer isn't invalidated if removed from Input::mInputLayers
		std::shared_ptr<AInputLayer> mInputLayer;

	public:
		EditorOrthoCameraController::EditorOrthoCameraController(OrthographicCamera& camera, std::shared_ptr<AInputLayer> inputLayer, float aspectRatio);
		EditorOrthoCameraController(const EditorOrthoCameraController& copy) = delete;
		EditorOrthoCameraController operator=(const EditorOrthoCameraController& assignment) = delete;

		virtual void onUpdate(float delta) override;
		virtual void onViewportResize(float aspectRatio) override;
		inline virtual ICamera& getCamera() const override { return mCamera; }

		inline void setInputLayer(std::shared_ptr<AInputLayer> inputLayer) { mInputLayer = inputLayer; }
		inline AInputLayer& getInputLayer() const { return *mInputLayer; }

		//All delta multiplications are expected to have been performed on arguments
		void translate(glm::vec3& translation);
		inline void translateX(float translation) { mPosition.x += translation; }
		inline void translateY(float translation) { mPosition.y += translation; }
		inline void translateZ(float translation) { mPosition.z += translation; }
		inline void translateXY(float x, float y) { mPosition.x += x; mPosition.y += y; };
		inline void setTranslationSpeed(float speed) { mTranslationSpeed = speed; };
		inline float getTranslationSpeed() const { return mTranslationSpeed; };
		inline void setPosition(glm::vec3& pos) { mPosition = pos; }
		inline void setPositionXYZ(float x, float y, float z) { mPosition.x = x; mPosition.y = y; mPosition.z = z; }
		inline const glm::vec3& getPosition() const { return mPosition; };
		inline void setCursorLastPos(glm::vec2& cursorLastPos) { mCursorLastPosUpdate = cursorLastPos; }
		inline void setCursorLastPosXY(float x, float y) { mCursorLastPosUpdate.x = x;  mCursorLastPosUpdate.y = y; }
		inline const glm::vec2& getCursorLastPos() const { return mCursorLastPosUpdate; }
		inline float getAspectRatio() const { return mAspectRatio; }
		inline void setAspectRatio(float aspectRatio) { mAspectRatio = aspectRatio; }
		inline bool isClickAndDragEnabled() const { return mClickAndDragEnabled; }
		inline void setClickAndDragEnabled(bool enabled) { mClickAndDragEnabled = enabled; }
		inline bool isClickAndDragActive() const { return mClickAndDragActive; }
		inline float getRotation() const { return mRotation; }
		inline void setRotation(float rotation) { mRotation = rotation; }

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
			mCamera.get().setProjection(
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
