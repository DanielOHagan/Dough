#include "editor/ui/EditorResourceUiViewer.h"

#include "editor/ui/EditorGui.h"
#include "dough/Logging.h"

namespace DOH::EDITOR {

	ResourceViewerUiTextureArray::ResourceViewerUiTextureArray(
		const char* title,
		const TextureArray& texArray,
		const bool display
	) : AResourceViewerUi(EResourceViewerUiType::TEXTURE_ARRAY, display),
		mTitle(title),
		mTextureArray(texArray)
	{
		mTextureSlotsUi.reserve(mTextureArray.getCurrentTextureCount());
		for (const TextureVulkan& texture : mTextureArray.getTextureSlots()) {
			mTextureSlotsUi.emplace_back(texture);
		}

		mFallbackTextureUi = std::make_unique<ResourceViewerUiTexture>(mTextureArray.getFallbackTexture());
	}

	void ResourceViewerUiTexture::draw(bool openAsWindow) {
		bool draw = true;

		if (openAsWindow) {
			ImGui::PushStyleVar(
				ImGuiStyleVar_WindowPadding,
				ImVec2(AResourceViewerUi::DEFAULT_WINDOW_PADDING_WIDTH, AResourceViewerUi::DEFAULT_WINDOW_PADDING_HEIGHT)
			);

			draw = ImGui::Begin(
				mTexture.getName().c_str(),
				&Display,
				ImGuiWindowFlags_HorizontalScrollbar
			);
		}

		if (draw) {
			std::string uniqueImGuiId = ImGuiWrapper::TEXTURE_LABEL + std::to_string(mTexture.getId());

			ImGui::Text("Texture ID: %i", mTexture.getId());
			ImGui::Text("Size: %i, %i", mTexture.getWidth(), mTexture.getHeight());
			ImGui::Text("Name: ");
			ImGui::SameLine();
			ImGui::Text(mTexture.getName().c_str());
			ImGui::Checkbox(("Match To Window Size" + uniqueImGuiId).c_str(), &MatchWindowSize);
			EditorGui::displayHelpTooltip("Match texture displayed width & height to window. Still affected by scale.");
			ImGui::DragFloat(("Scale" + uniqueImGuiId).c_str(), &Scale, 0.005f, 0.01f, 2.0f);

			glm::vec2 displaySize = { 0.0f, 0.0f };

			if (MatchWindowSize) {
				ImVec2 regionAvail = ImGui::GetContentRegionAvail();

				displaySize.x = regionAvail.x * Scale;
				displaySize.y = regionAvail.y * Scale;
			} else {
				displaySize.x = mTexture.getWidth() * Scale;
				displaySize.y = mTexture.getHeight() * Scale;
			}

			if (displaySize.y > 0.0f) {
				EditorGui::getGuiWrapper().drawTexture(mTexture, displaySize);
			}
		}

		if (openAsWindow) {
			ImGui::PopStyleVar(1);
			ImGui::End();
		}
	}

	void ResourceViewerUiMonoSpaceTextureAtlas::draw(bool openAsWindow) {
		bool draw = true;

		if (openAsWindow) {
			ImGui::PushStyleVar(
				ImGuiStyleVar_WindowPadding,
				ImVec2(AResourceViewerUi::DEFAULT_WINDOW_PADDING_WIDTH, AResourceViewerUi::DEFAULT_WINDOW_PADDING_HEIGHT)
			);

			draw = ImGui::Begin(
				mTextureAtlas.getName().c_str(),
				&Display,
				ImGuiWindowFlags_HorizontalScrollbar
			);
		}

		if (draw) {
			std::string uniqueImGuiId = ImGuiWrapper::TEXTURE_LABEL + std::to_string(mTextureAtlas.getId());

			ImGui::Text("Texture ID: %i", mTextureAtlas.getId());
			ImGui::Text("Size: %i, %i", mTextureAtlas.getWidth(), mTextureAtlas.getHeight());
			ImGui::Text("Column Count: %i\tRow Count: %i", mTextureAtlas.getColCount(), mTextureAtlas.getRowCount());
			ImGui::Text("Name: ");
			ImGui::SameLine();
			ImGui::Text(mTextureAtlas.getName().c_str());
			ImGui::Checkbox(("Match To Window Size" + uniqueImGuiId).c_str(), &MatchWindowSize);
			EditorGui::displayHelpTooltip("Match texture displayed width & height to window. Still affected by scale.");
			ImGui::DragFloat(("Scale" + uniqueImGuiId).c_str(), &Scale, 0.005f, 0.01f, 2.0f);

			glm::vec2 displaySize = { 0.0f, 0.0f };

			if (MatchWindowSize) {
				ImVec2 regionAvail = ImGui::GetContentRegionAvail();

				displaySize.x = regionAvail.x * Scale;
				displaySize.y = regionAvail.y * Scale;
			} else {
				displaySize.x = mTextureAtlas.getWidth() * Scale;
				displaySize.y = mTextureAtlas.getHeight() * Scale;
			}

			if (displaySize.y > 0.0f) {
				EditorGui::getGuiWrapper().drawTexture(mTextureAtlas, displaySize);
			}
		}

		if (openAsWindow) {
			ImGui::PopStyleVar(1);
			ImGui::End();
		}
	}

