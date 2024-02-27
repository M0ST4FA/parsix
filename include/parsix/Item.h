#pragma once
#include <set>
#include <string>
#include <vector>

#include "parsix/production.h"
#include "utility/common.h"

// DECLARATOIN
namespace m0st4fa::parsix {

	/**
	 * @brief A set of symbols that constitute the lookaheads of an Item.
	 * @tparam SymbolT The type of a lookahead symbol.
	*/
	template <typename SymbolT>
	using LookAheadSet = std::set<SymbolT>;

	/**
	 * @brief An LR item.
	 * @tparam ProductionT The type of the production component of the item.
	 * @attention This is for developers implementing any thing on this class: take care about non-symbol objects within productions.
	 */
	template <typename ProductionT>
	class Item {

		/**
		 * @brief The type of an element of the production component of an item.
		*/
		using ProductionElementType = decltype(ProductionT{}.at(0));

		/**
		 * @brief The type of a symbol used in a production.
		 */
		using SymbolT = decltype(ProductionT{}.prodHead);

		/**
		 * @brief The type of the second component in an item, the lookahead.
		 */
		using LookAheadSet = LookAheadSet<SymbolT>;

		/**
		 * @brief The type of an index.
		 */
		using pos_t = size_t;

	protected:

		/**
		 * @brief The logger of a given instance of an Item object or of one of its subclasses.
		 */
		Logger p_Logger{};

		/**
		 * @brief The position of the dot within the production body, without caring about the types of the elements of the production body.
		 */
		pos_t m_ActualDotPos = 0;

	public:

		/** 
		 *@brief The Item object representing an empty item, defined as a constant for easier reference.
		*/
	 	static const Item EMPTY_ITEM;

		// NOTE: when implementing items, take care of non-symbol objects within the production.

		/**
		 * @brief The first component of an item, the production.
		 * @details Dot position rules:
			* 0 => the dot is at the beginning of the body, just before the first symbol.
				* This also means the dot is before the 0th element.
			* *n*, *n* > 0 && *n* < `prodSize` => the dot is before the *n*\\th element.
			* `prodSize` => the dot is at the end of the production, after the last symbol.
		 * @attention The dot position does no take non-grammar-symbol elements in mind. I.e., the object returned can be an action record, ...etc..
		 * 
		*/
		ProductionT production;
		
		/**
		 * @brief The position of the dot within the production.
		 */
		pos_t dotPos = 0;

		/**
		 * @brief The second component of an item, the lookahead/s.
		 */
		LookAheadSet lookaheads{};

		/**
		 * @brief Default constructor.
		 */
		Item() = default;

		/**
		 * @brief Converting constructor.
		 * @param[in] production The first component of the item, the production.
		 * @param[in] dotPosition The position of the dot within the production.
		 * @param[in] lookaheads The second component of the item, the lookaheads.
		 */
		Item(const ProductionT& production, pos_t dotPosition, const LookAheadSet& lookaheads = {}) :
			production{ production }, dotPos{ dotPosition }, lookaheads{ lookaheads }
		{
			const auto& prodBody = this->production.prodBody;
			size_t psize = std::count_if(
				prodBody.begin(), prodBody.end(), 
				[](const auto& element) {return element.type == ProdElementType::PET_GRAM_SYMBOL; }
			);

			// check that the dot position is within range
			if ((this->dotPos > psize) or (this->dotPos < 0)) {
				this->p_Logger.log(LoggerInfo::ERR_INVALID_VAL, "Invalid dot position in item. Make sure the dot position is smaller than the size of the production body.");
				throw std::logic_error("Invalid dot position in item.");
			}

			pos_t encountered = 0;
			while (encountered != this->dotPos) {

				bool isSymbol = this->production.prodBody.at(this->m_ActualDotPos).type == ProdElementType::PET_GRAM_SYMBOL;

				if (isSymbol)
					encountered++;

				this->m_ActualDotPos++;
			}

		};

		/**
		* @brief Checks whether to Item objects are "equal".
		* @details Two Item objects are considered "equal" iff both of their components (the production and the lookaheads), as well as the dot position, are equal.
		* @param[in] other The second operand of the operator.
		* @return `true` iff both Item objects have the same first component, and identical lookahead sets; `false` otherwise.
		*/
		bool operator==(const Item& other) const {
			// test the dot position first since it is a common discrepancy and its test is faster.
			return (other.dotPos == this->dotPos) && (other.production == this->production) && (other.lookaheads == this->lookaheads);
		};

		/**
		 * @brief Converts Item object to which it is applied to a string. This is syntactic sugar over toString.
		 * @returns The string representation of the Item object to which the function is applied.
		 */
		operator std::string() const {
			return this->toString();
		};

