#include "gtest/gtest.h"

#include "../DFA.h"
#include "../LLParser.hpp"


int main(int argc, char** argv) {
	
	::testing::InitGoogleTest(&argc, argv);

	RUN_ALL_TESTS();

	return 0;
}