#pragma once

#include "dough/Utils.h"

#include <glm/glm.hpp>

namespace DOH {

	class AGeometry {
	protected:
		AGeometry(glm::vec3 pos, glm::vec2 size, float rotationRads = 0.0f)
		:	Position(pos),
			Size(size),
			Rotation(rotationRads)
		{};

	public:
		glm::vec3 Position;
		glm::vec2 Size;
		float Rotation;

		AGeometry() = delete;

		inline float getRotationRads() const { return Rotation; }
		inline float getRotationDegs() const { return glm::degrees(Rotation); }
		inline void setRotationRads(float rotationRads) { Rotation = rotationRads; }
		inline void setRotationDegs(float rotationDegs) { Rotation = glm::radians(rotationDegs); }
		inline void translateX(float x) { Position.x += x; }
		inline void translateY(float y) { Position.y += y; }
		inline void translateZ(float z) { Position.z += z; }
		inline void translateXY(float x, float y) { Position.x += x; Position.y += y; }
		inline void translateXYZ(float x, float y, float z = 0.0f) { Position.x += x; Position.y += y; Position.z += z; }
	};
}
