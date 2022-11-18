#pragma once

namespace DOH {

	struct AFileData {
	protected:
		AFileData() = default;

	public:
		AFileData(const AFileData& copy) = delete;
		AFileData operator=(const AFileData& assignment) = delete;
	};
}
