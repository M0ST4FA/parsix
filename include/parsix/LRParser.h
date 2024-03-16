#pragma once
#include <vector>

#include "Parser.h"

namespace m0st4fa::parsix {

	/**
	 * @brief An LR parser.
	 * @tparam GrammarT The type of the grammar object used by the parser. This type represents a vector of production objects.
	 * @tparam LexicalAnalyzerT The type of the lexical analyzer object used by the parser.
	 * @tparam SymbolT The type of grammar symbol objects of the language of the parser.
	 * @tparam StateT The type of an LR parsing state.
	 * @tparam ParsingTableT The type of the parsing table object used by the parser.
	 * @tparam FSMTableT The type of the finite state machine table used by the state machine that is used by the  lexical analyzer that is used by the parser.
	 * @tparam InputT The type of the input string.
	 */
	template <typename GrammarT, typename LexicalAnalyzerT,
		typename SymbolT, typename StateT,
		typename ParsingTableT,
		typename FSMTableT = fsm::FSMTable,
		typename InputT = std::string>
	class LRParser : public Parser<LexicalAnalyzerT, SymbolT, ParsingTableT, FSMTableT, InputT> {

		/**
		 * @brief Alias for the base parser class template.
		 *
		 * @details Defines a type for the base parser using the specified template parameters.
		 */
		using ParserBase = Parser<LexicalAnalyzerT, SymbolT, ParsingTableT, FSMTableT, InputT>;

		/**
		 * @brief Alias for the production type within a grammar.
		 *
		 * @details Represents the type of a production in the grammar, typically a rule defining how symbols can be replaced.
		 */
		using ProductionType = decltype(GrammarT{}.at(0));

		/**
		 * @brief Alias for the symbol type within a production.
		 *
		 * @details Represents the type of a symbol, which can be a terminal or a non-terminal in the context of a grammar production.
		 */
		using SymbolType = decltype(ProductionType{}.prodHead);

		/**
		 * @brief Alias for the terminal symbol type.
		 *
		 * @details Represents the type of a terminal symbol within the grammar's symbol type.
		 */
		using TerminalType = decltype(SymbolType{}.as.terminal);

		/**
		 * @brief Alias for the non-terminal variable type.
		 *
		 * @details Represents the type of a non-terminal variable within the grammar's symbol type.
		 */
		using VariableType = decltype(SymbolType{}.as.nonTerminal);

		/**
		 * @brief Alias for the stack element type.
		 *
		 * @details Represents the type of an element that can be stored on the parser's stack.
		 */
		using StackElementType = StateT;

		/**
		 * @brief Alias for the data type associated with a stack element.
		 *
		 * @details Represents the type of data that can be associated with a stack element.
		 */
		using DataType = decltype(StackElementType{}.data);

		/**
		 * @brief Alias for the stack type used in the parser.
		 *
		 * @details Represents the type of stack used for storing elements during parsing.
		 */
		using StackType = StackType<StackElementType>;

		/**
		 * @brief Alias for the token type produced by the lexical analyzer.
		 *
		 * @details Represents the type of token that the lexical analyzer can return.
		 */
		using TokenType = decltype(LexicalAnalyzerT{}.getNextToken());

		/**
		 * @brief Mutable member representing the parser's stack.
		 *
		 * Initialized to an empty stack with a starting capacity of 0.
		 */
		mutable StackType m_Stack{ 0 };

		/**
		 * @brief Mutable member representing the current top state of the parser.
		 *
		 * @details Initialized to the starting state of the parser.
		 */
		mutable StackElementType m_CurrTopState{ START_STATE };

		/**
		 * @brief Mutable member representing the current input token being processed.
		 *
		 * @details Initialized to a default-constructed instance of the token type.
		 */
		mutable TokenType m_CurrInputToken{ TokenType{} };

