#pragma once
#include <string>
#include <vector>
#include <set>

#include "parsix/exception.h"
#include "parsix/stack.h"
#include "utility/Logger.h"


namespace m0st4fa::parsix {

	template <typename SymbolT>
	class GrammaticalSymbolString;

	template <typename SymbolT, typename ProductionElementT>
	class ProductionRecord {
		using StackType = StackType<ProductionElementT>;
		using SymbolStringType = GrammaticalSymbolString<SymbolT>;

		/**
		* @brief The number of grammar *symbols* (terminal and non-terminal) in the production body.
		*/
		size_t m_Size = 0;

	protected:

		/**
		 * @brief The logger object used to log information to different output streams.
		 */
		Logger p_Logger;

	public:

		/**
		 * @brief The head symbol of the production. This must be a single non-terminal.
		 */
		SymbolT prodHead = SymbolT{};

		/**
		 * @brief The body of the production, a list of symbols and records.
		 */
		std::vector<ProductionElementT> prodBody;

		/**
		 * @brief The index (or number) of the production within a given production vector in which it exists. This variable is populated only when the production is an element of a ProductionVector. 
		 * @note It is important for many algorithms.
		 */
		size_t prodNumber = 0;

		/**
		* @brief The action to execute after matching the production.
		* @note Expected signature: `void(*)(StackType&, StackElementType&, ParserResultType&)`. This is a pointer to a function that returns `void` and takes an ordered triple of the type `(StackType&, StackElementType&, ParserResultType&)` as an argument.
		**/ 
		void* postfixAction = nullptr;

		/**
		 * @brief Default constructor for ProductionRecord.
		 *
		 * @details This is the default constructor for the ProductionRecord class. It initializes a new instance of the class with no parameters.
		 */
		ProductionRecord() = default;

		/**
		 * @brief Parameterized constructor for ProductionRecord.
		 *
		 * @param head The head of the production.
		 * @param body The body of the production.
		 * @param number The number of the production.
		 * @param postfixAct The postfix action of the production.
		 *
		 * @details This constructor initializes a new instance of the ProductionRecord class using the provided head, body, number, and postfix action.	
		 * It checks if the head is a terminal or the body is empty, it logs an error message and throws a logic error. 
		 *
		 * @throws std::logic_error If the head of a production is a terminal or if the body of a production is empty.
		 */
		ProductionRecord(const SymbolT& head, const std::vector<ProductionElementT>& body, const size_t number, void* postfixAct = nullptr) : prodHead{ head }, prodBody{ body }, prodNumber{ number }, postfixAction { postfixAct } {

			// the head must be a non-terminal
			if (head.isTerminal) {
				this->p_Logger.log(LoggerInfo::ERR_INVALID_VAL, "The head of a production must be a non-terminal.");

				throw std::logic_error("The head of a production must be a non-terminal.");
			}

			// the body cannot be empty
			if (prodBody.empty()) {
				this->p_Logger.log(LoggerInfo::ERR_EMPTY_PROD_BODY, "The body of a production cannot be empty.");

				throw std::logic_error("The body of a production cannot be empty.");
			}

			this->m_Size = std::count_if(prodBody.begin(), prodBody.end(), [this](const ProductionElementT& prodElement) {
				// count an element only if it is a grammar symbol
				return prodElement.type == ProdElementType::PET_GRAM_SYMBOL;
				});

		}

		/**
		 * @brief Copy assignment operator for ProductionRecord.
		 *
		 * @param other The ProductionRecord instance to be copied.
		 *
		 * @return A reference to the current instance.
		 *
		 * @details This is the copy assignment operator for the ProductionRecord class. It copies the state of an existing instance into the current	instance and returns a reference to the current instance.
		 */
		ProductionRecord& operator=(const ProductionRecord& other) {
			prodHead = other.prodHead;
			prodBody = other.prodBody;
			prodNumber = other.prodNumber;
			m_Size = other.m_Size;
			return *this;
		}

		/**
		 * @brief Equality operator for ProductionRecord.
		 *
		 * @param rhs The ProductionRecord instance to be compared.
		 *
		 * @return `true` if the heads and bodies of the two instances are equal; `false` otherwise.
		 *
		 * @details This is the equality operator for the ProductionRecord class. It checks if the head and body of the current instance are equal to the head and body of the given instance.
		 */
		bool operator==(const ProductionRecord& rhs) const {

			// if their heads are not equal, they are not equal
			if (this->prodHead != rhs.prodHead)
				return false;

			// if their heads are equal, then they are equal if their 
			// bodies are also equal
			return this->prodBody == rhs.prodBody;
		}

		/**
		 * @brief String conversion operator of the ProductionRecord class. It is just syntactic sugar over toString.
		 * @returns std::string representation of this ProductionRecord instance.
		 */
		operator std::string() const {
			return this->toString();
		}

		/**
		 * @brief Converting operator that converts this instance to SymbolStringType object.
		 * @returns SymbolStringType object that represents this ProductionRecord instance.
		 */
		operator SymbolStringType() const {
			return this->toSymbolString();
		}

		/**
		 * @brief Returns an iterator pointing to the beginning of the production body.
		 *
		 * This function returns an iterator that points to the first element in the production body. The iterator can be used to iterate through the elements in the production body from beginning to end.
		 *
		 * @return An iterator pointing to the first element in the production body.
		 */
		auto begin() const { return this->prodBody.begin(); }

		/**
		 * @brief Returns a reverse iterator pointing to the end of the production body.
		 *
		 * @details This function returns a reverse iterator that points to the last element in the production body. The reverse iterator can be used to iterate through the elements in the production body from end to beginning.
		 *
		 * @return A reverse iterator pointing to the last element in the production body.
		 */
		auto rbegin() const { return this->prodBody.rbegin(); }

		/**
		 * @brief Returns an iterator pointing one past the end of the production body.
		 *
		 * @details This function returns an iterator that points one past the last element in the production body. This iterator acts as a placeholder; attempting to access it results in undefined behavior.
		 *
		 * @return An iterator pointing one past the last element in the production body.
		 */
		auto end() const { return this->prodBody.end(); }

		/**
		 * @brief Returns a reverse iterator pointing to one position before the beginning of the production body.
		 *
		 * @details This function returns a reverse iterator that points to one position before the first element in the production body. This reverse iterator acts as a placeholder; attempting to access it results in undefined behavior.
		 *
		 * @return A reverse iterator pointing to one position before the first element in the production body.
		 */
		auto rend() const { return this->prodBody.rend(); }

		/**
		 * @brief Returns a reference to the element of the body at the given index.
		 *
		 * @param index The index of the element.
		 *
		 * @return A reference to the element of the body at the given index.
		 */
		ProductionElementT& at(size_t index) {
			return this->prodBody.at(index);
		}

		/**
		 * @brief Returns a const reference to the element of the body at the given index.
		 *
		 * @param index The index of the element.
		 *
		 * @return A const reference to the element of the body at the given index.
		 */
		const ProductionElementT& at(size_t index) const {
			return this->prodBody.at(index);
		}

