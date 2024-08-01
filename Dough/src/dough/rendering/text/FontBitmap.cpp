#include "dough/rendering/text/FontBitmap.h"

#include "dough/files/ResourceHandler.h"
#include "dough/Logging.h"
#include "dough/scene/geometry/primitives/Quad.h"
#include "dough/files/readers/JsonFileReader.h"
#include "dough/application/Application.h"

#include <tracy/public/tracy/Tracy.hpp>

namespace DOH {

	FontBitmap::FontBitmap(const char* filePath, const char* imageDir, ETextRenderMethod textRenderMethod)
	:	mTextRenderMethod(textRenderMethod),
		mPageCount(0),
		mSpaceWidthNorm(0.0f),
		mLineHeightNorm(0.0f),
		mBaseNorm(0.0f)
	{
		ZoneScoped;

		//IMPORTANT:: Assumes charset is ASCII or unicode

		//Prefer to use MSDF where possible
		if (ResourceHandler::isFileOfType(filePath, "json")) {

			//IMPORTANT:: Assumes the file is one created by msdf-atlas-gen (see: https://github.com/Chlumsky/msdf-atlas-gen)
			//With one REQUIRED addition: The name of the atlas texture file is added to the element called "atlas" under the element name: "textureName"
			//
			//Also, mBaseNorm isn't required for MSDF glyphs as it is accounted for in the glphy.Offset vector.

			std::shared_ptr<JsonFileData> fileData = ResourceHandler::loadJsonFile(filePath);

			if (fileData == nullptr || fileData->FileData.empty()) {
				LOG_ERR("Failed to load json file for font bitmap: " << filePath);
				return;
			}

			JsonElement& root = fileData->getRoot();

			JsonElement& atlasAndTextureInfo = root["atlas"];
			const float fontPixelSizeFloat = static_cast<float>(atlasAndTextureInfo["size"].getLong());
			const uint32_t fileWidth = static_cast<uint32_t>(atlasAndTextureInfo["width"].getLong());
			const uint32_t fileHeight = static_cast<uint32_t>(atlasAndTextureInfo["height"].getLong());
			const bool isBottomY = atlasAndTextureInfo["yOrigin"].getString().compare("bottom") == 0;

			JsonElement& metrics = root["metrics"];
			const float scale = 1.0f / static_cast<float>(metrics["ascender"].getDouble() - metrics["descender"].getDouble());
			mLineHeightNorm =  metrics["lineHeight"].getNumberAsFloat() * scale;
			mSpaceWidthNorm = (fontPixelSizeFloat * 0.25f) / fontPixelSizeFloat; //Default to 1/4 of general glyph size.

			JsonElement textureNames = atlasAndTextureInfo["textureName"];
			if (textureNames.isArray()) {
				//TODO:: Is multiple texture support required?
				LOG_ERR("Multiple textures for single MSDF font NOT SUPPORTED! FilePath: " << filePath);
				THROW("");
				return;
			} else if (textureNames.isString()) {
				auto& context = Application::get().getRenderer().getContext();
				std::string textureFileName = atlasAndTextureInfo["textureName"].getString();
				std::string textureFilePath = imageDir + textureFileName;
				std::shared_ptr<TextureVulkan> texture = context.createTexture(textureFilePath);
				mPageTextures.emplace_back(texture);
				mPageCount = 1;
			}

			std::vector<JsonElement>& glyphs = root["glyphs"].getArray();
			for (JsonElement& fileGlyph : glyphs) {
				const uint32_t unicode = static_cast<uint32_t>(fileGlyph["unicode"].getLong());
				//planeBounds and atlasBounds are required for a renderable glyph, all others are ignored, except space
				std::optional<JsonElement> planeBounds = fileGlyph.getElement("planeBounds");
				std::optional<JsonElement> atlasBounds = fileGlyph.getElement("atlasBounds");
				if (!planeBounds.has_value() || !atlasBounds.has_value()) {
					if (unicode == 32) { //Unicode decimal value for "space"

						//TODO:: Test this with an MSDF font!!! If a "space" char is found would it also require an offset fro planeBounds like a rendered glyph?

						mSpaceWidthNorm = fileGlyph["advance"].getNumberAsFloat() / fontPixelSizeFloat;
						continue;
					}
					continue;
				}

				GlyphData g = {};
				g.PageId = 0;

				JsonElement& plane = planeBounds.value();
				const float planeLeft = plane["left"].getNumberAsFloat();
				const float planeBottom = plane["bottom"].getNumberAsFloat();
				const float planeRight = plane["right"].getNumberAsFloat();
				const float planeTop = plane["top"].getNumberAsFloat();

				JsonElement& atlas = atlasBounds.value();
				const float atlasLeft = atlas["left"].getNumberAsFloat();
				const float atlasBottom = atlas["bottom"].getNumberAsFloat();
				const float atlasRight = atlas["right"].getNumberAsFloat();
				const float atlasTop = atlas["top"].getNumberAsFloat();

				g.Offset = {
					planeLeft,
					0.0f //y-axis calculation differs by isBottomY value, 0.0f set as default
				};

				if (isBottomY) {
					g.Offset.y = planeTop;
					g.Size = {
						((atlasRight - atlasLeft) * scale) / fontPixelSizeFloat,
						((atlasTop - atlasBottom) * scale) / fontPixelSizeFloat
					};
					g.TexCoordTopLeft = {
						atlasLeft / fileWidth,
						1.0f - (atlasTop / fileHeight)
					};
					g.TexCoordBotRight = {
						atlasRight / fileWidth,
						1.0f - (atlasBottom / fileHeight)
					};
				} else {
					//TODO:: This isn't fully tested as I use y as bottom in all altasses, plane top/bottom might change in the JSON file so *-1 might not be required but using planeBottom instead.
					//g.Offset.y = planeTop OR planeBottom; ??
					//g.Size = {
					//	((atlasRight - atlasLeft) * scale) / fontPixelSizeFloat,
					//	((atlasTop - atlasBottom) * scale) / fontPixelSizeFloat
					//};
					//g.TexCoordTopLeft = {
					//	atlasLeft / fileWidth,
					//	1.0f - (atlasTop / fileHeight)
					//};
					//g.TexCoordBotRight = {
					//	atlasRight / fileWidth,
					//	1.0f - (atlasBottom / fileHeight)
					//};
				}

				g.AdvanceX = fileGlyph["advance"].getNumberAsFloat() * scale;

				mGlyphMap.emplace(unicode, g);
			}

			//TODO:: Kernings, cause z-fighting when in a Perspective camera but not when in an Orthographic camera.
			//std::vector<JsonElement>& kernings = root["kernings"].getArray();

		} else if (ResourceHandler::isFileOfType(filePath, "fnt")) {
			auto& context = Application::get().getRenderer().getContext();
			std::shared_ptr<FntFileData> fileData = ResourceHandler::loadFntFile(filePath);

			if (fileData == nullptr) {
				LOG_ERR("Failed to load fnt file");
				return;
			}

			for (const FntFilePageData& page : fileData->Pages) {
				std::shared_ptr<TextureVulkan> pageTexture = context.createTexture(imageDir + page.PageFilepath);
				mPageTextures.emplace_back(pageTexture);
			}

			const float fileWidth = static_cast<float>(fileData->Width);
			const float fileHeight = static_cast<float>(fileData->Height);
			const float pixelSize = static_cast<float>(fileData->CharSize);

			mPageCount = fileData->PageCount;
			mLineHeightNorm = fileData->LineHeight / pixelSize;
			mBaseNorm = fileData->Base / pixelSize;

			bool spaceFound = false;
			for (const FntFileGlyphData& fileGlyph : fileData->Chars) {
				if (fileGlyph.Id == 32) { //ASCII & Unicode decimal value for "space"
					mSpaceWidthNorm = fileGlyph.AdvanceX / pixelSize;
					spaceFound = true;
					continue;
				}

				GlyphData g = {};
				g.PageId = fileGlyph.PageId;

				//IMPORTANT:: Invert Y-axis to align coordinate space from fnt's to DOH's
				//TODO:: When allowing for other types of font files, account for this where needed
				g.Offset = {
					fileGlyph.OffsetX / pixelSize,
					-fileGlyph.OffsetY / pixelSize
				};
				g.Size = {
					fileGlyph.Width / pixelSize,
					fileGlyph.Height / pixelSize
				};
				g.TexCoordTopLeft = {
					fileGlyph.X / fileWidth,
					fileGlyph.Y / fileHeight
				};
				g.TexCoordBotRight = {
					g.TexCoordTopLeft.x + (fileGlyph.Width / fileWidth),
					g.TexCoordTopLeft.y + (fileGlyph.Height / fileHeight)
				};
				g.AdvanceX = fileGlyph.AdvanceX / pixelSize;

				mGlyphMap.emplace(fileGlyph.Id, g);
			}

			if (!spaceFound) {
				mSpaceWidthNorm = (pixelSize * 0.25f) / pixelSize; //Default to 1/4 of general glyph size.
			}

			//for (const FntFileKerningData& fileKerning : fileData->Kernings) {
			//	KerningData k = {
			//		fileKerning.FirstGlyphId,
			//		fileKerning.SecondGlyphId,
			//		fileKerning.Amount / pixelSize
			//	};
			//	mKernings.emplace_back(k);
			//}
		}
	}
}
