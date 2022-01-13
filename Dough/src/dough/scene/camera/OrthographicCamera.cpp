#include "dough/scene/camera/OrthographicCamera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace DOH {

    OrthographicCamera::OrthographicCamera(
        float left,
        float right,
        float bottom,
        float top,
        float nearZ,
        float farZ
    ) : mProjectionMatrix(1.0f),
        mViewMatrix(1.0f),
        mProjectionViewMatrix(1.0f)
    {}

    void OrthographicCamera::updateProjectionViewMatrix() {
        mProjectionViewMatrix = glm::mat4x4(1.0f);
        mProjectionViewMatrix = mProjectionViewMatrix * mProjectionMatrix;
        mProjectionViewMatrix = mProjectionViewMatrix * mViewMatrix;
    }

    void OrthographicCamera::setView(const glm::vec3& pos, const float rotation) {
        mViewMatrix = glm::mat4x4(1.0f);
        mViewMatrix = glm::translate(mViewMatrix, pos);
        mViewMatrix = glm::rotate(mViewMatrix, rotation, { 0.0f, 0.0f, 1.0f });
        mViewMatrix = glm::inverse(mViewMatrix);
    }

    void OrthographicCamera::setProjection(
        float left,
        float right,
        float bottom,
        float top,
        float nearZ,
        float farZ
    ) {
        mProjectionMatrix = glm::ortho(left, right, bottom, top, nearZ, farZ);
        //NOTE:: GLM was designed for OpenGL where the y clip coord is inverted. This fixes it for Vulkan:
        mProjectionMatrix[1][1] *= -1;
    }
}
