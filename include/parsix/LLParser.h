#pragma once
// std includes
#include <vector>
#include <stdexcept>

// local includes
#include "parsix/Parser.h"

namespace m0st4fa::parsix {

	/**
	 * @brief An LL parser. The parser is a stack-based parser, instead of a recursive-procedure-based one. I.e., it uses an explicit stack instead of relying on the function call stack.
	 * @tparam GrammarT The type of the grammar object used by the parser. This type represents a vector of production objects.
	 * @tparam LexicalAnalyzerT The type of the lexical analyzer object used by the parser.
	 * @tparam SymbolT The type of grammar symbol objects of the language of the parser.
	 * @tparam ParsingTableT The type of the parsing table object used by the parser.
	 * @tparam FSMTableT The type of the finite state machine table used by the state machine that is used by the  lexical analyzer that is used by the parser.
	 * @tparam InputT The type of the input string.
	 */
	template <typename GrammarT,
		typename LexicalAnalyzerT,
		typename SymbolT,
		typename ParsingTableT, typename FSMTableT = fsm::FSMTable,
		typename InputT = std::string_view>
	class LLParser : public Parser<LexicalAnalyzerT, SymbolT, ParsingTableT, FSMTableT, InputT> {

		/**
		 * @brief The type of production objects stored in the grammar object.
		 */
		using ProductionType = decltype(GrammarT{}.getProdVector().at(0));

		/**
		 * @brief The type of the elements of the LL parsing stack.
		 */
		using StackElementType = decltype(ProductionType{}.get(0));

		/**
		 * @brief The type of synthetic record objects.
		 */
		using SynthesizedType = decltype(StackElementType{}.as.synRecord);

		/**
		 * @brief The type of the extra/context data stored in synthetic record objects.
		 */
		using SynDataType = decltype(SynthesizedType{}.data);

		/**
		 * @brief The type of the action record objects.
		 */
		using ActionType = decltype(StackElementType{}.as.actRecord);

		/**
		 * @brief The type of the extra/context data stored in action record objects.
		 */
		using ActDataType = decltype(ActionType{}.data);

		/**
		 * @brief The base class of this class.
		 */
		using ParserBase = Parser<LexicalAnalyzerT, SymbolT, ParsingTableT, FSMTableT, InputT>;

		/**
		 * @brief The type of a token object returned by the lexical analyzer.
		 */
		using TokenType = decltype(LexicalAnalyzerT{}.getNextToken());

		// a parsing table entry should contain indicies into this vector of productions
		
		/**
		 * @brief The grammar object. It is expected to be a vector of production objects.
		 */
		GrammarT& m_ProdRecords;

		/**
		 * @brief The LL parsing stack.
		 */
		mutable std::vector<StackElementType> m_Stack;

		/**
		 * @brief The top of element of the parsing stack.
		 */
		mutable StackElementType m_CurrTopElement;

		/**
		 * @brief The most recent token (TokenType object) returned by the lexical analyzer.
		 */
		mutable TokenType m_CurrInputToken;

		// PARSER FUNCTIONS
		/**
		 * @brief Resets the state of the LLParser object (i.e., sets the values of the most important fields to the default value).
		 * @details The following fields have their values reset: m_Stack, m_CurrTopElement, m_CurrInputToken and, if `resetProductions` is set to `true`, m_ProdRecords.
		 * @param resetProductions If `true`, resets the grammar object, i.e., empties the vector of all productions. If `false`, leaves the grammar object as it is.
		 */
		void reset_parser_state(bool resetProductions = false) {
			m_Stack.clear();
			m_CurrTopElement = StackElementType{};
			m_CurrInputToken = TokenType{};

			if (resetProductions)
				m_ProdRecords.clear();

			return;
		};
		void parse_grammar_symbol(ErrorRecoveryType);

