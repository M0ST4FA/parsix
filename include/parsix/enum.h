#pragma once
#include <string>
#include <ostream>

namespace m0st4fa::parsix {
	
	/**
	* @brief The type of error recovery the parser will do.
	* @attention Most of them are not implemented yet; only panic mode is implemented.
	* @todo Implement the rest of the unimplemented error recovery modes.
	* @note The effect of `ERT_NONE` is close to `ERT_PANIC_MODE` because of how the parser is architected. `ERT_PANIC_MODE` is more powerful though: whereas `ERT_NONE` can only sync in case we find a token from FIRST, `ERT_PANIC_MODE` can do a lot more, and is not limited to this only.
	*/
	enum struct ErrorRecoveryType {
		//! @brief The default recovery behavior in case of error.
		ERT_NONE = 0,
		//! @brief Choose panic-mode error recovery in case of error.
		ERT_PANIC_MODE,
		//! @brief Choose phrase-level error recovery in case of error.
		ERT_PHRASE_LEVE,
		//! @brief Choose global error recovery in case of error.
		ERT_GLOBAL,
		//! @brief Do not recover in case of error; abort the program instead.
		ERT_ABORT,
		//! @brief The number of ErrorRecoveryType enumerators.
		ERT_NUM,
	};
	std::string toString(ErrorRecoveryType);
	std::ostream& operator<<(std::ostream&, ErrorRecoveryType);
	
	/**
	 * @brief The type of an element within a production body.
	 */
	enum class ProdElementType {
		//! @brief Indicates the element is a grammar symbol.
		PET_GRAM_SYMBOL,
		//! @brief Indicates the element is a synthetic record.
		PET_SYNTH_RECORD,
		//! @brief Indicates the element is an action record.
		PET_ACTION_RECORD,
		//! @brief The number of ProdElementType enumerators.
		PET_COUNT
	};
	std::string toString(ProdElementType);
	std::ostream& operator<<(std::ostream&, ProdElementType);
}