		/**
		 * @brief Returns a copy of the element of the body at the given index.
		 *
		 * @param index The index of the element.
		 *
		 * @return A copy of the element of the body at the given index.
		 */
		ProductionElementT get(size_t index) const {
			return this->at(index);
		}

		/**
		 * @brief Converts the production record to a string.
		 *
		 * @details This function converts the production record to a string by concatenating the string representation of the head, the arrow symbol, and the body of the production. It relies on similar conversion functions for all of the previously mentioned symbols.
		 *
		 * @return A string representation of the production record.
		 */
		std::string toString() const {

			std::string str = this->prodHead.toString() + " ->";

			// body
			for (const ProductionElementT& prodElement : this->prodBody)
				str += " " + (std::string)prodElement;

			return str;
		}

		/**
		 * @brief Converts the production body to a SymbolString object.
		 *
		 * @details This function converts the production body to a symbol string by pushing back each grammar symbol in the body to a SymbolString object.
		 *
		 * @return A SymbolString representation of the production body.
		 */
		SymbolStringType toSymbolString() const {
			SymbolStringType string{};

			for (const ProductionElementT& pe : this->prodBody) {

				if (pe.type != ProdElementType::PET_GRAM_SYMBOL)
					continue;

				string.push_back(pe.as.gramSymbol);
			}

			return string;
		}


		/**
		 * @brief Returns the size of the production body.
		 *
		 * @return The size of the production body.
		 */
		size_t size() const {
			return this->m_Size;
		}

		/**
		 * @brief Checks if the production body contains a given symbol.
		 *
		 * @param symbol The symbol to be checked.
		 *
		 * @details This function checks if the production body contains a given symbol. 
		 * It iterates through each element in the production body. If the element is a grammar symbol and it is equal to the given symbol, it returns `true`. Otherwise, it returns `false`.
		 *
		 * @return `true` if the production body contains the given symbol; `false` otherwise.
		 */
		bool contains(const SymbolT& symbol) const {

			for (const auto& prodElement : this->prodBody) {

				if (prodElement.type != ProdElementType::PET_GRAM_SYMBOL)
					continue;

				// the stack element is a grammar symbol
				if (prodElement.as.gramSymbol == symbol)
					return true;
			}

			// if we reach here, we didn't find the symbol (otherwise, we would've returned from the loop with `true`)
			return false;
		}

		/**
		 * @brief Checks if the production body contains an epsilon.
		 *
		 * @return `true` if the production body contains an epsilon; `false` otherwise.
		 */
		bool isEpsilon() const {
			return this->contains(SymbolT::EPSILON);
		}

		/**
		 * @brief Returns the last grammar symbol in the production body.
		 *
		 * @return The last grammar symbol in the production body.
		 */
		SymbolT getLastSymbol() const {
			return (std::find_if(this->rbegin(), this->rend(), [](const ProductionElementT& pe) {
				return pe.type == ProdElementType::PET_GRAM_SYMBOL;
				})->as.gramSymbol);
		}
	};
	
	 /**
	  * @brief Overload of the stream insertion operator for ProductionRecord.
	  *
	  * @param os The output stream.
	  * @param prod The ProductionRecord to be inserted into the stream.
	  *
	  * @details This function overloads the stream insertion operator for the ProductionRecord class. It inserts the string representation of the ProductionRecord into the output stream.
	  *
	  * @return The output stream.
	  */
	template <typename SymbolT, typename ProductionElementT>
	std::ostream& operator<<(std::ostream& os, const ProductionRecord<SymbolT, ProductionElementT>& prod) {

		return std::cout << prod.toString() << "\n";
	};

	/**
	 * @brief Represents a grammatical symbol.
	 * @note **Usage**:@n
	 *	 - Use `isTerminal` to check whether the symbol is terminal or not.
	 *	 - If it is a terminal, access it as `.as.terminal`; otherwise, access it as `.as.nonTerminal`.
	 */
	template <typename TerminalT, typename VariableT>
	struct GrammaticalSymbol {

		/**
		 * @brief `true` if this grammatical symbol is a terminal; `false` otherwise.
		 */
		bool isTerminal = false;

		/**
		 * @brief Union for storing the terminal or non-terminal object representation.
		 */
		union {
			TerminalT terminal;
			VariableT nonTerminal;
		} as;
		
		static const GrammaticalSymbol EPSILON;
		
		static const GrammaticalSymbol END_MARKER;

		/**
		 * @brief Checks if the grammatical symbol is equal to a given token. Particularly, it checks whether the token's name is identical to the grammatical symbol.
		 *
		 * @param token The token to be checked.
		 *
		 * @return `true` if the grammatical symbol is a terminal and it is equal to the token's name; `false` otherwise.
		 */
		template <typename TokenT>
			requires requires (TokenT tok) { tok.name; }
		bool operator==(const TokenT& token) const {

			if (this->isTerminal)
				return this->as.terminal == token.name;

			return false;
		}

		/**
		 * @brief Checks if the grammatical symbol is equal to another grammatical symbol.
		 *
		 * @param other The other grammatical symbol to be checked.
		 *
		 * @return `true` if both symbols are equal; `false` otherwise.
		 */
		bool operator==(const GrammaticalSymbol& other) const {

			if (isTerminal)
				return as.terminal == other.as.terminal;
			else
				return as.nonTerminal == other.as.nonTerminal;
			
		};

		/**
		 * @brief Checks if the grammatical symbol is less than another grammatical symbol.
		 *
		 * @param other The other grammatical symbol to be checked.
		 *
		 * @return `true` if this symbol is a terminal and the other is a non-terminal, or if both symbols are terminals and the current symbol is less than the other symbol (by converting them to integers first), or if both symbols are non-terminals and the current symbol is less than the other symbol (also by converting them to integers first); `false` otherwise.
		 */
		bool operator<(const GrammaticalSymbol& other) const {
			// one is a terminal and the other is non-terminal
			if (isTerminal != other.isTerminal)
				if (this->isTerminal)
					return true;
				else return false;

			// both are terminals
			if (isTerminal && other.isTerminal)
				return (size_t)this->as.terminal < (size_t)other.as.terminal;

			// both are non-terminals
			return (size_t)this->as.nonTerminal < (size_t)other.as.nonTerminal;
		}

		/**
		 * @brief Converts the grammatical symbol to a string. This function is syntactic sugar over toString.
		 * @returns A string representation of the grammatical symbol.
		 */
		operator std::string() const {
			return this->toString();
		}

		/**
		 * @brief Converts the grammatical symbol to a string.
		 *
		 * @details This function converts the grammatical symbol to a string. If the symbol is a terminal, it returns the string representation of the terminal. If the symbol is a non-terminal, it returns the string representation of the non-terminal enclosed in angle brackets (to differentiate it from terminals.)
		 *
		 * @return A string representation of the grammatical symbol.
		 */
		std::string toString() const {
			return this->isTerminal ? toString(this->as.terminal) : std::string("<") + toString(this->as.nonTerminal) + ">";
		}

	};