		/**
		 * @brief Converts the Item object to which it is applied to a string.
		 * @attention **This is for developers:** be cautious about non-grammar symbols in the production body.
		 * @returns The string representation of the Item object to which the function is applied.
		 */
		std::string toString() const {
			// CAUTION: be cautious of non-grammar-symbol objects!!!
			const auto& prodBody = this->production.prodBody;
			size_t psize = this->production.size();
			bool dotEmitted = false;
			std::string msg = "["+ this->production.prodHead.toString() + " ->";

			for (size_t i = 0; const auto& element : this->production.prodBody) {

				// skip non-grammar-symbol objects
				if (element.type != ProdElementType::PET_GRAM_SYMBOL) {
					msg += " " + (std::string)element;
					continue;
				}

				// the element is a grammar symbol
				const SymbolT& symbol = element.as.gramSymbol;

				// if the index of the symbol is the same as the dot position
				if (i == this->dotPos) {
					// this means that the dot precedes that symbol
					msg += " . " + (std::string)symbol;
					dotEmitted = true;
					i++;
					continue;
				}

				// if the dot is not at this symbol
				msg += " " + (std::string)symbol;

				// check if it is at the end
				if (i == psize - 1 && not dotEmitted)
					msg += " .";

				i++; 
			}

			// if this item has any lookaheads
			if (this->lookaheads.size() > 0) {
				msg += ", " + (std::string)*this->lookaheads.begin();

				for (const SymbolT& lookahead : this->lookaheads) {
					// if this is the first lookahead, return since we have already included it
					if (lookahead == *this->lookaheads.begin())
						continue;

					// if this is not the first lookahead
					msg += " / " + (std::string)lookahead;
				}
			}

			return msg + "]";
		}

		/**
		 * @brief Gets the actual position of the dot within the production body. This counts non-symbol elements within the production body.
		 * @returns The actual position of the dot within the production body.
		 */
		size_t getActualDotPosition() const { return this->m_ActualDotPos; }

		/**
		 * @brief Checks whether the dot is at the end of the production body.
		 * @returns `true` if the dot is at the end of the production body; `false` otherwise.
		 */
		bool isDotPositionAtEnd() const { return this->m_ActualDotPos == this->production.size(); }

		/**
		* @brief Gets the ProductionElementType object of the production body at the current position of the dot (i.e. the element just after the dot).
		* @note This doesn't take into account the type of the element. I.e., the element could as well be a non-symbol element, e.g., a synthetic record or an action record element.
		* @return The ProductionElementType object at the dot position if the dot is not at the end of the production body; otherwise a default-constructed ProductionElementType object.
		*/
		ProductionElementType atDotPosition() const { 
			if (this->m_ActualDotPos < this->production.size())
				return this->production.at(this->m_ActualDotPos);

			return ProductionElementType{};
		}

		/**
		* @brief A shortcut function for getting the SymbolT object at the current dot position (without having to get the ProductionElementType object and then accessing the other one from this.)
		* @details Gets the SymbolT object at the dot position. This is different from atDotPosition in that the latter returns the ProductionElementType object, instead of the SymbolT object.
		* @returns The SymbolT object at the dot position iff the dot is not at the end of the production body; otherwise a default-constructed SymbolT object.
		*/
		SymbolT symbolAtDotPosition() const { 
			if (this->m_ActualDotPos < this->production.size())
				return this->production.at(this->m_ActualDotPos).as.gramSymbol; 

			return SymbolT{};
		}
		
		/**
		 * @brief Checks whether two Item objects have identical cores (referring to the production as well as the position of the dot).
		 * @returns `true` iff both Item objects have identical cores; `false` otherwise.
		 */
		bool hasIdenticalCore(const Item& other) const {
			return dotPos == other.dotPos && m_ActualDotPos == other.m_ActualDotPos && production == other.production;
		}
	};

	template <typename ProductionT>
	const Item<ProductionT>
	Item<ProductionT>::EMPTY_ITEM {};

	/**
	 * @brief A set of Item objects.
	 * @note The ItemSet object doesn't have a grammar associated with it (i.e., doesn't have an attribute storing a particular grammar, which means that each time you want to calculate any thing requiring a grammar, you will have to input it). This may seem nonsense, however, the idea was that the same object could be utilized at different times for different grammars. This is because the object is large and expensive. I'm considering changing this behavior to the more logical one.
	 * @todo Consider associating ItemSet objects with a grammar.
	 * @tparam ItemT The type of an item in the set.
	 */
	template <typename ItemT> 
	class ItemSet {

		/**
		 * @brief The type of the production of an Item object.
		 */
		using ProductionType = decltype(ItemT{}.production);

