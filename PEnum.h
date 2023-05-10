#pragma once
#include <string>
#include <ostream>

namespace m0st4fa {

	/**
	* @brief The type of error recovery the parser will do.
	* Most of them are not implemented yet; only panic mode is implemented.
	* The effect of `ERT_NONE` is close to `ERT_PANIC_MODE` because of how the parser is architected.
	* `ERT_PANIC_MODE` is more powerful though: whereas `ERT_NONE` can only sync in case we find a token from FIRST, `ERT_PANIC_MODE` can do a lot more, and is not limited to this only.
	*/
	enum struct ErrorRecoveryType {
		ERT_NONE = 0,
		ERT_PANIC_MODE,
		ERT_PHRASE_LEVE,
		ERT_GLOBAL,
		ERT_ABORT,
		ERT_NUM,
	};
	std::string toString(ErrorRecoveryType);
	std::ostream& operator<<(std::ostream&, ErrorRecoveryType);
	
	//! @brief The type of an element within a production body.
	enum class ProdElementType {
		PET_GRAM_SYMBOL,
		PET_SYNTH_RECORD,
		PET_ACTION_RECORD,
		PET_COUNT
	};
	std::string toString(ProdElementType);
	std::ostream& operator<<(std::ostream&, ProdElementType);
}