	/**
	 * @brief The epsilon grammatical symbol. Represents an empty symbol (and also an empty string.)
	 */
	template <typename TerminalT, typename VariableT>
	const GrammaticalSymbol<TerminalT, VariableT>
	GrammaticalSymbol<TerminalT, VariableT>::EPSILON = { true, {.terminal = TerminalT::T_EPSILON} };

	/**
	 * @brief The file (or source) end marker (i.e.that marks the end of the file or source.)
	 */
	template <typename TerminalT, typename VariableT>
	const GrammaticalSymbol<TerminalT, VariableT>
		GrammaticalSymbol<TerminalT, VariableT>::END_MARKER = { true, {.terminal = TerminalT::T_EOF} };

	template <typename TerminalT, typename VariableT>
	using Symbol = GrammaticalSymbol<TerminalT, VariableT>;

	/**
	 * @brief Overload of the stream insertion operator for Symbol.
	 *
	 * @param os The output stream.
	 * @param symbol The Symbol to be inserted into the stream.
	 *
	 * @details This function overloads the stream insertion operator for the Symbol class. It inserts the string representation of the Symbol into the output stream.
	 *
	 * @return The output stream.
	 */
	template <typename TerminalT, typename VariableT>
	std::ostream& operator<<(std::ostream& os, const Symbol<TerminalT, VariableT>& symbol) {

		return os << symbol.toString();
		
	}
	
	/**
	 * @brief Class representing a string of grammatical symbols.
	 */
	template <typename SymbolT>
	class GrammaticalSymbolString {

		/**
		 * @brief Alias for the type of a symbol object.
		 */
		using SymbolType = SymbolT;

		/**
		 * @brief Alias for the type of a set (`std::set`) of symbol objects.
		 */
		using SetType = std::set<SymbolType>;

		/**
		 * @brief Alias for the of a vector (`std::vector`) of symbol objects.
		 */
		using SymVecType = std::vector<SymbolType>;

		/**
		 * @brief Alias for the type of an iterator to the beginning of a vector of symbol objects.
		 */
		using SymVecItType = std::vector<SymbolType>::iterator;

		/**
		 * @brief Represents the type of an object representing the FIRST set of a grammar. Alias to the type of a vector of sets of symbol objects.
		 */
		using FIRSTVecType = std::vector<SetType>;

		/**
		 * @brief Indicates whether the FIRST set of this string based on the FIRST set of some grammar is already calculated and cached.
		 */
		bool m_CalculatedFIRST = false;

		/**
		 * @brief The cached first set of this grammar symbol string, calculated from the FIRST set of some grammar.
		 */
		SetType FIRST{ };

		/**
		 * @brief The tuple of symbol objects that make up the string.
		 */
		SymVecType symbols;

	protected:

		/**
		 * @brief The logger used to write information to the different output streams.
		 */
		Logger m_Logger;

	public:

		/**
		 * @brief The default constructor.
		 */
		GrammaticalSymbolString() = default;

		/**
		 * @brief A converting constructor that initializes a GrammaticalSymbolString from a list of SymbolT objects.
		 * 
		 * @param[in] symbols The list of symbols from which the string is to be initialized.
		 */
		GrammaticalSymbolString(const std::initializer_list<SymbolType>& symbols) : symbols{ symbols } 
		{};

		/**
		 * @brief A converting constructor that initializes a GrammaticalSymbolString object from a vector of SymbolT objects by taking the iterators that point to the beginning and end of that vector.
		 * 
		 * @param[in] begin The iterator that points to the beginning of the vector of SymbolT objects.
		 * @param[in] end The iterator that points to the end of the vector of SymbolT objects.
		 */
		GrammaticalSymbolString(const SymVecItType begin, const SymVecItType end) : symbols{ begin, end }
		{};

		/**
		 * @brief Converts the grammatical symbol string to a string. This function is syntactic sugar over toString.
		 * @returns A string representation of the grammatical symbol string.
		 */
		operator std::string() const {
			return this->toString();
		}

		/**
		 * @brief Converts the grammatical symbol string to a string.
		 *
		 * @details This function converts the grammatical symbol string to a string. It does so by appending the string representation of every symbol it contains, in order.
		 *
		 * @return A string representation of the grammatical symbol string.
		 */
		std::string toString() const {
			std::string temp;

			// if the symbol string is empty
			if (this->symbols.empty())
				return R"("")";

			// if it is not empty (contains at least one element)
			temp += (std::string)this->symbols.at(0);

			for (size_t i = 0; const auto & sym : symbols) {
				if (i == 0) {
					i++;
					continue;
				}

				temp += " " + sym.toString();
			};

			return temp;
		}

		/**
		 * @brief Appends a symbol to the end of the string.
		 * @returns void.
		 */
		void push_back(const SymbolType& symbol) {
			this->symbols.push_back(symbol);
		}
		
		/**
		 * @brief Pops a symbol from the end of the string.
		 * @returns void.
		 */
		void pop_back() {
			this->symbols.pop_back();
		}

		bool calculateFIRST(const FIRSTVecType&);

		/**
		 * @brief Checks whether the FIRST set for this string is already calculated.
		 * @return `true` if the FIRST is already calculated for this string of symbols; `false` otherwise.
		 */
		bool FIRSTCalculated() { return this->m_CalculatedFIRST; };

		/**
		 * @brief Returns the cached FIRST set if already calculated; otherwise it logs an error message and throws an exception.
		 * @throws runtime_error If the FIRST set is not already calculated.
		 * @returns The cached FIRST set if already calculated.
		 */
		const SetType& getFIRST() const { 

			// if FIRST is already calculated
			if (this->m_CalculatedFIRST)
				return this->FIRST; 

			static std::string msg = "The FIRST set of the non-terminals of this grammar symbol string is yet to be calculated.";

			// if FIRST is not already calculated
			this->m_Logger.log(LoggerInfo::ERR_MISSING_VAL, msg);
			throw std::runtime_error(msg);
		}
	};

	/**
	 * @brief Type alias for GrammaticalSymbolString.
	 * 
	 * @tparam SymbolT The type of a symbol object (the characters of a symbol string).
	 */
	template <typename SymbolT>
	using SymbolString = GrammaticalSymbolString<SymbolT>;

	/**
	 * @brief Overload of the stream insertion operator for SymbolString.
	 *
	 * @param os The output stream.
	 * @param symbolString The SymbolString to be inserted into the stream.
	 *
	 * @return The output stream.
	 */
	template <typename TerminalT, typename VariableT>
	std::ostream& operator<<(std::ostream& os, const SymbolString<Symbol<TerminalT, VariableT>>& symbolString) {

		std::cout << symbolString.toString();

		return os;
	}

	/**
	 * @brief Represents a vector of productions.
	 */
	template<typename ProductionT>
	class ProductionVector {

		/**
		 * @brief Aliases the type of a grammatical symbol.
		 */
		using SymbolType = decltype(ProductionT{}.prodHead);

