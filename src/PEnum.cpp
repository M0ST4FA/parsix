#include "parsix/enum.h"

namespace m0st4fa {

	std::string toString(ErrorRecoveryType type) {
		static constexpr const char* const names[] = {
			"NONE",
			"PANIC_MODE",
			"PHRASE_LEVE",
			"GLOBAL",
			"ABORT",
		};

		const char* name = type == ErrorRecoveryType::ERT_NUM ?
			std::to_string((unsigned)ErrorRecoveryType::ERT_NUM).data() : names[static_cast<int>(type)];

		return name;
	}
	std::ostream& operator<<(std::ostream& os, ErrorRecoveryType type) {
		return os << toString(type);
	};

	std::string toString(ProdElementType type) {
		static_assert((size_t)ProdElementType::PET_COUNT == 3);
		static constexpr const char* const names[] = {
			"GRAM_SYMBOL",
			"SYNTH_RECORD",
			"ACTION_RECORD",
		};

		const char* name = type == ProdElementType::PET_COUNT ?
			std::to_string((unsigned)ProdElementType::PET_COUNT).data() : names[static_cast<int>(type)];

		return name;
	}
	std::ostream& operator<<(std::ostream& os, ProdElementType type) {
		return os << toString(type);
	};
}