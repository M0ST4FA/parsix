#pragma once
#include <array>
#include <iostream>
#include <format>

#include "parsix/stack.h"
#include "parsix/production.h"

// LL PARSER
namespace m0st4fa {

	struct LLTableEntry {
		bool isError = true;

		union {
			size_t prodIndex = SIZE_MAX;
			void* action;
		};

		bool isEmpty = true;

		std::string toString() {

			std::string msg = isError ? "Error Entry\n" : std::format("Production Index of table entry: {}\n", prodIndex);

			return msg;
		};
	};

	template <typename GrammarT, typename TerminalT, typename VariableT>
	struct LLParsingTable {
		using EntryArrType = std::array<LLTableEntry, (size_t)TerminalT::T_COUNT>;
		using ArrType = std::array<EntryArrType, (size_t)VariableT::NT_COUNT>;
		using ItType = ArrType::iterator;
		using ConstItType = ArrType::const_iterator;

		GrammarT grammar;
		mutable ArrType table;


		//! \throw std::range_error This is thrown in case `variable` >= `NT_COUNT` or `terminal` >= `T_COUNT`.
		LLTableEntry& operator()(const VariableT variable, const TerminalT terminal) {
			const size_t intVariable = (size_t)variable;
			const size_t intTerminal = (size_t)terminal;

			if (intTerminal >= (size_t)TerminalT::T_COUNT) {
				const char* tname = typeid(terminal).name();

				throw std::range_error(std::format("`T_COUNT` for the terminal type whose typeid-name is `{}` is not set correctly.", tname));
			}

			return this->at(variable)[intTerminal];
		}

		//! \throw std::range_error This is thrown in case `variable` >= `NT_COUNT` or `terminal` >= `T_COUNT`.
		const LLTableEntry& operator()(const VariableT variable, const TerminalT terminal) const {
			const size_t intVariable = (size_t)variable;
			const size_t intTerminal = (size_t)terminal;
			
			if (intTerminal >= (size_t)TerminalT::T_COUNT) {
				const char* tname = typeid(terminal).name();

				throw std::range_error(std::format("`T_COUNT` for the terminal type whose typeid-name is `{}` is not set correctly.", tname));
			}

			return this->at(variable).at(intTerminal);
		}

		//! \throw std::range_error This is thrown in case `variable` >= `NT_COUNT`.
		const EntryArrType& operator[](const VariableT variable) const {
			const size_t intVariable = (size_t)variable;

			if (intVariable >= table.size()) {
				const char* tname = typeid(variable).name();

				throw std::range_error(std::format("`NT_COUNT` for the nonterminal type whose typeid-name is `{}` is not set correctly.", tname));
			}

			return table.at(intVariable);
		}

		//! \throw std::range_error This is thrown in case `variable` >= `NT_COUNT`.
		const EntryArrType& operator[](const VariableT variable) {
			const size_t intVariable = (size_t)variable;

			if (intVariable >= table.size()) {
				const char* tname = typeid(variable).name();

				throw std::range_error(std::format("`NT_COUNT` for the nonterminal type whose typeid-name is `{}` is not set correctly.", tname));
			}

			return table.at(intVariable);
		}

		//! \throw std::range_error This is thrown in case `variable` >= `NT_COUNT`.
		const EntryArrType& at(const VariableT variable) const {
			const size_t intVariable = (size_t)variable;

			if (intVariable >= table.size()) {
				const char* tname = typeid(variable).name();

				throw std::range_error(std::format("`NT_COUNT` for the nonterminal type whose typeid-name is `{}` is not set correctly.", tname));
			}

			return table.at(variable);
		}

		//! \throw std::range_error This is thrown in case `variable` >= `NT_COUNT`.
		const EntryArrType& at(const VariableT variable) {
			const size_t intVariable = (size_t)variable;

			if (intVariable >= table.size()) {
				const char* tname = typeid(variable).name();

				throw std::range_error(std::format("`NT_COUNT` for the nonterminal type whose typeid-name is `{}` is not set correctly.", tname));
			}

			return table.at(intVariable);
		}


		ItType begin() {
			return table.begin();
		}
		ConstItType begin() const {
			return table.begin();
		}

		ItType end() {
			return table.end();
		}
		ConstItType end() const {
			return table.end();
		}


	};

}

// LR PARSER
namespace m0st4fa {

	// LR TABLE ENTRY
	enum class LRTableEntryType
	{
		TET_ACTION_SHIFT,
		TET_ACTION_REDUCE,
		TET_GOTO,
		TET_ACCEPT,
		TET_ERROR,
		TET_COUNT
	};
	std::string toString(const LRTableEntryType);

	struct LRTableEntry {
		bool isEmpty = true;
		LRTableEntryType type = LRTableEntryType::TET_ERROR;
		size_t number = SIZE_MAX;

		operator std::string() const {
			return this->toString();
		}
		bool operator==(const LRTableEntry& other) const {
			bool initCond = isEmpty == other.isEmpty && type == other.type;

			if (not initCond)
				return false;

			// initCond == true

			if (type == LRTableEntryType::TET_ACTION_SHIFT || type == LRTableEntryType::TET_ACTION_REDUCE)
				return number == other.number;
						
			return true;
		}
		std::string toString() const {

			std::string msg;

			switch (type) {
			case LRTableEntryType::TET_ACTION_SHIFT:
				msg += std::format("SHIFT {}", number);
				break;
			case LRTableEntryType::TET_ACTION_REDUCE:
				msg += std::format("REDUCE {}", number);
				break;
			case LRTableEntryType::TET_GOTO:
				msg += std::format("{}", number);
				break;
			
			default: // ERROR & EMTPY produce an empty string
				return msg;
			}

			return msg;
		}

