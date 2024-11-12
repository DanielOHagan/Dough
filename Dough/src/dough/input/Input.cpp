#include "dough/input/Input.h"

#include "dough/input/InputCodes.h"
#include "dough/Logging.h"

namespace DOH {

	std::unique_ptr<Input> Input::INSTANCE = nullptr;

	const std::array<int, 53> Input::DEFAULT_KEY_CODES = {
		//Alphabet
		DOH_KEY_A, DOH_KEY_B, DOH_KEY_C, DOH_KEY_D, DOH_KEY_E, DOH_KEY_F,
		DOH_KEY_G, DOH_KEY_H, DOH_KEY_I, DOH_KEY_J, DOH_KEY_K, DOH_KEY_L,
		DOH_KEY_M, DOH_KEY_N, DOH_KEY_O, DOH_KEY_P, DOH_KEY_Q, DOH_KEY_R,
		DOH_KEY_S, DOH_KEY_T, DOH_KEY_U, DOH_KEY_V, DOH_KEY_W, DOH_KEY_X,
		DOH_KEY_Y, DOH_KEY_Z,

		//Numeric
		DOH_KEY_0, DOH_KEY_1, DOH_KEY_2, DOH_KEY_3, DOH_KEY_4, DOH_KEY_5,
		DOH_KEY_6, DOH_KEY_7, DOH_KEY_8, DOH_KEY_9,

		//Functions
		DOH_KEY_ESCAPE, DOH_KEY_SPACE, DOH_KEY_ENTER, DOH_KEY_LEFT_SHIFT,
		DOH_KEY_RIGHT, DOH_KEY_LEFT, DOH_KEY_UP, DOH_KEY_DOWN,
		DOH_KEY_F1, DOH_KEY_F2, DOH_KEY_F3, DOH_KEY_F4, DOH_KEY_F5
	};

	const std::array<int, 3> Input::DEFAULT_MOUSE_BUTTON_CODES = {
		DOH_MOUSE_BUTTON_LEFT, DOH_MOUSE_BUTTON_MIDDLE, DOH_MOUSE_BUTTON_RIGHT
	};

	Input::Input()
	:	mMousePosX(0.0f),
		mMousePosY(0.0f)
	{}

	void Input::init() {
		INSTANCE = std::make_unique<Input>();
	}

	void Input::close() {
		if (INSTANCE != nullptr) {
			INSTANCE->mInputLayers.clear();
			INSTANCE.reset();
			INSTANCE = nullptr;
		}
	}

	void Input::onKeyPressedEvent(int keyCode, bool pressed) {
		for (auto& inputLayer : mInputLayers) {
			if (inputLayer->isEnabled() && inputLayer->handleKeyPressed(keyCode, pressed)) {
				return;
			}
		}
	}

	void Input::onMouseButtonPressedEvent(int button, bool pressed) {
		for (auto& inputLayer : mInputLayers) {
			if (inputLayer->isEnabled() && inputLayer->handleMouseButtonPressed(button, pressed)) {
				return;
			}
		}
	}

	void Input::onMouseMoveEvent(float x, float y) {
		mMousePosX = x;
		mMousePosY = y;
		for (auto& inputLayer : mInputLayers) {
			if (inputLayer->isEnabled() && inputLayer->handleMouseMoved(x, y)) {
				return;
			}
		}
	}

	void Input::onMouseScrollEvent(float offsetX, float offsetY) {
		for (auto& inputLayer : mInputLayers) {
			if (inputLayer->isEnabled() && inputLayer->handleMouseScroll(offsetX, offsetY)) {
				return;
			}
		}
	}

	void Input::resetCycleData() {
		for (auto& inputLayer : mInputLayers) {
			inputLayer->resetCycleData();
		}
	}

	void Input::addInputLayer(std::shared_ptr<AInputLayer> inputLayer) {
		for (auto itr = INSTANCE->mInputLayers.begin(); itr != INSTANCE->mInputLayers.end(); itr++) {
			if (strcmp(itr->get()->getName(), inputLayer->getName()) == 0) {
				LOG_ERR("Input Layer add failed, name taken: " << inputLayer->getName());
				return;
			}
		}

		INSTANCE->mInputLayers.emplace_back(inputLayer);
	}

	void Input::removeInputLayer(const char* name) {
		for (auto itr = INSTANCE->mInputLayers.begin(); itr != INSTANCE->mInputLayers.end(); itr++) {
			if (strcmp(itr->get()->getName(), name) == 0) {
				INSTANCE->mInputLayers.erase(itr);
				return;
			}
		}

		LOG_WARN("Input Layer not found when trying to remove: " << name);
	}

	std::vector<std::shared_ptr<AInputLayer>>& Input::getInputLayers() {
		return INSTANCE->mInputLayers;
	}

	std::optional<std::reference_wrapper<AInputLayer>> Input::getInputLayer(const char* name) {
		for (auto itr = INSTANCE->mInputLayers.begin(); itr != INSTANCE->mInputLayers.end(); itr++) {
			if (strcmp(itr->get()->getName(), name) == 0) {
				return { *itr->get() };
			}
		}

		LOG_ERR("Failed to get Input Layer: " << name);
		return {};
	}

	std::optional<std::shared_ptr<AInputLayer>> Input::getInputLayerPtr(const char* name) {
		for (auto itr = INSTANCE->mInputLayers.begin(); itr != INSTANCE->mInputLayers.end(); itr++) {
			if (strcmp(itr->get()->getName(), name) == 0) {
				auto d = itr->get();
				return { *itr };
			}
		}

		LOG_ERR("Failed to get Input Layer: " << name);
		return {};
	}
}