		/**
		 * @brief Resets the parser state.
		 *
		 * @details This function resets the parser state. It clears the parser's stack, sets the current top state to the starting state, and sets  the current input token to a default-constructed instance of the token type. It also logs the resetting process.
		 *
		 * @return void
		 */
		void _reset_parser_state() const {
			this->p_Logger.log(LoggerInfo::INFO, "RESETTING PARSER.");
			m_Stack.clear();
			m_CurrTopState = START_STATE;
			m_CurrInputToken = TokenType{};
		}

		void _reduce(size_t);

		/**
		 * @brief Pushes a state object onto the stack.
		 *
		 * @param state The state object to be pushed onto the stack. It must be of type
		 *        `StateT`.
		 *
		 * @post
		 * - The `state` is appended to the `m_Stack` member variable.
		 * - The `m_CurrTopState` member variable is updated to point to the top of
		 *   the stack.
		 * - A log message is generated using the `p_Logger` member variable
		 *   indicating that the state was pushed and containing the current state
		 *   and the contents of the stack. The log message level is `INFO`.
		 * 
		 * @returns void
		 */
		void _push_state(const StateT& state) {
			this->m_Stack.push_back(state);
			this->m_CurrTopState = this->m_Stack.back();

			this->p_Logger.log(LoggerInfo::INFO, std::format("Pushing state {}\nCurrent stack: {}", (std::string)state, toString(this->m_Stack)));
		}

		/**
		 * @brief Pops a state object from the internal state storage.
		 *
		 * @throws std::runtime_error If the state storage is empty and attempting to pop results in an underflow condition.
		 *
		 * @post
		 * - If successful, the top element is removed from the `m_Stack` member.
		 * - The `m_CurrTopState` member is updated to point to the new top of the stack.
		 * - A log message is generated using the `p_Logger` member variable indicating that the state was popped and containing the current state (obtained internally). The log message level is `INFO`.
		 * 
		 * @returns void. This function does not return a value.
		 */
		void _pop_state() {
			if (this->m_Stack.size() <= 1) {
				std::string msg = std::format("Cannot pop more states from the LR stack. The stack cannot reach an empty stated.");

				this->p_Logger.log(LoggerInfo::ERR_STACK_UNDERFLOW, msg);

				throw std::runtime_error((std::string)"Stack underflow: " + msg);
			}

			this->m_CurrTopState = this->m_Stack.back();
			this->p_Logger.log(LoggerInfo::INFO, std::format("Popping state {}\nCurrent stack: {}", (std::string)this->m_CurrTopState, toString(this->m_Stack)));

			this->m_Stack.pop_back();
		}

		/**
		 * @brief Pops a specified number of states from the parser's stack.
		 *
		 * @param num The number of states to be popped from the stack.
		 *
		 * This function pops a specified number of states from the parser's stack. 
		 * It checks if the stack size is less than the number of states to be popped plus one. If so, it logs an error message and throws a runtime error. Otherwise, it creates a temporary stack with the states to be popped, erases these states from the original stack, logs the popped states and the current stack, and sets the current top state to the back of the stack.
		 *
		 * @throws std::runtime_error If the number of states to be popped is greater than the current stack  size.
		 *
		 * @return void
		 */
		void _pop_states(size_t num) {
			if (this->m_Stack.size() < num + 1) {
				std::string msg = std::format("Cannot pop {} states from the LR stack. The stack cannot reach an empty stated.", num);

				this->p_Logger.log(LoggerInfo::ERR_STACK_UNDERFLOW, msg);

				throw std::runtime_error((std::string)"Stack underflow: " + msg);
			}

			const auto end = this->m_Stack.end();

			StackType temp{ end - num, end };

			this->m_Stack.erase(end - num, end);
			this->p_Logger.log(LoggerInfo::INFO, std::format("Popping states {}\nCurrent stack: {}", toString(temp), toString(this->m_Stack)));

			this->m_CurrTopState = this->m_Stack.back();
		}

		bool _check_and_resolve_parsing_errors(size_t, ErrorRecoveryType);