		/**
		 * @brief The type of a stack element.
		 */
		using StackElemType = decltype(ProductionType{}.at(0));

		/**
		 * @brief The type of grammatical symbols, whether terminal or non-terminal.
		 */
		using SymbolType = decltype(ProductionType{}.prodHead);

		/**
		 * @brief The type of non-terminals.
		 */
		using VariableType = decltype(SymbolType{}.as.nonTerminal);

		/**
		 * @brief The type of a string of symbols of the type SymbolString.
		 */
		using SymbolStringType = SymbolString<SymbolType>;

		/**
		 * @brief The type of a vector/set of productions of the type ProductionVector.
		 */
		using ProdVecType = ProductionVector<ProductionType>;

		/**
		 * @brief The type of a lookahead set of the type LookAheadSet.
		 */
		using LookAheadSet = LookAheadSet<SymbolType>;

		friend Item<ProductionType>;

		/**
		 * @brief The set of Item objects.
		 */
		std::vector<ItemT> m_Set {};

		/**
		 * @brief A cache of the closure of the Item set.
		 */
		std::vector<ItemT> m_Closure{};

		bool _add_to_closure_no_lookaheads(const ProdVecType&);
		bool _add_to_closure_lookaheads(const ProdVecType&, const LookAheadSet&);

		/**
		 * @brief Sets the closure cache of the ItemSet to `newClosure`, overriding whatever value was stored there.
		 * @param[in] newClosure The closure to which the closure of this ItemSet is to be set.
		 */
		void _set_closure(const ItemSet& newClosure) {
			this->m_Closure = newClosure;
		}

	protected:

		/**
		 * @brief The logger of a given instance of an Item object or of one of its subclasses.
		 */
		Logger p_Logger;

		/**
		 * @brief Gets the iterator pointing to a given Item object. The Item object can be in the ItemSet collection or in the cached closure of the ItemSet collection of Item objects.
		 * @param[in] production The production associated with the Item object.
		 * @param[in] dotPosition The position of the dot within `production`.
		 * @param[in] fromClosure If `true`, search the cached closure of the ItemSet object. If `false`, search the set itself.
		 * @returns If an Item object is found, a vector iterator object pointing to the found Item object; otherwise (if no Item object matches), returns vector iterator object pointing to the end of either the vector object used to store the Item object set or the vector object used to store the cached closure.
		 */
		std::vector<ItemT>::iterator get_item_it(const ProductionType& production, size_t dotPosition, bool fromClosure = false) {
			auto begin = fromClosure ? this->m_Closure.begin() : this->m_Set.begin();
			auto end = fromClosure ? this->m_Closure.end() : this->m_Set.end();

			auto predicate = [this, &production, &dotPosition](const ItemT& item) {
				const auto& itemProd = item.production;
				size_t itemDotPos = item.dotPos;

				return (itemDotPos == dotPosition) && (itemProd == production);
			};

			const auto& it = std::find_if(begin, end, predicate);

			return it;
		}

		/**
		 * @brief Gets the end iterator of the underlying vector object storing the set.
		 * @note It is intended to be used with get_item_it to check whether an Item object has been found or not.
		 * @returns An iterator to the end of the underlying std::vector object storing the Item object.
		 */
		std::vector<ItemT>::iterator get_set_end() const {
			return this->m_Set.end();
		}

		/**
		 * @brief Gets the end iterator of the underlying vector object storing the cached closure of the set.
		 * @note It is intended to be used with get_item_it to check whether an Item object has been found or not.
		 * @returns An iterator to the end to the underlying std::vector object storing the Item objects of the cached closure.
		 */
		std::vector<ItemT>::iterator get_closure_end() const {
			return this->m_Closure.end();
		}

		/**
		 * @brief A constructor for constructing an ItemSet object for which the set is its own closure, to start with.
		 * @param items The Item objects to be stored in the ItemSet object.
		 * @param sameClosure A redundant parameter for function overloading.
		 */
		ItemSet(const std::vector<ItemT> items, bool sameClosure) : m_Set{ items }, m_Closure{ items } {}
	public:

		/**
		 * @brief Default constructor.
		 */
		ItemSet() = default;

		/**
		 * @brief Constructs an ItemSet object from an initializer list of Item objects.
		 * @param[in] items The initializer list of Item objects.
		 */
		ItemSet(const std::initializer_list<ItemT>& items) : m_Set{ items } {};

		/**
		 * @brief Constructs an ItemSet object from a vector of Item objects.
		 * @param[in] items The vector of Item objects.
		 */
		ItemSet(const std::vector<ItemT> items) : m_Set{ items } {};

