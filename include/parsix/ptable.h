#pragma once
#include <array>

#include <format>

#include "parsix/production.h"

// LL PARSER
namespace m0st4fa::parsix {

	/**
	 * @brief Represents a table entry in an LL parsing table.
	 */
	struct LLTableEntry {

		/**
		 * @brief Whether this table entry represents an error or not.
		 */
		bool isError = true;

		union {
			/**
			 * @brief The production index within the production vector of the grammar. This is in case this entry represents a production.
			 */
			size_t prodIndex = SIZE_MAX;

			/**
			 * @brief A pointer to a function to execute. This is in case this entry represents a an action.
			 */
			void* action;
		};

		/**
		 * @brief Whether this table entry is empty or not.
		 */
		bool isEmpty = true;

		std::string toString() {

			std::string msg = isError ? "Error Entry\n" : std::format("Production Index of table entry: {}\n", prodIndex);

			return msg;
		};
	};

	/**
	 * @brief An LL parsing table.
	 * 
	 * @tparam GrammarT The type of the object representing the grammar (typically the production vector containing the productions of the grammar).
	 * @tparam TerminalT The type of a terminal in the grammar.
	 * @tparam VariableT The type of a non-terminal in the grammar.
	 */
	template <typename GrammarT, typename TerminalT, typename VariableT>
	struct LLParsingTable {

		/**
		 * @brief Aliases the type of an array of entry objects.
		 */
		using EntryArrType = std::array<LLTableEntry, (size_t)TerminalT::T_COUNT>;

		/**
		 * @brief Aliases the type of an array of EntryArrType objects.
		 */
		using EntryArrType2D = std::array<EntryArrType, (size_t)VariableT::NT_COUNT>;

		/**
		 * @brief Aliases the type of an iterator of EntryArrType2D.
		 */
		using ItType = EntryArrType2D::iterator;

		/**
		 * @brief Aliases the type of a constant iterator of EntryArrType2D.
		 */
		using ConstItType = EntryArrType2D::const_iterator;

		/**
		 * @brief The grammar that this parsing table is for.
		 * @note The reason for storing the grammar here is due to the large number of references made to it. Basically almost every function has to access it. This would be very inconvenient.
		 * @attention When making a structure that has LLParsingTable field, make sure you don't use a GrammarT field. This is because such a field would be redundant and will use up a huge amount of memory. Instead, you can make a reference field or use the field of LLParsingTable directly or via a method (which I prefer).
		 */
		GrammarT grammar;

		/**
		 * @brief The underlying storage of the table.
		 */
		mutable EntryArrType2D table;

		/**
		 * @brief Accesses a given entry within the table, using a variable as the 1D index and the terminal as the 2D index.
		 * @param[in] variable The variable to use to access the table entry.
		 * @param[in] terminal The terminal to use to access the table entry.
		 * @returns A reference to the table entry that corresponds to `variable` and `terminal`.
		 * @throws std::range_error If `variable` >= `NT_COUNT` or `terminal` >= `T_COUNT`
		 */
		LLTableEntry& operator()(const VariableT variable, const TerminalT terminal) {
			const size_t intVariable = (size_t)variable;
			const size_t intTerminal = (size_t)terminal;

			if (intTerminal >= (size_t)TerminalT::T_COUNT) {
				const char* tname = typeid(terminal).name();

				throw std::range_error(std::format("`T_COUNT` for the terminal type whose typeid-name is `{}` is not set correctly.", tname));
			}

			return this->at(variable)[intTerminal];
		}