		// ERROR RECOVERY FUNCTIONS
		/**
		 * @brief Handles error recovery.
		 *
		 * @tparam GrammarT The type of the grammar object.
		 * @tparam LexicalAnalyzerT The type of the lexical analyzer object.
		 * @tparam SymbolT The type of the symbol object.
		 * @tparam ParsingTableT The type of the parsing table object.
		 * @tparam FSMTableT The type of the FSM table object.
		 * @tparam InputT The type of the input string.
		 *
		 * @param errRecovType The type of error recovery to be used.
		 *
		 * @details This function handles error recovery. It checks the error recovery limit and throws a runtime error if the limit is exceeded. It also handles invalid arguments and throws an invalid argument exception if the argument is ErrorRecoveryType::ERT_NUM. Depending on the type of error recovery, it calls the corresponding error recovery function and returns the result. If the error recovery type is ErrorRecoveryType::ERT_ABORT, it aborts the parser.
		 *
		 * @return `true` if error recovery was successful (the error was recovered successfully); `false` otherwise.
		 */
		bool error_recovery(ErrorRecoveryType errRecovType = ErrorRecoveryType::ERT_NONE) {
			// check the error recovery limit
			static size_t numOfDetectedErrs = 0;

			if (errRecovType != ErrorRecoveryType::ERT_NONE &&
				errRecovType != ErrorRecoveryType::ERT_ABORT
				)
			{
				if (numOfDetectedErrs == ParserBase::ERR_RECOVERY_LIMIT) {
					LoggerInfo errInfo = { .level = LOG_LEVEL::LL_ERROR };

					this->p_Logger.log(errInfo, std::format("Exceeded error recovery limit\nNote: error recovery limit {}", ParserBase::ERR_RECOVERY_LIMIT));

					throw std::runtime_error{ "Limit of recovered-from errors exceeded" };
				};
			}

			// handle invalid arguments
			if (errRecovType == ErrorRecoveryType::ERT_NUM) {
				LoggerInfo info = { .level = LOG_LEVEL::LL_ERROR };

				this->p_Logger.log(info, "[ERR_RECOVERY]: Invalid argument.");

				throw std::invalid_argument("Argument ERT_NONE cannot be used in this context.\nNote: it is just for knowing the number of possible values of this enum.");
			}

			this->p_Logger.logDebug("[ERR_RECOVERY]: started error recovery: " + toString(errRecovType));

			switch (errRecovType) {
			case ErrorRecoveryType::ERT_NONE:
				return false;

			case ErrorRecoveryType::ERT_PANIC_MODE:
				return panic_mode();

			case ErrorRecoveryType::ERT_PHRASE_LEVE:
				return phrase_level();

			case ErrorRecoveryType::ERT_GLOBAL:
				return global();

			case ErrorRecoveryType::ERT_ABORT:
				// TODO: handle this in a different way (possibly by throwing an exception to abort the parser)
				std::abort();
				break;

			default:
				// TODO
				return false;
			};
		};

		bool panic_mode();
		bool panic_mode_try_sync_variable(TokenType&);

		// TODO: implement phrase level and global error recovery
		bool phrase_level() const { return false; };
		bool global() const { return false; };

		// error helper functions

		void check_prod_body(const ProductionType&) const;

		/**
		 * @brief Prints a synchronization message.
		 *
		 * @param pos The position in the input stream at which to report. It is an ordered pair of line number and character number.
		 *
		 * This function prints a synchronization message. It formats a message with the current position in the input stream and the current input token, and logs the message at the info level.
		 *
		 * @return void
		 */
		void print_sync_msg(const std::pair<size_t, size_t> pos) const {
			static constexpr LoggerInfo info = { .level = LOG_LEVEL::LL_INFO };

			std::string msg = std::format("({}, {}) Synchronized successfully. Current input token {}",
				pos.first,
				pos.second,
				(std::string)m_CurrInputToken);

			this->p_Logger.log(info, msg);
		};

	public:

		/**
		 * @brief Default Constructor.
		 */
		LLParser() = default;

		/**
		 * @brief Constructs an LLParser object.
		 *
		 * This constructor initializes an LLParser object. It initializes the base Parser object with the given lexer, parsing table, and start symbol. It also initializes the production records with the grammar from the parsing table. The stack, current input token, and current top element are initialized to their default values.
		 */
		LLParser(
			const SymbolT& startSymbol,
			const ParsingTableT& parsingTable,
			const lexana::LexicalAnalyzer<TokenType, FSMTableT>& lexer
		) :
			Parser<LexicalAnalyzerT, SymbolT, ParsingTableT, FSMTableT, InputT>
		{ lexer, parsingTable, startSymbol },
			m_ProdRecords{ this->p_Table.grammar },
			m_Stack{},
			m_CurrInputToken{},
			m_CurrTopElement{}
		{};