		/**
		 * @brief Copy constructor.
		 */
		ItemSet(const ItemSet& other) : m_Set{ other.m_Set }, m_Closure{ other.m_Closure } {};

		/**
		 * @brief Move constructor.
		 */
		ItemSet(ItemSet&& other) : m_Set{ std::move(other.m_Set) }, m_Closure{ std::move(other.m_Closure) } {};

		// CAUTION: when implementing these functions, be cautious that a single item object may represent different items.

		// OPERATOR FUNCTIONS
		/**
		* @brief Compares two ItemSet objects for equality.
		* @param[in] rhs The right hand side (the second operand) of the comparison operator.
		* @returns `true` iff the two ItemSet objects are identical; `false` otherwise.
		*/
		bool operator==(const ItemSet& rhs) const {
			return size() == size() && this->m_Set == rhs.m_Set;
		};

		/**
		 * @brief Checks whether the size of the first (left operand) ItemSet object is less than the size of the second ItemSet object.
		 * @param[in] rhs The right hand side (second operand) of the comparison.
		 * @returns `true` iff the size of the first object (left operand) is less than that of the second; `false` otherwise.
		 */
		bool operator<(const ItemSet& rhs) const {
			return this->m_Set.size() < rhs.m_Set.size();
		}

		/**
		 * @brief Converts this ItemSet object to string. This is syntactic sugar over toString.
		 * @returns The string representation of this ItemSet object.
		 */
		operator std::string() const {
			return this->toString();
		}

		/**
		 * @brief Converts this ItemSet object to an std::vector object. This is syntactic sugar over getItemVector.
		 * @returns The std::vector representation of this ItemSet object. Particularly, it returns a copy of the underlying (private) vector in which the Item objects are stored.
		 */
		operator std::vector<ItemT>() const {
			return this->getItemVector();
		}

		// NON-OPERATOR FUNCTIONS

		/**
		 * @brief Converts this ItemSet object to a string.
		 * @returns String representation of this ItemSet object.
		 */
		std::string toString() const {

			std::string str;

			for (const auto& item : this->m_Set)
				str += (std::string)item + "\n";

			if (str.empty())
				return "{ }";

			return (std::string)"{\n" + str + "}";
		};

		/**
		 * @brief Converts this ItemSet object to an std::vector object.
		 * @returns The std::vector representation of this ItemSet object. Particularly, it returns a copy of the underlying (private) vector in which the Item objects are stored.
		 */
		std::vector<ItemT> getItemVector() const { return this->m_Set; }
		
		/**
		 * @brief Gets a reference to the Item object at a given index.
		 * @throws std::out_of_range Thrown when the `index` is out of range.
		 * @param[in] index The index of the Item object within the set.
		 * @returns A reference to the Item object with the `index` position within the set.
		 */
		const ItemT& at(size_t index) const { return this->m_Set.at(index); }

		/**
		 * @brief Gets the iterator pointing at the beginning of the underlying set storing the Item objects.
		 */
		auto begin() { return this->m_Set.begin(); }

		/**
		 * @brief Gets the iterator pointing at the end of the underlying set of Item objects.
		 */
		auto end() { return this->m_Set.end(); }

		/**
		 * @brief Gets the constant iterator pointing at the beginning of the underlying set storing the Item objects.
		*/
		auto begin() const { return this->m_Set.begin(); }

		/**
		 * @brief Gets the constant iterator pointing at the end of the underlying set storing the Item objects.
		*/
		auto end() const { return this->m_Set.end(); }

		/**
		 * @brief Gets the size of the underlying set.
		 */
		size_t size() const { return this->m_Set.size(); }

		/**
		 * @brief Checks whether the underlying set is empty or not.
		 * @returns `true` iff the underlying set is empty; `false` otherwise.
		 */
		bool empty() const { return this->m_Set.empty(); }

		bool insert(const ItemT&);
		
		bool merge(const ItemSet&);
		
		ItemT get(const ProductionType&, size_t) const;
		
		bool contains(const ItemT&) const;

		bool hasIdenticalCore(const ItemSet&) const;

		ItemSet CLOSURE(ProdVecType*const);
		
		ItemSet GOTO(const SymbolType&, ProdVecType*const);

	};
}

// IMPLEMENTATION
namespace m0st4fa::parsix {

