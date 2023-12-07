#pragma once

#include "dough/Core.h"
#include "dough/scene/geometry/primitives/Quad.h"
#include "dough/rendering/text/FontBitmap.h"
#include "dough/rendering/text/ETextFlags.h"

namespace DOH {

	//TODO:: keep this as AGeometry or make a "ACollection" that extends AGeometry
	class TextString : public AGeometry {

	private:
		std::vector<Quad> mStringQuads;
		const char* mString;
		FontBitmap& mFontBitmap;
		float mScale;
		glm::vec4 mColour;

	public:
		TextString(const char* string, FontBitmap& fontBitmap, const float scale = 1.0f);
		TextString(const TextString& copy) = delete;
		TextString& operator=(const TextString& assignment) = delete;

		void setString(const char* string);
		void setRoot(glm::vec3 root);
		void setScale(const float scale);
		void setColour(const glm::vec4& colourRgba);
		//void setFontBitmap(const FontBitmap& fontBitmap);

		inline std::vector<Quad>& getQuads() { return mStringQuads; }
		inline const char* getString() const { return mString; }
		inline const FontBitmap& getCurrentFontBitmap() const { return mFontBitmap; }
		inline float getScale() const { return mScale; }
		inline const glm::vec4& getColour() const { return mColour; }
		inline size_t getLength() const { return strlen(mString); }

		static inline std::vector<Quad> getStringAsQuads(
			const char* string,
			const FontBitmap& bitmap,
			const float scale = 1.0f,
			const glm::vec4& colour = { 1.0f, 1.0f, 1.0f, 1.0f },
			const ETextFlags2d flags = ETextFlags2d::NONE
		) {
			return TextString::getStringAsQuads(string, bitmap, { 0.0f, 0.0f, 0.0f }, scale, colour, flags);
		}
		static std::vector<Quad> getStringAsQuads(
			const char* string,
			const FontBitmap& bitmap,
			const glm::vec3 rootPos,
			const float scale = 1.0f,
			const glm::vec4& colour = { 1.0f, 1.0f, 1.0f, 1.0f },
			const ETextFlags2d flags = ETextFlags2d::NONE
		);
	};
}