		/**
		 * @brief Default destructor.
		 */
		~LLParser() = default;

		template<typename ParserResultT>
		ParserResultT parse(ErrorRecoveryType = ErrorRecoveryType::ERT_NONE);
	};

	// IMPLEMENTATIONS
	/**
	 * @brief Parses the input based on a grammar.
	 *
	 * @tparam GrammarT The type of the grammar object.
	 * @tparam LexicalAnalyzerT The type of the lexical analyzer object.
	 * @tparam SymbolT The type of the symbol object.
	 * @tparam ParsingTableT The type of the parsing table object.
	 * @tparam FSMTableT The type of the FSM table object.
	 * @tparam InputT The type of the input string.
	 * @tparam ParserResultT The type of the parser result object.
	 *
	 * @param errRecoveryType The type of error recovery to be used.
	 *
	 * @details This function parses the input based on a grammar. It initializes the parser to the initial configuration and gets the next input token. It then loops until the stack is empty. For each symbol on top of the stack, it makes the parsing decision based on the current symbol and the current input. If the symbol is a grammar symbol, it parses the grammar symbol. If the symbol is a synthesized record or an action record, it executes the corresponding action. After all symbols are processed, it resets the parser state and returns the result.
	 * @details If the input string `w` is in `L(G)` (`L` is the language and `G` is the grammar), it produces a leftmost derivation of `w`; otherwise, it produces an error indication.
	 * 
	 * @note It might execute actions during the leftmost derivation, for example, to make a parsing or syntax tree.
	 *
	 * @return The result of the parsing. Right now, the object is not filled with anything. This is for future implementation.
	 */
	template<typename GrammarT, typename LexicalAnalyzerT,
		typename SymbolT,
		typename ParsingTableT, typename FSMTableT,
		typename InputT>
	template<typename ParserResultT>
	ParserResultT LLParser<GrammarT, LexicalAnalyzerT, SymbolT, ParsingTableT, FSMTableT, InputT>::parse(ErrorRecoveryType errRecoveryType)
	{
		using StackType = std::vector<StackElementType>;

		ParserResultT res{};
		LoggerInfo info{ LoggerInfo::INFO };

		// Initialize the algorithm, such that the parser is in the initial configuration
		m_Stack.push_back({ .type = ProdElementType::PET_GRAM_SYMBOL, .as = {.gramSymbol = this->get_start_symbol() } });

		this->m_CurrInputToken = this->getLexicalAnalyzer().get_next_token();

		/** Basic algorithm:
		* Loop until the stack is empty.
		* Consider the current symbol on top of the stack and the current input.
		* Make the parsing decision based on these two tokens.
		* Whenever a symbol is matched, it is popped off the stack.
		* The purpose is to pop the start symbol off the stack (to match it, leaving the stack empty) and not produce any errors.
		*/
		while (-not m_Stack.empty()) {
			// get the current symbol on top of the stack, pop it and get the next input token
			m_CurrTopElement = m_Stack.back();
			m_Stack.pop_back();

			// switch on the type of the top symbol

			switch (m_CurrTopElement.type) {
			case ProdElementType::PET_GRAM_SYMBOL:
				parse_grammar_symbol(errRecoveryType);
				continue;

				/**
				* When you come to execute the action on a record, recall that the synthesized record is already popped off the stack.
				* This affects the stack size as well as indices you use to access other records.
				*/
			case ProdElementType::PET_SYNTH_RECORD: {
				// extract the record
				SynthesizedType topRecord = m_CurrTopElement.as.synRecord;
				auto action = static_cast<void(*)(StackType&, SynDataType&)>(topRecord.action);

				// execute the action if any
				if (action)
					action(this->m_Stack, topRecord.data);
				continue;
			}

			case ProdElementType::PET_ACTION_RECORD: {
				// extract the record
				ActionType topRecord = m_CurrTopElement.as.actRecord;
				auto action = static_cast<void(*)(StackType&, ActDataType&)>(topRecord.action);

				// execute the action if any
				if (action)
					action(this->m_Stack, topRecord.data);

				continue;
			}

			default:
				break;
			}
		}

		reset_parser_state();
		return res;
	}