	/**
	 * @brief Adds any new non-kernel Item objects, constructed from the given "grammar", to the (possibly already existing) CLOSURE set of this ItemSet object.
	 * @details The new Item objects are all non-kernel, having the dot position always at 0.
	 * @details Basically, what this function does is the following: it goes through every production in the grammar, constructs the non-kernel Item object and then inserts that Item object into this ItemSet object, if it wasn't there before.
	 * @details The new Item objects will have no lookaheads.
	 * @tparam ItemT The type of an element of an ItemSet object.
	 * @param[in] symbolProductions All of the productions of the grammar.
	 * @return `true` if at least one element has been added to the CLOSURE set (i.e., if the CLOSURE set has been modified); `false` otherwise.
	 */
	template<typename ItemT>
	bool ItemSet<ItemT>::_add_to_closure_no_lookaheads(const ProdVecType& symbolProductions)
	{
		bool inserted = false;
		this->p_Logger.logDebug(std::format("(Possibly) adding new items to the closure of an item set"));
		this->p_Logger.logDebug(std::format("Closure before:\n {}", m0st4fa::toString(this->m_Closure, false)));

		// go through every production and insert the non-kernel items you create into the closure.
		for (const ProductionType& production : symbolProductions) {
			const ItemT item = ItemT{ production, 0 };

			// find an item with the same first component
			const auto it = this->get_item_it(item.production, item.dotPos, false);
			const auto end = this->m_Closure.end();
			bool found = it != end;

			// if no entry with the same first component is found
			if (not found) {
				this->m_Closure.push_back(item);
				inserted = true;
			} // if one was found, we cannot insert it again

		}
		this->p_Logger.logDebug(std::format("Closure after:\n {}", m0st4fa::toString(this->m_Closure, false)));

		return inserted;
	}

	/**
	 * @brief Adds any new non-kernel Item objects, constructed from the given "grammar", to the (possibly already existing) CLOSURE set of this ItemSet object.
	 * @details The new Item objects are all non-kernel, having the dot position always at 0.
	 * @details Basically, what this function does is the following: it goes through every production in the grammar, constructs the non-kernel Item object and then inserts that Item object into this ItemSet object, if it wasn't there before.
	 * @details The new Item objects will have lookaheads, given as an argument to this function.
	 * @tparam ItemT The type of an element of an ItemSet object.
	 * @param[in] symbolProductions All of the productions of the grammar.
	 * @param[in] lookaheads The lookaheads of the non-kernel Item objects that will be added to this ItemSet object.
	 * @return `true` if at least one element has been added to the CLOSURE set (i.e., if the CLOSURE set has been modified); `false` otherwise.
	 */
	template<typename ItemT>
	bool ItemSet<ItemT>::_add_to_closure_lookaheads(const ProdVecType& symbolProductions, const LookAheadSet& lookaheads)
	{
		bool inserted = false;
		this->p_Logger.logDebug(std::format("Closure before:\n {}", m0st4fa::toString(this->m_Closure, false)));

		// go through every production and insert the non-kernel items you create into the closure.
		for (const ProductionType& production : symbolProductions) {
			const ItemT item = ItemT{ production, 0 };

			// go through every production and insert the non-kernel items you create into the closure
			for (const ProductionType& production : symbolProductions) {

				// find an item with the same first component
				const auto it = this->get_item_it(production, 0, false);
				const auto end = this->m_Closure.end();
				bool found = it != end;

				// if an entry with the same first component is found
				if (found) {
					if (insertAndAssert(lookaheads, it->lookaheads))
						inserted = true;
				}
				else {
					// if no entry with the same first component is found
					const auto item = ItemT{ production, 0, lookaheads };
					this->m_Closure.push_back(item);
					inserted = true;
					this->p_Logger.logDebug(std::format("Adding new item {} to the closure of the item set", item.toString()));
				}

			}

		}		
		
		this->p_Logger.logDebug(std::format("Closure after:\n {}", m0st4fa::toString(this->m_Closure, false)));

		return inserted;
	}

	/**
	 * @brief Checks whether a given Item objects "exists" in this ItemSet object.
	 * @details `exists` in this context means that there exists an Item object with the same first component. The second component, however, must be a subset (i.e., they don't need to have the same lookahead set.)
	 * @param[in] item The Item object to search for its "existence" within this ItemSet object.
	 * @return `true` if such an Item object "exists" in this ItemSet object; `false` otherwise.
	 */
	template<typename ItemT>
	inline bool ItemSet<ItemT>::contains(const ItemT& item) const
	{
		auto begin = this->m_Set.begin();
		auto end = this->m_Set.end();

		auto predicate = [this, &item](const ItemT& i) {
			// 'item' is considered found if we found an 'i' such that:
			// 1. the first components are the same.
			// 2. the second component of 'item' is a subset of its 'i's counterpart.

			// check for equality of the first components
			if (item.dotPos != i.dotPos && item.production != i.production)
				return false;

			// check if item.lookaheads is a subset of i.lookaheads
			// i.e. if i.lookaheads includes item.lookaheads

			const auto ibegin = i.lookaheads.begin();
			const auto iend = i.lookaheads.end();
			const auto itembegin = item.lookaheads.begin();
			const auto itemend = item.lookaheads.end();

			return std::includes(ibegin, iend, itembegin, itemend);
		};

		return (std::find_if(begin, end, predicate) != end);
	}