		/**
		 * @brief Accesses a given entry within the table, using a variable as the 1D index and the terminal as the 2D index.
		 * @param[in] variable The variable to use to access the table entry.
		 * @param[in] terminal The terminal to use to access the table entry.
		 * @returns A **constant** reference to the table entry that corresponds to `variable` and `terminal`.
		 * @throws std::range_error If `variable` >= `NT_COUNT` or `terminal` >= `T_COUNT`
		 */
		const LLTableEntry& operator()(const VariableT variable, const TerminalT terminal) const {
			const size_t intVariable = (size_t)variable;
			const size_t intTerminal = (size_t)terminal;
			
			if (intTerminal >= (size_t)TerminalT::T_COUNT) {
				const char* tname = typeid(terminal).name();

				throw std::range_error(std::format("`T_COUNT` for the terminal type whose typeid-name is `{}` is not set correctly.", tname));
			}

			return this->at(variable).at(intTerminal);
		}

		/**
		 * @brief Accesses an array of table entries corresponding a given variable. No boundary-checking.
		 * @param[in] variable The variable corresponding to the array of table entries to be retrieved.
		 * @returns A **constant** reference to the array of table entries corresponding to `variable`.
		 * @throws std::range_error If `variable` >= `NT_COUNT`.
		 */
		const EntryArrType& operator[](const VariableT variable) const {
			const size_t intVariable = (size_t)variable;

			if (intVariable >= table.size()) {
				const char* tname = typeid(variable).name();

				throw std::range_error(std::format("`NT_COUNT` for the nonterminal type whose typeid-name is `{}` is not set correctly.", tname));
			}

			return table[intVariable];
		}

		/**
		 * @brief Accesses an array of table entries corresponding a given variable. No boundary-checking.
		 * @param[in] variable The variable corresponding to the array of table entries to be retrieved.
		 * @returns A **constant** reference to the array of table entries corresponding to `variable`.
		 * @throws std::range_error If `variable` >= `NT_COUNT`.
		 */
		const EntryArrType& operator[](const VariableT variable) {
			const size_t intVariable = (size_t)variable;

			if (intVariable >= table.size()) {
				const char* tname = typeid(variable).name();

				throw std::range_error(std::format("`NT_COUNT` for the nonterminal type whose typeid-name is `{}` is not set correctly.", tname));
			}

			return table[intVariable];
		}

		/**
		 * @brief Accesses an array of table entries corresponding a given variable.
		 * @param[in] variable The variable corresponding to the array of table entries to be retrieved.
		 * @returns A **constant** reference to the array of table entries corresponding to `variable`.
		 * @throws std::range_error If `variable` >= `NT_COUNT`.
		 */
		const EntryArrType& at(const VariableT variable) const {
			const size_t intVariable = (size_t)variable;

			if (intVariable >= table.size()) {
				const char* tname = typeid(variable).name();

				throw std::range_error(std::format("`NT_COUNT` for the nonterminal type whose typeid-name is `{}` is not set correctly.", tname));
			}

			return table.at(variable);
		}

		/**
		 * @brief Accesses an array of table entries corresponding a given variable.
		 * @param[in] variable The variable corresponding to the array of table entries to be retrieved.
		 * @returns A **constant** reference to the array of table entries corresponding to `variable`.
		 * @throws std::range_error If `variable` >= `NT_COUNT`.
		 */
		const EntryArrType& at(const VariableT variable) {
			const size_t intVariable = (size_t)variable;

			if (intVariable >= table.size()) {
				const char* tname = typeid(variable).name();

				throw std::range_error(std::format("`NT_COUNT` for the nonterminal type whose typeid-name is `{}` is not set correctly.", tname));
			}

			return table.at(intVariable);
		}

		/**
		 * @brief Gets the iterator pointing at the beginning of the underlying array object storing the entry objects.
		*/
		ItType begin() {
			return table.begin();
		}

		/**
		 * @brief Gets the constant iterator pointing at the beginning of the underlying array object storing the entry objects.
		*/
		ConstItType begin() const {
			return table.begin();
		}

		/**
		 * @brief Gets the iterator pointing at the end of the underlying array object storing the entry objects.
		*/
		ItType end() {
			return table.end();
		}

		/**
		 * @brief Gets the constant iterator pointing at the end of the underlying array object storing the entry objects.
		*/
		ConstItType end() const {
			return table.end();
		}


	};

}

