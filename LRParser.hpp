#pragma once
#include <vector>

#include "Parser.h"

namespace m0st4fa {
	template <typename GrammarT, typename LexicalAnalyzerT,
		typename SymbolT, typename StateT,
		typename ParsingTableT,
		typename FSMTableT = FSMTable<>,
		typename InputT = std::string>
	class LRParser : public Parser<LexicalAnalyzerT, SymbolT, ParsingTableT, FSMTableT, InputT> {
		using ParserBase = Parser<LexicalAnalyzerT, SymbolT, ParsingTableT, FSMTableT, InputT>;

		using ProductionType = decltype(GrammarT{}.at(0));
		using SymbolType = decltype(ProductionType{}.prodHead);
		using TerminalType = decltype(SymbolType{}.as.terminal);
		using VariableType = decltype(SymbolType{}.as.nonTerminal);

		using StackElementType = StateT;
		using DataType = decltype(StackElementType{}.data);
		using StackType = StackType<StackElementType>;

		using TokenType = decltype(LexicalAnalyzerT{}.getNextToken());

		mutable StackType m_Stack{ 0 };
		mutable StackElementType m_CurrTopState{ START_STATE };
		mutable TokenType m_CurrInputToken{ TokenType{} };

		void _reset_parser_state() const {
			this->p_Logger.log(LoggerInfo::INFO, "RESETTING PARSER.");
			m_Stack.clear();
			m_CurrTopState = START_STATE;
			m_CurrInputToken = TokenType{};
		}

		void _reduce(size_t);

		void _push_state(const StateT& state) {
			this->m_Stack.push_back(state);
			this->m_CurrTopState = this->m_Stack.back();

			this->p_Logger.log(LoggerInfo::INFO, std::format("Pushing state {}\nCurrent stack: {}", (std::string)state, toString(this->m_Stack)));
		}

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

		/**
		* @output `false`: there is no error
		*		  `true`:  there is an error and has been resolved
		*		  exception: there is an error that could not be resolved
		*/
		bool _check_resolve_parsing_errors(size_t, ErrorRecoveryType);
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
				this->p_Logger.log(LoggerInfo::FATAL_ERROR, "Unable to synchronize! implement professional handling of this.");
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
		/**
		* @output `true`: accepted
		*		  `false`: didn't accept
		*         exception: the entry is incorrect
		*/
		bool _take_parsing_action(auto&);
	protected:
		static const StateT START_STATE;

	public:

		LRParser() = default;
		LRParser(LexicalAnalyzerT& lexer,
			const ParsingTableT& parsingTable,
			const SymbolT& startSymbol) : ParserBase{ lexer, parsingTable, startSymbol } {
			this->p_Table.grammar.calculateFIRST();
			this->p_Table.grammar.calculateFOLLOW();
		};
		LRParser(const LRParser& other) = default;
		LRParser& operator=(const LRParser& rhs) {
			this->ParserBase::operator=(rhs);
			this->m_Stack = rhs.m_Stack;
			this->m_CurrTopState = rhs.m_CurrTopState;
			this->m_CurrInputToken = rhs.m_CurrInputToken;

			return *this;
		}

		template<typename ParserResultT = ParserResult>
		ParserResultT parse(const ParserResultT&, ErrorRecoveryType = ErrorRecoveryType::ERT_NONE);
	};

	template<typename GrammarT, typename LexicalAnalyzerT, typename SymbolT, typename StateT, typename ParsingTableT, typename FSMTableT, typename InputT>
	const StateT LRParser<GrammarT, LexicalAnalyzerT, SymbolT, StateT, ParsingTableT, FSMTableT, InputT>::START_STATE{ 0 };

	// IMPLEMENTATION


	template<typename GrammarT, typename LexicalAnalyzerT, typename SymbolT, typename StateT, typename ParsingTableT, typename FSMTableT, typename InputT>
	inline bool LRParser<GrammarT, LexicalAnalyzerT, SymbolT, StateT, ParsingTableT, FSMTableT, InputT>::_check_resolve_parsing_errors(size_t errorNum, ErrorRecoveryType errorRecoveryType)
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

		// if the error was resolved, continue to the next iteration
		return true;
	};

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
			if(_check_resolve_parsing_errors(errorNum, errorRecoveryType))
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