		/**
		 * @brief Aliases the type of a non-terminal (enumeration type).
		 */
		using VariableType = decltype(SymbolType{}.as.nonTerminal);

		/**
		 * @brief Aliases the type of a vector of productions.
		 */
		using ProdVecType = std::vector<ProductionT>;
		// TODO: consider making this use terminals instead for storage efficiency
		/**
		 * @brief Aliases the type of a vector of sets of symbol objects.
		 * @todo **FOR DEVELOPERS** Consider changing it into a vector of sets of *terminals* for storage efficiency.
		 */
		using VectorSetSymbolType = std::vector<std::set<SymbolType>>;

		/**
		 * @brief Aliases the type of a symbol string.
		 */
		using SymbolStringType = SymbolString<SymbolType>;

		/**
		* @brief Caches the FIRST set of this production vector.
		* @details The index of a non-terminal will hold its FIRST set.
		*/
		VectorSetSymbolType FIRST {0};

		/**
		 * @brief Caches the FOLLOW set of this production vector.
		 * @details The index of a non-terminal will hold its FIRST or FOLLOW set.
		 */
		VectorSetSymbolType FOLLOW {0};
		
		/**
		 * @brief Indicates whether the FIRST set of this production vector was calculated and is cached or not.
		 */
		bool m_CalculatedFIRST = false;

		/**
		 * @brief Indicates whether the FOLLOW set of this production vector was calculated and is cached or not.
		 */
		bool m_CalculatedFOLLOW = false;

		bool _augment_FIRST_of_prod_using_symbol(const ProductionT&, const SymbolType&, bool&, size_t);
		
		bool _augment_FOLLOW_of_nonTerminal(VariableType, size_t, size_t);

	protected:

		/**
		 * @brief The vector storing the production objects.
		 */
		ProdVecType p_Vector{};

		/**
		 * @brief The logger object used to log information to output streams.
		 */
		Logger p_Logger{};

	public:

		// constructors

		/**
		 * @brief Default constructor.
		 */
		ProductionVector() = default;

		/**
		 * @brief Copy constructor.
		 */
		explicit(true) ProductionVector(const ProdVecType& vec) : p_Vector { vec } {};

		// production vector access methods

		/**
	     * @return A constant reference to the internal container of the productions.
	     */
		const ProdVecType& getProdVector() { return this->p_Vector; }

		/**
	     * @return A copy of the internal container of the productions.
	     */
		ProdVecType getProdVectorValue() { return this->p_Vector; }

		/**
	     * @brief Appends a new production rule to the end of the production vector.
	     *
	     * @param[in] prod The production rule to be added.
	     * 
	     * @returns void.
	     */
		void pushProduction(const ProductionT& prod) { this->p_Vector.push_back(prod); }

		// element access methods

		/**
		 * @brief Accesses a particular production at a given index within the production vector.
		 * @param[in] i The index of the production to be accessed.
		 * @returns A constant reference to the production object at index `i` within the production vector.
		 */
		const ProductionT& operator [] (size_t i) const { return this->p_Vector.at(i); }

		/**
		 * @brief Accesses a particular production at a given index within the production vector. This is identical to operator[].
		 * @param[in] i The index of the production to be accessed.
		 * @returns A constant reference to the production object at index `i` within the production vector.
		 */
		const ProductionT& at(size_t i) const { return this->p_Vector.at(i); };

		/**
		 * @brief Accesses the internal container of the production objects.
		 * @returns A constant reference to the internal container storing the production objects.
		 */
		const auto& getVector() const { return this->p_Vector; }

		// other methods

		/**
	     * @return An iterator pointing to the first production object (in the internal `std::vector` container).
	     */
		auto begin() const { return this->p_Vector.begin(); }

		/**
		 * @return An iterator pointing to the past the last production object (in the internal `std::vector` container).
		 */
		auto end() const { return this->p_Vector.end(); }

		/**
	     * @brief Gets the number of production rules currently stored in the ProductionVector.
	     * @return The size of the internal container (number of production rules).
	     */
		size_t size() const { return this->p_Vector.size(); }

		/**
		 * @brief Clears (eliminates all of the elements of) the production vector.
		 * @returns void.
		 */
		void clear() { this->p_Vector.clear(); this->FIRST.clear(); this->FOLLOW.clear(); }

		/**
		 * @brief Checks whether the production vector is empty or not.
		 * @returns `true` if the production vector has at least one element; `false` otherwise.
		 */
		bool empty() const { return this->p_Vector.empty(); }

		/**
		 * @brief Appends a new production rule to the end of the production vector. This is identical to pushProduction.
		 *
		 * @param[in] prod The production rule to be added.
		 *
		 * @returns void.
		 */
		void push_back(const ProductionT& production) { p_Vector.push_back(production); }

		/**
		 * @brief Removes the last element from the production vector.
		 * @returns void.
		 */
		void pop_back() { p_Vector.pop_back(); }

		// conversion methods
		/**
		 * @brief Converts this ProductionVector object to a string. This is just syntactic sugar over toString.
		 * @returns A string representation of this ProductionVector object.
		 */
		explicit(false) operator std::string() const {
			return this->toString();
		}

		/**
		 * @brief Converts this ProductionVector object to a string.
		 * @returns A string representation of this ProductionVector object.
		 */
		std::string toString() const {

			std::string str;

			for (const auto& prod : this->p_Vector)
				str += (std::string)prod + "\n";

			return str;
		}

		// FIRST and FOLLOW calculation methods
		bool calculateFIRST();

		/**
		 * @returns `true` if the FIRST set of this production vector is already calculated; `false` otherwise.
		 */
		bool FIRSTCalculated() { return this->m_CalculatedFIRST; };

		/**
		 * @brief Gets the FIRST set of a particular terminal using the FIRST set of this production vector.
		 * @param[in] nonTerminal The non-terminal whose FIRST set is to be returned.
		 * @return FIRST(`nonTerminal`).
		 * @throw MissingValueException If FIRST(`nonTerminal`) is not yet calculated.
		 */
		std::set<SymbolType> getFIRST(VariableType nonTerminal) const { 
				
			// if FIRST is already calculated
			if (this->m_CalculatedFIRST)
				return this->FIRST[(size_t)nonTerminal];

			// handle the non-presence of the FIRST set for this production vector
			this->p_Logger.log(LoggerInfo::ERR_MISSING_VAL, "The FIRST set of the non-terminals of this production vector is yet to be calculated.");
			throw MissingValueException( "The FIRST set of the non-terminals of this production vector is yet to be calculated." );
		};

		/**
		 * @brief Gets a constant reference to the entire FIRST (calculated so far) for this production vector.
		 * @return A constant reference to the entire FIRST set (calculated so far) for this production vector.
		 */
		const VectorSetSymbolType& getFIRST() const {
			return this->FIRST;
		}

		bool calculateFOLLOW();

		/**
		 * @returns `true` if the FOLLOW set of this production vector is already calculated; `false` otherwise.
		 */
		bool FOLLOWCalculated() { return this->m_CalculatedFOLLOW; };
		