// LR PARSER
namespace m0st4fa::parsix {

	// LR TABLE ENTRY

	/**
	 * @brief The type of an LR table entry. These entries generally indicate an action for the LR parser to perform. Additional meta-data needed for action is included within the actual entry object.
	 */
	enum class LRTableEntryType
	{
		/**
		 * @brief Indicates a shift entry; this entry means: take a shift action.
		 */
		TET_ACTION_SHIFT,

		/**
		 * @brief Indicates a reduce entry; this entry means: take a reduction action.
		 */
		TET_ACTION_REDUCE,

		/**
		 * @brief Indicates a GOTO entry; this entry means: look in the GOTO table.
		 */
		TET_GOTO,

		/**
		 * @brief Indicates an accept entry; this entry means: the current string is accepted by the parser as part of the grammar.
		 */
		TET_ACCEPT,

		/**
		 * @brief Indicates an error entry; this entry means: the current string doesn't accept.
		 */
		TET_ERROR,

		/**
		 * @brief The total number of LRTableEntryType enumerations, i.e., the total number of types of LR table entries.
		 */
		TET_COUNT
	};

	std::string toString(const LRTableEntryType);

	/**
	 * @brief Represents an entry in an LR parsing table.
	 */
	struct LRTableEntry {

		/**
		 * @brief Whether this entry is empty or not.
		 */
		bool isEmpty = true;

		/**
		 * @brief The type of this entry.
		 */
		LRTableEntryType type = LRTableEntryType::TET_ERROR;

		/**
		 * @brief The state to shift or the number of states to reduce. This is used for comparison between different entries.
		 */
		size_t number = SIZE_MAX;

		/**
		 * @brief Converts this LRTableEntry object to string. This is just syntactic sugar over toString.
		 * @returns The string representation of this LRTableEntry object.
		 */
		operator std::string() const {
			return this->toString();
		}

		/**
		 * @brief Compares this LRTableEntry with another one for equality.
		 * @details Equality here means the following: 1) They both are empty or both are not empty, 2) They have the same type, 3) They shift the same state (in case they are both of shift type) or reduce by the same number of states (in case they are both of the reduce type.)
		 * @param[in] rhs The right-hand side of the comparison.
		 * @returns `true` if this object and `rhs` are equal; `false` otherwise.
		 */
		bool operator==(const LRTableEntry& rhs) const {
			bool initCond = isEmpty == rhs.isEmpty && type == rhs.type;

			if (!initCond)
				return false;

			// initCond == true

			if (type == LRTableEntryType::TET_ACTION_SHIFT || type == LRTableEntryType::TET_ACTION_REDUCE)
				return number == rhs.number;
						
			return true;
		}

		/**
		 * @brief Converts this LRTableEntry object to string.
		 * @returns A string representation of this LRTableEntry object.
		 */
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
		* @brief Checks whether the entry is an error entry.
		* @note Being empty is considered erroneous. "Empty" is defined as: being of type LRTableEntryType::TET_ERROR or being empty.
		* @returns `true` if this entry is of type LRTableEntryType::TET_ERROR or is empty; `false` otherwise.
		**/ 
		inline bool isError() const {
			return isEmpty || this->type == LRTableEntryType::TET_ERROR;
		}

		/**
		 * @brief Checks whether this is an accept entry.
		 * @returns `true` if the type of this entry is LRTableEntryType::TET_ACCEPT; `false` otherwise.
		 */
		inline bool isAccept() const {
			return this->type == LRTableEntryType::TET_ACCEPT;
		}
	};

	// TABLE ENTRY MACROS

	/**
	 * @brief Creates a table entry of type LRTableEntryType::TET_ACTION_SHIFT.
	 */
#define TE_SHIFT(state) m0st4fa::LRTableEntry{false, m0st4fa::LRTableEntryType::TET_ACTION_SHIFT, state}