		/**
		 * @brief Performs error recovery in panic mode.
		 *
		 * This function performs error recovery in panic mode.
		 * It goes through the stack top-down and considers each state.
		 * For every non-terminal, it checks if the state has a goto on the non-terminal and records the non-terminal. If no state is found with at least a single goto entry on some non-terminal, it logs a  fatal error and aborts the program (as synchronization is impossible at this point).
		 * @todo Handle the case where no state is found with at least a single goto entry on some non-terminal more professionally. Right now it just aborts after printing a message.
		 * Otherwise, it loops through the remaining terminals of the input. For each terminal, it checks whether the terminal is in FOLLOW(nonTerminal). If so, it shifts GOTO[S][V] and return (synchronization complete). If not, it continues on to the next token.
		 * If it couldn't synchronize, it logs an error message and throws a logic error.
		 *
		 * @throws std::logic_error If unable to synchronize.
		 *
		 * @return void
		 */
		void _error_recov_panic_mode() {
			/** Algorithm
			* Go through the stack top-down and consider state S, the top on the stack:
			* For every non-terminal V:
			* If S has a goto on non-terminal V, record non-terminal V.
			* See whether any non-terminal V has been record for S:
			* If so, found = true and break.
			* If this is not the case, pop S and continue on to the next state on top of the stack.

			* If (not found) ERROR(could not synchronize).

			* For each token left on the input T:
			* If reachedEnd == true, break.              // the order is important
			* If T == EOF, set reachedEnd = true.        // between these two steps
			* For every non-terminal V, check whether T is in FOLLOW(V):
			* If So, shift GOTO[S][V] and set synchronized = true and break.
			* If not, continue on to the next token T.

			* If (not synchronized) ERROR(could not synchronize).
			*/

			auto rbegin = this->m_Stack.rbegin();
			auto rend = this->m_Stack.rend();

			std::vector<VariableType> nonTerminals;
			bool found = false;

			// find a state with at least a single GOTO entry on some non-terminal
			// record all non-terminals on which it has a GOTO
			for (; rbegin < rend; rbegin = this->m_Stack.rbegin()) {
				auto currState = *rbegin;
				size_t stateNum = currState.state;

				// get all non-error goto entries for this state
				nonTerminals = this->p_Table.getGotos(stateNum);

				// if this state has at least a single GOTO
				// we have found the state we seek; break in this case
				if (!nonTerminals.empty()) {
					found = true;
					break;
				}

				// if this state does not have any GOTOs,
				// we still didn't find the state we seek
				this->m_Stack.pop_back();
			}

			// if no state was found
			if (!found) {
				this->p_Logger.log(LoggerInfo::FATAL_ERROR, "Unable to synchronize! TODO: implement professional handling of this case.");
				std::abort();
			}

			// if some state could be found

			// loop through the remaining terminals of the input

			bool hasReachedEnd = false;
			for (; ; this->m_CurrInputToken = this->get_next_token()) {
				if (hasReachedEnd)
					break;

				hasReachedEnd = this->m_CurrInputToken == TokenType::TEOF;

				TerminalType currT = this->m_CurrInputToken.name;
				auto toSymbol = [](const TerminalType t) {
					return SymbolT{ .isTerminal = true, .as {.terminal = t} };
				};

				for (VariableType nonTerminal : nonTerminals) {
					const auto& follow = this->p_Table.grammar.getFOLLOW(nonTerminal);

					// check whether the current terminal is in FOLLOW(nonTerminal)
					// if it does not exist
					if (bool exists = isIn(toSymbol(currT), follow); !exists)
						continue;

					const StackElementType& topState = this->m_Stack.back();
					this->p_Logger.log(LoggerInfo::DEBUG, std::format("Synchronized with:\n Top state {}\nNon-terminal {}\nTerminal", topState.toString(), toString(nonTerminal), toString(currT)));

					LRTableEntry entry = this->p_Table.atGoto(topState.state, nonTerminal);
					assert(not entry.isError());

					StackElementType newState{ entry.number };
					_push_state(newState);
					return;
				}
			}
		}

