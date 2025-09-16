#include "editor/ui/EditorGui.h"

#include "dough/application/Application.h"
#include "editor/EditorPerspectiveCameraController.h"
#include "editor/EditorOrthoCameraController.h"

#include <tracy/public/tracy/Tracy.hpp>

namespace DOH::EDITOR {

	std::unique_ptr<EditorGui> EditorGui::INSTANCE = nullptr;

	void EditorGui::init() {
		ZoneScoped;

		if (INSTANCE == nullptr) {
			INSTANCE = std::make_unique<EditorGui>();

			INSTANCE->mResourceViewerMap = {};
			INSTANCE->mTextureArrayViewerWindows = {};
		} else {
			LOG_WARN("EditorGui already initialised");
		}
	}

	void EditorGui::close() {
		ZoneScoped;

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
		ZoneScoped;

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
		ZoneScoped;

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
		ZoneScoped;

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
		ZoneScoped;

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
		ZoneScoped;

		const auto& texItr = mResourceViewerMap.find(textureId);
		if (texItr != mResourceViewerMap.end()) {
			mResourceViewerMap.erase(texItr);
			return;
		}
	}

	void EditorGui::closeMonoSpaceTextureAtlasViewerWindowImpl(const uint32_t textureId) {
		ZoneScoped;

		const auto& monoSpaceTexAtlasItr = mResourceViewerMap.find(textureId);
		if (monoSpaceTexAtlasItr != mResourceViewerMap.end()) {
			mResourceViewerMap.erase(monoSpaceTexAtlasItr);
			return;
		}
	}