	/**
	  * @brief Creates a table entry of type LRTableEntryType::TET_ACTION_REDUCE.
	  */
#define TE_REDUCE(prodNum) m0st4fa::LRTableEntry{false, m0st4fa::LRTableEntryType::TET_ACTION_REDUCE, prodNum}

	/**
	   * @brief Creates a table entry of type LRTableEntryType::TET_ACTION_GOTO.
	   */
#define TE_GOTO(state) m0st4fa::LRTableEntry{false, m0st4fa::LRTableEntryType::TET_GOTO, state}
		
	 /**
	   * @brief Creates a table entry of type LRTableEntryType::TET_ACTION_ACCEPT.
	   */
#define TE_ACCEPT() m0st4fa::LRTableEntry{false, m0st4fa::LRTableEntryType::TET_ACCEPT}

	 /**
		* @brief Creates a table entry of type LRTableEntryType::TET_ACTION_ERROR.
		*/
#define TE_ERROR() m0st4fa::LRTableEntry{false, m0st4fa::LRTableEntryType::TET_ERROR}

	/**
	 * @brief Represents an LR parsing table.
	 * @tparam GrammarT The type of object representing the grammar. Generally, it is a vector of production record objects.
	 */
	template <typename GrammarT>
	struct LRParsingTable {

		/**
		 * @brief Aliases the type of a production.
		 */
		using ProductionType = decltype(GrammarT{}.at(0));

		/**
		 * @brief Aliases the type of a	grammar symbol.
		 */
		using SymbolType = decltype(ProductionType{}.prodHead);

		/**
		 * @brief Aliases the type of a non-terminal.
		 */
		using VariableType = decltype(SymbolType{}.as.nonTerminal);

		/**
		 * @brief Aliases the type of a terminal.
		 */
		using TerminalType = decltype(SymbolType{}.as.terminal);

		/**
		 * @brief The total number of non-terminals in the grammar.
		 */
		static constexpr const size_t VAR_COUNT = (size_t)VariableType::NT_COUNT;

		/**
		 * @brief The total number of terminals in the grammar.
		 */
		static constexpr const size_t TER_COUNT = (size_t)TerminalType::T_COUNT;

		/**
		 * @brief Aliases the type of an array of LRTableEntry objects intended to store action entries. The difference between this and GotoArrayType is in the total number of entries.
		 */
		using ActionArrayType = std::array<LRTableEntry, TER_COUNT>;

		/**
		 * @brief Aliases the type of an array of LRTableEntry objects intended to store GOTO entries. The difference between this and ActionArrayType is in the total number of entries.
		 */
		using GotoArrayType = std::array<LRTableEntry, VAR_COUNT>;

		/**
		 * @brief The grammar.
		 */
		GrammarT grammar;

		/**
		 * @brief The table storing action entries. Accessed for every terminal and state to get the action to execute for this terminal and state.
		 */
		std::vector<ActionArrayType> actionTable;

		/**
		 * @brief The table storing GOTO entries. Used for reductions to figure out what state to reduce by.
		 */
		std::vector<GotoArrayType> gotoTable;

		/**
		 * @brief Default constructor.
		 */
		LRParsingTable() = default;

		/**
		 * @brief Converting constructor. Initializes the LRParsingTable object with the given grammar object.
		 * @param[in] grammar The grammar to initialize the LRParsingTable object with.
		 */
		LRParsingTable(const GrammarT& grammar): grammar{ grammar } {}

		/**
		 * @brief Converting constructor. Initializes the LRParsingTable with the given grammar, action table and GOTO table.
		 * @param[in] grammar The grammar with which to initialize the parsing table.
		 * @param[in] actionTable The action table with which to initialize the parsing table.
		 * @param[in] gotoTable The GOTO table with which to initialize the parsing table.
		 */
		LRParsingTable(const GrammarT& grammar,
			const std::vector<ActionArrayType>& actionTable,
			const std::vector<GotoArrayType>& gotoTable)
			: grammar{ grammar }, actionTable{ actionTable }, gotoTable{ gotoTable }
		{}

