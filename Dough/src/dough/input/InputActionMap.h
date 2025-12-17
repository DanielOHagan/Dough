#pragma once

#include "dough/Logging.h"
#include "dough/Core.h"

namespace DOH {

	class AInputLayer;

	constexpr static const int EDeviceInputTypeCount = 6;

	enum class EDeviceInputType {
		NONE = 0,			//Used to represnt no more action codes in InputAction.ActionCodesByDevice
		KEY_PRESS,
		MOUSE_PRESS,
		MOUSE_SCROLL_DOWN,
		MOUSE_SCROLL_UP,
		MOUSE_MOVE
		//,
		//CONTROLLER //a.k.a Gamepad
	};

	static std::array<const char*, EDeviceInputTypeCount> EDeviceInputType_Strings = {
		"NONE",
		"KEY_PRESS",
		"MOUSE_PRESS",
		"MOUSE_SCROLL_DOWN",
		"MOUSE_SCROLL_UP",
		"MOUSE_MOVE"
	};

	struct InputAction {
	public:
		constexpr static const char* JSON_ACTION_TYPE_STRING = "t";
		constexpr static const char* JSON_ACTION_VALUE_STRING = "v";

		//DeviceInputType (method of action e.g. key_press, mouse_press, etc...) with corresponding input code.
		std::array<std::pair<EDeviceInputType, int>, 3> ActionCodesByDevice;

		InputAction() = default;

		InputAction(std::initializer_list<std::pair<EDeviceInputType, int>> actionCodesByDevice) {
			uint32_t i = 0;
			for (auto& actionCode : actionCodesByDevice) {
				if (i == 3) {
					LOG_WARN("InputAction given above max action codes: " << actionCodesByDevice.size());
					break;
				}
				ActionCodesByDevice[i] = actionCode;
				i++;
			}

			//Fill remaining to NONE
			for (uint32_t remaining = i; remaining < 3; remaining++) {
				ActionCodesByDevice[remaining] = { EDeviceInputType::NONE, 0 };
			}
		}

		static EDeviceInputType getEDeviceInputTypeFromString(const char* string);
	};

	class InputActionMap {
	private:
		std::unordered_map<std::string, InputAction> mActions;

	public:
		InputActionMap();
		InputActionMap(const char* filePath);
		
		//Read from a .json file. Adds actions to map if name not already taken.
		void addActionsFromFile(const char* filePath);

		bool addAction(const char* name, InputAction& action);
		void updateAction(const char* name, InputAction& action);
		void removeAction(const char* name);
		bool hasAction(const char* name);

		bool isActionActive(const char* name, const AInputLayer& inputLayer);
		bool isActionActiveConsume(const char* name, AInputLayer& inputLayer);

		inline std::unordered_map<std::string, InputAction> getActions() { return mActions; }
	};
}
