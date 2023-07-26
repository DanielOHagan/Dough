#pragma once

#include "dough/Maths.h"
#include "dough/scene/geometry/primitives/Quad.h"

#include <vector>
#include <memory>

namespace DOH {

	class AGeometry;

	class BoundingBox2d {

	private:
		//The quad used to define this BoundingBox
		std::unique_ptr<Quad> mQuad;

		//TODO:: Potentially store a ref to all children. So when removing a child search through to find one of UUID (TODO:: UUIDs) and remove it.
		//std::vector<std::reference_wrapper<AGeometry>> mChildren;

	public:
		BoundingBox2d();
		BoundingBox2d(glm::vec3& pos, glm::vec2& size);
		BoundingBox2d(AGeometry& geo);

		inline void setPosition(const glm::vec3& pos) { mQuad->Position = pos; }
		inline void setSize(const glm::vec2& size) { mQuad->Size = size; }

		inline void reset() { mQuad->Position = { 0.0f, 0.0f, 0.0f }; mQuad->Size = { 0.0f,0.0f }; }
		/** 
		* Change, if needed, this bounding box position or size value to accommodate given geo.
		* 
		* @param geo The geometry instance to include.
		*/
		void resizeToFit(AGeometry& geo);
		/**
		* Change, if needed, this bounding box position or size value to accommodate the geo in the given array.
		*
		* @param geoArr Array of geometry instances to include.
		*/
		void resizeToFitAll(std::vector<AGeometry>& geoArr);
		/**
		 * Check if this bounding box encloses the given boundingBox.
		 * 
		 * @param boundingBox The bounding box to check if enclosed.
		 * @returns True if given bounding box is enclosed by this bounding box. False if not completely enclosed.
		 */
		bool encloses(AGeometry& geo);
		/**
		* Check if a 2D point is inside of the bounding box.
		* 
		* @param vec2
		* @returns True if vec2 point is inside this bounding box
		*/
		inline bool isVec2Inside(const glm::vec2& vec2) const { return isVec2InsideXY(vec2.x, vec2.y); };
		/**
		* Check if a 2D point is inside of the bounding box, but doesn't include the edge of the bounding box.
		* 
		* @param vec2 The 2D point to check.
		* @returns True if coordinate is inside this bounding box and not on the edge.
		*/
		inline bool isVec2InsideNotEdge(const glm::vec2& vec2) const { return isVec2InsideNotEdgeXY(vec2.x, vec2.y); }

		bool isVec2InsideXY(float x, float y) const;
		bool isVec2InsideNotEdgeXY(float x, float y) const;

		inline const Quad& getQuad() const { return *mQuad; }
	};
}
