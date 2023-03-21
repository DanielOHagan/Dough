#include "dough/scene/geometry/collections/TextString.h"

#include "dough/Logging.h"

namespace DOH {

	TextString::TextString(const char* string, FontBitmap& fontBitmap, const float scale)
	:	AGeometry({0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, 0.0f),
		mString(string),
		mFontBitmap(fontBitmap),
		mScale(scale),
		mColour({ 1.0f, 1.0f, 1.0f, 1.0f })
	{
		if (strlen(string) == 0) {
			LOG_WARN("TextString given empty string");
		}

		mStringQuads = TextString::getStringAsQuads(string, fontBitmap, Position, mScale, mColour);
	}

	void TextString::setString(const char* string) {
		size_t length = strlen(string);
		if (length == 0) {
			mStringQuads.clear();
			mString = "";
			return;
		}

		mString = string;

		//Immediately change quad data
		mStringQuads = TextString::getStringAsQuads(mString, mFontBitmap, Position, mScale, mColour);
	}

	void TextString::setScale(const float scale) {
		if (mScale == scale) {
			return;
		} else if (mStringQuads.size() == 0) {
			mScale = scale;
			return;
		}

		//Immediately change quad data
		const size_t stringLength = strlen(mString);
		size_t quadIndex = 0;
		for (size_t stringIndex = 0; stringIndex < stringLength; stringIndex++) {

			//Special characters not represented by quads
			const char currentChar = mString[stringIndex];
			if (currentChar == 32 || currentChar == '\n' || currentChar == '\t') {
				continue;
			} else { //Characters represented by quads
				Quad& quad = mStringQuads[quadIndex];

				//Remove old scale transformations
				quad.Size /= mScale;
				quad.Position /= mScale;

				//Apply new scale transformations
				quad.Size *= scale;
				quad.Position *= scale;

				quadIndex++;
			}
		}

		mScale = scale;
	}

	void TextString::setColour(const glm::vec4& colourRgba) {
		mColour = colourRgba;
		
		//Immediately change quad data
		for (Quad& quad : mStringQuads) {
			quad.Colour = colourRgba;
		}
	}

	std::vector<Quad> TextString::getStringAsQuads(
		const char* string,
		const FontBitmap& bitmap,
		const glm::vec3 rootPos,
		const float scale,
		const glm::vec4& colour,
		const ETextFlags2d flags
	) {
		const size_t stringLength = strlen(string);

		if (stringLength == 0) {
			return {};
		}

		std::vector<Quad> quads;
		quads.reserve(stringLength);

		glm::vec3 currentPos = rootPos;

		uint32_t lastCharId = 0;

		for (size_t i = 0; i < stringLength; i++) {
			//TODO:: currently doesn't support a UTF-8 conversion so a cast to uint produces ASCII decimal values
			const uint32_t charId = static_cast<uint32_t>(string[i]);

			//Handle special characters
			if (charId == 32) { //space
				currentPos.x += bitmap.getSpaceWidthNorm() * scale;
				lastCharId = 0;
				continue;
			} else if (charId == 10) { //new line
				currentPos.y -= bitmap.getLineHeightNorm() * scale;
				currentPos.x = rootPos.x;
				lastCharId = 0;
				continue;
			} else if (charId == 9) { //tab
				currentPos.x += bitmap.getTabWidthNorm() * scale;
				lastCharId = 0;
				continue;
			}

			const auto& g = bitmap.getGlyphMap().find(charId);
			if (g != bitmap.getGlyphMap().end()) {

				const float glyphHeightScaled = g->second.Size.y * scale;

				Quad quad = {};
				quad.Position = {
					currentPos.x + (g->second.Offset.x * scale),
					currentPos.y - glyphHeightScaled + ((bitmap.getBaseNorm() + g->second.Offset.y) * scale),
					currentPos.z
				};
				quad.Size = {
					g->second.Size.x * scale,
					glyphHeightScaled
				};

				quad.TextureCoords = {
					g->second.TexCoordTopLeft.x,
					g->second.TexCoordBotRight.y,

					g->second.TexCoordBotRight.x,
					g->second.TexCoordBotRight.y,

					g->second.TexCoordBotRight.x,
					g->second.TexCoordTopLeft.y,

					g->second.TexCoordTopLeft.x,
					g->second.TexCoordTopLeft.y
				};

				quad.Colour = colour;

				lastCharId = charId;

				quad.setTexture(*bitmap.getPageTexture(g->second.PageId));
				quads.emplace_back(quad);

				currentPos.x += g->second.AdvanceX * scale;

			} else {
				LOG_WARN("Failed to find charId: " << charId << " in bitmap");
			}
		}

		return quads;
	}
}
