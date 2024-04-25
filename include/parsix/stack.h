#pragma once
#include <vector>
#include <type_traits>
#include <sstream>
#include <format>

#include "parsix/enum.h"

// defined constants

/**
 * @brief Extracts the terminal enumerator object from a symbol object.
 * @attention The type of the symbol object must be terminal or the behavior is undefined.
 */
#define EXTRACT_TERMINAL(stackElement) (size_t)stackElement.as.gramSymbol.as.terminal

 /**
  * @brief Extracts the non-terminal enumerator object from a symbol object.
  * @attention The type of the symbol object must be non-terminal or the behavior is undefined.
  */
#define EXTRACT_VARIABLE(stackElement) (size_t)stackElement.as.gramSymbol.as.nonTerminal

// SHARED (LL AND LR)
namespace m0st4fa::parsix {

	/**
	 * @brief The type of a stack of any data type. `std::vector` is the underlying data type. 
	 */
	template <typename StackElementT>
	using StackType = std::vector<StackElementT>;

}

// LL PARSING
namespace m0st4fa::parsix {

	// FORWARD DECLARATIONS
	template <typename DataT>
		requires requires (DataT data) { std::string(data); data == data; }
	struct LLSynthesizedRecord;

	template <typename DataT>
		requires requires (DataT data) { std::string(data); data == data; }
	struct LLActionRecord;

	template <typename SymbolT, typename SynthesizedT, typename ActionT>
	concept StackElementConstraints = requires (SymbolT sym1, SymbolT sym2) { sym1 = sym2; } && std::is_trivially_destructible_v<SynthesizedT> && std::is_trivially_destructible_v<ActionT>;

	/**
	 * @brief Represents an element of the stack of an LL parser.
	 * @tparam SymbolT The type of symbol objects stored in the stack.
	 * @tparam SynthesizedT The type of synthetic record objects stored in the stack.
	 * @tparam ActionT The type of action record objects stored in the stack.
	 */
	template <
		typename SymbolT,
		typename SynthesizedT,
		typename ActionT
	>
		requires StackElementConstraints<SymbolT, SynthesizedT, ActionT>
	struct LLStackElement {
		
		/**
		 * @brief The type of this stack element.
		 */
		ProdElementType type;

		// TODO: try enhance this
		/**
		 * @brief Used to store the actual element. The element can be of any of the types: SymbolT, SynthesizedT or ActionT. The type is indicated by the field `type`.
		 * @details The element can be of any of three types.
		 * @todo This should have been a `union` type, but then non-trivial fields of the union would not be possible and this would be a huge limit on what members can store. It is a dilemma.
		 */
		struct {

			/**
			 * @brief The grammar symbol.
			 */
			SymbolT gramSymbol;

			/**
			 * @brief The synthetic record.
			 */
			SynthesizedT synRecord;

			/**
			 * @brief The action record.
			 */
			ActionT actRecord;
		} as;

		/**
		 * @brief The copy operator of LLStackElement.
		 * @param[in] other The rhs (right-hand side) of the copy operator.
		 * @return A reference to this LLStackElement object after being (possibly) modified.
		 */
		LLStackElement& operator=(const LLStackElement& other) {
			type = other.type;
			switch (type) {

			case ProdElementType::PET_GRAM_SYMBOL:
				as.gramSymbol = other.as.gramSymbol;
				break;

			case ProdElementType::PET_SYNTH_RECORD:
				as.synRecord = other.as.synRecord;
				break;

			case ProdElementType::PET_ACTION_RECORD:
				as.actRecord = other.as.actRecord;
				break;
			}

			return *this;
		}

		/**
		 * @brief Equality operator.
		 * @param[in] other The rhs of this operator.
		 * @returns `true` if this object and `other` are equal; `false` otherwise.
		 */
		bool operator==(const LLStackElement& other) const {

			if (this->type != other.type)
				return false;

			switch (type) {

			case ProdElementType::PET_GRAM_SYMBOL:
				return as.gramSymbol == other.as.gramSymbol;

			case ProdElementType::PET_SYNTH_RECORD:
				return as.synRecord == other.as.synRecord;

			case ProdElementType::PET_ACTION_RECORD:
				return as.actRecord == other.as.actRecord;

			}

			return false;
		}
		
		/**
		 * @brief Converts this object to string. Syntactic sugar over toString.
		 * @returns The string representation of this object.
		 */
		operator std::string() const {
			return this->toString();
		}

