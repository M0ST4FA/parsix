#include "parsix/enum.h"

namespace m0st4fa::parsix {

	/**
	 * @brief Converts an ErrorRecoveryType enumerator to a string.
	 * @param[in] type An object of ErrorRecoveryType type.
	 * @returns A string representation of `type`.
	 */
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

	/**
	 * @brief Prints an ErrorRecoveryType enumerator to the standard output stream.
	 * @param[in] os The output stream to which the object is printed.
	 * @param[in] type The ErrorRecoveryType object to be printed.
	 * @return The output stream to which `type` was printed.
	 */
	std::ostream& operator<<(std::ostream& os, ErrorRecoveryType type) {
		return os << toString(type);
	};

	/**
	 * @brief Converts a ProdElementType enumerator to a string.
	 * @param[in] type An object of ProdElementType type.
	 * @returns A string representation of `type`.
	 */
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

	/**
	 * @brief Prints an ProdElementType enumerator to the standard output stream.
	 * @param[in] os The output stream to which the object is printed.
	 * @param[in] type The ProdElementType object to be printed.
	 * @return The output stream to which `type` was printed.
	 */
	std::ostream& operator<<(std::ostream& os, ProdElementType type) {
		return os << toString(type);
	};
}