#pragma once

#include <cstdint>

namespace DOH {

	struct AudioTransferTickData {
		AudioTransferTickData(void* data, uint32_t queuedSampleCount)
		:	Data(data),
			QueuedSampleCount(queuedSampleCount)
		{}

		void* Data = nullptr;
		uint32_t QueuedSampleCount = 0;
		//TODO:: EDataType DataType; EDataType::FLOAT / INT

		inline bool isValid() const { return Data != nullptr || QueuedSampleCount == 0; }
	};
}