		bool _take_parsing_action(auto&);
	protected:

		/**
		 * @brief The start state of the state machine used by the parser.
		 */
		static const StateT START_STATE;

	public:

		/**
		 * @brief Default constructor for LRParser.
		 *
		 * @details This is the default constructor for the LRParser class. It initializes a new instance of the class with no parameters.
		 */
		LRParser() = default;

		/**
		 * @brief Parameterized constructor for LRParser.
		 *
		 * @param lexer The lexical analyzer to be used by the parser.
		 * @param parsingTable The parsing table to be used by the parser.
		 * @param startSymbol The start symbol for the grammar.
		 *
		 * @details This constructor initializes a new instance of the LRParser class using the provided lexical analyzer, parsing table, and start symbol. It also calculates the FIRST and FOLLOW sets for the grammar.
		 */
		LRParser(LexicalAnalyzerT& lexer, const ParsingTableT& parsingTable, const SymbolT& startSymbol) : ParserBase{ lexer, parsingTable, startSymbol } {
			this->p_Table.grammar.calculateFIRST();
			this->p_Table.grammar.calculateFOLLOW();
		};

		/**
		 * @brief Copy constructor for LRParser.
		 *
		 * @param other The LRParser instance to be copied.
		 *
		 * @details This is the copy constructor for the LRParser class. It initializes a new instance of the class that is a copy of an existing instance.
		 */
		LRParser(const LRParser& other) = default;

		/**
		 * @brief Copy assignment operator for LRParser.
		 *
		 * @param rhs The LRParser instance to be copied.
		 *
		 * @details This is the copy assignment operator for the LRParser class. It copies the state of an existing instance into the current instance and returns a reference to the current instance.
		 *
		 * @return A reference to the current instance.
		 */
		LRParser& operator=(const LRParser& rhs) {
			this->ParserBase::operator=(rhs);
			this->m_Stack = rhs.m_Stack;
			this->m_CurrTopState = rhs.m_CurrTopState;
			this->m_CurrInputToken = rhs.m_CurrInputToken;

			return *this;
		};


		template<typename ParserResultT = ParserResult>
		ParserResultT parse(const ParserResultT&, ErrorRecoveryType = ErrorRecoveryType::ERT_NONE);
	};

	template<typename GrammarT, typename LexicalAnalyzerT, typename SymbolT, typename StateT, typename ParsingTableT, typename FSMTableT, typename InputT>
	const StateT LRParser<GrammarT, LexicalAnalyzerT, SymbolT, StateT, ParsingTableT, FSMTableT, InputT>::START_STATE{ 0 };

	// IMPLEMENTATION