		/**
		* @return Whether the entry is of type `TET_ERROR`; i.e. whether it is an error entry.
		* @note Being empty is considered erroneous.
		**/ 
		inline bool isError() const {
			return isEmpty || this->type == LRTableEntryType::TET_ERROR;
		}

		//! @return Whether the entry is of type `TET_ACCEPT`; i.e. whether it is an accept entry.
		inline bool isAccept() const {
			return this->type == LRTableEntryType::TET_ACCEPT;
		}
	};

	// TABLE ENTRY MACROS
#define TE_SHIFT(state) m0st4fa::LRTableEntry{false, m0st4fa::LRTableEntryType::TET_ACTION_SHIFT, state}
#define TE_REDUCE(prodNum) m0st4fa::LRTableEntry{false, m0st4fa::LRTableEntryType::TET_ACTION_REDUCE, prodNum}
#define TE_GOTO(state) m0st4fa::LRTableEntry{false, m0st4fa::LRTableEntryType::TET_GOTO, state}
#define TE_ACCEPT() m0st4fa::LRTableEntry{false, m0st4fa::LRTableEntryType::TET_ACCEPT}
#define TE_ERROR() m0st4fa::LRTableEntry{false, m0st4fa::LRTableEntryType::TET_ERROR}

	// @brief Data structure representing an LR parsing table.
	template <typename GrammarT>
	struct LRParsingTable {
		using ProductionType = decltype(GrammarT{}.at(0));
		using SymbolType = decltype(ProductionType{}.prodHead);
		using VariableType = decltype(SymbolType{}.as.nonTerminal);
		using TerminalType = decltype(SymbolType{}.as.terminal);

		static constexpr const size_t VariableCount = (size_t)VariableType::NT_COUNT;
		static constexpr const size_t TerminalCount = (size_t)TerminalType::T_COUNT;
		using ActionArrayType = std::array<LRTableEntry, TerminalCount>;
		using GotoArrayType = std::array<LRTableEntry, VariableCount>;

		GrammarT grammar;
		std::vector<ActionArrayType> actionTable;
		std::vector<GotoArrayType> gotoTable;

		LRParsingTable() = default;
		LRParsingTable(const GrammarT& grammar): grammar{ grammar } {}
		LRParsingTable(const GrammarT& grammar,
			const std::vector<ActionArrayType>& actionTable,
			const std::vector<GotoArrayType>& gotoTable)
			: grammar{ grammar }, actionTable{ actionTable }, gotoTable{ gotoTable }
		{}

		/**
		* @brief This is a convenience function that is an amalgamation of `atAction()` and `atGoto()`.
		* @return Return the given Action or Goto entry for this `state` and this `symbol`.
		**/
		LRTableEntry& at(size_t state, SymbolType symbol) {
			if (symbol.isTerminal)
				return atAction(state, symbol.as.terminal);

			return atGoto(state, symbol.as.nonTerminal);
		}

		//! @return Return the given Action entry for this `state` and this `terminal`.
		LRTableEntry& atAction(size_t state, TerminalType terminal) noexcept(true) {
			if (state >= this->actionTable.size())
				actionTable.resize(state + 1, {});

			return this->actionTable.at(state).at((size_t)terminal);
		};

		//! @return Return all the Action entries for `state`.
		const ActionArrayType& atActionRow(size_t state) const noexcept(true) {
			return this->actionTable.at(state);
		}

		//! @return Return all non-error action entries for this state.
		std::vector<TerminalType> getActions(size_t state) const noexcept(true) {
			std::vector<TerminalType> res;

			for (size_t terminal = 0; terminal < TerminalCount; terminal++)
				if (not this->actionTable[state][terminal].isError())
					res.push_back((TerminalType)terminal);

			return res;
		}

		//! @return Return the given Goto entry for this `state` and this `nonTerminal`.
		LRTableEntry& atGoto(size_t state, VariableType nonTerminal) noexcept(true) {
			if (state >= this->gotoTable.size())
				gotoTable.resize(state + 1, {});

			return this->gotoTable.at(state).at((size_t)nonTerminal);
		};

		//! @return Return all the Goto entries for `state`.
		const GotoArrayType& atGotoRow(size_t state) const noexcept(true) {
			return this->gotoTable.at(state);
		}

		//! @return Return all non-error goto entries for `state`.
		std::vector<VariableType> getGotos(size_t state) const noexcept(true) {
			std::vector<VariableType> res;

			for (size_t variable = 0; variable < VariableCount; variable++)
				if (not this->gotoTable[state][variable].isError())
					res.push_back((VariableType)variable);

			return res;
		}

		//! @brief Reserve rows in the Action and Goto tables.
		void reserveRows(size_t newRowNum) noexcept(true) {
			this->actionTable.resize(newRowNum);
			this->gotoTable.resize(newRowNum);
		}

	};

};