#pragma once

namespace DOH {

	//TODO:: Some kind of universal EFileType for analytics?
	struct AFileData {
	protected:
		AFileData() = default;

	public:
		AFileData(const AFileData& copy) = delete;
		AFileData operator=(const AFileData& assignment) = delete;
	};
}
