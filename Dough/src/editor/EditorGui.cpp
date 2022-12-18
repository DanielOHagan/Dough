#include "editor/EditorGui.h"

#include "dough/application/Application.h"

namespace DOH::EDITOR {

	std::unique_ptr<EditorGui> EditorGui::INSTANCE = nullptr;

	void EditorGui::init() {
		if (INSTANCE == nullptr) {
			INSTANCE = std::make_unique<EditorGui>();

			INSTANCE->mTextureViewerWindows = {};
		} else {
			LOG_WARN("EditorGui already initialised");
		}
	}

	void EditorGui::close() {
		if (INSTANCE != nullptr) {
			INSTANCE->mTextureViewerWindows.clear();
			INSTANCE.reset();
		} else {
			LOG_WARN("EditorGui already closed");
		}
	}

	ImGuiWrapper& EditorGui::getGuiWrapper() {
		return Application::get().getRenderer().getContext().getImGuiWrapper();
	}

	void EditorGui::openTextureViewerWindowImpl(TextureVulkan& texture) {
		const auto& itr = mTextureViewerWindows.find(texture.getId());
		if (itr != mTextureViewerWindows.end()) {
			itr->second.Display = true;
		} else {
			mTextureViewerWindows.emplace(
				texture.getId(),
				ImGuiTextureViewerWindow(
					texture,
					true,
					false,
					1.0f
				)
			);
		}
	}

	void EditorGui::closeTextureViewerWindowImpl(uint32_t textureId) {
		const auto& itr = mTextureViewerWindows.find(textureId);
		if (itr != mTextureViewerWindows.end()) {
			mTextureViewerWindows.erase(itr);
		}
	}

	bool EditorGui::hasTextureViewerWindowImpl(uint32_t textureId) {
		return mTextureViewerWindows.find(textureId) != mTextureViewerWindows.end();
	}

	void EditorGui::imGuiDrawTextureViewerWindowsImpl() {
		for (auto& textureViewer : mTextureViewerWindows) {
			if (textureViewer.second.Display) {
				imGuiDrawTextureViewerWindowImpl(textureViewer.second);
			}
		}
	}

	void EditorGui::imGuiDrawTextureViewerWindowImpl(ImGuiTextureViewerWindow& textureViewer) {
		ImGuiWrapper& imGuiWrapper = EditorGui::getGuiWrapper();

		const std::string title = "Texture ID: " + std::to_string(textureViewer.Texture.getId());
		const int paddingWidth = 5;
		const int paddingHeight = 5;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(paddingWidth, paddingHeight));

		if (ImGui::Begin(
			title.c_str(),
			&textureViewer.Display,
			ImGuiWindowFlags_HorizontalScrollbar
		)) {
			ImGui::Text("Size: %i, %i", textureViewer.Texture.getWidth(), textureViewer.Texture.getHeight());
			ImGui::Checkbox("Match To Window Size", &textureViewer.MatchWindowSize);
			EditorGui::displayHelpTooltip("Match texture displayed width & height to window. Still affected by scale.");
			ImGui::DragFloat("Scale", &textureViewer.Scale, 0.005f, 0.01f, 2.0f);

			glm::vec2 displaySize = { 0.0f, 0.0f };

			if (textureViewer.MatchWindowSize) {
				ImVec2 regionAvail = ImGui::GetContentRegionAvail();

				displaySize.x = regionAvail.x * textureViewer.Scale;
				displaySize.y = regionAvail.y * textureViewer.Scale;
			} else {
				displaySize.x = textureViewer.Texture.getWidth() * textureViewer.Scale;
				displaySize.y = textureViewer.Texture.getHeight() * textureViewer.Scale;
			}

			if (displaySize.y > 0.0f) {
				imGuiWrapper.drawTexture(textureViewer.Texture, displaySize);
			}
		}

