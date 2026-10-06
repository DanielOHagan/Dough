#pragma once

#include <dough/Core.h>

namespace DOH {

	enum class EMusicNote {
		A = 0u,
		A_SHARP,
		B,
		C,
		C_SHARP,
		D,
		D_SHARP,
		E,
		F,
		F_SHARP,
		G,
		G_SHARP
	};

	constexpr static const std::array<const char*, 14> EMusicNote_Strings = {
		"A",
		"A_SHARP",
		"B",
		"C",
		"C_SHARP",
		"D",
		"D_SHARP",
		"E",
		"F",
		"F_SHARP",
		"G",
		"G_SHARP"
	};

	//Ordered alphabetically
	constexpr static const std::array<float, 14> MUSIC_NOTE_FREQUENCY_4 = {
		440.00f,	//A_4
		466.16f,	//A_SHARP_4
		493.88f,	//B_4
		261.63f,	//C_4
		277.18f,	//C_SHARP_4
		293.66f,	//D_4
		311.13f,	//D_SHARP_4
		329.63f,	//E_4
		349.23f,	//F_4
		369.99f,	//F_SHARP_4
		392.00f,	//G_4
		415.30f		//G_SHARP_4
	};

	//Taken from https://blog.demofox.org/2012/05/19/diy-synthesizer-chapter-2-common-wave-forms/
	inline static float getNoteFreq(EMusicNote note, float octave) {
		return static_cast<float>(
			440 * pow(
				2.0,
				(
					static_cast<double>((octave - 4) * 12 + static_cast<double>(note))
				) / 12.0
			)
		);
	}
}