	/**
	 * @brief Parses a grammar symbol.
	 * 
	 * @tparam GrammarT The type of the grammar object.
	 * @tparam LexicalAnalyzerT The type of the lexical analyzer object.
	 * @tparam SymbolT The type of the symbol object.
	 * @tparam ParsingTableT The type of the parsing table object.
	 * @tparam FSMTableT The type of the FSM table object.
	 * @tparam InputT The type of the input string.
	 *
	 * @param errRecoveryType The type of error recovery to be used.
	 *
	 * @details This function parses a grammar symbol. If the symbol at the top of the stack is a terminal symbol, it matches it explicitly. If the symbol is a non-terminal symbol, it gets the production record for the current symbol and input. If the table entry is an error, it performs error recovery. Otherwise, it pushes the body of the production on top of the stack.
	 *
	 * @return void
	 */
	template<typename GrammarT, typename LexicalAnalyzerT,
		typename SymbolT,
		typename ParsingTableT, typename FSMTableT,
		typename InputT>
	void LLParser<GrammarT, LexicalAnalyzerT, SymbolT, ParsingTableT, FSMTableT, InputT>::parse_grammar_symbol(ErrorRecoveryType errRecoveryType) {
		using StackElement = StackElementType;

		LoggerInfo info{ .level = LOG_LEVEL::LL_INFO };
		const SymbolT topSymbol = m_CurrTopElement.as.gramSymbol;

		// if the symbol at the top of the stack is a terminal symbol
		if (topSymbol.isTerminal) {
			// epsilon matches with nothing
			if (topSymbol == TokenType::EPSILON)
				return;

			// match it explicitly
			bool matched = (topSymbol == m_CurrInputToken);

			this->p_Logger.logDebug(std::format("Stack size before: {}", m_Stack.size() + 1));
			this->p_Logger.log(info, std::format("Matched {:s} with {:s}: {:s}", (std::string)topSymbol, (std::string)m_CurrInputToken, matched ? "true" : "false"));

			// get the next input token
			m_CurrInputToken = this->getLexicalAnalyzer().get_next_token();

			// if the symbol at the top of the stack is not a terminal symbol and the input token is not matched,
			if (!matched)
				error_recovery(errRecoveryType);
		}
		// if the symbol is a non-terminal symbol
		else {
			// get the production record for the current symbol and input
			const LLTableEntry tableEntry = this->p_Table[EXTRACT_VARIABLE(this->m_CurrTopElement)][(size_t)m_CurrInputToken.name];

			// if the table entry is an error
			if (tableEntry.isError) {
				// TODO: do more robust logic based on the boolean returned from the function
				error_recovery(errRecoveryType);
				return; // assuming we have synchronized and are ready to continue parsing
			}

			// if the table entry is not an error

			// Caution: this is a reference
			const auto& prod = this->m_ProdRecords[tableEntry.prodIndex];
			const auto& prodBody = prod.prodBody;

			// check that the production body is not empty
			check_prod_body(prod);

			// push the body of the production on top of the stack
			for (auto it = prodBody.rbegin(); it != prodBody.rend(); ++it) {
				StackElement se = *it;
				m_Stack.push_back(se);
			};

			this->p_Logger.logDebug(std::format("Stack size before: {}", m_Stack.size() + 1));
			this->p_Logger.log(info, std::format("Expanded {:s} with {:s}: {:s}", (std::string)topSymbol, (std::string)m_CurrInputToken, (std::string)prod));
		}

		return;
	}