	/**
	 * @brief Checks for the existence of and resolves (if possible) parsing errors.
	 *
	 * @param errorNum The number of errors encountered so far.
	 * @param errorRecoveryType The type of error recovery to be used.
	 *
	 * @details This function checks for parsing errors and attempts to resolve them. 
	 * It gets the current state number, current token name, and current table entry. 
	 * It starts by checking the current entry: if it is not empty nor an error, it returns `false`. This indicates that no error has been found (which also means that no error has been resolved).
	 * It then checks if the number of errors from which it was recovered has already reached the maximum limit. If so, it logs an error message and throws a logic error.
	 * At this point, either the current entry is empty or is an error. 
	 * If the current entry is empty, it logs an error message and proceeds to error recovery. Otherwise, it proceeds to error recovery immediately. 
	 * If error	recovery is not enabled, it logs an error message and throws a logic error. This is to abort the parsing, as we can no longer continue.
	 * If error recovery is enabled, it switches on the error recovery type and performs the appropriate error recovery action. 
	 * If the error was resolved, it returns `true`. `true` indicates an error was found and was resolved.
	 *
	 * @throws std::logic_error If the error recovery limit is exceeded, or if the string does not belong to the grammar (because there's an error that the parser cannot recover from), or if an unsupported error recovery type is used.
	 * @todo Improve exception throwing. Throw a distinct type for each case.
	 *
	 * @return `true` if there has an been an error AND the error was resolved; `false` otherwise (there hasn't been an error or there has been an error that couldn't be resolved).
	 */
	template<typename GrammarT, typename LexicalAnalyzerT, typename SymbolT, typename StateT, typename ParsingTableT, typename FSMTableT, typename InputT>
	inline bool LRParser<GrammarT, LexicalAnalyzerT, SymbolT, StateT, ParsingTableT, FSMTableT, InputT>::_check_and_resolve_parsing_errors(size_t errorNum, ErrorRecoveryType errorRecoveryType)
	{

		TerminalType currTokenName = this->m_CurrInputToken.name;
		size_t currStateNum = this->m_CurrTopState.state;
		LRTableEntry currEntry = this->p_Table.atAction(currStateNum, currTokenName);

		if (!(currEntry.isEmpty || currEntry.type == LRTableEntryType::TET_ERROR))
			return false;

		const std::string_view src = this->get_source_code();
		// check we have not reached the maximum number of encountered errors
		if (errorNum == ParserBase::ERR_RECOVERY_LIMIT) {
			this->p_Logger.log(LoggerInfo::ERR_RECOV_LIMIT_EXCEEDED, std::format("Maximum number of errors to recover from is `{}` which has been exceeded.", ParserBase::ERR_RECOVERY_LIMIT));
			throw std::logic_error("Error recovery limit exceeded!");
		}

		errorNum++;

		if (currEntry.isEmpty) {
			std::string msg{ std::format("LR parsing table entry is empty!\nCurrent stack: {}\nCurrent token: {}\nCurrent input: {}", toString(this->m_Stack), m_CurrInputToken.toString(), src)};
			this->p_Logger.log(LoggerInfo::ERR_INVALID_TABLE_ENTRY, msg);
		}

		// if error recovery is not enabled
		if (errorRecoveryType == ErrorRecoveryType::ERT_NONE) {
			std::string msg = std::format("Cannot continue further with the parse! Error entry encountered; It looks like this string does not belong to the grammar.\nCurrent stack: {}\n Current input: {}", toString(this->m_Stack), src);
			this->p_Logger.log(LoggerInfo::ERR_UNACCEPTED_STRING, msg);

			throw std::logic_error("Cannot continue further with the parse! Error entry encountered; It looks like this string does not belong to the grammar.");
		}

		// if error recovery is enabled, switch on the type
		switch (errorRecoveryType) {
		case ErrorRecoveryType::ERT_PANIC_MODE: {
			_error_recov_panic_mode();
			break;
		}
		default: {
			std::string errMsg = std::format("Cannot continue further with the parse! Error entry encountered; It looks like this string does not belong to the grammar.\nCurrent stack: {}\n Current input: {}", toString(this->m_Stack), src);
			std::string noteMsg = std::format("Error recovery type `{}` is not yet supported for LR parsing.", toString(errorRecoveryType));
			std::string fullMsg = std::format("{}\nNote: ", errMsg, noteMsg);

			this->p_Logger.log(LoggerInfo::ERR_UNACCEPTED_STRING, fullMsg);
			throw std::logic_error("Cannot continue further with the parse! Error entry encountered; It looks like this string does not belong to the grammar.");
		}
		}

		// if the error was resolved
		return true;
	};