	// TODO: OPTIMIZE THIS FUNCTION
	/**
	 * @brief Checks whether this ItemSet object has an identical core with another ItemSet object.
	 * @details The core of an item is its production together with the dot position. The core of an item set is the set of the cores of all of its items (the set of all of its items, without the lookaheads.) I.e., what this function does is that it checks that, for every item `i` within one of them, the other set has an item with an identical core to `i`.
	 * @todo Optimize this function.
	 * @param[in] other The ItemSet object with which to check for having the same core as this ItemSet object.
	 * @returns `true` if this ItemSet object has the same core as `other`; `false` otherwise.
	 */
	template<typename ItemT>
	bool ItemSet<ItemT>::hasIdenticalCore(const ItemSet& other) const
	{
		if (this->size() != other.size())
			return false;

		const auto thisBegin = this->m_Set.begin();
		const auto thisEnd = this->m_Set.end();
		const auto otherBegin = other.m_Set.begin();
		const auto otherEnd = other.m_Set.end();

		const auto pred = [this, &other, otherBegin, otherEnd](const ItemT& thisItem) {
			bool t = std::any_of(otherBegin, otherEnd, [&thisItem](const ItemT& otherItem) {
				return thisItem.hasIdenticalCore(otherItem);
				});

			return t;
		};

		// if they have the same size, they may have the same cores
		return std::all_of(thisBegin, thisEnd, pred);
	}

	/**
	 * @brief Retrieves all of the items with a given first component (production and dot) in this item set, if any.
	 * @param[in] production The production of the first component of the Item objects to be retrieved.
	 * @param[in] dotPosition The position of the dot within the production of the first component of the Item objects to be retrieved.
	 * @return All of the Item objects with the given first component (specified by the arguments), if any; empty Item objects if no such Item objects exist.
	 */
	template<typename ItemT>
	inline ItemT ItemSet<ItemT>::get(const ProductionType& production, size_t dotPosition) const
	{
		auto begin = this->m_Set.begin();
		auto end = this->m_Set.end();

		auto predicate = [this, &production, &dotPosition](const ItemT& item) {
			const auto& itemProd = item.production;
			size_t itemDotPos = item.dotPos;

			return (itemDotPos == dotPosition) && (itemProd == production);
		};

		const auto& it = std::find_if(begin, end, predicate);

		// return an item if it is found, else return an empty item
		return (it == end ? ItemT{} : *it);
	}

	/**
	 * @brief Inserts a new item into the item set. If an item with the first component is already present, it only inserts the lookahead.
	 * @param[in] item The Item object to be added into this ItemSet object (or its lookahead set modified.)
	 * @return `true` if a new item was inserted; `false` otherwise.
	 */
	template<typename ItemT>
	bool ItemSet<ItemT>::insert(const ItemT& item)
	{
		// find an item with the same first component
		const auto it = this->get_item_it(item.production, item.dotPos);
		const auto end = this->m_Set.end();
		bool found = it != end;

		// if an entry with the same first component is found
		if (found)
			return insertAndAssert(item.lookaheads, it->lookaheads);

		// if no entry with the same first component is found
		this->m_Set.push_back(item);

		return true;
	}

	// TODO: OPTIMIZE THIS FUNCTION
	/**
	 * @brief Merges an ItemSet object (`other`) with this ItemSet object.
	 * @param[in] other The ItemSet object to be merged into this ItemSet object.
	 * @details For each Item object `i` in `other`, either `i` already exists in this ItemSet object or not: 
		* If `i` exists this ItemSet object, then any lookaheads that this `i` doesn't have will be added to its lookahead set.
		* Otherwise (i.e. `i` doesn't exist), `i` will be added to this ItemSet object.
	 * @todo Optimize this function.
	 * @returns `true` if any new Item objects have been added after the merge is done; `false` otherwise.
	 */
	template<typename ItemT>
	bool ItemSet<ItemT>::merge(const ItemSet& other)
	{

		for (const ItemT& item : other.m_Set) {
			const auto itemIt = std::find_if(m_Set.begin(), m_Set.end(), [&item](const ItemT& i) {
				return item.hasIdenticalCore(i);
				});

			// if the item does not exist
			if (itemIt == m_Set.end()) {
				this->m_Set.push_back(item);
				continue;
			}

			// if such an item exists
			itemIt->lookaheads.insert(item.lookaheads.begin(), item.lookaheads.end());

		}

		return true;
	}

