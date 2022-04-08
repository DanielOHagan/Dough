#pragma once

#include "dough/Utils.h"

#include <glm/glm.hpp>

namespace DOH {

	class AGeometry {

	protected:
		glm::vec3 mPosition;
		glm::vec2 mSize;
		float mRotation;

		AGeometry(glm::vec3 pos, glm::vec2 size, float rotationRads = 0.0f)
		:	mPosition(pos),
			mSize(size),
			mRotation(rotationRads)
		{};

	public:
		AGeometry() = delete;

		inline glm::vec3 getPosition() const { return mPosition; }
		inline void setPosition(glm::vec3 pos) { mPosition = pos; }
		inline glm::vec2 getSize() const { return mSize; }
		inline void setSize(glm::vec2 size) { mSize = size; }
		inline void setSize(float width, float height) { mSize.x = width; mSize.y = height; }
		inline float getRotationRads() const { return mRotation; }
		inline float getRotationDegs() const { return glm::degrees(mRotation); }
		inline void setRotationRads(float rotationRads) { mRotation = rotationRads; }
		inline void setRotationDegs(float rotationDegs) { mRotation = glm::radians(rotationDegs); }
		inline void translateX(float x) { mPosition.x += x; }
		inline void translateY(float y) { mPosition.y += y; }
		inline void translateZ(float z) { mPosition.z += z; }
		inline void translateXY(float x, float y) { mPosition.x += x; mPosition.y += y; }
		inline void translateXYZ(float x, float y, float z = 0.0f) { mPosition.x += x; mPosition.y += y; mPosition.z += z; }
	};
}
