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

			const int innerTextureCount = static_cast<int>(mTextureAtlas.getInnerTextures().size());
			ImGui::Text("Inner Texture Count: %i", innerTextureCount);
			ImGui::Text("Inner Textures:");

			if (innerTextureCount > 10) { //Use dropdown
				if (ImGui::BeginCombo("Preview", mPreviewName.c_str())) {
					for (const auto& innerTexture : mTextureAtlas.getInnerTextures()) {
						bool selected = false;
						ImGui::Selectable(innerTexture.first.c_str(), &selected);
						if (selected) {
							mPreviewName = innerTexture.first;
							mPreviewedInnerTexture = innerTexture.second;
							break;
						}
					}
					ImGui::EndCombo();
				}
			} else { //Use buttons
				int i = 0;
				for (const auto& innerTexture : mTextureAtlas.getInnerTextures()) {
					bool previewButton = mPreviewedInnerTexture.has_value() && mPreviewName == innerTexture.first;
					if (previewButton) {
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.5f, 0.0f, 1.0f));
					}
					if (
						(i > 0) &&
						(i < innerTextureCount) &&
						(i % 5 > 0) //Wrap to new line every 5 buttons
					) {
						ImGui::SameLine();
					}
					if (ImGui::Button(innerTexture.first.c_str())) {
						mPreviewName = innerTexture.first;
						mPreviewedInnerTexture = innerTexture.second;
					}
					if (previewButton) {
						ImGui::PopStyleColor(1);
					}
					i++;
				}
			}

			ImGui::Text("Preview: ");
			ImGui::SameLine();
			ImGui::Text(mPreviewName.c_str());
			if (mPreviewedInnerTexture.has_value()) {
				InnerTexture& innerTexture = mPreviewedInnerTexture.value();
				ImGui::Text("Texel Coords:");
				EditorGui::displayHelpTooltip("Texel Coords use the top left of the texture as the origin.");
				ImGui::Text("TL(%itx, %itx)", innerTexture.TexelCoords[0], innerTexture.TexelCoords[1]);
				ImGui::Text("TR(%itx, %itx)", innerTexture.TexelCoords[2], innerTexture.TexelCoords[3]);
				ImGui::Text("BR(%itx, %itx)", innerTexture.TexelCoords[4], innerTexture.TexelCoords[5]);
				ImGui::Text("BL(%itx, %itx)", innerTexture.TexelCoords[6], innerTexture.TexelCoords[7]);
				ImGui::Text("Size(%itx, %itx)", innerTexture.getWidthTexels(), innerTexture.getHeightTexels());
				ImGui::DragFloat(("Preview Scale" + uniqueImGuiId).c_str(), &mPreviewScale, 0.005f, 0.01f, 5.0f);

				EditorGui::getGuiWrapper().drawTexture(
					mTextureAtlas,
					{
						(innerTexture.getWidthTexCoord() * mTextureAtlas.getWidth()) * mPreviewScale,
						(innerTexture.getHeightTexCoord() * mTextureAtlas.getHeight()) * mPreviewScale
					},
					{ innerTexture.TextureCoords[0], innerTexture.TextureCoords[1] },
					{ innerTexture.TextureCoords[4], innerTexture.TextureCoords[5] }
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