	/**
	 * @brief Calculates (if not cached) and caches the CLOSURE set, for a given grammar, of this ItemSet object.
	 * @details The CLOSURE set of an item set, given a given grammar, is a set that, for every non-terminal in the grammar, stores all of the items of the item set in which the dot is just before this non-terminal in the production of that item. 
	 * @details The name is generic; we think of it as constituting the "closing" calculation on the item set. It is used extensively for generating parsing tables.
	 * @details The CLOSURE set is cached so that next time it is returned immediately. The reason for the caching is that the calculation is very expensive (especially as the number of lookaheads for each item (i.e., the number of items) increases).
	 * @param[in] grammarPtr A pointer to the grammar (production vector) object that will be used for calculating the CLOSURE set.
	 * @note You must input the grammar as, by design right now, an ItemSet object is not associated with any grammar.
	 * @return The CLOSURE set of this ItemSet object.
	 */
	template<typename ItemT>
	ItemSet<ItemT> ItemSet<ItemT>::CLOSURE(ProdVecType*const grammarPtr)
	{
		// check if the closure is already calculated
		if (this->m_Closure.size() > 0) {
			this->p_Logger.logDebug("CLOSURE set for this item set has already been calculated!");
			return this->m_Closure;
		}

		ProdVecType& grammar = *grammarPtr;
		// check if the set is not empty (its closure would therefore be empty)
		if (this->m_Set.size() == 0) {
			this->p_Logger.log(LoggerInfo::WARNING, "Item set is empty. Returning empty CLOSURE!");
			return *this;
		}

		// keep track of alternative productions of symbols
		std::vector<size_t> alternativeProduction[(size_t)VariableType::NT_COUNT]{};
		for (size_t prodIndex = 0; const ProductionType & prod : grammar.getVector()) {
			size_t varIndex = (size_t)prod.prodHead.as.nonTerminal;
			alternativeProduction[varIndex].push_back(prodIndex);
			prodIndex++;
		}

		this->p_Logger.logDebug("\nCALCULATING CLOSURE SET:\n");
		this->p_Logger.logDebug(std::format("Items set:\n {}", (std::string)*this));

		// initialize the closure to the item set
		this->m_Closure = ItemSet{this->m_Set};

		// check whether the items are LR(0) or they have lookaheads
		bool isLR0 = this->m_Closure.at(0).lookaheads.size() == 0;
		std::vector<std::set<SymbolType>> first{};
		if (not isLR0)
			grammar.calculateFIRST(), first = grammar.getFIRST();

		this->p_Logger.logDebug(std::format("The items in this set are LR({})", isLR0 ? "0" : "1"));

		// for every item of the closure, calculate the closure of that item
		for (size_t i = 0; i < this->m_Closure.size(); i++) {
			const ItemT item = this->m_Closure.at(i);
			this->p_Logger.logDebug(std::format("Current item being scanned for CLOSURE:{}",item.toString()));
			const ProductionType& itemProd = item.production;

			// if the dot is at the end of the production
			if (itemProd.size() == item.dotPos)
				continue; // there is nothing more to add

			// get the symbol before the dot; note that it is guaranteed to be a symbol
			// BUG: item.getActualDotPosition()
			const SymbolType& symbolAfterDot = itemProd.at(item.getActualDotPosition()).as.gramSymbol;
			this->p_Logger.logDebug(std::format("Symbol after dot is {}", symbolAfterDot.toString()));
			
			// check if this symbol is a non-terminal
			if (symbolAfterDot.isTerminal) {
				// if it is a terminal, the closure of the item is itself and the itemisalrea there in the closure (since we are checking it now)
				this->p_Logger.logDebug("No other item has been added to CLOSURE");
				continue;
			}

			// if we are here, the symbol is a non-terminal
			// get all the productions for that non-terminal (all of its alternatives)
			ProdVecType symbolProductions {};
			const std::vector<size_t>& prods = alternativeProduction[(size_t)symbolAfterDot.as.nonTerminal];
			for (size_t prodIndex : prods)
				symbolProductions.pushProduction(grammar.at(prodIndex));

			this->p_Logger.logDebug(std::format("The alternatives of non-terminal {} are: {}",symbolAfterDot.toString(), symbolProductions.toString()));

			// add the new items to the closure
			if (isLR0)
				_add_to_closure_no_lookaheads(symbolProductions);
			else // it is LR(1)
			{
				auto predIsGramSymbol = [](const auto& stackElement) {
					return stackElement.type == ProdElementType::PET_GRAM_SYMBOL;
				};

				// calculate the lookaheads
				size_t prodSize = itemProd.prodBody.size();
				size_t startIndex = item.getActualDotPosition() + 1;
				SymbolStringType symString{};
				
				// the following loop corresponds to: std::copy_if(begin + startIndex, end, symString.symbols, predIsGramSymbol);
				const auto& prodBody = itemProd.prodBody;
				for (size_t i = startIndex; i < prodBody.size(); i++) {
					const auto& se = prodBody.at(i);

					// if `se` is a grammar symbol, concatenate its corresponding grammar symbol with `symString`
					if (predIsGramSymbol(se))
						symString.push_back(se.as.gramSymbol);

				}

				this->p_Logger.logDebug(std::format("For item {} => Initial string after the grammar symbol: {}", item.toString(), symString.toString()));

				// go through each lookahead; each lookahead corresponds to a distinct item
				for (const SymbolType& lookahead : item.lookaheads) {
					// the string for the current item
					SymbolStringType tempSymString = symString;
					tempSymString.push_back(lookahead);

					// the lookaheads of the current item

					tempSymString.calculateFIRST(first);
					LookAheadSet lookaheads{tempSymString.getFIRST()};

					this->p_Logger.logDebug(std::format("The lookaheads for the current item is: {}", m0st4fa::toString(lookaheads)));

					// add the item to the CLOSURE set
					_add_to_closure_lookaheads(symbolProductions, lookaheads);
				}

			}
		}

		ItemSet res{ this->m_Closure, true };
		return res;
	}

