#pragma once

#include "dough/Utils.h"

#include <glm/glm.hpp>

namespace DOH {

	class AGeometry2D {

	protected:
		std::unique_ptr<glm::vec3> mPosition;
		std::unique_ptr<glm::vec2> mSize;
		float mRotation;


		AGeometry2D(glm::vec3& pos, glm::vec2& size, float rotationRads = 0.0f)
		:	mPosition(std::make_unique<glm::vec3>(pos)),
			mSize(std::make_unique<glm::vec2>(size)),
			mRotation(rotationRads)
		{};

	public:
		AGeometry2D() = delete;

		inline glm::vec3& getPosition() const { return *mPosition; }
		inline void setPosition(glm::vec3& pos) { mPosition->x = pos.x; mPosition->y = pos.y; mPosition->z = pos.z; }
		inline void setPosition(float x, float y, float z) { mPosition->x = x; mPosition->y = y; mPosition->z = z; }
		inline glm::vec2& getSize() const { return *mSize; }
		inline void setSize(glm::vec2& size) { mSize->x = size.x; mSize->y = size.y; }
		inline void setSize(float width, float height) { mSize->x = width; mSize->y = height; }
		inline float getRotationRads() const { return mRotation; }
		inline float getRotationDegs() const { return glm::degrees(mRotation); }
		inline void setRotationRads(float rotationRads) { mRotation = rotationRads; }
		inline void setRotationDegs(float rotationDegs) { mRotation = glm::radians(rotationDegs); }
	};
}