		/**
		 * @brief Gets the FOLLOW set of a particular terminal using the FOLLOW set of this production vector.
		 * @param[in] nonTerminal The non-terminal whose FOLLOW set is to be returned.
		 * @return FOLLOW(`nonTerminal`).
		 * @throw MissingValueException If FOLLOW(`nonTerminal`) is not yet calculated.
		 */
		std::set<SymbolType> getFOLLOW(const VariableType nonTerminal) {

			// if FOLLOW is already calculated
			if (this->m_CalculatedFOLLOW)
				return this->FOLLOW[(size_t)nonTerminal];

			// handle the non-presence of the FOLLOW set for this production vector
			this->p_Logger.log(LoggerInfo::ERR_MISSING_VAL, "The FOLLOW set of the non-terminals of this production vector is yet to be calculated.");
			throw MissingValueException("The FOLLOW set of the non-terminals of this production vector is yet to be calculated.");
		};
		
		/**
		 * @brief Gets a constant reference to the entire FOLLOW (calculated so far) for this production vector.
		 * @return A constant reference to the entire FOLLOW set (calculated so far) for this production vector.
		 */
		const VectorSetSymbolType& getFOLLOW() {
			return this->FOLLOW;
		}

	};

	// ALIASES
	template<typename ProductionT>
	using ProdVec = ProductionVector<ProductionT>;
}

namespace m0st4fa::parsix {

	/** 1st Algorithm, assuming non-terminal `E` and a boolean isCalculated
	* If isCaculated, return; else proceed.
	* Go through every production of `E`.
	* Take the FIRST set of the first grammar symbol `T` of that production (and cache it for possible use later).
	* Ignore that grammar symbol if it is currSymbol (this avoids lots of indirect recursion).
		* Note that this is not a problem, since in this case the FIRST set of the `E` is a subset of the FIRST set of `T`.
	*	 - However, we are already calculating the FIRST set of `E`.
	*	 - Operate in two modes, the mode in which you have a currSymbol and the in which you do not.
	* Loop and do this until the FIRST set of that grammar symbol does not contain epsilon.
	*	 - If, while looping you encounter `E` or `E` is the first grammar symbol of the production, put the production in pending list with the index of `E`.
	*		 - In case epsilon is in the FIRST set of `E`, we will need to rescan this production again from after index.
	*	 - If you reach the end of the production, if the last (cached) FIRST set has epsilon, make sure it is in FIRST set of `E`.
	* Set isCalculated to true.
	*/

	/** 
	* **Algorithm**\n
	* - Loop through every production of the grammar until you cannot add another terminal to the first set of any non-terminal.
	* - Variable added asserts whether we added any terminal in the last iteration to the FIRST set of any non-terminal.
	* - For every production `P` and its head `H`:
	*	 - For every grammar symbol `S` of the body of `P`:
	*		- If `S` == `H`, check whether `FIRST(H)` contains epsilon:
	*			- If it does, check whether we are at the end of the body of `P`:
	*				- If we are, add epsilon to `FIRST(H)` [Note: already added].
	*				- If we are not, move on to the next grammar symbol.
	*			- If it does not, move on to the next production.
	*		- If `S` is a terminal `T`, add `T` to `FIRST(H)` and move on to the next production.
	*			- If `T` is added (was not already there), set added to true.
	*		- If `S` is a non-terminal `N`:
	*			- `F` = `FIRST(N)`, `FE` = `F` has epsilon?, `HE` = `S` has epsilon?
	*			- Add the current `F` to `FIRST(H)`.
	*				- If any terminal was added (was not already there) set added to true.
	*			- If (`FE` && !`HE`) remove the epsilon from `FIRST(H)`.
	*				- Note: the condition means: if `F` has epsilon and `FIRST(H)` didn't have epsilon before merging the two FIRST sets.
	*			- If FE, check whether we are at the end of the body of `P`:
	*				- If we are, add epsilon to `FIRST(H)`.
	*					- If it was added (was not already there) set added to true.
	*				- If we are not, move on to the next grammar symbol of the body of `P`.
	*		- Move on to the next production.
	*
	*/

	/**
	 * @brief Calculates the FIRST set for all productions in the ProductionVector.
	 *
	 * @tparam ProductionT The type of production objects.
	 * 
	 * @return `true` if the FIRST set has been calculated, `false` otherwise.
	 */
	template<typename ProductionT>
	bool ProductionVector<ProductionT>::calculateFIRST()
	{

		// if FIRST is already calculated, return
		if (this->m_CalculatedFIRST) {
			this->p_Logger.logDebug("FIRST set for this production vector has already been calculated!");
			return true;
		}

		using SetPair = std::pair<std::set<SymbolType>, bool>;

		// resize the FIRST set to accommodate an entry for all non-terminals
		this->FIRST.resize((size_t)decltype(SymbolType().as.nonTerminal)::NT_COUNT);
		bool added = false;

		this->p_Logger.logDebug("\nCALCULATING FIRST SET:\n");
		this->p_Logger.logDebug(std::format("Productions:\n {}", (std::string)*this));

		while (true) {

			// loop through every production
			for (const ProductionT& prod : this->getProdVector()) {

				// loop through every symbol of the production
				for (size_t index = 1; const auto & stackElement : prod.prodBody) {

					// if the current stack element of the production is not a grammar symbol
					if (stackElement.type != ProdElementType::PET_GRAM_SYMBOL)
						// move on to the next stack element of the production
						continue;

					// if the current stack element is a grammar symbol

					// get the symbol and its first set (if any)
					const SymbolType& symbol = stackElement.as.gramSymbol;

					// continue or not to the next symbol?
					bool cont = _augment_FIRST_of_prod_using_symbol(prod, symbol, added, index);

					// increment the index of the current element
					++index;

					if (!cont)
						break;

				}

			}

			this->p_Logger.logDebug(std::format("Is there any new terminal added in this round? {}", added ? "true" : "false"));
			if (-not added) {

				//// if we still have extra rounds
				//if (extraRound--)
				//	// reduce the number of rounds and continue to the next iteration
				//	continue;

				this->p_Logger.logDebug("Finished creating the FIRST set of all non-terminals of this grammar");

#ifdef _DEBUG	
				for (size_t i = 0; const std::set<SymbolType>&set : this->FIRST) {

					// if the set is empty, continue (in this case it does not belong to a non-terminal)
					if (set.empty()) {
						i++;
						continue;
					}

					// get variable
					auto variable = (decltype(set.start()->as.nonTerminal))i++;

					this->p_Logger.logDebug(std::format("FIRST({}) = {}", toString(variable), m0st4fa::toString(set)));
				}
#endif


				break;
			}

			added = false;
		}

		// if we reached here, that means that FIRST has been calculated
		return this->m_CalculatedFIRST = true;
	}

