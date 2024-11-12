#include "dough/physics/BoundingBox2d.h"

#include "dough/scene/geometry/AGeometry.h"

#include "dough/Logging.h"

namespace DOH {

	BoundingBox2d::BoundingBox2d()
	:	mQuad(std::make_unique<Quad>(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec4(1.0f, 1.0f, 1.0f, 0.0f)))
	{}

	BoundingBox2d::BoundingBox2d(glm::vec3& pos, glm::vec2& size)
	:	mQuad(std::make_unique<Quad>(pos, size, glm::vec4(1.0f, 1.0f, 1.0f, 0.0f)))
	{}

	BoundingBox2d::BoundingBox2d(AGeometry& geo)
	:	mQuad(std::make_unique<Quad>(geo.Position, geo.Size, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), geo.Rotation))
	{}

	void BoundingBox2d::resizeToFit(AGeometry& geo) {
		if (geo.Size.x == 0.0f || geo.Size.y == 0.0f) {
			LOG_WARN("BoundingBox2d: resizeToFit Geo size: x 0, y 0");
			return;
		}

		//If is first geo added
		if (mQuad->Size.x == 0.0f && mQuad->Size.y == 0.0f) {
			mQuad->Position = geo.Position;
			mQuad->Size = geo.Size;
			return;
		}

		//if (encloses(geo)) {
		//	//Geo already within bounding box
		//	LOG_INFO("enclosed");
		//	return;
		//}

		//TODO:: Does NOT account for rotation

		//Calculate "bottom left" and "top right" for both the bounding box and geo.
		//Geo bounding box botLeft or topRight are outside of bounding box then update bounding box values.

		const float boxTopRightX = mQuad->Position.x + mQuad->Size.x;
		const float boxTopRightY = mQuad->Position.y + mQuad->Size.y;

		//geo.Size can have negative values min/max values of coords need to be checked
		const float geoBotLeftX = std::min(geo.Position.x, geo.Position.x + geo.Size.x);
		const float geoBotLeftY = std::min(geo.Position.y, geo.Position.y + geo.Size.y);
		const float geoTopRightX = std::max(geo.Position.x, geo.Position.x + geo.Size.x);
		const float geoTopRightY = std::max(geo.Position.y, geo.Position.y + geo.Size.y);

		//Compare geo values to bounding box's
		mQuad->Position.x = std::min(geoBotLeftX, mQuad->Position.x);
		mQuad->Position.y = std::min(geoBotLeftY, mQuad->Position.y);
		mQuad->Size.x = std::max(geoTopRightX, boxTopRightX) - mQuad->Position.x;
		mQuad->Size.y = std::max(geoTopRightY, boxTopRightY) - mQuad->Position.y;

		mQuad->Position.z = geo.Position.z; //Since this is a 2d bounding box take the Z from the last used geo
	}

	void BoundingBox2d::resizeToFitAll(std::vector<AGeometry>& geoArr) {
		for (AGeometry& geo : geoArr) {
			resizeToFit(geo);
		}
	}

	//IMPORTANT:: Only works if bounding box and geo are in the same space.
	bool BoundingBox2d::encloses(AGeometry& geo) {
		//Check if top right and bottom left points are inside this bounding box.
		if (!isVec2InsideXY(
			geo.Position.x + mQuad->Size.x,
			geo.Position.y + mQuad->Size.y
		)) return false;
		if (!isVec2InsideXY(
			geo.Position.x,
			geo.Position.y
		)) return false;

		return true;
	}

	bool BoundingBox2d::isVec2InsideXY(float x, float y) const {
		float boxPosX = mQuad->Position.x;
		float boxPosY = mQuad->Position.y;

		return !(y <= boxPosY) &&
			!(y >= boxPosY + mQuad->Size.y) &&
			!(x >= boxPosX + mQuad->Size.x || x <= boxPosX);
	}

	bool BoundingBox2d::isVec2InsideNotEdgeXY(float x, float y) const {
		float boxPosX = mQuad->Position.x;
		float boxPosY = mQuad->Position.y;

		return !(y < boxPosY) &&
			!(y > boxPosY + mQuad->Size.y) &&
			!(x > boxPosX + mQuad->Size.x || x < boxPosX);
	}
}
