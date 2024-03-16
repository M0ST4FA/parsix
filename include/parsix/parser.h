#pragma once
#include <concepts>

#include "lexana/LexicalAnalyzer.h"
#include "parsix/PDataStructs.h"

// Parser class
namespace m0st4fa::parsix {

	// TODO: work on this concept
	template <typename ParsingTableT>
	concept ParserRequirments = requires (ParsingTableT table) {
		//require the parsing table to be callable
		//table[EXTRACT_VARIABLE(gramSym)][EXTRACT_TERMINAL(token)];

		// require SymbolT to be comparable by equality with TokenT and returns a boolean
		//{gramSym == token} -> std::same_as<bool>;
		// require SymbolT to have operator== and operator<
		//gramSym == gramSym;

		true;
	};

	/**
	* @brief A general parser class, designed to contain things common to any parser.
	* @tparam LexicalAnalyzerT The type of the lexical analyzer object used by the parser.
	* @tparam SymbolT The type of grammar symbol objects of the language of the parser.
	* @tparam ParsingTableT The type of the parsing table object used by the parser.
	* @tparam FSMTableT The type of the finite state machine table used by the state machine that is used by the lexical analyzer that is used by the parser.
	* @tparam InputT The type of the input string.
	*/
	template <typename LexicalAnalyzerT,
		typename SymbolT,
		typename ParsingTableT, typename FSMTableT = fsm::FSMTable, // set them to have a default value
		typename InputT = std::string_view>
		requires ParserRequirments<ParsingTableT>
	class Parser {
		using TokenType = decltype(LexicalAnalyzerT{}.getNextToken());

		/**
		 * @brief The lexical analyzer used by the parser.
		 */
		LexicalAnalyzerT* mp_LexicalAnalyzer;

		/**
		 * @brief The rhs (or head) of the first production (or set of productions) of the grammar.
		 */
		SymbolT m_StartSymbol;

	protected:

		/**
		 * @brief The parsing table of the parser.
		 */
		mutable ParsingTableT p_Table;

		/**
		 * @brief The logger used to log information to the different streams.
		 */
		Logger p_Logger;

		/**
		 * @brief The maximum number of errors that you can possibly recover from before failing (aborting).
		 */
		static constexpr size_t ERR_RECOVERY_LIMIT = 5;

		/**
		 * @brief Gets the source code against which the parsing is being done.
		 */
		std::string_view get_source_code() const { return this->mp_LexicalAnalyzer->getSourceCode(); }

		/**
		 * @brief Gets the next token from the lexical analyzer.
		 */
		TokenType get_next_token() const { return this->mp_LexicalAnalyzer->getNextToken((unsigned)lexana::LA_FLAG::LAF_ALLOW_WHITE_SPACE_CHARS); }

		/**
		 * @brief Gets the head of the first production (or set of productions) in the grammar.
		 */
		SymbolT get_start_symbol() const { return m_StartSymbol; }

	public:

		/**
		 * @brief Default constructor.
		 */
		Parser() = default;
		
		/**
		 * @brief Main converting constructor.
		 * @todo Check for argument "validity".
		 * @param[in] lexer The lexical analyzer used by the parser.
		 * @param[in] parsingTable The parsing table object of the finite state machine object that the parser uses.
		 * @param[in] startSymbol The head symbol for the first production (or set of productions) of the grammar.
		 */
		Parser(
			LexicalAnalyzerT& lexer, 
			const ParsingTableT& parsingTable, 
			const SymbolT& startSymbol) :
			mp_LexicalAnalyzer{ &lexer }, p_Table{ parsingTable }, m_StartSymbol{ startSymbol }
		{
			// TODO: check for argument correctness

		};


	};


};