		/**
		 * @brief Converts this object to string.
		 * @returns The string representation of this object.
		 */
		std::string toString() const {
			static_assert((size_t)ProdElementType::PET_COUNT == 3);

			switch (this->type) {
			case ProdElementType::PET_GRAM_SYMBOL:
				return this->as.gramSymbol.toString();

			case ProdElementType::PET_SYNTH_RECORD:
				return this->as.synRecord.toString();
				

			case ProdElementType::PET_ACTION_RECORD:
				return this->as.actRecord.toString();
				
			default:
				return "Unreachable: Stack element un recognized";
			}

		}

	};

	/*
	template <
		typename SymbolT,
		typename SynthesizedT,
		typename ActionT
	>
	std::string toString(
		typename LLStackElement<SymbolT, SynthesizedT, ActionT>::StackElementType type) 
	{
		using StackElementType = LLStackElement<SymbolT, SynthesizedT, ActionT>::StackElementType;

		static_assert(StackElementType::SET_COUNT == 3);
		static constexpr const char* const names[] = {
			"GRAM_SYMBOL",
			"SYNTH_RECORD",
			"ACTION_RECORD",
		};


		const char* name = type == StackElementType::SET_COUNT ?
			std::to_string((unsigned)StackElementType::SET_COUNT).data() : names[static_cast<int>(type)];

		return name;
	};

	template <
		typename SymbolT,
		typename SynthesizedT,
		typename ActionT
	>
	std::ostream& operator<<(std::ostream& os, 
		typename LLStackElement<SymbolT, SynthesizedT, ActionT>::StackElementType type) {
		return os << toString(type);
	};
	*/

	/**
	* @brief A simple base class for synthesized records from which you can derive more complex classes.
	* @tparam DataT The type of the data stored in this record.
	*/
	template <typename DataT>
		requires requires (DataT data) { std::string(data); data == data; }
	struct LLSynthesizedRecord {

		/**
		 * @brief The data stored in this synthetic record.
		 */
		DataT data;

		/**
		* @brief The action to be executed (on `data` and the stack) when encountering this record. The action generally is to copy the data to some other record (action or synthetic) down the stack.
		* @attention The signature of the function is expected to be: void(StackType, DataT). The type of this field should be cast to a pointer to such a function before execution.
		*/
		void* action = nullptr;

		/**
		 * @brief Converts this object to string. This is syntactic sugar over toString.
		 * @returns The string representation of this object.
		 */
		operator std::string() const {
			std::stringstream ss;
			ss << action;

			return std::string("{ data: ") + std::string(data) + ", action: 0x" + ss.str() + " }";
		}

		/**
		 * @brief Converts this object to string.
		 * @returns The string representation of this object.
		 */
		std::string toString() const {
			return std::string(*this);
		}

		/**
		 * @brief Equality operator.
		 * @param[in] other The rhs of this operator.
		 * @returns `true` if this object and `other` are equal; `false` otherwise.
		 */
		bool operator==(const LLSynthesizedRecord& other) const {
			return this->action == other.action && this->data == other.data;
		}

	};

	/**
	* @brief A simple base class for action records from which you can derive more complex classes.
	* @tparam DataT The type of the data stored in this record.
	*/
	template <typename DataT>
		requires requires (DataT data) { std::string(data); data == data; }
	struct LLActionRecord {

		/**
		 * @brief The data stored in this action record.
		 */
		DataT data;

		/**
		* @brief The action to be executed (on `data` and the stack) when encountering this record.
		* @attention The signature of the function is expected to be: void(StackType, DataT). The type of this field should be cast to a pointer to such a function before execution.
		*/
		void* action = nullptr;

		/**
		 * @brief Converts this object to string. This is syntactic sugar over toString.
		 * @returns The string representation of this object.
		 */
		operator std::string() const {
			std::stringstream ss;
			ss << action;

			return std::string("{ data: ") + std::string(data) + ", action: 0x" + ss.str() + " }";
		}

		/**
		 * @brief Converts this object to string.
		 * @returns The string representation of this object.
		 */
		std::string toString() const {
			return std::string(*this);
		}

		/**
		 * @brief Equality operator.
		 * @param[in] other The rhs of this operator.
		 * @returns `true` if this object and `other` are equal; `false` otherwise.
		 */
		bool operator==(const LLActionRecord& other) const {
			return this->action == other.action && this->data == other.data;
		}

	};

}

// LR PARSING 
namespace m0st4fa::parsix {

	template<typename DataT, typename TokenT>
	struct LRState;

	/**
	 * @brief Aliases the type of the numerical representation of a state of an LR parsing stack.
	 */
	using lrstate_t = size_t;