		ImGui::PopStyleVar(1);
		ImGui::End();
	}

	void EditorGui::imGuiRemoveHiddenTextureViewerWindowsImpl() {
		for (auto& textureViewer : mTextureViewerWindows) {
			if (!textureViewer.second.Display) {
				mTextureViewerWindows.erase(textureViewer.first);
			}
		}
	}

	void EditorGui::imGuiDisplayHelpTooltipImpl(const char* message) {
		ImGui::SameLine();
		ImGui::Text("(?)");
		if (ImGui::IsItemHovered()) {
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(message);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}

	void EditorGui::imGuiBulletTextWrappedImpl(const char* message) {
		ImGui::Bullet();
		ImGui::SameLine();
		ImGui::TextWrapped(message);
	}

	void EditorGui::imGuiPrintDrawCallTableColumnImpl(const char* pipelineName, uint32_t drawCount) {
		//IMPORTANT:: Assumes already inside the Draw Call Count debug info table
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text(pipelineName);
		ImGui::TableNextColumn();
		ImGui::Text("%i", drawCount);
	}

	void EditorGui::imGuiPrintMat4x4Impl(const glm::mat4x4& mat, const char* name) {
		ImGui::Text(name);
		ImGui::BeginTable(name, 4);
		ImGui::TableSetupColumn("0");
		ImGui::TableSetupColumn("1");
		ImGui::TableSetupColumn("2");
		ImGui::TableSetupColumn("3");
		ImGui::TableHeadersRow();
		ImGui::TableSetColumnIndex(0);

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text("%f", mat[0][0]);
		ImGui::TableNextColumn();
		ImGui::Text("%f", mat[0][1]);
		ImGui::TableNextColumn();
		ImGui::Text("%f", mat[0][2]);
		ImGui::TableNextColumn();
		ImGui::Text("%f", mat[0][3]);

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text("%f", mat[1][0]);
		ImGui::TableNextColumn();
		ImGui::Text("%f", mat[1][1]);
		ImGui::TableNextColumn();
		ImGui::Text("%f", mat[1][2]);
		ImGui::TableNextColumn();
		ImGui::Text("%f", mat[1][3]);

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text("%f", mat[2][0]);
		ImGui::TableNextColumn();
		ImGui::Text("%f", mat[2][1]);
		ImGui::TableNextColumn();
		ImGui::Text("%f", mat[2][2]);
		ImGui::TableNextColumn();
		ImGui::Text("%f", mat[2][3]);

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text("%f", mat[3][0]);
		ImGui::TableNextColumn();
		ImGui::Text("%f", mat[3][1]);
		ImGui::TableNextColumn();
		ImGui::Text("%f", mat[3][2]);
		ImGui::TableNextColumn();
		ImGui::Text("%f", mat[3][3]);

		ImGui::EndTable();
	}

	void EditorGui::drawTextureViewerWindows() {
		INSTANCE->imGuiDrawTextureViewerWindowsImpl();
	}

	void EditorGui::openTextureViewerWindow(TextureVulkan& texture) {
		INSTANCE->openTextureViewerWindowImpl(texture);
	}

	void EditorGui::closeTextureViewerWindow(uint32_t textureId) {
		INSTANCE->closeTextureViewerWindowImpl(textureId);
	}

	void EditorGui::removeHiddenTextureViewerWindows() {
		INSTANCE->imGuiRemoveHiddenTextureViewerWindowsImpl();
	}

	bool EditorGui::hasTextureViewerWindow(uint32_t textureId) {
		return INSTANCE->hasTextureViewerWindowImpl(textureId);
	}

	void EditorGui::displayHelpTooltip(const char* message) {
		INSTANCE->imGuiDisplayHelpTooltipImpl(message);
	}

	void EditorGui::bulletTextWrapped(const char* message) {
		INSTANCE->imGuiBulletTextWrappedImpl(message);
	}

	void EditorGui::printDrawCallTableColumn(const char* pipelineName, uint32_t drawCount) {
		INSTANCE->imGuiPrintDrawCallTableColumnImpl(pipelineName, drawCount);
	}

	void EditorGui::printMat4x4(const glm::mat4x4& mat, const char* name) {
		INSTANCE->imGuiPrintMat4x4Impl(mat, name);
	}
}
