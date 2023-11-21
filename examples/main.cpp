#include "gtest/gtest.h"

#include "fsm/DFA.h"
#include "parsix/LLParser.h"


int main(int argc, char** argv) {
	
	::testing::InitGoogleTest(&argc, argv);

	RUN_ALL_TESTS();

	return 0;
}