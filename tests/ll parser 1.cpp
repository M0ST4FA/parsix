#include "gtest/gtest.h"
#include "ll parser.h"
#include "../LLParser.hpp"

using m0st4fa::LexicalAnalyzer;
using m0st4fa::LLParser;
using m0st4fa::ProductionVector;

class LLParserTests : public LLParserInfo {

protected:
	
	static TokenType tokenFactory_id (const FSMStateType state, std::string_view lexeme) {
		
		switch (state) {

		case 2:
			return TokenType{ Terminal::T_ID, lexeme };

		case 3:
			return TokenType{ Terminal::T_EQUAL, lexeme };

		case 4:
			return TokenType{ Terminal::T_NUM, lexeme };

		default:
			Logger{}.log(LoggerInfo::FATAL_ERROR, std::format("state {} is not recognized as a final state in function `tokenFactory_id`.", state));
			std::abort();
		}

	};


};

TEST_F(LLParserTests, id_eq_num) {

	// Data structures	
	typename FSMTableType table{};
	this->initTranFn_id_eq_num(table);
	typename TranFn tranFn{ table };
	DFAType testFSM{ {2, 3, 4}, tranFn };


	// strings
	std::string_view str1 = "x = 10";
	std::string_view str2 = "newid \n newidverylong \n = \n 555";
	std::string_view str3 = "x = 10\ny = 50\nz = x + y";
	std::string_view str4 = "aabb\naabbb\naabb\nabb";

	
	LexicalAnalyzerType lexicalAnalyzer{ testFSM, tokenFactory_id, str1 };

	/*
	 * Grammar:
	 * E -> ID = NUM\nE
	 * E -> EPSILON
	 * E -> EOF
	 */
	// YOU NEED TO FIRST FINISH THE PARSER GENERATOR LIBRARY TO BE ABLE TO USE IT HERE
	// ADD TO IT THE FEATURE OF BEING ABLE TO WRITE THE TABLE TO A FILE
	LLParsingTableType parsingTable{};
	SymbolType startSymbol;


}