	/**
	 * @brief Augments FIRST for the head of a production, based on the currently available information (the current context).
	 * 
	 * @tparam ProductionT The type of the production.
	 * @param[in] prod The production for which to calculate the FIRST set.
	 * @param[in] symbol The symbol of the production body whose FIRST set will be added to the FIRST set of this production.
	 * @param[out] added A boolean flag that is set to true if a new terminal (from FIRST(`symbol`)) is added to the FIRST set of the head of the production.
	 * @param[in] index The index of the `symbol` in the production body. This is used to check whether `symbol` is the last symbol in the production body or not.
	 * 
	 * @details This function implements logic for augmenting the FIRST set of a production's head symbol using another symbol. It considers various cases depending on whether the `symbol` is a terminal or a non-terminal and the context within the production body.
	 *
	 * - If the `symbol` is a terminal, it's directly added to the FIRST set of the
	 *   head symbol.
	 * - If the `symbol` is a non-terminal:
	 *   - The FIRST set of the non-terminal is used to augment the head's FIRST set.
	 *   - Epsilon might be added to the head's FIRST set depending on whether the
	 *     non-terminal's FIRST set contains epsilon, the position within the
	 *     production body, and whether the head itself has epsilon in its FIRST set.
	 *
	 * @note This function is an internal implementation detail and should not be called directly by user code.
	 * 
	 * @returns `true` if the FIRST set of `symbol` has epsilon and is not the last symbol in the production body (and thus, we should seek the FIRST of the next symbol in order in the production body); `false` otherwise.
	 */
	template<typename ProductionT>
	bool ProductionVector<ProductionT>::_augment_FIRST_of_prod_using_symbol(const ProductionT& prod, const SymbolType& symbol, bool& added, size_t index)
	{

		const SymbolType& head = prod.prodHead;
		auto setIndexH = (size_t)head.as.nonTerminal;
		std::set<SymbolType>& fsetH = this->FIRST[setIndexH];

		// check whether the first set contains epsilon
		bool containsEpsilonH = fsetH.contains(SymbolType::EPSILON);

		// if the symbol is a non-terminal
		if (!symbol.isTerminal) {

			// get the first set of the symbol	
			auto setIndexN = (size_t)symbol.as.nonTerminal;
			std::set<SymbolType>& fsetN = this->FIRST[setIndexN];

			// check whether the first set contains epsilon
			bool containsEpsilonN = fsetN.contains(SymbolType::EPSILON);
			// if the symbol is the same as head
			if (symbol == head) {
				/**
				* If `S` ==`H`, check whether `FIRST(H)` contains epsilon:
				*	 - If it does, check whether we are at the end of the body of `P`:
				*		 - If we are, add epsilon to `FIRST(H)` [Note: already added].
				*		 - If we are not, move on to the next grammar symbol.
				*	 - If it does not, move on to the next production.
				*/

				// 1. if the first set does not contain epsilon
				if (!containsEpsilonN)
					// move on to the next production
					return false;

				// 2. if FIRST contains epsilon

				// move on to the next grammar symbol, which will automatically move on to the next production if this is the last grammar symbol
				// Note: epsilon is already added (that's why we are here in the first place)
				return true;
			}

			// if the symbol is different from head
			/**
			* If `S` is a non-terminal `N`:
				* `F` = `FIRST(N)`, `FE` = `F` has epsilon?, `HE` = `S` has epsilon?
				* Add the current `F` to `FIRST(H)`.
					* If any terminal was added (was not already there) set added to true.
				* If (`FE` && !`HE`) remove the epsilon from `FIRST(H)`.
					* Note: the condition means: if `F` has epsilon and `FIRST(H)` didn't have epsilon before mergin the two FIRST sets.
				* If FE, check whether we are at the end of the body of `P`:
					* If we are, add epsilon to `FIRST(H)`.
					* If it was added (was not already there) set added to true.
					* If we are not, move on to the next grammar symbol of the body of `P`.
			*/

			if (fsetN.empty())
				return false;

			// loop through each symbol of the FIRST set of the current non-terminal
			for (const auto& symbol : fsetN) {

				// skip to the next symbol if the current is epsilon (optimize this out if you could)
				if (symbol == SymbolType::EPSILON)
					return true;

				// add the symbol to the FIRST set of the head
				// BUG: the symbol is inserted although it exists
				auto p = this->FIRST[setIndexH].insert(symbol);

				// if it was added, set the boolean
				if (p.second) {
					added = true;

					this->p_Logger.logDebug(
						std::format("Added terminal {} to FIRST({}), which is now: {}",
							(std::string)symbol,
							(std::string)head,
							m0st4fa::toString(this->FIRST[setIndexH])));

				}
			}

			// if the non-terminal contains epsilon
			if (containsEpsilonN) {
				// check whether we are at the end of the production
				bool atEnd = index == prod.prodBody.size();

				// if we are not at the end of the production
				if (!atEnd)
					// continue to the next grammar symbol
					return false;

				// if we are at the end of the production

				if (!containsEpsilonH) {
					// add epsilon to the FIRST set of the head
					auto p = this->FIRST[setIndexH].insert(SymbolType::EPSILON);

					if (p.second) {
						added = true;

						this->p_Logger.logDebug(
							std::format("Added terminal {} to FIRST({}), which is now: {}",
								(std::string)SymbolType::EPSILON,
								(std::string)head,
								m0st4fa::toString(this->FIRST[setIndexH])));
					}
				}

			}

			// if does not, continue to the next grammar symbol
			return false;
		}
		else {
			/**
				*If `S` is a terminal `T`, add `T` to `FIRST(H)` and move on to the next production.
				*	 - If `T` is added(was not already there), set added to true.
			*/

			// add epsilon to the FIRST set of the head
			if (auto p = this->FIRST[setIndexH].insert(symbol); p.second) {
				added = true;

				this->p_Logger.logDebug(
					std::format("Added terminal {} to FIRST({}), which is now: {}",
						(std::string)symbol,
						(std::string)head,
						m0st4fa::toString(this->FIRST[setIndexH])));
			}

			// continue to the next production
			return false;
		}

		// THIS IS UNREACHABLE
		// go to the next production by default
		return false;
	}