	/**
	 * @brief Reduces the production based on the current state and token.
	 * 
	 * @throws std::logic_error If it comes across an entry in the parsing table that is not of type `GOTO`.
	 *
	 * @param prodNumber The number of the production to be reduced.
	 *
	 * @details This function reduces the production based on the current state and token. 
	 * It gets the production from the grammar at the given production number. 
	 * It then executes the action associated with the production if any. 
	 * It determines the length of the body of the production, pops that many elements from the top of the  stack, and gets the next entry. 
	 * It checks if the current entry type is not 'GOTO'. If so, it logs an error message and throws a logic error. Otherwise, it pushes the new state onto the stack.
	 *
	 * @return void
 */
	template<typename GrammarT, typename LexicalAnalyzerT, typename SymbolT, typename StateT, typename ParsingTableT, typename FSMTableT, typename InputT>
	inline void LRParser<GrammarT, LexicalAnalyzerT, SymbolT, StateT, ParsingTableT, FSMTableT, InputT>::_reduce(size_t prodNumber)
	{
		const std::string_view src = this->get_source_code();

		// get the production
		const auto& production = this->p_Table.grammar.at(prodNumber);

		StackElementType newState = StackElementType{};

		// execute the action, if any
		if (auto action = static_cast<void(*)(StackType&, StackElementType&)>(production.postfixAction); action != nullptr)
			action(this->m_Stack, newState);

		// determine the length of the body of the production
		const size_t prodBodyLength = production.prodBody.size();

		// pop prodBodyLength elements from the top of the stack and get the next entry
		this->_pop_states(prodBodyLength);
		size_t stateNum = this->m_CurrTopState.state;
		LRTableEntry currEntry = this->p_Table.atGoto(stateNum, production.prodHead.as.nonTerminal);
		newState.state = currEntry.number;

		// Note: errors are never detected when consulting the GOTO table
		// this here is just a precaution for possible (probably logic) bugs
		if (currEntry.type != LRTableEntryType::TET_GOTO) {
			std::string msg{ std::format("Incorrect entry type! Expected type `GOTO` within function reduce after accessing the GOTO table.\nCurrent stack: {}\n Current input: {}", toString(this->m_Stack), src) };
			this->p_Logger.log(LoggerInfo::ERR_INVALID_VAL, msg);

			throw std::logic_error("Incorrect entry type! Expected type `GOTO` within function reduce after accessing the GOTO table.");
		}

		// if the current entry is not an error
		this->_push_state(newState);
	}

	/**
	 * @brief Takes the parsing action based on the current state and token.
	 *
	 * @tparam ParserResultT The type of the parser result.
	 * @param result The accumulative result of the parser up to the moment of the call. This function adds more to the aggregate. 
	 * 
	 * @note Right now, the `result` argument is not affected, but this behavior may change in the future.
	 *
	 * @details This function takes the parsing action based on the current state and token. 
	 * It gets the current state number, current token name, and current table entry. It then switches on the type of the current entry. 
		* If the entry type is shift, it pushes the state onto the stack and gets the next input token.
		* If the entry type is reduce, it reduces the current entry number. 
		* If the entry type is accept, it gets the production,executes the action if any, and returns `true`.  
		* If the entry type is none of the above, it logs a fatal error and	aborts the program. 
	 * The function returns `false` (means that the parser didn't accept) after every action, except the action where it accepts. In this case, it returns `true` to indicate acceptance.
	 *
	 * @return `true` if the parser has accepted after taking the action; `false` otherwise.
 */
	template<typename GrammarT, typename LexicalAnalyzerT, typename SymbolT, typename StateT, typename ParsingTableT, typename FSMTableT, typename InputT>
	inline bool LRParser<GrammarT, LexicalAnalyzerT, SymbolT, StateT, ParsingTableT, FSMTableT, InputT>::_take_parsing_action(auto& result)
	{
		using ParserResultType = decltype(result);

		size_t currStateNum = this->m_CurrTopState.state;
		TerminalType currTokenName = this->m_CurrInputToken.name;
		LRTableEntry currEntry = this->p_Table.atAction(currStateNum, currTokenName);
		const std::string_view src = this->get_source_code();

		switch (currEntry.type)
		{
		case LRTableEntryType::TET_ACTION_SHIFT: {
			StateT s = StateT{ currEntry.number };
			s.token = this->m_CurrInputToken;
			this->_push_state(s);
			this->m_CurrInputToken = this->get_next_token();
			break;
		}

		case LRTableEntryType::TET_ACTION_REDUCE:
			_reduce(currEntry.number);
			break;

		case LRTableEntryType::TET_ACCEPT: {
			// get the production
			const auto& production = this->p_Table.grammar.at(0);

			StackElementType newState = StackElementType{};

			// execute the action, if any
			if (auto action = static_cast<void(*)(StackType&, StackElementType&, ParserResultType&)>(production.postfixAction); action != nullptr)
				action(this->m_Stack, newState, result);
			else
				std::cout << "ACCEPTED!";

			return true;
		}

		default: // TODO: ENHANCE THIS
			this->p_Logger.log(LoggerInfo::FATAL_ERROR, std::format("[Unreachable] Invalid entry type `{}` on switch statement!\nCurrent stack: {}\n Current input: {}", toString(currEntry.type), toString(this->m_Stack), src));
			std::string srcLoc = this->p_Logger.getCurrSourceLocation();
			assert(std::format("Source code location:\n{}", srcLoc).data());
			std::abort();
		}

		return false;
	}