	void ResourceViewerUiIndexedTextureAtlas::draw(bool openAsWindow) {
		bool draw = true;

		if (openAsWindow) {
			ImGui::PushStyleVar(
				ImGuiStyleVar_WindowPadding,
				ImVec2(AResourceViewerUi::DEFAULT_WINDOW_PADDING_WIDTH, AResourceViewerUi::DEFAULT_WINDOW_PADDING_HEIGHT)
			);

			draw = ImGui::Begin(
				mTextureAtlas.getName().c_str(),
				&Display,
				ImGuiWindowFlags_HorizontalScrollbar
			);
		}

		if (draw) {
			std::string uniqueImGuiId = ImGuiWrapper::TEXTURE_LABEL + std::to_string(mTextureAtlas.getId());

			ImGui::Text("Texture ID: %i", mTextureAtlas.getId());
			ImGui::Text("Size: %i, %i", mTextureAtlas.getWidth(), mTextureAtlas.getHeight());
			ImGui::Text("Name: ");
			ImGui::SameLine();
			ImGui::Text(mTextureAtlas.getName().c_str());
			ImGui::Checkbox(("Match To Window Size" + uniqueImGuiId).c_str(), &mMatchWindowSize);
			EditorGui::displayHelpTooltip("Match texture displayed width & height to window. Still affected by scale.");
			ImGui::DragFloat(("Scale" + uniqueImGuiId).c_str(), &mScale, 0.005f, 0.01f, 2.0f);

			glm::vec2 displaySize = { 0.0f, 0.0f };

			if (mMatchWindowSize) {
				ImVec2 regionAvail = ImGui::GetContentRegionAvail();

				displaySize.x = regionAvail.x * mScale;
				displaySize.y = regionAvail.y * mScale;
			} else {
				displaySize.x = mTextureAtlas.getWidth() * mScale;
				displaySize.y = mTextureAtlas.getHeight() * mScale;
			}

			EditorGui::getGuiWrapper().drawTexture(mTextureAtlas, displaySize);

			ImGui::Text("Inner Texture Count: %i", static_cast<int>(mTextureAtlas.getInnerTextures().size()));
			ImGui::Text("Inner Textures:");
			std::array<uint32_t, 8> previewTexelCoords = {};
			size_t i = 0;
			for (const auto& innerTexture : mTextureAtlas.getInnerTextures()) {
				const char* texName = innerTexture.first.c_str();
				bool previewButton = texName == mPreviewedInnerTexture;
				if (previewButton) {
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.5f, 0.0f, 1.0f));
				}
				if (i > 0 && i < mTextureAtlas.getInnerTextures().size()) {
					ImGui::SameLine();
				}
				if (ImGui::Button(innerTexture.first.c_str())) {
					//NOTE:: Preview function made before InnerTextures stored TextureCoords

					mPreviewedInnerTexture = innerTexture.first.c_str();
					std::array<uint32_t, 2> botLeft = innerTexture.second.getBottomLeftTexels();
					std::array<uint32_t, 2> topRight = innerTexture.second.getTopRightTexels();

					mPreviewTexelCoords = {
						static_cast<float>(botLeft[0]),
						static_cast<float>(botLeft[1]),
						static_cast<float>(topRight[0]),
						static_cast<float>(topRight[1])
					};

					if (mPreviewTexelCoords[0] > 0.0f) mPreviewTexelCoords[0] /= mTextureAtlas.getWidth();
					if (mPreviewTexelCoords[1] > 0.0f) mPreviewTexelCoords[1] /= mTextureAtlas.getHeight();
					if (mPreviewTexelCoords[2] > 0.0f) mPreviewTexelCoords[2] /= mTextureAtlas.getWidth();
					if (mPreviewTexelCoords[3] > 0.0f) mPreviewTexelCoords[3] /= mTextureAtlas.getHeight();
				}

				if (previewButton) {
					ImGui::PopStyleColor(1);

					previewTexelCoords = innerTexture.second.TexelCoords;
				}

				i++;
			}
			ImGui::Text("Preview: ");
			ImGui::SameLine();
			ImGui::Text(mPreviewedInnerTexture);
			if (mPreviewedInnerTexture != "No Preview") {
				ImGui::Text("Texel Coords TL(%itx, %itx)", previewTexelCoords[0], previewTexelCoords[1]);
				ImGui::Text("Texel Coords TR(%itx, %itx)", previewTexelCoords[2], previewTexelCoords[3]);
				ImGui::Text("Texel Coords BR(%itx, %itx)", previewTexelCoords[4], previewTexelCoords[5]);
				ImGui::Text("Texel Coords BL(%itx, %itx)", previewTexelCoords[6], previewTexelCoords[7]);

				ImGui::DragFloat(("Preview Scale" + uniqueImGuiId).c_str(), &mPreviewScale, 0.005f, 0.01f, 5.0f);

				const float width = (mPreviewTexelCoords[2] - mPreviewTexelCoords[0]) * mTextureAtlas.getWidth();
				const float height = (mPreviewTexelCoords[1] - mPreviewTexelCoords[3]) * mTextureAtlas.getHeight();

				EditorGui::getGuiWrapper().drawTexture(
					mTextureAtlas,
					{ width * mPreviewScale, height * mPreviewScale },
					{ mPreviewTexelCoords[0], mPreviewTexelCoords[3] },
					{ mPreviewTexelCoords[2], mPreviewTexelCoords[1] }
				);
			} else {
				EditorGui::displayHelpTooltip("Click a button to show a preview of an inner texture.");
			}