	/**
	 * @brief Augments the FOLLOW set of nonterminal, using (only) a single input production.
	 * 
	 * @tparam ProductionT The type of the production.
	 * @param[in] nonTerminal The non-terminal whose FOLLOW set is to be calculated and augmented.
	 * @param[in] prodIndex The index of the production that will be used to calculate FOLLOW in the ProductionVector.
	 * @param[in] variableIndex The index of the non-terminal (whose FOLLOW will be calculated) in the production.
	 * 
	 * @return `true` if a new terminal was added to the FOLLOW set; `false` otherwise.
	 * 
	 * @details  This function calculates the FOLLOW set of a non-terminal symbol within a production. It considers the position of the non-terminal within the production body and the FOLLOW set of the production's head symbol.
	 *
	 * The algorithm iterates through subsequent symbols in the production body after the target non-terminal:
	 *
	 * - If a subsequent symbol is a terminal, it's directly added to the FOLLOW set of the target non-terminal.
	 * - If a subsequent symbol is a non-terminal:
	 *   - The FIRST set of the non-terminal is used to contribute to the FOLLOW set of the target non-terminal.
	 *   - Epsilon might be removed from the FOLLOW set of the target non-terminal depending on whether the subsequent non-terminal's FIRST set contains epsilon and the position within the production body.
	 */
	template<typename ProductionT>
	bool ProductionVector<ProductionT>::_augment_FOLLOW_of_nonTerminal(VariableType nonTerminal, size_t prodIndex, size_t variableIndex)
	{

		// head-related values
		const ProductionT& production = this->getProdVector().at(prodIndex);
		const size_t prodBdySz = production.prodBody.size();
		const SymbolType& head = production.prodHead;
		size_t headIndex = (size_t)head.as.nonTerminal;
		const std::set<SymbolType>& headFollow = this->FOLLOW.at(headIndex);

		// current symbol related values
		const size_t currSymIndex = (size_t)nonTerminal;
		std::set<SymbolType>& currSymFollow = this->FOLLOW.at(currSymIndex);
		bool added = false;

		auto cpyToFollow = [this, currSymIndex, nonTerminal, &currSymFollow, &added](const SymbolType& sym) {
			auto p = currSymFollow.insert(sym);

			// if a new element was added (exclude epsilon)
			if (p.second && *p.first != SymbolType::EPSILON) {
				added = true;
				this->p_Logger.logDebug(
					std::format("Added terminal {} to FOLLOW({}), which is now: {}",
						(std::string)sym,
						toString(nonTerminal),
						m0st4fa::toString(this->FOLLOW[currSymIndex])));

			}

		};


		// test if this non-terminal is the last of its production body
		bool lastSymbol = variableIndex == prodBdySz - 1;

		// if this is the last symbol of the production body
		if (lastSymbol) {
			// add FOLLOW(head) to FOLLOW(currSymbol)
			std::for_each(headFollow.begin(), headFollow.end(), cpyToFollow);
			return added;
		};

		for (size_t varIndex = variableIndex + 1; varIndex < prodBdySz; varIndex++) {

			// check if the current symbol we are examining in the production is a grammar symbol 
			if (production.prodBody.at(varIndex).type != ProdElementType::PET_GRAM_SYMBOL) {
				// check if we are at the end of the production
				bool atEnd = varIndex + 1 == prodBdySz;

				// continue to the next symbol if we are not at the end of the production
				if (-not atEnd)
					continue;

				// if we are at the end of the production add FOLLOW(head) to FOLLOW(currSym)
				std::for_each(headFollow.begin(), headFollow.end(), cpyToFollow);

				break;
			}

			const SymbolType symbol = production.prodBody.at(varIndex).as.gramSymbol;
			size_t symIndex = (size_t)symbol.as.nonTerminal;

			// if the symbol is a terminal
			if (symbol.isTerminal) {
				auto p = this->FOLLOW.at(currSymIndex).insert(symbol);

				this->p_Logger.logDebug(
					std::format("Added terminal {} to FOLLOW({}), which is now: {}",
						(std::string)symbol,
						toString(nonTerminal),
						m0st4fa::toString(this->FOLLOW[currSymIndex])));

				// return whether a new element has been inserted
				return p.second;
			}

			// if the symbol is a non-terminal 
			// assumes that FIRST(symbol) is already calculated
			const std::set<SymbolType>& symbolFrst = this->FIRST.at(symIndex);
			std::for_each(symbolFrst.begin(), symbolFrst.end(), cpyToFollow);

			// Try erase epsilon from follow: returns 1 or 0, the number of removed elements
			// Note: this also indicates whether we had epsilon or not.
			bool containsEpsilon = currSymFollow.erase(SymbolType::EPSILON);

			// if the FOLLOW set of the curr symbol does not contain epsilon
			if (-not containsEpsilon)
				return added;

			/**
			* Check if we are at the end of `P`:
				* If we are, add `FOLLOW(H)` to `FOLLOW(S)`.
				* If we are not, continue to the next symbol.
			*/
			// check if we are at the end of the production
			bool atEnd = varIndex + 1 == prodBdySz;

			if (-not atEnd)
				continue;

			// if we are at the end of the production add FOLLOW(head) to FOLLOW(currSym)
			std::for_each(headFollow.begin(), headFollow.end(), cpyToFollow);

		}

		return added;
	}