	/**
	 * @brief Handles error recovery using panic mode.
	 *
	 * @tparam GrammarT The type of the grammar object.
	 * @tparam LexicalAnalyzerT The type of the lexical analyzer object.
	 * @tparam SymbolT The type of the symbol object.
	 * @tparam ParsingTableT The type of the parsing table object.
	 * @tparam FSMTableT The type of the FSM table object.
	 * @tparam InputT The type of the input string.
	 *
	 * @brief This function handles error recovery using panic mode. It gets the top stack element and the current input token. It then loops until the input and the stack are synchronized. 
		* If the top symbol is a terminal, it adds the lexeme to the input stream, pops the token off the stack, and returns to the parser. 
		* If the top symbol is a non-terminal, it peaks to see the next token and checks if it can be used to synchronize with the parser. 
		* If the synchronization is successful, it breaks the loop. 
		* If the end of the input is reached and synchronization is not possible with the current token, it pops the element off the stack. 
		* If the stack is empty, it logs the failure to synchronize and returns false. Otherwise, it continues the loop. The function logs the start and end of the error recovery process and the result of the synchronization.
	 *
	 * @return `true` if synchronization and recovery were successful; `false` otherwise.
	 */
	template<typename GrammarT, typename LexicalAnalyzerT,
		typename SymbolT,
		typename ParsingTableT, typename FSMTableT,
		typename InputT>
	bool LLParser<GrammarT, LexicalAnalyzerT, SymbolT, ParsingTableT, FSMTableT, InputT>::panic_mode()
	{
		using StackType = StackType<StackElementType>;

		// get top stack element
		auto currInputToken = this->m_CurrInputToken;
		const SymbolT topSymbol = m_CurrTopElement.as.gramSymbol;

		// set up the logger state
		LoggerInfo info{ .level = LOG_LEVEL::LL_INFO };
		LoggerInfo errInfo{ .level = LOG_LEVEL::LL_ERROR };

		this->p_Logger.log(errInfo, std::format(
			"({}, {}) Didn't expect token {:s}",
			this->getLexicalAnalyzer().getLine(),
			this->getLexicalAnalyzer().getCol(),
			(std::string)this->m_CurrInputToken
		));

		// loop until the input and the stack are synchronized
		while (true) {
			// if the top symbol is a terminal
			if (topSymbol.isTerminal) {
				this->p_Logger.log(info, std::format(
					"Added lexeme {:s} to the input stream.", currInputToken.attribute)
				);

				// pop the token of the stack and return to the parser
				m_Stack.pop_back();
				return true;
			}
			// if the top symbol is a non-terminal
			else {
				// peak to see the next token
				currInputToken = this->getLexicalAnalyzer().peak();

				// check if it can be used to sync with the parser
				bool synced = this->panic_mode_try_sync_variable(currInputToken);

				if (synced)
					break;

				// if we've reached the end of the input and could not sync with this token
				if (m_CurrInputToken == TokenType{}) {
					// pop this element since there is no way to synchronize using it
					m_Stack.pop_back();

					// if the stack is empty, return to the caller
					if (m_Stack.empty()) {
						this->p_Logger.log(info, std::format("[ERROR_RECOVERY] ({}, {}) Failed to synchronize: current input: {:s}",
							this->getLexicalAnalyzer().getLine(),
							this->getLexicalAnalyzer().getCol(),
							(std::string)currInputToken
						));

						return false;
					};
				};
			}
		}

		return true;
	}

