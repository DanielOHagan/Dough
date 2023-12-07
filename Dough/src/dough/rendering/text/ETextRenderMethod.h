#pragma once

namespace DOH {

	/**
	* The type of text atlas used to render.
	*
	* See https://github.com/Chlumsky/msdf-atlas-gen#atlas-types for more information.
	*/
	enum class ETextRenderMethod {
		NONE,

		//HARD_MASK, //Hard Mask //NOT SUPPORTED!
		SOFT_MASK, //Soft Mask
		//SDF, //Signed Distance Field //NOT SUPPORTED!
		//PSDF, //Psuedo Signed Distance Field //NOT SUPPORTED!
		MSDF, //Multi-Channel Signed Distance Field
		//MTSDF //MSDF-SDF mix //NOT SUPPORTED!
	};
}
