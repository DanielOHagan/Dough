#include "dough/scene/camera/PerspectiveCamera.h"

#include "dough/Maths.h"

namespace DOH {

	PerspectiveCamera::PerspectiveCamera(float aspectRatio, float fovDegrees, float nearZ, float farZ)
	:	mProjectionMatrix(glm::perspective(glm::radians(fovDegrees), aspectRatio, nearZ, farZ)),
		mViewMatrix(1.0f),
		mProjectionViewMatrix(1.0f)
	{
		mProjectionMatrix[1][1] *= -1;
	}

	void PerspectiveCamera::updateProjectionViewMatrix() {
		mProjectionViewMatrix = mProjectionMatrix;
		mProjectionViewMatrix = mProjectionViewMatrix * mViewMatrix;
	}

	void PerspectiveCamera::setView(const glm::vec3& pos, const glm::vec3& direction, const glm::vec3& up) {
		mViewMatrix = glm::lookAt(pos, direction, up);
	}

	void PerspectiveCamera::setProjection(float fovDegrees, float aspectRatio, float nearZ, float farZ) {
		mProjectionMatrix = glm::perspective(glm::radians(fovDegrees), aspectRatio, nearZ, farZ);
		mProjectionMatrix[1][1] *= -1;
	}
}