			if (mTextureAtlas.getAnimations().size() > 0) {
				ImGui::NewLine();
				ImGui::Text("Animations");
				for (const auto& animation : mTextureAtlas.getAnimations()) {
					ImGui::Text(animation.first.c_str());
					ImGui::SameLine();
					const size_t frameCount = animation.second.getInnerTextures().size();
					ImGui::Text("Fame Count: %i", frameCount);

					uint32_t i = 0;
					for (const auto& innerTexture : animation.second.getInnerTextures()) {
						std::array<uint32_t, 2> botLeft = innerTexture.getBottomLeftTexels();
						std::array<uint32_t, 2> topRight = innerTexture.getTopRightTexels();

						std::array<float, 4> texCoords = {
							static_cast<float>(botLeft[0]),
							static_cast<float>(botLeft[1]),
							static_cast<float>(topRight[0]),
							static_cast<float>(topRight[1])
						};

						if (texCoords[0] > 0.0f) texCoords[0] /= mTextureAtlas.getWidth();
						if (texCoords[1] > 0.0f) texCoords[1] /= mTextureAtlas.getHeight();
						if (texCoords[2] > 0.0f) texCoords[2] /= mTextureAtlas.getWidth();
						if (texCoords[3] > 0.0f) texCoords[3] /= mTextureAtlas.getHeight();

						if (i > 0 && i < frameCount) {
							ImGui::SameLine();
						}

						EditorGui::getGuiWrapper().drawTexture(
							mTextureAtlas,
							{ static_cast<float>(innerTexture.getWidthTexels()), static_cast<float>(innerTexture.getHeightTexels()) },
							{ texCoords[0], texCoords[3] },
							{ texCoords[2], texCoords[1] }
						);

						i++;
					}
				}
			}
		}

		if (openAsWindow) {
			ImGui::PopStyleVar(1);
			ImGui::End();
		}
	}

	void ResourceViewerUiTextureArray::draw(bool openAsWindow) {
		bool draw = true;

		if (openAsWindow) {
			ImGui::PushStyleVar(
				ImGuiStyleVar_WindowPadding,
				ImVec2(AResourceViewerUi::DEFAULT_WINDOW_PADDING_WIDTH, AResourceViewerUi::DEFAULT_WINDOW_PADDING_HEIGHT)
			);

			draw = ImGui::Begin(mTitle, &Display, ImGuiWindowFlags_HorizontalScrollbar);
		}

		if (draw) {
			//Texture array info
			const uint32_t texCount = mTextureArray.getCurrentTextureCount();
			ImGui::Text("Current Texture Count: %i", texCount);
			ImGui::Text("Max Texture Count: %i", mTextureArray.getMaxTextureCount());

			//Texture slots
			if (texCount > 0) {
				ImGui::TreePush();
				ImGui::Unindent();

				//Textures are added to the arrays linearly and there shouldn't be any gaps.
				uint32_t slot = 0;
				for (ResourceViewerUiTexture& viewer : mTextureSlotsUi) {
					if (ImGui::TreeNode(("[" + std::to_string(slot) + "]").c_str())) {
						viewer.draw(false);
						ImGui::TreePop();
					}

					slot++;
				}

				ImGui::Indent();
				ImGui::TreePop();

				//Separate texture slots from fallback texture
				ImGui::NewLine();
			} else {
				ImGui::Text("Texture slots are empty, all slots are currently set to the Fallback Texture");
			}

			//Fallback texture
			ImGui::Text("Fallback Texture");
			EditorGui::displayHelpTooltip("Vulkan Texture Arrays must have all descriptor sets updated, the fallback texture is used to fill the empty slots.");
			mFallbackTextureUi->draw(false);
		}

		if (openAsWindow) {
			ImGui::PopStyleVar(1);
			ImGui::End();
		}
	}
}
