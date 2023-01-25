#include "editor/ui/EditorResourceUiViewer.h"

#include "editor/ui/EditorGui.h"

namespace DOH::EDITOR {

	void ResourceViewerUiTexture::drawUi() {
		ImGuiWrapper& imGuiWrapper = EditorGui::getGuiWrapper();

		ImGui::PushStyleVar(
			ImGuiStyleVar_WindowPadding,
			ImVec2(AResourceViewerUi::DEFAULT_WINDOW_PADDING_WIDTH, AResourceViewerUi::DEFAULT_WINDOW_PADDING_HEIGHT)
		);

		if (ImGui::Begin(
			mTexture.getName().c_str(),
			&Display,
			ImGuiWindowFlags_HorizontalScrollbar
		)) {
			drawUiInner();
		}

		ImGui::PopStyleVar(1);
		ImGui::End();
	}

	void ResourceViewerUiTexture::drawUiInner() {
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

	void ResourceViewerUiMonoSpaceTextureAtlas::drawUi() {
		ImGui::PushStyleVar(
			ImGuiStyleVar_WindowPadding,
			ImVec2(AResourceViewerUi::DEFAULT_WINDOW_PADDING_WIDTH, AResourceViewerUi::DEFAULT_WINDOW_PADDING_HEIGHT)
		);

		if (ImGui::Begin(
			mTextureAtlas.getName().c_str(),
			&Display,
			ImGuiWindowFlags_HorizontalScrollbar
		)) {
			drawUiInner();
		}

		ImGui::PopStyleVar(1);
		ImGui::End();
	}

	void ResourceViewerUiMonoSpaceTextureAtlas::drawUiInner() {
		//TODO:: render lines showing the columns & rows
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

	void ResourceViewerUiTextureArray::drawUi() {
		ImGui::PushStyleVar(
			ImGuiStyleVar_WindowPadding,
			ImVec2(AResourceViewerUi::DEFAULT_WINDOW_PADDING_WIDTH, AResourceViewerUi::DEFAULT_WINDOW_PADDING_HEIGHT)
		);

		if (ImGui::Begin(
			mTitle,
			&Display,
			ImGuiWindowFlags_HorizontalScrollbar
		)) {
			drawUiInner();
		}

		ImGui::PopStyleVar(1);
		ImGui::End();
	}

	ResourceViewerUiTextureArray::ResourceViewerUiTextureArray(
		const char* title,
		const TextureArray& texArray,
		const bool display
	) : AResourceViewerUi(display),
		mTitle(title),
		mTextureArray(texArray)
	{
		mTextureSlotsUi.reserve(mTextureArray.getCurrentTextureCount());
		for (const TextureVulkan& texture : mTextureArray.getTextureSlots()) {
			mTextureSlotsUi.emplace_back(texture);
		}

		mFallbackTextureUi = std::make_unique<ResourceViewerUiTexture>(mTextureArray.getFallbackTexture());
	}

	void ResourceViewerUiTextureArray::drawUiInner() {
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
			for (AResourceViewerUi& viewer : mTextureSlotsUi) {
				if (ImGui::TreeNode(("[" + std::to_string(slot) + "]").c_str())) {
					viewer.drawUiInner();
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
		mFallbackTextureUi->drawUiInner();
	}
}
