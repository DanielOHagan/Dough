#pragma once

#include "dough/scene/camera/ICamera.h"

namespace DOH {

	class OrthographicCamera : public ICamera {
	private:
		glm::mat4x4 mProjectionMatrix;
		glm::mat4x4 mViewMatrix;
		glm::mat4x4 mProjectionViewMatrix;

	public:
		OrthographicCamera(
			float left,
			float right,
			float bottom,
			float top,
			float nearZ	= 0.0f,
			float farZ = 100.0f
		);
		OrthographicCamera(const OrthographicCamera& copy) = delete;
		OrthographicCamera operator=(const OrthographicCamera& assignment) = delete;

		virtual void updateProjectionViewMatrix() override;

		inline virtual glm::mat4x4& getViewMatrix() override { return mViewMatrix; }
		inline virtual glm::mat4x4& getProjectionMatrix() override { return mProjectionMatrix; }
		inline virtual glm::mat4x4& getProjectionViewMatrix() override { return mProjectionViewMatrix; }

		void setView(const glm::vec3& pos, const float rotation);
		void setProjection(
			float left,
			float right,
			float bottom,
			float top,
			float nearZ = 0.0f,
			float farZ = 100.0f
		);
	};
}
