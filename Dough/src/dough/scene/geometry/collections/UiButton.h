#pragma once

#include "dough/Core.h"
#include "dough/Maths.h"
#include "dough/physics/BoundingBox2d.h"
#include "dough/scene/geometry/collections/TextString.h"
#include "dough/rendering/Colour.h"
#include "dough/input/AInputLayer.h"

namespace DOH {

	//TODO:: cache this and make it accessible to other _Menu classes
	//TODO:: Should individual games have to add these to their inputActions.json files or
	//	should the engine handle that some way? Not every game will need all these actions.
	constexpr static const char* UI_CONFIRM_LABEL = "ui-conf";
	//constexpr static const char* UI_UP_LABEL = "ui-up";
	//constexpr static const char* UI_DOWN_LABEL = "ui-down";
	//constexpr static const char* UI_LEFT_LABEL = "ui-left";
	//constexpr static const char* UI_RIGHT_LABEL = "ui-right";

	enum class EButtonState {
		NONE,

		HOVERED,	//When cursor or controller "focus" is on. Works as both hovered over & selected when using a controller/directional selection.
		PRESSED		//TODO:: Separate this into PRESS_DOWN & PRESS_UP to allow for Applications to use either?
		,
		//DISABLED ?
	};

	constexpr static std::array<const char*, 3> EButtonState_Strings = {
		"NONE",

		"HOVERED",
		"PRESSED"
	};

	struct ButtonStyle {
		glm::vec4 PlainColour;
		glm::vec4 HoveredColour;
		glm::vec4 PressedColour;
	};

	class UiButtonQuad {
	public:
		static ButtonStyle DEFAULT_STYLE;

		Quad Button;
		BoundingBox2d BoundingBox;
		std::unique_ptr<TextString> Text;
		std::reference_wrapper<ButtonStyle> Style;
		EButtonState State;
		//NOTE:: If I want more control on when, exactly, I want this button to be considered activated
		// then I can use IsActive. See places in update() where IsActive is used to show where button can be activated.
		// TODO:: Could this be an EButtonState value instead of a separate member?
		// TODO:: Since pressed is determined by inputLayer.isActionActive, IsActive HAS to be manually set to false after
		//	it has been acted upon.
		//	AND call inputLayer.consumeAction(UI_CONFIRM_LABEL) (probably best to get a reference to the action itself to save a map query)
		//		to prevent multiple btns being confirmed at the same time.
		//	AND if more than one btn is in scope then you need to track if one is already pressed to prevent more from being pressed
		//		in the same update tick.
		//bool IsActive;

		UiButtonQuad(ButtonStyle& style, glm::vec3& pos, glm::vec2& size);
		UiButtonQuad();

		void update(AInputLayer& inputLayer, glm::vec3& cursorPos);
		void render();
		void renderLines(const glm::vec4& colour = Colour::MAGENTA);
		void setPosition(glm::vec3& pos);
		void setText(const char* text, FontBitmap& fontBitmap, float scale);

		inline bool isHovered() const { return State == EButtonState::HOVERED; }
		inline bool isPressed() const { return State == EButtonState::PRESSED; }
	};
}