	/**
	 * @brief Calculates the FOLLOW set for all non-terminals in the grammar (heads of productions, specifically) using the productions in this ProductionVector.
	 *
	 * @tparam ProductionT The type of production objects.
	 *
	 * @return `true` if the FOLLOW set has been calculated, `false` otherwise.
	 * 
	 * @todo Change the behavior to calculate FIRST in case it is not calculated instead of raising an exception.
	 * 
	 * @throws runtime_error if the FIRST set of the production is not already calculated. You should make sure calculateFIRST is called at least once before calling this function.
	 */
	template<typename ProductionT>
	bool ProductionVector<ProductionT>::calculateFOLLOW()
	{

		// if follow is already calculated, return
		if (this->m_CalculatedFOLLOW) {
			this->p_Logger.logDebug("FOLLOW set for this production vector has already been calculated!");
			return true;
		}

		// check that FIRST is calculated before proceeding
		if (!this->m_CalculatedFIRST) {
			this->p_Logger.log(LoggerInfo::ERR_MISSING_VAL, "FIRST set is not calculated for the production vector: FIRST set must be calculated for a production vector before proceeding to calculate the FOLLOW set for that production vector.");
			throw std::runtime_error("FIRST set is not calculated for the production vector!");
		};

		/** 
		# General Algorithm:
		* - Loop through every production of the grammar until you cannot add another terminal to the FOLLOW set of any non-terminal.
		* - Variable added asserts whether we added any terminal in the last iteration to the FOLLOW set of any non-terminal.
		* - For every production `P` and its head `H`:
		*		- For every grammar symbol `S` of the body of `P`:
		*			- If `S` is a terminal, get to the next grammar symbol.
		*			- If `S` is a non-terminal call `getFOLLOWOfNonTerminal`.
		**/

		/**
		## Algorithm for getFOLLOWOfNonTerminal():
		#### input:
		*- Non-terminal `N`.
		*- Index `I1` of the current production `P` within the production vector.
		*- Index of `N` within this production.
		#### output: 
		*- Augments FOLLOW(N) using currently available information.
		#### method: 
		*- For every symbol `S` after `N` in production `P` whose index is `I2`:
		*	- If `S` is a terminal, add `S` to `FOLLOW(N)` and return.
		*	- If `S` is a non-terminal, add FIRST(`S`) to `FOLLOW(N)`, then check whether `FOLLOW(N)` contains epsilon:
		*		- If `FOLLOW(N)` contains epsilon:
		*			- remove epsilon.
		*				- Check if we are at the end of `P`:
		*					- If we are, add `FOLLOW(H)` to FOLLOW(`S`).
		*					- If we are not, continue to the next symbol.
		*		- If `FOLLOW(N)` doesn't contain epsilon, return.
		*/

		// make sure FOLLOW can hold all the non-terminals
		this->FOLLOW.resize((size_t)decltype(SymbolType().as.nonTerminal)::NT_COUNT);

		
		bool added = false;

		this->p_Logger.logDebug("\nCALCULATING FOLLOW SET:\n");
		this->p_Logger.logDebug(std::format("Productions:\n {}", (std::string)*this));

		const ProductionT& startProd = getProdVector().at(0);
		const SymbolType& startSym = startProd.prodHead;
		size_t startHeadIndex = (size_t)startSym.as.nonTerminal;

		this->FOLLOW.at(startHeadIndex).insert(SymbolType::END_MARKER);

		while (true) {

			/** \cond
			* Loop through every production of the grammar until you cannot add another terminal to the FOLLOW set of any non-terminal.
			* Variable added asserts whether we added any terminal in the last iteration to the FOLLOW set of any non-terminal.
			* For every production `P` and its head `H`:
				* For every grammar symbol `S` of the body of `P`:
					* If `S` is a terminal, get to the next grammar symbol.
					* If `S` is a non-terminal call `getFOLLOWOfNonTerminal`.
			\endcond
			*/

			for (size_t prodIndex = 0; const auto & prod : this->getProdVector()) {

				for (size_t symIndex = 0; const auto& stackElement : prod.prodBody) {

					// if the current stack element of the production is not a grammar symbol
					if (stackElement.type != ProdElementType::PET_GRAM_SYMBOL) {
						// move on to the next stack element of the production
						symIndex++;
						continue;
					}

					// if the current stack element is a grammar symbol

					// get the symbol and its first set (if any)
					const SymbolType& symbol = stackElement.as.gramSymbol;

					// if the symbol is a terminal, get to the next symbol
					if (symbol.isTerminal) {
						symIndex++;
						continue;
					}

					// if the symbol is a non-terminal
					bool _added = this->_augment_FOLLOW_of_nonTerminal(symbol.as.nonTerminal, prodIndex, symIndex);

					// set added
					if (_added)
						added = true;

					// increment the index of the symbol
					symIndex++;
				}

				prodIndex++;
			}

			this->p_Logger.logDebug(std::format("Is there any new terminal added to any FOLLOW set in this round? {}", added ? "true" : "false"));
			if (-not added) {

				//// if we still have extra rounds
				//if (extraRound--)
				//	// reduce the number of rounds and continue to the next iteration
				//	continue;

				this->p_Logger.logDebug("Finished creating the FOLLOW set of all non-terminals of this grammar");
#if defined(_DEBUG)
				for (size_t i = 0; const std::set<SymbolType>&set : this->FOLLOW) {

					// if the set is empty, continue (in this case it does not belong to a non-terminal)
					if (set.empty()) {
						i++;
						continue;
					}

					// get variable
					auto variable = (decltype(set.start()->as.nonTerminal))i++;

					this->p_Logger.logDebug(std::format("FOLLOW({}) = {}", toString(variable), m0st4fa::toString(set)));

				}
#endif

				break;
			}

			added = false;

		}

		// if we reached here, that means that FOLLOW has been calculated
		return this->m_CalculatedFOLLOW = true;
	}

	/**
	 * @brief Calculates the FIRST set for this grammar symbol string.
	 * @param[in] prodVecFIRST The FIRST set of a particular grammar.
	 * @return `true` if the FIRST set has been calculated successfully; `false` otherwise.
	 */
	template<typename SymbolT>
	bool GrammaticalSymbolString<SymbolT>::calculateFIRST(const FIRSTVecType& prodVecFIRST)
	{

		// if FIRST is already calculated, return
		if (this->m_CalculatedFIRST) {
			this->m_Logger.logDebug(std::format("FIRST({}) has not already been calculated!", (std::string)*this));
			return true;
		}

		/** 
		## Algorithm:
		* For every grammar symbol `GS` of the string `S`:
		*	- If `S` is a non-terminal:
		*	- Get FIRST(GS) and add it to `FIRST(S)`.
		*	- Check whether FIRST(GS) has epsilon and remove the epsilon, if any:
		*			- If FIRST(GS) has epsilon check to see if this is the last symbol of the string:
		*				- If it does, add epsilon to FIRST(GS)
		*				- If it doesn't, continue to the next symbol.
		*			- If `FIRST(S)` does not have epsilon, we are done with `FIRST(S)`.
		*	- If `S` is a terminal, check to see if it is epsilon:
		*		- If it is epsilon, check to see if this is the last symbol of the string:
		*			- If it is, add epsilon to FIRST(GS).
		*			- If it is not, continue to the next symbol.
		*		- If it is not epsilon:
		*			- Add the terminal to FIRST(GS) and break.
		*/

		for (size_t i = 0; const SymbolType & symbol : this->symbols) {

			bool isLastSymbol = i == this->symbols.size() - 1;

			// if the symbol is a terminal
			if (symbol.isTerminal) {

				// check whether the symbol is epsilon
				bool isEpsilon = symbol == SymbolType::EPSILON;

				// if the symbol is epsilon and is the last symbol of the string or
				// the symbol is not epsilon, whether the last symbol or not
				if (not isEpsilon or isLastSymbol)
					this->FIRST.insert(symbol);

				// if this symbol is not epsilon
				if (not isEpsilon)
					break;

				// if we reach here, continue to the next symbol (or break if this is the last)
				continue;
			}

			// if the symbol is a non-terminal
			size_t symIndex = (size_t)symbol.as.nonTerminal;

			// get FIRST(S)
			const SetType& fset = prodVecFIRST.at(symIndex);

			// check to see if `FIRST(S)` is empty
			// in this case there is probably a logic error made by the programmar
			if (fset.empty()) {
				std::string msg = (std::string)"FIRST(" + (std::string)symbol + ") is empty.";
				this->m_Logger.log(LoggerInfo::ERR_MISSING_VAL, msg + "\n\t\t\tThis may be due to:\n\t\t\t\t1.`A` wrong FIRST set (possibly one of a different grammar).\n\t\t\t\t2. An incomplete FIRST set.");
				throw std::logic_error(msg);
			}

			// if `FIRST(S)` is not empty, add it to FIRST(SS)
			this->FIRST.insert(fset.begin(), fset.end());

			// check if FIRST(SS) contains epsilon
			bool containsEpsilon = this->FIRST.erase(SymbolType::EPSILON);

			// if it does not contain epsilon, we are done with FIRST(SS)
			if (not containsEpsilon)
				break;
			
			// if FIRST(SS) contains epsilon

			// check to see if it is the last symbol
			if (isLastSymbol)
				this->FIRST.insert(SymbolType::EPSILON);

			// if it contains epsilon, and it is not the last symbol, continue to the next symbol
			i++;
		}

#ifdef _DEBUG
		if (this->FIRST.empty()) {
			this->m_Logger.logDebug(std::format("FIRST({}) is empty", (std::string)*this));
			goto epilogue;
		}

		this->m_Logger.logDebug(std::format("FIRST({}) = {}", (std::string)*this, m0st4fa::toString(this->FIRST)));
#endif

	epilogue:
		return this->m_CalculatedFIRST = true;
	}

}