	/**
	 * @brief Tries to synchronize the parser with the current input token during panic mode.
	 *
	 * @tparam GrammarT The type of the grammar object.
	 * @tparam LexicalAnalyzerT The type of the lexical analyzer object.
	 * @tparam SymbolT The type of the symbol object.
	 * @tparam ParsingTableT The type of the parsing table object.
	 * @tparam FSMTableT The type of the FSM table object.
	 * @tparam InputT The type of the input string.
	 *
	 * @param currInputToken The current input token object.
	 *
	 * @details This function tries to synchronize the parser with the current input token in panic mode. 
		* If the non-terminal has an epsilon production, make it the default.
		* If there is a token whose associated table entry has an error action, execute it.
		* If we find a token that is in the first set of the current token, expand by it.
	 *
	 * @return `true` if synchronization was successful; `false` otherwise.
	 */
	template<typename GrammarT, typename LexicalAnalyzerT,
		typename SymbolT,
		typename ParsingTableT, typename FSMTableT,
		typename InputT>
	bool LLParser<GrammarT, LexicalAnalyzerT, SymbolT, ParsingTableT, FSMTableT, InputT>::panic_mode_try_sync_variable(TokenType& currInputToken) {
		using StackType = StackType<StackElementType>;

		LoggerInfo info{ .level = LOG_LEVEL::LL_INFO };

		// Check for whether the non-terminal has an epsilon production and use it to reduce that non-terminal.
		LLTableEntry tableEntry = this->p_Table[EXTRACT_VARIABLE(this->m_CurrTopElement)][(size_t)TokenType::EPSILON.name];

		// if it is not an err, there is an epsilon production
		if (-not tableEntry.isError) {
			// expand with this production and assume we are in sync

			// Caution: this is a reference
			const auto& prod = this->m_ProdRecords[tableEntry.prodIndex];
			const auto& prodBody = prod.prodBody;

			// check that the production body is not empty
			check_prod_body(prod);

			// push the body of the production on top of the stack
			for (auto it = prodBody.rbegin(); it != prodBody.rend(); ++it) {
				StackElementType se = *it;
				m_Stack.push_back(se);
			};

			this->p_Logger.log(info, std::format("[ERROR_RECOVERY] Expanded {:s} with {:s}: {:s}",
				toString(m_CurrTopElement.as.gramSymbol.as.nonTerminal),
				m_CurrInputToken.toString(),
				prod.toString()));

			print_sync_msg(this->getLexicalAnalyzer().getPosition());

			return true;
		}

		// get the production record for the current symbol and input
		tableEntry = this->p_Table[EXTRACT_VARIABLE(this->m_CurrTopElement)][(size_t)currInputToken.name];

		/**
		* Assume the synchronization set of each non-terminal contains the first set of that non-terminal.
		* For the rest of the tokens, the action to take would be specified in the table entry.
		*/

		/**
		* If the entry is an error, check whether it has an action or no.
		* If it has an associated action, do it, otherwise, continue to skip tokens.
		*/
		if (tableEntry.isError) {
			// if the entry has an action
			if (tableEntry.action) {
				auto action = static_cast<bool (*)(StackType, StackElementType, TokenType)>(tableEntry.action);

				// if the action results in a synchronization
				if (action(m_Stack, m_CurrTopElement, currInputToken)) {
					m_CurrInputToken = this->getLexicalAnalyzer().get_next_token();

					print_sync_msg(this->getLexicalAnalyzer().getPosition());

					return true;
				}
			}

			// if the entry has no action or the action has failed to synchronize

			// get the next input and try again to synchronize
			m_CurrInputToken = this->getLexicalAnalyzer().get_next_token();
			return false;
		}

		//---------------------------------------------Synchronized-------------------------------------------
		/**
		* Upon reaching here, this means the parser has synchronized with the current token.
		* Since we've just peaked to see whether we can sync with it or not, now we need to fetch it so that parsing can continue from it.
		*/
		m_CurrInputToken = this->getLexicalAnalyzer().get_next_token();
		print_sync_msg(this->getLexicalAnalyzer().getPosition());

		/**
		* If the table entry is not an error, then we have synchronized correctly using FIRST set and possibly using FOLLOW set.
		* Since the state is now synchronized, we have to return to the parser to continue parsing the source.
		*/

		return true;
	}

	/**
	 * @brief Checks if the production body is empty.
	 *
	 * @tparam GrammarT The type of the grammar object.
	 * @tparam LexicalAnalyzerT The type of the lexical analyzer object.
	 * @tparam SymbolT The type of the symbol object.
	 * @tparam ParsingTableT The type of the parsing table object.
	 * @tparam FSMTableT The type of the FSM table object.
	 * @tparam InputT The type of the input string.
	 *
	 * @param prod The production to be checked.
	 *
	 * @details This function checks if the production body is empty. If the production body is empty, it logs an error message and throws a logic  error.
	 *
	 * @return void
	 */
	template<typename GrammarT, typename LexicalAnalyzerT,
		typename SymbolT,
		typename ParsingTableT, typename FSMTableT,
		typename InputT>
	void LLParser<GrammarT, LexicalAnalyzerT, SymbolT, ParsingTableT, FSMTableT, InputT>::check_prod_body(const ProductionType& prod) const {
		LoggerInfo errorInfo{ .level = LOG_LEVEL::LL_ERROR };

		const auto& prodBody = prod.prodBody;

		if (prodBody.empty()) {
			this->p_Logger.log(errorInfo, std::format("Production body is empty: {:s}", prod.toString()));
			throw std::logic_error("Production body is empty.");
		};
	}
}