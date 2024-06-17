#pragma once

namespace DOH {

	static const char* EShaderStageStrings[3] = {
		"NONE",

		"VERTEX",
		"FRAGMENT"
	};

	enum class EShaderStage {
		NONE = 0,

		VERTEX,
		FRAGMENT
	};
}
