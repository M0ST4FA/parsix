#include "fsm/DFA.h"
#include "lexana/LexicalAnalyzer.h"
#include "parsix/LLParser.h"

//using namespace m0st4fa; // For convenience; further, all important stuff are confined within their own namespace.

using FSMTableType = m0st4fa::fsm::FSMTable;
using TransFnType = m0st4fa::fsm::TransFn<>;
using DFAType = m0st4fa::fsm::DFA<TransFnType, std::string>;


/**
 * @brief Example for LL parser.
 * @details This example builds a parser for the language whose grammar consists of pronouns followed by a verb and then by a direct object. The pronouns, verbs and direct objects are limited by the alphabet.
 */
int main(int argc, char** argv) {
	
	// 1. building the transition function

/**
* Dead state is always 0 (fixed by the library).
* Initial state will be 1 (chosen by me).
* Final states: {2, 4}
* 2 -> First person pronoun
* 4 -> Second person pronoun
* 9 -> Third person pronoun
* 13 -> Third/second person verb
* 14 -> First person verb
* 17 -> Article
**/

	FSMTableType table{ };
	// set I, He, She and They
	table(1, 'I') = 2;

	table(1, 'H') = 3;
	table(3, 'e') = 4;

	table(1, 'S') = 5;
	table(5, 'h') = 3;

	table(1, 'T') = 6;
	table(6, 'h') = 7;
	table(7, 'e') = 8;
	table(8, 'y') = 9;

	// set play, plays
	table(1, 'p') = 10;
	table.set(10, std::string("lays"));
	/*table(10, 'l') = 11;
	table(11, 'a') = 12;
	table(12, 'y') = 13;
	table(13, 's') = 14;*/

	// set article `the`
	table(1, 't') = 15;
	table(1, 'T') = 15;
	table(15, 'h') = 16;
	table(16, 'e') = 17;

	// set noun `piano`
	table(1, 'p') = 18;
	table(1, 'i') = 19;


	for (char c = 'Z'; c <= 'z'; c++)
		table(1, c) = 2;
	for (char c = 'A'; c <= 'Z'; c++)
		table(1, c) = 2;

	// set (\w|\d|_)* following \w+
	for (char c = 'a'; c <= 'z'; c++)
		table(2, c) = 2;
	for (char c = 'A'; c <= 'Z'; c++)
		table(2, c) = 2;

	for (char c = '0'; c <= '9'; c++)
		table(2, c) = 2;

	table(2, '_') = 2;

	TransFnType transFunction{ table }; // remember: `TransFn<>` is just an abstraction, the actual table/function is `table`

	// 2. construct the DFA
	DFAType automaton{ {2, 4, 9}, transFunction };
	// here, the set of final states is {2} and `transFunction` is our transition function.

}