		/**
		* @brief This is a convenience function that is an amalgamation of `atAction()` and `atGoto()`.
		* @param[in] state The state used as the first index of the table.
		* @param[in] symbol The symbol used as the second index of the table.
		* @return The given Action (if `symbol` is terminal) or GOTO (if `symbol` is non-terminal) entry for this `state` and this `symbol`.
		**/
		LRTableEntry& at(size_t state, SymbolType symbol) {
			if (symbol.isTerminal)
				return this->atAction(state, symbol.as.terminal);

			return this->atGoto(state, symbol.as.nonTerminal);
		}

		/**
		 * @brief Gets the Action entry at this `state` and this `terminal`.
		 * @param[in] state The state used as the first index of the table.
		 * @param[in] terminal The terminal used as the second index of the table.
		 * @returns The Action entry for this `state` and this `terminal`.
		 */
		LRTableEntry& atAction(size_t state, TerminalType terminal) noexcept(true) {
			if (state >= this->actionTable.size())
				actionTable.resize(state + 1, {});

			return this->actionTable.at(state).at((size_t)terminal);
		};

		/**
		 * @brief Gets all of the **Action table** entries corresponding to a given state.
		 * @param[in] state The state used as the index of the Action table.
		 * @returns The entire row of the Action table indexed by `state`.
		 */
		const ActionArrayType& atActionRow(size_t state) const noexcept(true) {
			return this->actionTable.at(state);
		}

		/**
		 * @brief Gets all of the **non-error** Action table entries corresponding to a given state.
		 * @param[in] state The state used as the index of the Action table.
		 * @returns Only the non-error entries from the row of the Action table indexed by `state`.
		 */
		std::vector<TerminalType> getActions(size_t state) const noexcept(true) {
			std::vector<TerminalType> res;

			for (size_t terminal = 0; terminal < TER_COUNT; terminal++)
				if (not this->actionTable[state][terminal].isError())
					res.push_back((TerminalType)terminal);

			return res;
		}

		/**
		 * @brief Gets the GOTO entry at this `state` and this `terminal`.
		 * @param[in] state The state used as the first index of the table.
		 * @param[in] nonTerminal The non-terminal used as the second index of the table.
		 * @returns The GOTO entry for this `state` and this `terminal`.
		 */
		LRTableEntry& atGoto(size_t state, VariableType nonTerminal) noexcept(true) {
			if (state >= this->gotoTable.size())
				gotoTable.resize(state + 1, {});

			return this->gotoTable.at(state).at((size_t)nonTerminal);
		};

		/**
		 * @brief Gets all of the **GOTO table** entries corresponding to a given state.
		 * @param[in] state The state used as the index of the GOTO table.
		 * @returns The entire row of the GOTO table indexed by `state`.
		 */
		const GotoArrayType& atGotoRow(size_t state) const noexcept(true) {
			return this->gotoTable.at(state);
		}

		/**
		 * @brief Gets all of the **non-error** GOTO table entries corresponding to a given state.
		 * @param[in] state The state used as the index of the GOTO table.
		 * @returns Only the non-error entries from the row of the GOTO table indexed by `state`.
		 */
		std::vector<VariableType> getGotos(size_t state) const noexcept(true) {
			std::vector<VariableType> res;

			for (size_t variable = 0; variable < VAR_COUNT; variable++)
				if (not this->gotoTable[state][variable].isError())
					res.push_back((VariableType)variable);

			return res;
		}

		/**
		 * @brief Reserves rows in both the Action and GOTO tables. It simply resizes (using the `resize` method of vectors) the tables.
		 * @param[in] newRowNum The new size of the rows of both tables.
		 */
		void reserveRows(size_t newRowNum) noexcept(true) {
			this->actionTable.resize(newRowNum);
			this->gotoTable.resize(newRowNum);
		}

	};

};