	/**
	 * @brief Parses an input stream using the LR parsing algorithm.
	 * 
	 * @tparam ParserResultT The type of the result of the parser.
	 * @param[in] initResult The initial parser result. Right now, this also sets the return value by the function (i.e., this is also the returned result of this function).
	 * @param[in] errorRecoveryType The type of recovery technique to use in case of an error.
	 * 
	 * @note Right now, the `result` argument is not affected, but this behavior may change in the future.
	 * @todo Implement returned results via `ParserResultT` objects. The result could include a pointer to the parsing tree, semantic information (especially if it returns an annotated tree) and error messages. Right now, the parser returns an empty object.
	 *
	 * @details This function implements the core parsing logic using the LR parsing algorithm.
	 * It iterates through the input stream using a loop, performing actions based on the current state on the stack and the next token from the input.
	 * The function employs a combination of shift, reduce, and goto actions defined by the parsing tables to construct the parse tree or identify errors.
	 * Error recovery is handled based on the specified `errorRecoveryType`.
	 *
	 * @returns The result of the parsing. Right now, it just returns `initResult`.
	 */
	template<typename GrammarT, typename LexicalAnalyzerT, typename		SymbolT, typename StateT,
		typename ParsingTableT,
		typename FSMTableT, typename InputT>
	template<typename ParserResultT>
	ParserResultT LRParser<GrammarT, LexicalAnalyzerT,
		SymbolT, StateT,
		ParsingTableT, FSMTableT, InputT>::
		parse(const ParserResultT& initResult, ErrorRecoveryType errorRecoveryType)
	{
		ParserResultT result{initResult};

		/** Algorithm
		* Initialize the stack to contain only the start state.
		* Loop inifinitly until: the parser accepts or an error is produced.
		* For every state I on top of the stack and token T from the input:
			* A = ACTION[I][T.asTerminal] and switch on A:
				* If A == SHIFT J:
					* Push J on top of the stack and get next input.
				* Else if A == REDUCE J:
					* Call _reduce(J).
				* Else ERROR.

		* _reduce(J):
			* Get to production P indexed J and do the following:
				* If P contains an action, execute the action before reduction.
				* Get the length L of the production body, and remove from top of the stack L states.
				* Assume the state on top of the stack is now state I, and the head of production J is nonterminal H:
					* Push GOTO[I][H.as.nonTerminal] on top of the stack.
				* Check for whether the new top state is an error.
		* _error_recovery(errorRecoveryType): TBD.
		*/

		this->_reset_parser_state();
		this->_push_state(START_STATE);
		this->m_CurrInputToken = this->get_next_token();

		// this variable keeps track of the number of errors encountered thus far
		size_t errorNum = 0;

		// main parser loop
		while (true) {
			// check for errors and resolve them if any
			if(_check_and_resolve_parsing_errors(errorNum, errorRecoveryType))
				continue; // if there is no error and has been resolved

			// no error:

			// do the action and break in case we accept
			if (this->_take_parsing_action(result))
				break;
			
		}

		return result;
	}
}

namespace m0st4ta {

}