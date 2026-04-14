#include "dough/scene/geometry/collections/UiButton.h"

#include "dough/rendering/ShapeRenderer.h"
#include "dough/rendering/LineRenderer.h"
#include "dough/rendering/text/TextRenderer.h"

namespace DOH {

	ButtonStyle UiButtonQuad::DEFAULT_STYLE = { Colour::WHITE, Colour::GREY, Colour::LIME };

	UiButtonQuad::UiButtonQuad(ButtonStyle& style, glm::vec3& pos, glm::vec2& size)
	:	Button(pos, size, style.PlainColour),
		BoundingBox(Button),
		Style(style),
		State(EButtonState::NONE)
		//IsActive(false)
	{}

	UiButtonQuad::UiButtonQuad()
	:	Button({}, {}, UiButtonQuad::DEFAULT_STYLE.PlainColour),
		BoundingBox(Button),
		Style(UiButtonQuad::DEFAULT_STYLE),
		State(EButtonState::NONE)
		//IsActive(false)
	{
		Style = DEFAULT_STYLE;
	}

	void UiButtonQuad::update(AInputLayer& inputLayer, glm::vec3& cursorPos) {
		
		//TODO:: Some kind of inputLayer.getUniversalActions() ? Something to prevent having to call
		//	.isActionActive_ with a const char* as that requires a map.find call. Much better to have a ref to the actions
		//	and pass that to the input layer.

		ButtonStyle& style = Style.get();
		if (State != EButtonState::PRESSED) {
			if (BoundingBox.isVec2Inside(cursorPos)) {
				if (State != EButtonState::HOVERED) {
					//onHoverOn
					Button.Colour = style.HoveredColour;
					State = EButtonState::HOVERED;
				}

				if (inputLayer.isActionActiveAND(UI_CONFIRM_LABEL)) {
					//onPressDown
					State = EButtonState::PRESSED;
					Button.Colour = style.PressedColour;

					//Activate when button is pressed down.
					//IsActive = true;
				}
			} else {
				if (isHovered()) {
					//onHoverOff
					State = EButtonState::NONE;
					Button.Colour = style.PlainColour;
				}
			}
		} else {
			//if menu confirm action is still active then keep button "pressed" even if cursor/selection method moves off 
			if (!inputLayer.isActionActiveAND(UI_CONFIRM_LABEL)) {
				//onPressOff

				inputLayer.consumeAction(UI_CONFIRM_LABEL);

				if (BoundingBox.isVec2Inside(cursorPos)) {
					//still hovering
					Button.Colour = style.HoveredColour;
					State = EButtonState::HOVERED;
				} else {
					//no longer hovering
					Button.Colour = style.PlainColour;
					State = EButtonState::NONE;
				}

				//Activate when button press is released.
				//IsActive = true;
			} else {
				//onPressHold

				//NOTE:: Could have a button that requires holding for a certain amount of time.
				//e.g. if (HoldTime > HoldActivatetime) IsActive = true;

				if (!BoundingBox.isVec2Inside(cursorPos)) {
					//no longer pressing on button. Cursor MUST be on button to be pressed
					Button.Colour = style.PlainColour;
					State = EButtonState::NONE;

					inputLayer.consumeAction(UI_CONFIRM_LABEL);
				}
			}
		}
	}

	void UiButtonQuad::render() {
		ShapeRenderer::drawQuadUi(Button);
		if (Text != nullptr) TextRenderer::drawTextStringUi(*Text);
	}

	void UiButtonQuad::renderLines(const glm::vec4& colour) {
		LineRenderer::drawQuadUi(Button, colour);
		if (Text != nullptr) {
			for (Quad& quad : Text->getQuads()) {
				LineRenderer::drawQuadUi(quad, colour);
			}
		}
	}

	void UiButtonQuad::setPosition(glm::vec3& pos) {
		Button.Position = pos;
		BoundingBox.setPosition(pos);
		if (Text != nullptr) Text->setRoot(pos);
	}

	void UiButtonQuad::setText(const char* text, FontBitmap& fontBitmap, float scale) {
		Text = std::make_unique<TextString>(text, fontBitmap, scale);
		Text->setRoot(Button.Position);
	}
};
