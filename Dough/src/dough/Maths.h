#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace DOH {
	constexpr auto DOH_X_RIGHT = 1.0f;
	constexpr auto DOH_X_LEFT = -1.0f;
	constexpr auto DOH_Y_UP = 1.0f;
	constexpr auto DOH_Y_DOWN = -1.0f;
	constexpr auto DOH_Z_FORWARD = -1.0f;
	constexpr auto DOH_Z_BACKWARD = 1.0f;

	static constexpr inline glm::vec2 convertPixelToLeftHandedNDC(float x, float y, float maxX, float maxY) {
		return {
			(- 1 + (2 * (x / maxX))),
			1 - (2 * (y / maxY))
		};
	}
}
