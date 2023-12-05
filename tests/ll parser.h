#pragma once

#include "parsix/PDataStructs.h"
#include "lexana/LADataStructs.h"
#include "fsm.h"


struct LLParserInfo : public ::testing::Test, public FSMSharedInfo, public LASharedInfo {

	m0st4fa::Logger logger;

	struct SynData {
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
	struct ActData {
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

	enum NonTerminal {
		NT_E,
		NT_EP,
		

	};

	using FSMStateType = m0st4fa::FSMStateType;
	using SymbolType = m0st4fa::GrammaticalSymbol<Terminal, NonTerminal>;

	using enum m0st4fa::FSM_MODE;
	using enum Terminal;
	using LexicalAnalyzerType = m0st4fa::LexicalAnalyzer<TokenType, m0st4fa::FSMTable, std::string_view>;
	using Result = m0st4fa::LexicalAnalyzerResult<TokenType, std::string_view>;

	using SynthesizedType = m0st4fa::LLSynthesizedRecord<SynData>;
	using ActionType = m0st4fa::LLActionRecord<ActData>;

	using StackElementType = m0st4fa::LLStackElement<SymbolType, SynthesizedType, ActionType>;
	using StackType = m0st4fa::StackType<StackElementType>;

	using ProductionType = m0st4fa::ProductionRecord<SymbolType, StackElementType>;
	using GrammarType = m0st4fa::ProductionVector<ProductionType>;

	using LLParsingTableType = m0st4fa::LLParsingTable<GrammarType, Terminal, NonTerminal>;
	using LLParserType = LLParser<GrammarType, LexicalAnalyzerType, SymbolType, LLParsingTableType, FSMTableType, std::string_view>;

};