	/**
	 * @brief Aliases the type of an LR parsing stack.
	 * @tparam DataT The type of data that can be stored in a state of the stack.
	 * @tparam TokenT The type of a token returned by the lexical analyzer.
	 */
	template<typename DataT, typename TokenT>
	using LRStackType = StackType<LRState<DataT, TokenT>>;

	/**
	 * @brief An element in the body of a production of a grammar whose (potential) strings are to be parsed by an LR parser.
	 * @tparam SymbolT The type of a symbol in the grammar.
	 */
	template<typename SymbolT>
	struct LRProductionElement {

		/**
		 * @brief The type of this production body element.
		 * @note For now, it should only be a symbol and should never be changed.
		 */
		ProdElementType type = ProdElementType::PET_GRAM_SYMBOL;

		/**
		 * @brief The actual element.
		 * @note This is a union (instead of a normal field) to allow extension (in the future) to other possible elements that can exist in the production of a grammar whose (potential) strings are to be parsed by an LR parser.
		 */
		union {
			SymbolT gramSymbol;
		} as;

		/**
		 * @brief Default constructor.
		 */
		LRProductionElement() = default;

		/**
		 * @brief Default copy constructor.
		 */
		LRProductionElement(const LRProductionElement&) = default;

		/**
		 * @brief Converting constructor. Initializes this object using `symbol`.
		 * @param[in] symbol The grammatical symbol used to initialize this object.
		 */
		LRProductionElement(const SymbolT& symbol) : as{ symbol } {};

		/**
		 * @brief Converts this object to string. This is syntactic sugar over toString.
		 * @returns The string representation of this object.
		 */
		operator std::string() const {
			return this->toString();
		};

		/**
		 * @brief Equality operator.
		 * @param[in] rhs The rhs of this operator.
		 * @returns `true` if this object and `rhs` are equal; `false` otherwise.
		 */
		bool operator==(const LRProductionElement& rhs) const {
			return this->type == rhs.type && this->as.gramSymbol == rhs.as.gramSymbol;
		}

		/**
		 * @brief Converts this object to string.
		 * @returns The string representation of this object.
		 */
		std::string toString() const {
			return this->as.gramSymbol.toString();
		}

	};

	/**
	 * @brief The type of a state on the LR parsing stack.
	 * @tparam DataT The type of the data associated with the state.
	 * @tparam TokenT The type of tokens returned by the lexical analyzer used with the LR parser.
	 */
	template<typename DataT, typename TokenT>
	struct LRState {

		/**
		 * @brief The numerical representation of this state.
		 */
		lrstate_t state = SIZE_MAX;

		/**
		 * @brief Data associated with this state.
		 */
		DataT data{};

		/**
		 * @brief The token represented by this state. 
		 * @details This is the token returned by the lexical analyzer when this state was pushed on the stack (i.e., the one that caused it to be pushed on the stack.) The state can be thought of as a "lighter and simpler" representation of this token **in the particular context** in which it was found. It thus carries much more information than merely the token carries (namely: token + context).
		 */
		TokenT token = TokenT::EPSILON;

		// methods

		/**
		 * @brief The default constructor.
		 */
		LRState() = default;

		/**
		 * @brief Converting constructor.
		 * @param[in] state The numerical value used to initialize the state.
		 * @param[in] data The data that will be associated initially with the state.
		 */
		LRState(lrstate_t state, const DataT& data) : state{ state }, data{ data } {};

		/**
		 * @brief Converting constructor.
		 * @param[in] state The numerical value used to initialize the state.
		 */
		LRState(lrstate_t state) : state{ state } {};

		/**
		 * @brief Converts this object to string. This is syntactic sugar for toString.
		 * @returns The string representation of this object.
		 */
		operator std::string() const {
			return this->toString();
		}

		/**
		 * @brief Equality operator.
		 * @param[in] other The rhs of this operator.
		 * @returns `true` if this object and `other` are equal; `false` otherwise.
		 */
		bool operator==(const LRState& rhs) {
			return this->state == rhs.state && this->data == rhs.data && token == token;
		}

		/**
		 * @brief Checks whether there's data associated with this state or not.
		 * @returns `true` if there's data stored in this state; `false` otherwise.
		 */
		bool hasData() const {
			return (bool)data;
		}

		/**
		 * @brief Converts this object to string.
		 * @returns The string representation of this object.
		 */
		std::string toString() const {
			std::string res = std::format("<{}", state);

			if (this->hasData())
				res += ", " + (std::string)this->data;

			res += ">";

			return res;
		}

	};
}