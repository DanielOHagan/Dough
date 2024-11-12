#pragma once

#include "dough/Utils.h"
#include "dough/Window.h"

namespace DOH {

	struct ApplicationInitSettings {
		//Window
		std::string ApplicationName = "Dough Application";
		uint32_t WindowWidth = 1280;
		uint32_t WindowHeight = 720;
		EWindowDisplayMode WindowDisplayMode = EWindowDisplayMode::WINDOWED;

		//Application Loop
		float TargetForegroundFps = 60.0f;
		float TargetForegroundUps = 60.0f;
		bool RunInBackground = true;
		float TargetBackgroundFps = 15.0f;
		float TargetBackgroundUps = 15.0f;

		//TODO:: some kind of custom debug callback or dump

		void setWindowed(uint32_t width, uint32_t height) {
			WindowDisplayMode = EWindowDisplayMode::WINDOWED;
			WindowWidth = width;
			WindowHeight = height;
		}

		void setTargetFps(float foreground, float background = 15.0f) {
			TargetForegroundFps = foreground;
			TargetBackgroundFps = background;
		}

		void setTargetUps(float foreground, float background = 15.0f) {
			TargetForegroundUps = foreground;
			TargetBackgroundUps = background;
		}
	};
}