	void EditorGui::closeTextureArrayViewerWindowImpl(const char* title) {
		ZoneScoped;

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
		ZoneScoped;

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

	void EditorGui::imGuiDisplayWarningTooltipImpl(const char* message) {
		ImGui::SameLine();
		ImGui::Text("(!)");
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

	void EditorGui::imGuiControlsCircleImpl(Circle& circle, const char* name) {
		imGuiControlsAGeometryImpl(circle, name);

		std::string colourLabel = "Colour##";
		colourLabel.append(name);
		ImGui::ColorEdit4(colourLabel.c_str(), glm::value_ptr(circle.Colour));
		const char* text = (circle.hasTexture() ? circle.getTexture().getName().c_str() : "No Texture");
		ImGui::Text("Texture: %s", text);
		//TODO:: Texture Coord edit?

		std::string thicknessLabel = "Thickness##";
		thicknessLabel.append(name);
		ImGui::DragFloat(thicknessLabel.c_str(), &circle.Decorations.x, 0.005f, 0.0f, 1.0f);
		std::string fadeLabel = "Fade##";
		fadeLabel.append(name);
		ImGui::DragFloat(fadeLabel.c_str(), &circle.Decorations.y, 0.005f, 0.0001f, 1.0f);
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
			"Texture Coords: bl.x %f, bl.y %f, tr.x %f, tr.y %f",
			quad.TextureCoords[0], quad.TextureCoords[1],
			quad.TextureCoords[2], quad.TextureCoords[3]
		);
	}

	void EditorGui::imGuiInfoCircleImpl(Circle& circle, const char* name) {
		imGuiInfoAGeometryImpl(circle, name);

		float tempColour[4] = { circle.Colour.r, circle.Colour.g, circle.Colour.b, circle.Colour.a };
		std::string colourLabel = "Colour##";
		colourLabel.append(name);
		ImGui::ColorEdit4(colourLabel.c_str(), tempColour);
		EditorGui::displayHelpTooltip("ImGui doesn't have a non-edit colour picker, this doesn't change the actual colour value/");

		const char* text = (circle.hasTexture() ? circle.getTexture().getName().c_str() : "No Texture");
		ImGui::Text("Texture: %s", text);
		ImGui::TextWrapped(
			"Texture Coords: bl.x %f, bl.y %f, tr.x %f, tr.y %f",
			circle.TextureCoords[0], circle.TextureCoords[1],
			circle.TextureCoords[2], circle.TextureCoords[3]
		);

		//TODO:: Decorations
	}

	void EditorGui::imGuiControlsEditorPerspectiveCameraControllerImpl(EditorPerspectiveCameraController& cameraController, const char* name) {
		std::string uniqueSuffix = std::string("##").append(name);
		float tempPos[3] = {};
		{
			const glm::vec3 pos = cameraController.getPosition();
			tempPos[0] = pos.x;
			tempPos[1] = pos.y;
			tempPos[2] = pos.z;
		}
		if (ImGui::DragFloat3(std::string("Position").append(uniqueSuffix).c_str(), tempPos)) {
			cameraController.setPositionXYZ(tempPos[0], tempPos[1], tempPos[2]);
		}
		float tempDir[3] = {};
		{
			const glm::vec3 dir = cameraController.getDirection();
			tempDir[0] = dir.x;
			tempDir[1] = dir.y;
			tempDir[2] = dir.z;
		}
		if (ImGui::DragFloat3(std::string("Direction").append(uniqueSuffix).c_str(), tempDir)) {
			cameraController.setDirectionXYZ(tempDir[0], tempDir[1], tempDir[2]);
		}
		EditorGui::displayHelpTooltip("This is a point in 3D space where the camera is looking at.");

		//If pos and dir are in the same place the camera is "looking into itself". 
		if (tempPos[2] == tempDir[2]) {
			EditorGui::displayWarningTooltip("Position and Direction depth should NOT be equal. It causes the camera to \"look into itself\".");
		}

		float tempCursorPos[2] = {};
		{
			const glm::vec2 cursorPos = cameraController.getCursorLastPos();
			tempCursorPos[0] = cursorPos.x;
			tempCursorPos[1] = cursorPos.y;
		}
		if (ImGui::DragFloat2(std::string("CursorPos").append(uniqueSuffix).c_str(), tempCursorPos)) {
			cameraController.setCursorLastPos(tempCursorPos[0], tempCursorPos[1]);
		}
		float tempAspectRatio = cameraController.getAspectRatio();
		if (ImGui::DragFloat(std::string("Aspect Ratio").append(uniqueSuffix).c_str(), &tempAspectRatio)) {
			cameraController.setAspectRatio(tempAspectRatio);
		}
		float tempFov = cameraController.getFov();
		if (ImGui::DragFloat(std::string("FOV").append(uniqueSuffix).c_str(), &tempFov, 1.0f, 30.0f, 180.0f)) {
			cameraController.setFov(tempFov);
		}
		float tempTranslationSpeed = cameraController.getTranslationSpeed();
		if (ImGui::DragFloat(std::string("Translation Speed").append(uniqueSuffix).c_str(), &tempTranslationSpeed)) {
			cameraController.setTranslationSpeed(tempTranslationSpeed);
		}
		bool tempClickAndDragEnabled = cameraController.isClickAndDragEnabled();
		if (ImGui::Checkbox(std::string("Click & Drag").append(uniqueSuffix).c_str(), &tempClickAndDragEnabled)) {
			cameraController.setClickAndDragEnabled(tempClickAndDragEnabled);
		}
		if (tempClickAndDragEnabled) {
			ImGui::SameLine();
			ImGui::Text("Click & Drag Active: ");
			ImGui::SameLine();
			ImGui::Text(cameraController.isClickAndDragActive() ? "Active" : "Inactive");
		}


		//TODO:: How to show Camera ?
		//TODO:: How to show InputLayer ?
	}
	
	void EditorGui::imGuiControlsEditorOrthoCameraControllerImpl(EditorOrthoCameraController& cameraController, const char* name) {
		std::string uniqueSuffix = std::string("##").append(name);
		float tempPos[3] = {};
		{
			const glm::vec3 pos = cameraController.getPosition();
			tempPos[0] = pos.x;
			tempPos[1] = pos.y;
			tempPos[2] = pos.z;
		}
		if (ImGui::DragFloat3(std::string("Position").append(uniqueSuffix).c_str(), tempPos)) {
			cameraController.setPositionXYZ(tempPos[0], tempPos[1], tempPos[2]);
		}
		float tempCursorLastPos[2] = {};
		{
			const glm::vec2 cursorLastPos = cameraController.getCursorLastPos();
			tempCursorLastPos[0] = cursorLastPos.x;
			tempCursorLastPos[1] = cursorLastPos.y;
		}
		if (ImGui::DragFloat2(std::string("Cursor Pos").append(uniqueSuffix).c_str(), tempCursorLastPos)) {
			cameraController.setCursorLastPosXY(tempCursorLastPos[0], tempCursorLastPos[1]);
		}
		float tempAspectRatio = cameraController.getAspectRatio();
		if (ImGui::DragFloat(std::string("Aspect Ratio").append(uniqueSuffix).c_str(), &tempAspectRatio)) {
			cameraController.setAspectRatio(tempAspectRatio);
		}
		float tempTranslationSpeed = cameraController.getTranslationSpeed();
		if (ImGui::DragFloat(std::string("Translation Speed").append(uniqueSuffix).c_str(), &tempTranslationSpeed)) {
			cameraController.setTranslationSpeed(tempTranslationSpeed);
		}
		bool tempClickAndDragEnabled = cameraController.isClickAndDragEnabled();
		if (ImGui::Checkbox(std::string("Enable Click & Drag").append(uniqueSuffix).c_str(), &tempClickAndDragEnabled)) {
			cameraController.setClickAndDragEnabled(tempClickAndDragEnabled);
		}
		if (tempClickAndDragEnabled) {
			ImGui::SameLine();
			ImGui::Text("Click & Drag Active: ");
			ImGui::SameLine();
			ImGui::Text(cameraController.isClickAndDragActive() ? "Active" : "Inactive");
		}
		float tempZoomMin = cameraController.getZoomMin();
		if (ImGui::DragFloat(std::string("Zoom Min").append(uniqueSuffix).c_str(), &tempZoomMin)) {
			cameraController.setZoomMin(tempZoomMin);
		}
		float tempZoomMax = cameraController.getZoomMax();
		if (ImGui::DragFloat(std::string("Zoom Max").append(uniqueSuffix).c_str(), &tempZoomMax)) {
			cameraController.setZoomMax(tempZoomMax);
		}
		float tempZoomLevel = cameraController.getZoomLevel();
		if (ImGui::DragFloat(std::string("Zoom Level").append(uniqueSuffix).c_str(), &tempZoomLevel, 1.0f, tempZoomMin, tempZoomMax)) {
			cameraController.setZoomLevel(tempZoomLevel);
		}
		float tempZoomSpeed = cameraController.getZoomSpeed();
		if (ImGui::DragFloat(std::string("Zoom Speed").append(uniqueSuffix).c_str(), &tempZoomSpeed)) {
			cameraController.setZoomSpeed(tempZoomSpeed);
		}

		//TODO:: How to show Camera ?
		//TODO:: How to show InputLayer ?
	}

	void EditorGui::imGuiInfoEditorPerspectiveCameraControllerImpl(EditorPerspectiveCameraController& cameraController, const char* name) {
		const glm::vec3& pos = cameraController.getPosition();
		const glm::vec3& dir = cameraController.getDirection();
		const glm::vec2& cursorPos = cameraController.getCursorLastPos();

		ImGui::Text("Position X: %f, Y: %f, Z: %f", pos.x, pos.y, pos.z);
		ImGui::Text("Direction X: %f, Y: %f, Z: %f", dir.x, dir.y, dir.z);
		EditorGui::displayHelpTooltip("This is a point in 3D space where the camera is looking at.");
		//If pos and dir are in the same place the camera is "looking into itself".
		if (pos.z == dir.z) {
			EditorGui::displayWarningTooltip("Position and Direction depth should NOT be equal. It causes the camera to \"look into itself\".");
		}
		ImGui::Text("Cursor Position X: %f, Y: %f", cursorPos.x, cursorPos.y);
		ImGui::Text("Aspect Ratio: %f", cameraController.getAspectRatio());
		ImGui::Text("FOV: %f", cameraController.getFov());
		ImGui::Text("Translation Speed: %f", cameraController.getTranslationSpeed());
		ImGui::Text("Click & Drag Enabled: ");
		ImGui::SameLine();
		ImGui::Text(cameraController.isClickAndDragEnabled() ? "Enabled" : "Disabled");
		ImGui::SameLine();
		ImGui::Text("Click & Drag Active: ");
		ImGui::SameLine();
		const bool clickAndDragEnabled = cameraController.isClickAndDragEnabled();
		if (clickAndDragEnabled) {
			ImGui::Text("Enabled");
			ImGui::SameLine();
			ImGui::Text("Click & Drag Active: ");
			ImGui::SameLine();
			ImGui::Text(cameraController.isClickAndDragActive() ? "Active" : "Inactive");
		} else {
			ImGui::Text("Disabled");
		}

		//TODO:: How to show Camera ?
		//TODO:: How to show InputLayer ?
	}
	
	void EditorGui::imGuiInfoEditorOrthoCameraControllerImpl(EditorOrthoCameraController& cameraController, const char* name) {
		const glm::vec3& pos = cameraController.getPosition();
		const glm::vec2& cursorLastPos = cameraController.getCursorLastPos();

		ImGui::Text("Position X: %f, Y: %f, Z: %f", pos.x, pos.y, pos.z);
		ImGui::Text("Cursor Position X: %f, Y: %f", cursorLastPos.x, cursorLastPos.y);
		ImGui::Text("Aspect Ratio: %f", cameraController.getAspectRatio());
		ImGui::Text("Translation Speed: %f", cameraController.getTranslationSpeed());
		ImGui::Text("Click & Drag Enabled: ");
		ImGui::SameLine();
		const bool clickAndDragEnabled = cameraController.isClickAndDragEnabled();
		if (clickAndDragEnabled) {
			ImGui::Text("Enabled");
			ImGui::SameLine();
			ImGui::Text("Click & Drag Active: ");
			ImGui::SameLine();
			ImGui::Text(cameraController.isClickAndDragActive() ? "Active" : "Inactive");
		} else {
			ImGui::Text("Disabled");
		}
		
		ImGui::Text("Zoom Min: %f", cameraController.getZoomMin());
		ImGui::Text("Zoom Max: %f", cameraController.getZoomMax());
		ImGui::Text("Zoom Level: %f", cameraController.getZoomLevel());
		ImGui::Text("Zoom Speed: %f", cameraController.getZoomSpeed());

		//TODO:: How to show Camera ?
		//TODO:: How to show InputLayer ?
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

	bool EditorGui::imGuiIsGuiFocusedImpl() {
		return ImGui::IsAnyItemActive() ||
			ImGui::IsAnyItemFocused() ||
			(ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow) && ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow));
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
}