	/**
	 * @brief Calculates the GOTO set of this ItemSet objects on a particular symbol for a particular grammar.
	 * @details This function calculates CLOSURE set (if not already calculated) and then moves on to calculate the FIRST set. This is because calculating the FIRST set requires knowledge of the CLOSURE set.
	 * @details The calculation is done as follows: first get the kernel items for the GOTO set and then take the CLOSURE of the kernel item set.
	 * @param[in] symbol The symbol whose GOTO is to be calculated.
	 * @param[in] grammarPtr A pointer to the grammar (production vector) that will be used for calculating the GOTO set for `symbol`.
	 * @note You must input the grammar as, by design right now, an ItemSet object is not associated with any grammar.
	 * @return The GOTO set of this item set on a particular symbol.
	 */
	template<typename ItemT>
	ItemSet<ItemT> ItemSet<ItemT>::GOTO(const SymbolType& symbol, ProdVecType*const grammarPtr)
	{
		// check whether the closure is already calculated
		if (this->m_Closure.size() == 0) {
			this->p_Logger.log(LoggerInfo::INFO, "CLOSURE set is not already calculated for this set! Calculating it now.");

			this->CLOSURE(grammarPtr);
		}

		ProdVecType& grammar = *grammarPtr;
		this->p_Logger.logDebug("\nCALCULATING GOTO SET:\n");
		this->p_Logger.logDebug(std::format("Item set:\n {}", (std::string)*this));
		this->p_Logger.logDebug(std::format("GOTO symbol is {}", (std::string)symbol));

		ItemSet<ItemT> result = ItemSet{};

		/* assuming symbol is `B`
			figure out whether there exists an item of the form:
			[A -> alpha .B beta, a]
		*/
		ItemSet<ItemT> foundItems{};
		for (size_t i = 0; i < this->m_Closure.size(); i++) {
			auto predicate = [&symbol](const ItemT& item) {
				size_t dotPos = item.getActualDotPosition();

				// skip this item if the dot is at the end
				if (item.production.size() == dotPos)
					return false;

				const SymbolType& sym = item.production.at(dotPos).as.gramSymbol;
				return sym == symbol;
			};

			const ItemT currItem = this->m_Closure.at(i);

			// if symbolAfterDot == symbol
			if (predicate(currItem))
				foundItems.insert(currItem);
		}

		// figure out whether such an item exists
		bool found = foundItems.size() > 0;

		// if such an item is found
		if (found) {
			const auto& itemVec = foundItems.getItemVector();
			for (const ItemT& item : itemVec) {
				// create new item
				size_t newDotPos = item.dotPos + 1;

				// ERR: THE EXCEPTION IS HERE!!!
				ItemT kernelItem = ItemT{ item.production, newDotPos, item.lookaheads };

				// insert the item in `result`
				result.insert(kernelItem);
				this->p_Logger.log(LoggerInfo::DEBUG, std::format("Inserting kernel item {} to the GOTO set.", kernelItem.toString()));
			}

		}

		const ItemSet<ItemT> resultClosure = result.CLOSURE(grammarPtr);
		this->p_Logger.logDebug(std::format("GOTO Set for the items: \n{}", (std::string)resultClosure));
		return resultClosure;
	}

}