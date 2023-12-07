#include "editor/ui/EditorGui.h"

#include "dough/application/Application.h"

namespace DOH::EDITOR {

	std::unique_ptr<EditorGui> EditorGui::INSTANCE = nullptr;

	void EditorGui::init() {
		if (INSTANCE == nullptr) {
			INSTANCE = std::make_unique<EditorGui>();

			INSTANCE->mResourceViewerMap = {};
			INSTANCE->mTextureArrayViewerWindows = {};
		} else {
			LOG_WARN("EditorGui already initialised");
		}
	}

	void EditorGui::close() {
		if (INSTANCE != nullptr) {
			INSTANCE->mResourceViewerMap.clear();
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
		const auto& itr = mResourceViewerMap.find(texture.getId());
		if (itr != mResourceViewerMap.end()) {
			itr->second->Display = true;
		} else {
			mResourceViewerMap.emplace(
				texture.getId(),
				std::make_shared<ResourceViewerUiTexture>(
					texture,
					true,
					false,
					1.0f
				)
			);
		}
	}

	void EditorGui::openMonoSpaceTextureAtlasViewerWindowImpl(const MonoSpaceTextureAtlas& textureAtlas) {
		const auto& itr = mResourceViewerMap.find(textureAtlas.getId());
		if (itr != mResourceViewerMap.end()) {
			itr->second->Display = true;
		} else {
			mResourceViewerMap.emplace(
				textureAtlas.getId(),
				std::make_shared<ResourceViewerUiMonoSpaceTextureAtlas>(
					textureAtlas,
					true,
					false,
					1.0f
				)
			);
		}
	}

	void EditorGui::openIndexedTextureAtlasViewerWindowImpl(const IndexedTextureAtlas& textureAtlas) {
		const auto& itr = mResourceViewerMap.find(textureAtlas.getId());
		if (itr != mResourceViewerMap.end()) {
			itr->second->Display = true;
		} else {
			mResourceViewerMap.emplace(
				textureAtlas.getId(),
				std::make_shared<ResourceViewerUiIndexedTextureAtlas>(
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
		const auto& texItr = mResourceViewerMap.find(textureId);
		if (texItr != mResourceViewerMap.end()) {
			mResourceViewerMap.erase(texItr);
			return;
		}
	}

	void EditorGui::closeMonoSpaceTextureAtlasViewerWindowImpl(const uint32_t textureId) {
		const auto& monoSpaceTexAtlasItr = mResourceViewerMap.find(textureId);
		if (monoSpaceTexAtlasItr != mResourceViewerMap.end()) {
			mResourceViewerMap.erase(monoSpaceTexAtlasItr);
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
		return mResourceViewerMap.find(textureId) != mResourceViewerMap.end();
	}

	bool EditorGui::hasMonoSpaceTextureAtlasViewerWindowImpl(const uint32_t textureId) {
		return mResourceViewerMap.find(textureId) != mResourceViewerMap.end();
	}

	bool EditorGui::hasTextureArrayViewerWindowImpl(const char* title) {
		return mTextureArrayViewerWindows.find(title) != mTextureArrayViewerWindows.end();
	}

	void EditorGui::imGuiDrawTextureViewerWindowsImpl() {
		for (auto& resource : mResourceViewerMap) {
			if (resource.second->Display) {
				resource.second->draw(true);
			}
		}

		for (auto& texArrayViewer : mTextureArrayViewerWindows) {
			if (texArrayViewer.second.Display) {
				texArrayViewer.second.draw(true);
			}
		}
	}

	void EditorGui::imGuiRemoveHiddenTextureViewerWindowsImpl() {
		for (auto& resViewer : mResourceViewerMap) {
			if (!resViewer.second->Display) {
				mResourceViewerMap.erase(resViewer.first);
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

	void EditorGui::imGuiPrintDrawCallTableColumnImpl(const char* pipelineName, const uint32_t drawCount, const char* renderPass) {
		//IMPORTANT:: Assumes already inside the Draw Call Count debug info table
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text(pipelineName);
		ImGui::TableNextColumn();
		ImGui::Text("%i", drawCount);
		ImGui::TableNextColumn();
		ImGui::Text(renderPass);
	}

	void EditorGui::imGuiSingleUnitTimeImpl(const uint64_t nanos, const bool attachSuffix) {
		if (attachSuffix) {
			ImGui::Text("%lli %s", nanos, ETimeUnitStringsShorthand[ETimeUnit::NANOSECOND]);
		} else {
			ImGui::Text("%lli", nanos);
		}
	}

	void EditorGui::imGuiSingleUnitTimeImpl(const long micros, const bool attachSuffix) {
		if (attachSuffix) {
			ImGui::Text("%li %s", micros, ETimeUnitStringsShorthand[ETimeUnit::MICROSECOND]);
		} else {
			ImGui::Text("%li", micros);
		}
	}

	void EditorGui::imGuiSingleUnitTimeImpl(const double millis, const bool attachSuffix) {
		if (attachSuffix) {
			ImGui::Text("%lf %s", millis, ETimeUnitStringsShorthand[ETimeUnit::MILLISECOND]);
		} else {
			ImGui::Text("%lf", millis);
		}
	}

	void EditorGui::imGuiSingleUnitTimeImpl(const float seconds, const bool attachSuffix) {
		if (attachSuffix) {
			ImGui::Text("%f %s", seconds, ETimeUnitStringsShorthand[ETimeUnit::SECOND]);
		} else {
			ImGui::Text("%f", seconds);
		}
	}

	bool EditorGui::isGuiHandlingKeyboardInputImpl() {
		return ImGui::GetIO().WantCaptureKeyboard;
	}

	bool EditorGui::isGuiHandlingMouseInputImpl() {
		return ImGui::GetIO().WantCaptureMouse;
	}

	bool EditorGui::isGuiHandlingTextInputImpl() {
		return ImGui::GetIO().WantTextInput;
	}

	void EditorGui::imGuiControlsAGeometryImpl(AGeometry& geo, const char* name) {
		//TODO:: Arbitrary limits copied from an ObjDemo DragFloat3 call, change these so they aren't so limiting.

		std::string posLabel = "Pos##";
		std::string sizeLabel = "Size##";
		std::string rotationLabel = "Rotation##";
		posLabel.append(name);
		sizeLabel.append(name);
		rotationLabel.append(name);

		ImGui::DragFloat3(posLabel.c_str(), glm::value_ptr(geo.Position), 0.05f, -10.0f, 10.0f);
		ImGui::DragFloat2(sizeLabel.c_str(), glm::value_ptr(geo.Size), 0.05f, -10.0f, 10.0f);
		ImGui::DragFloat(rotationLabel.c_str(), &geo.Rotation);
		displayHelpTooltip("INFO:: Rotation not currently supported by quad renderer.");
	}

	void EditorGui::imGuiControlsQuadImpl(Quad& quad, const char* name) {
		imGuiControlsAGeometryImpl(quad, name);

		std::string colourLabel = "Colour##";
		colourLabel.append(name);
		ImGui::ColorEdit4(colourLabel.c_str(), glm::value_ptr(quad.Colour));
		const char* text = (quad.hasTexture() ? quad.getTexture().getName().c_str() : "No Texture");
		ImGui::Text("Texture: %s", text);
		//TODO:: Texture Coord edit?
	}

	void EditorGui::imGuiInfoAGeometryImpl(AGeometry& geo, const char* name) {
		ImGui::Text("Pos: X: %f, Y: %f, Z: %f", geo.Position.x, geo.Position.y, geo.Position.z);
		ImGui::Text("Size: W: %f, H: %f", geo.Size.x, geo.Size.y);
		ImGui::Text("Rotation: %f", geo.Rotation);
		displayHelpTooltip("INFO:: Rotation not currently supported by quad renderer.");
	}

	void EditorGui::imGuiInfoQuadImpl(Quad& quad, const char* name) {
		imGuiInfoAGeometryImpl(quad, name);

		float tempColour[4] = { quad.Colour.r, quad.Colour.g, quad.Colour.b, quad.Colour.a };
		std::string colourLabel = "Colour##";
		colourLabel.append(name);
		ImGui::ColorEdit4(colourLabel.c_str(), tempColour);
		EditorGui::displayHelpTooltip("ImGui doesn't have a non-edit colour picker, this doesn't change the actual colour value/");

		const char* text = (quad.hasTexture() ? quad.getTexture().getName().c_str() : "No Texture");
		ImGui::Text("Texture: %s", text);
		ImGui::TextWrapped(
			"Texture Coords: %f, %f, %f, %f, %f, %f, %f, %f",
			quad.TextureCoords[0], quad.TextureCoords[1],
			quad.TextureCoords[2], quad.TextureCoords[3],
			quad.TextureCoords[4], quad.TextureCoords[5],
			quad.TextureCoords[6], quad.TextureCoords[7]
		);
	}

	void EditorGui::imGuiJsonElementImpl(JsonElement& element, const char* name) {
		switch (element.Type) {
			case EJsonElementType::NONE:
				ImGui::Text((std::string(name) + ": ").c_str());
				ImGui::SameLine();
				ImGui::Text("null");
				break;
			case EJsonElementType::DATA_LONG:
				ImGui::Text((std::string(name) + ": ").c_str());
				ImGui::SameLine();
				ImGui::Text("%li", element.getLong());
				break;
			case EJsonElementType::DATA_DOUBLE:
				ImGui::Text((std::string(name) + ": ").c_str());
				ImGui::SameLine();
				ImGui::Text("%lf", element.getDouble());
				break;
			case EJsonElementType::DATA_BOOL:
			{
				std::string label = std::string(name) + ": ";
				ImGui::Text(label.c_str());
				ImGui::SameLine();
				ImGui::Text(element.getBool() ? "true" : "false");
				
				//ImGui checkboxes display text after (to the right of) the checkbox, commented out as it doesn't fit with the name: value format
				//ImGui::SameLine();
				//bool elementBoolVal = element.getBool();
				//ImGui::Checkbox(label.c_str(), &elementBoolVal);
				break;
			}
			case EJsonElementType::DATA_STRING:
				ImGui::Text((std::string(name) + ": ").c_str());
				ImGui::SameLine();
				ImGui::TextWrapped(("\"" + element.getString() + "\"").c_str());
				break;
			case EJsonElementType::OBJECT:
				imGuiJsonObjectImpl(element, name);
				break;
			case EJsonElementType::ARRAY:
				imGuiJsonArrayImpl(element, name);
				break;
		}
	}

	void EditorGui::imGuiJsonObjectImpl(JsonElement& object, const char* name) {
		if (ImGui::TreeNode(name)) {
			for (auto& element : object.getObject()) {
				imGuiJsonElementImpl(element.second, element.first.c_str());
			}

			ImGui::TreePop();
		}
	}

	void EditorGui::imGuiJsonArrayImpl(JsonElement& array, const char* name) {
		if (ImGui::TreeNode(name)) {
			uint32_t i = 0;
			for (auto& element : array.getArray()) {
				std::string label = std::to_string(i);
				imGuiJsonElementImpl(element, label.c_str());
				i++;
			}

			ImGui::TreePop();
		}
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

	void EditorGui::resourceViewer(AResourceViewerUi& uiViewer, bool openAsWindow) {
		switch (uiViewer.getResourceUiType()) {
			case EResourceViewerUiType::TEXTURE:
				((ResourceViewerUiTexture&) uiViewer).draw(openAsWindow);
				break;
			case EResourceViewerUiType::MONO_SPACE_TEXTURE_ATLAS:
				((ResourceViewerUiMonoSpaceTextureAtlas&) uiViewer).draw(openAsWindow);
				break;
			case EResourceViewerUiType::INDEXED_TEXTURE_ATLAS:
				((ResourceViewerUiIndexedTextureAtlas&) uiViewer).draw(openAsWindow);
				break;

			case EResourceViewerUiType::TEXTURE_ARRAY:
				((ResourceViewerUiTextureArray&) uiViewer).draw(openAsWindow);
				break;

			case EResourceViewerUiType::NONE:
			default:
				LOG_ERR("uiViewer of type:" << EResourceViewerUiTypeStrings[static_cast<uint32_t>(uiViewer.getResourceUiType())] << "can nott be drawn.");
				break;
		}
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

	void EditorGui::openIndexedTextureAtlasViewerWindow(const IndexedTextureAtlas& textureAtlas) {
		INSTANCE->openIndexedTextureAtlasViewerWindowImpl(textureAtlas);
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

	void EditorGui::printDrawCallTableColumn(const char* pipelineName, const uint32_t drawCount, const char* renderPass) {
		INSTANCE->imGuiPrintDrawCallTableColumnImpl(pipelineName, drawCount, renderPass);
	}

	void EditorGui::printMat4x4(const glm::mat4x4& mat, const char* name) {
		INSTANCE->imGuiPrintMat4x4Impl(mat, name);
	}

	void EditorGui::singleUnitTime(const uint64_t nanos, const bool attachSuffix) {
		INSTANCE->imGuiSingleUnitTimeImpl(nanos, attachSuffix);
	}

	void EditorGui::singleUnitTime(const long micros, const bool attachSuffix) {
		INSTANCE->imGuiSingleUnitTimeImpl(micros, attachSuffix);
	}

	void EditorGui::singleUnitTime(const double millis, const bool attachSuffix) {
		INSTANCE->imGuiSingleUnitTimeImpl(millis, attachSuffix);
	}

	void EditorGui::singleUnitTime(const float seconds, const bool attachSuffix) {
		INSTANCE->imGuiSingleUnitTimeImpl(seconds, attachSuffix);
	}

	bool EditorGui::isGuiHandlingKeyboardInput() {
		return INSTANCE->isGuiHandlingKeyboardInputImpl();
	}

	bool EditorGui::isGuiHandlingMouseInput() {
		return INSTANCE->isGuiHandlingMouseInputImpl();
	}

	bool EditorGui::isGuiHandlingTextInput() {
		return INSTANCE->isGuiHandlingTextInputImpl();
	}

	void EditorGui::controlsAGeometry(AGeometry& geo, const char* name) {
		INSTANCE->imGuiControlsAGeometryImpl(geo, name);
	}

	void EditorGui::controlsQuad(Quad& quad, const char* name) {
		INSTANCE->imGuiControlsQuadImpl(quad, name);
	}

	void EditorGui::infoAGeometry(AGeometry& geo, const char* name) {
		INSTANCE->imGuiInfoAGeometryImpl(geo, name);
	}

	void EditorGui::infoQuad(Quad& quad, const char* name) {
		INSTANCE->imGuiInfoQuadImpl(quad, name);
	}

	void EditorGui::jsonElement(JsonElement& element, const char* name) {
		INSTANCE->imGuiJsonElementImpl(element, name);
	}

	void EditorGui::jsonObject(JsonElement& object, const char* name) {
		INSTANCE->imGuiJsonObjectImpl(object, name);
	}

	void EditorGui::jsonArray(JsonElement& array, const char* name) {
		INSTANCE->imGuiJsonArrayImpl(array, name);
	}
}
