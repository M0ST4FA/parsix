module;

#include <string>
#include <iostream>
#include <map>

#include "fsm/FiniteStateMachine.h"
#include "parsix/LLParser.h"

export module Tests;

// FSM ------------------------------------------------------------
export using m0st4fa::fsm::TransitionFunction;
export using m0st4fa::fsm::DFA;
export using m0st4fa::fsm::FSMTable;
export using m0st4fa::fsm::FSMStateSetType;
export using m0st4fa::fsm::FSMStateType;

export template<typename T>
constexpr void initTranFn_ab(T& fun) {
	fun[1]['a'] = 2;
	fun[2]['a'] = 3;
	fun[3]['a'] = 3;
	fun[3]['b'] = 4;
	fun[4]['a'] = 3;
	fun[4]['b'] = 5;
	fun[5]['a'] = 5;
	fun[5]['b'] = 5;
}

export template<typename T>
constexpr void initTranFn_ab_NFA(T& fun) {

	/*
	  -> a -> a
	s    |
	  -> b
	*/

	fun[1]['a'] = { 2 };
	fun[1]['b'] = { 3 };

	fun[2]['a'] = { 2, 4 };
	fun[2]['b'] = { 3 };

	fun[3]['a'] = { 2, 4 };
	fun[3]['b'] = { 3 };

	fun[4]['a'] = { 2, 4 };
	fun[4]['b'] = { 2 };

};

export enum TOKEN {
	T_AAB,
	T_EOF,
	T_MAX,
};

export struct token_t {
	TOKEN type = T_EOF;
	std::string lexeme;
};

export std::ostream& operator<<(std::ostream&, const token_t);

export token_t fact(FSMStateType, std::string);

// PARSER -----------------------------------------------------------

export enum struct _TERMINAL {
	T_ID,
	T_INT,
	T_PLUS,
	T_STAR,
	T_LEFT_PAREN,
	T_RIGHT_PAREN,
	T_EOF,
	T_EPSILON,
	T_COUNT
};

export std::string toStriEng(_TERMINAL);
export std::ostream& operator<<(std::ostream&, const _TERMINAL);

// so that no collisions occur with map
export enum struct _NON_TERMINAL {
	NT_E = (unsigned)_TERMINAL::T_COUNT,
	NT_EP,
	NT_T,
	NT_TP,
	NT_F,
	NT_COUNT
};

export std::string toString(_NON_TERMINAL);
export std::ostream& operator<<(std::ostream&, const _NON_TERMINAL);

export using Symbol = m0st4fa::Symbol<_TERMINAL, _NON_TERMINAL>;

export Symbol toSymbol(_TERMINAL);
export Symbol toSymbol(_NON_TERMINAL);

// IMPORTANT: these two data structures cause problems if they do not have trivial destructors.


// LL PARSER DATA STRUCTURES
export struct SynData {
	const char* str = "[SynData] Hello world from Synthesized Data";

	operator std::string() const {
		return std::string(str);
	}

	std::string toString() const {
		return std::string(*this);
	}

	bool operator== (const SynData& lhs) const {
		return not (std::strcmp(str, lhs.str));
	}

};
export struct ActData {
	const char* str = "[ActData] Hello world from Action Data";

	operator std::string() const {
		return std::string(str);
	}

	std::string toString() const {
		return std::string(*this);
	}

	bool operator== (const ActData& lhs) const {
		return not (std::strcmp(str, lhs.str));
	}
};
export using Synthesized = m0st4fa::LLSynthesizedRecord<SynData>;
export using Action = m0st4fa::LLActionRecord<ActData>;
export using TokenType = m0st4fa::lexana::Token<_TERMINAL>;
export using LexicalAnalyzerType = m0st4fa::lexana::LexicalAnalyzer<TokenType>;

export using LLStackElementType = m0st4fa::LLStackElement<Symbol, m0st4fa::LLSynthesizedRecord<SynData>, m0st4fa::LLActionRecord<ActData>>;
export using LLStackType = m0st4fa::StackType<LLStackElementType>;

export using ProductionType = m0st4fa::ProductionRecord<Symbol, LLStackElementType>;
export using GrammarType = m0st4fa::ProductionVector<ProductionType>;

// LR PARSER DATA STRUCTURES
export using LRProductionElementType = m0st4fa::LRProductionElement<Symbol>;
export using LRProductionType = m0st4fa::ProductionRecord<Symbol, LRProductionElementType>;

export using LRGrammarType = m0st4fa::ProductionVector<LRProductionType>;
export struct LRDataType {
	size_t data{};
	
	operator std::string() const {
		return std::to_string(data);
	}

};
export using LRStateType = m0st4fa::LRState<LRDataType, TokenType>;
export using LRStackType = m0st4fa::LRStackType<LRDataType, TokenType>;
export using ItemType = m0st4fa::Item<LRProductionType>;
export using ItemSet = m0st4fa::ItemSet<ItemType>;

export using FirstType = std::vector<std::set<Symbol>>;

export using LLParsingTableType = m0st4fa::LLParsingTable<GrammarType, _TERMINAL, _NON_TERMINAL>;
export using LLParserType = m0st4fa::LLParser<GrammarType, LexicalAnalyzerType, Symbol, LLParsingTableType>;

// FUNCTION SIGNATURES

// LL PARSER FUNCTIONS
export GrammarType grammar_expression();
export void define_table_llparser(m0st4fa::LLParsingTable<GrammarType, _TERMINAL, _NON_TERMINAL>&);

// LR PARSER FUNCTIONS
export LRGrammarType grammar_expression_LR();
export void define_table_lrparser(m0st4fa::LRParsingTable<LRGrammarType>&);

// FSM FOR PARSERS
export void initFSMTable_parser(m0st4fa::fsm::FSMTable&);
export m0st4fa::lexana::Token<_TERMINAL> token_fact_parser(FSMStateType, std::string_view);

// ACTIONS
export void synDataAct(LLStackType&, SynData&);

export void actDataAct(LLStackType&, ActData&);

