#pragma once

#include "dough/Maths.h"

namespace DOH {

	//List of commonly used colours for convenience.
	//8-bit RGBA normalised to float values. Alpha channel always set to 1.0f (255).
	struct Colour {
		static constexpr const glm::vec4 BLACK =		{ 0.0f,		0.0f,	0.0f,	1.0f };
		static constexpr const glm::vec4 BLUE =			{ 0.0f,		0.0f,	1.0f,	1.0f };
		static constexpr const glm::vec4 BROWN =		{ 0.647f,	0.165f,	0.165f,	1.0f };
		static constexpr const glm::vec4 CYAN =			{ 0.0f,		1.0f,	1.0f,	1.0f };
		static constexpr const glm::vec4 GREEN =		{ 0.0f,		0.5f,	0.0f,	1.0f };
		static constexpr const glm::vec4 GREY =			{ 0.5f,		0.5f,	0.5f,	1.0f };
		static constexpr const glm::vec4 INDIGO =		{ 0.294f,	0.0f,	0.51f,	1.0f };
		static constexpr const glm::vec4 LIME =			{ 0.0f,		1.0f,	0.0f,	1.0f };
		static constexpr const glm::vec4 MAGENTA =		{ 1.0f,		0.0f,	1.0f,	1.0f };
		static constexpr const glm::vec4 MAROON =		{ 0.5f,		0.0f,	0.0f,	1.0f };
		static constexpr const glm::vec4 NAVY =			{ 0.0f,		0.0f,	0.5f,	1.0f };
		static constexpr const glm::vec4 ORANGE =		{ 1.0f,		0.647f,	0.0f,	1.0f };
		static constexpr const glm::vec4 PINK =			{ 1.0f,		0.753f,	0.8f,	1.0f };
		static constexpr const glm::vec4 PURPLE =		{ 0.5f,		0.0f,	0.5f,	1.0f };
		static constexpr const glm::vec4 RED =			{ 1.0f,		0.0f,	0.0f,	1.0f };
		static constexpr const glm::vec4 SADDLE_BROWN =	{ 0.545f,	0.271f,	0.075f,	1.0f };
		static constexpr const glm::vec4 SILVER =		{ 0.753f,	0.753f,	0.753f,	1.0f };
		static constexpr const glm::vec4 WHITE =		{ 1.0f,		1.0f,	1.0f,	1.0f };
		static constexpr const glm::vec4 YELLOW =		{ 1.0f,		1.0f,	0.0f,	1.0f };
	};
}
