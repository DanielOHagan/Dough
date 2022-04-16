#pragma once

#include "dough/scene/camera/ICamera.h"

namespace DOH {

	class PerspectiveCamera : public ICamera {

	private:
		glm::mat4x4 mProjectionMatrix;
		glm::mat4x4 mViewMatrix;
		glm::mat4x4 mProjectionViewMatrix;

	public:
		PerspectiveCamera(float aspectRatio, float fov = 60.0f, float nearZ = 0.0f, float farZ = 1.0f);
		PerspectiveCamera(const PerspectiveCamera& copy) = delete;
		PerspectiveCamera operator=(const PerspectiveCamera& assignment) = delete;

		virtual void updateProjectionViewMatrix() override;

		inline virtual glm::mat4x4& getViewMatrix() override { return mViewMatrix; }
		inline virtual glm::mat4x4& getProjectionMatrix() override { return mProjectionMatrix; }
		inline virtual glm::mat4x4& getProjectionViewMatrix() override { return mProjectionViewMatrix; }

		void setView(const glm::vec3 pos, const glm::vec3 direction, const glm::vec3 up);
		void setProjection(float fovDegrees, float aspectRatio, float nearZ = 0.0f, float farZ = 1.0f);
	};
}
