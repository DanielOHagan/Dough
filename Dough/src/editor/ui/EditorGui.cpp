#include "editor/ui/EditorGui.h"

#include "dough/application/Application.h"

namespace DOH::EDITOR {

	std::unique_ptr<EditorGui> EditorGui::INSTANCE = nullptr;

	void EditorGui::init() {
		if (INSTANCE == nullptr) {
			INSTANCE = std::make_unique<EditorGui>();

			INSTANCE->mTextureViewerWindows = {};
			INSTANCE->mMonoSpaceTextureAtlasViewerWindows = {};
		} else {
			LOG_WARN("EditorGui already initialised");
		}
	}

	void EditorGui::close() {
		if (INSTANCE != nullptr) {
			INSTANCE->mTextureViewerWindows.clear();
			INSTANCE->mMonoSpaceTextureAtlasViewerWindows.clear();
			INSTANCE->mTextureArrayViewerWindows.clear();
			INSTANCE.reset();
		} else {
			LOG_WARN("EditorGui already closed");
		}
	}

	ImGuiWrapper& EditorGui::getGuiWrapper() {
		return Application::get().getRenderer().getContext().getImGuiWrapper();
	}

	void EditorGui::openTextureViewerWindowImpl(const TextureVulkan& texture) {
		const auto& itr = mTextureViewerWindows.find(texture.getId());
		if (itr != mTextureViewerWindows.end()) {
			itr->second.Display = true;
		} else {
			mTextureViewerWindows.emplace(
				texture.getId(),
				ResourceViewerUiTexture(
					texture,
					true,
					false,
					1.0f
				)
			);
		}
	}

	void EditorGui::openMonoSpaceTextureAtlasViewerWindowImpl(const MonoSpaceTextureAtlas& textureAtlas) {
		const auto& itr = mMonoSpaceTextureAtlasViewerWindows.find(textureAtlas.getId());
		if (itr != mMonoSpaceTextureAtlasViewerWindows.end()) {
			itr->second.Display = true;
		} else {
			mMonoSpaceTextureAtlasViewerWindows.emplace(
				textureAtlas.getId(),
				ResourceViewerUiMonoSpaceTextureAtlas(
					textureAtlas,
					true,
					false,
					1.0f
				)
			);
		}
	}

	void EditorGui::openTextureArrayViewerWindowImpl(const char* title, const TextureArray& texArray) {
		const auto& itr = mTextureArrayViewerWindows.find(title);
		if (itr != mTextureArrayViewerWindows.end()) {
			itr->second.Display = true;
		} else {
			mTextureArrayViewerWindows.emplace(
				std::string(title),
				ResourceViewerUiTextureArray(
					title,
					texArray,
					true
				)
			);
		}
	}

	void EditorGui::closeTextureViewerWindowImpl(const uint32_t textureId) {
		const auto& texItr = mTextureViewerWindows.find(textureId);
		if (texItr != mTextureViewerWindows.end()) {
			mTextureViewerWindows.erase(texItr);
			return;
		}
	}

	void EditorGui::closeMonoSpaceTextureAtlasViewerWindowImpl(const uint32_t textureId) {
		const auto& monoSpaceTexAtlasItr = mMonoSpaceTextureAtlasViewerWindows.find(textureId);
		if (monoSpaceTexAtlasItr != mMonoSpaceTextureAtlasViewerWindows.end()) {
			mMonoSpaceTextureAtlasViewerWindows.erase(monoSpaceTexAtlasItr);
			return;
		}
	}

	void EditorGui::closeTextureArrayViewerWindowImpl(const char* title) {
		bool closed = false;
		const auto& itr = mTextureArrayViewerWindows.find(title);
		if (itr != mTextureArrayViewerWindows.end()) {
			mTextureArrayViewerWindows.erase(itr);
			closed = true;
			return;
		}
		
		if (!closed) {
			LOG_WARN("Failed to find texture array viewer: " << title);
		}
	}

	bool EditorGui::hasTextureViewerWindowImpl(const uint32_t textureId) {
		return mTextureViewerWindows.find(textureId) != mTextureViewerWindows.end() ||
			mMonoSpaceTextureAtlasViewerWindows.find(textureId) != mMonoSpaceTextureAtlasViewerWindows.end();
	}

	bool EditorGui::hasMonoSpaceTextureAtlasViewerWindowImpl(const uint32_t textureId) {
		return mMonoSpaceTextureAtlasViewerWindows.find(textureId) != mMonoSpaceTextureAtlasViewerWindows.end();
	}

	bool EditorGui::hasTextureArrayViewerWindowImpl(const char* title) {
		return mTextureArrayViewerWindows.find(title) != mTextureArrayViewerWindows.end();
	}

	void EditorGui::imGuiDrawTextureViewerWindowsImpl() {
		for (auto& textureViewer : mTextureViewerWindows) {
			if (textureViewer.second.Display) {
				textureViewer.second.drawUi();
			}
		}

		for (auto& monoSpaceTexAtlasViewer : mMonoSpaceTextureAtlasViewerWindows) {
			if (monoSpaceTexAtlasViewer.second.Display) {
				monoSpaceTexAtlasViewer.second.drawUi();
			}
		}

		for (auto& texArrayViewer : mTextureArrayViewerWindows) {
			if (texArrayViewer.second.Display) {
				texArrayViewer.second.drawUi();
			}
		}
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

	void EditorGui::openTextureViewerWindow(const TextureVulkan& texture) {
		INSTANCE->openTextureViewerWindowImpl(texture);
	}

	void EditorGui::openMonoSpaceTextureAtlasViewerWindow(const MonoSpaceTextureAtlas& textureAtlas) {
		INSTANCE->openMonoSpaceTextureAtlasViewerWindowImpl(textureAtlas);
	}

	void EditorGui::openTextureArrayViewerWindow(const char* title, const TextureArray& texArray) {
		INSTANCE->openTextureArrayViewerWindowImpl(title, texArray);
	}

	void EditorGui::closeTextureViewerWindow(const uint32_t textureId) {
		INSTANCE->closeTextureViewerWindowImpl(textureId);
	}

	void EditorGui::closeMonoSpaceTextureAtlasViewerWindow(const uint32_t textureId) {
		INSTANCE->closeMonoSpaceTextureAtlasViewerWindowImpl(textureId);
	}
	void EditorGui::closeTextureArrayViewerWindow(const char* title) {
		INSTANCE->closeTextureArrayViewerWindowImpl(title);
	}

	void EditorGui::removeHiddenTextureViewerWindows() {
		INSTANCE->imGuiRemoveHiddenTextureViewerWindowsImpl();
	}

	bool EditorGui::hasTextureViewerWindow(const uint32_t textureId) {
		return INSTANCE->hasTextureViewerWindowImpl(textureId);
	}

	bool EditorGui::hasMonoSpaceTextureAtlasViewerWindow(const uint32_t textureId) {
		return INSTANCE->hasMonoSpaceTextureAtlasViewerWindowImpl(textureId);
	}

	bool EditorGui::hasTextureArrayViewerWindow(const char* title) {
		return INSTANCE->hasTextureArrayViewerWindowImpl(title);
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
