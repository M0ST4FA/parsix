#pragma once

#include <stdexcept>

namespace m0st4fa::parsix {

	/**
	 * @brief Value inexistent (yet). Intended to be used when a value that has yet to be calculated is retrieved.
	 */
	struct MissingValueException : public std::runtime_error {
	};


}

