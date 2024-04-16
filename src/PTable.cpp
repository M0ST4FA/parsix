#include <assert.h>

#include "parsix/PTable.h"

/**
 * @brief Converts this object to string.
 * @param[in] type The LRTableEntryType object to be converted to string.
 * @returns The string representation of `type`.
*/
std::string m0st4fa::toString(const LRTableEntryType type)
{
	assert((size_t)LRTableEntryType::TET_COUNT == 5);

	static const char* names[] = {
		"SHIFT",
		"REDUCE",
		"GOTO",
		"ACCEPT",
		"ERROR"
	};

	if (type == LRTableEntryType::TET_COUNT)
		return std::format("Number of types is {}", (size_t)LRTableEntryType::TET_COUNT);

	return std::string(names[(size_t)type]);
}