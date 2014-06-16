#ifndef UNORDERED_BUFFER_H
#define UNORDERED_BUFFER_H

#include <chrono>
#include <random>
#include <vector>
#include <list>
#include <tuple>
#include <ctime>


/**
 * @brief Class which is used to store a buffer of values that don't have a 
 * particular ordering. A priority is kept, which is incremented with repeated
 * hits, and decremented when collisions occur. It is expected that a large 
 * amount collisions will occurr, causing replacement of key/value pairs that
 * are not frequently used.
 *
 * It is important to use accessors that will alter the priority. These include
 * insert, emplace, []. Unless you want to query the current state, you shouldn't
 * use find, at, count, equal_range, bucket, because these do NOT affect the 
 * prioity (which is based on hits of a given key).
 *
 * @tparam Key	Key type
 * @tparam T	Value Type
 * @tparam Hash	Hash class
 */
template <class Key, class T, class Hash = std::hash<Key>>
class unordered_buffer
{
public:
	class iterator;
	class const_iterator;

/******************************************************************************
 *
 * Data
 *
 ******************************************************************************/
private:
	
	struct Element
	{
		int priority;
		iterator pos; 	//position in used list
		std::pair<Key, T> value;			//actual values
	};

	// big array of the data, 0 = priority, 1 = key, 2 = data
	// keeps the priority, 0 indicates unused
	std::vector<Element> m_data;
	std::list<Element*> m_used;

	// it = m_used.begin();
	// *it is a pair
	// it->first

	std::default_random_engine m_rng;
	std::uniform_real_distribution<double> m_rdist;
	const Hash m_hasher;

	const int MAX_PRIORITY = 1000;

/******************************************************************************
 *
 * Functions 
 *
 ******************************************************************************/
public:
#ifndef NDEUBG

	/**
	 * @brief Verbosity
	 */
	bool loud;	
#endif //NDEBUG
	/**
	 * @brief Iterator
	 */
	class iterator {
	public:
		
		/**
		 * @brief Default Constructor
		 */
		iterator(){ };


		/**
		 * @brief Copy Constructor
		 *
		 * @param other
		 */
		iterator(const iterator& other){
			it = other.it;
		};
	

		/**
		 * @brief Dereference opterator
		 *
		 * @return 
		 */
		std::pair<Key,T>& operator*(){
			return (*it).value;
		};
		
		/**
		 * @brief Dereference and . operator
		 *
		 * @return 
		 */
		std::pair<Key,T>* operator->(){
			return &((*it)->value);
		};
		
		////////////////////////
		// Movement
		////////////////////////
		
		/**
		 * @brief Postfix, Advance iterator, note this makes a copy of the 
		 * key and value
		 *
		 * @param unused
		 *
		 * @return Key/value pair
		 */
		std::pair<Key,T> operator++(int unused){
			(void)(unused);
			iterator tmp = *this;
			++*this;
			return tmp;
		};
		

		/**
		 * @brief Prefix, advance iterator
		 *
		 * @return Key/value pair
		 */
		std::pair<Key,T>& operator++(){
			it++;
			return *this;
		};
	
		/**
		 * @brief Postfix step back, note that it makes a copy of the key
		 * and value
		 *
		 * @param unused
		 *
		 * @return Key/value pair
		 */
		std::pair<Key,T> operator--(int unused){
			(void)(unused);
			iterator tmp = *this;
			--*this;
			return tmp;
		};
		
		/**
		 * @brief Prefix step back
		 *
		 * @return Key/value pair
		 */
		std::pair<Key,T>& operator--(){
			it--;
			return *this;
		};

	private:
		friend class unordered_buffer<Key,T,Hash>;

		typename std::list<Element*>::iterator it;
	};

	
	/**
	 * @brief Constant Iterator
	 */
	class const_iterator {
	public:

		/**
		 * @brief Default Constructor
		 */
		const_iterator(){ };
		
		/**
		 * @brief Copy Constructor
		 *
		 * @param other
		 */
		const_iterator(const iterator& other){
			it = other.it;
		};

		/**
		 * @brief Dereference opterator
		 *
		 * @return 
		 */
		const_iterator(const const_iterator& other){
			it = other.it;
		};
		
		/**
		 * @brief Dereference and . operator
		 *
		 * @return 
		 */
		const std::pair<Key,T>& operator*() const {
			return (*it).value;
		};
		
		/**
		 * @brief Postfix, Advance iterator, note this makes a copy of the 
		 * key and value
		 *
		 * @return 
		 */
		const std::pair<Key,T>* operator->(){
			return &(*(*it));
		};
		
		////////////////////////
		// Movement
		////////////////////////
		
		/**
		 * @brief Prefix, advance iterator
		 *
		 * @param unused	indicator
		 *
		 * @return Key/Value Pair
		 */
		std::pair<Key,T> operator++(int unused){
			(void)(unused);
			iterator tmp = *this;
			++*this;
			return tmp;
		};
		
		/**
		 * @brief Postfix step back, note that it makes a copy of the key
		 * and value
		 *
		 * @return Key/Value Pair
		 */
		const std::pair<Key,T>& operator++(){
			it++;
			return *this;
		};
	
		/**
		 * @brief Postfix step back, note that it makes a copy of the key
		 * and value
		 *
		 * @param unused	indicator
		 *
		 * @return Key/Value Pair
		 */
		std::pair<Key,T> operator--(int unused){
			(void)(unused);
			iterator tmp = *this;
			--*this;
			return tmp;
		};
		
		/**
		 * @brief Prefix step back
		 *
		 * @return Key/Value Pair
		 */
		const std::pair<Key,T>& operator--(){
			--it;
			return *this;
		};

	private:
		friend class unordered_buffer<Key,T,Hash>;
		
		typename std::list<Element*>::const_iterator it;
	};

	
	/**
	 * @brief Get begin iterator
	 *
	 * @return A modifiable iterator
	 */
	iterator begin()
	{
		iterator tmp;
		tmp.it = m_used.begin();
		return tmp;
	};
	

	/**
	 * @brief Get the end iterator.
	 *
	 * @return An iterator 1 past the end.
	 */
	iterator end()
	{
		iterator tmp;
		tmp.it = m_used.end();
		return tmp; 
	};
	

	/**
	 * @brief Get the const_iterator at the beginning.
	 *
	 * @return 
	 */
	const_iterator cbegin()
	{
		const_iterator tmp;
		tmp.it = m_used.cbegin();
		return tmp;
	};
	
	/**
	 * @brief Get the const_iterator at the end.
	 *
	 * @return 
	 */
	const_iterator cend()
	{
		const_iterator tmp;
		tmp.it = m_used.cend();
		return tmp;
	};

	/**************************************************************************
	 * Constructors
	**************************************************************************/

	/**
	 * @brief Constructor that initializes the underlying hash table. Default
	 * size is 1024.
	 *
	 * @param size	The number of bins for the hash table, this stays constant
	 * 				unless resize is called.
	 */
	unordered_buffer(size_t size = 1024) : m_rng(time(NULL)), m_rdist(0,1), m_hasher()
	{
		m_data.resize(size);

		// set used variable to false
		for(size_t ii=0; ii<m_data.size(); ii++) {
			m_data[ii].priority = 0;
		}

		m_used.clear();
	};


	/**
	 * @brief Constructs a new unordered buffer with add elements from the 
	 * given input range of another iterable container.
	 *
	 * @tparam InputIterator	Intput iterator type.
	 * @param first				First iterator in range to add
	 * @param last				One past last iterator in range to add
	 * @param size				Size of underlying hash table.
	 */
	template<class InputIterator>
	unordered_buffer(InputIterator first, InputIterator last, size_t size=1024) 
		: m_rng(time(NULL)), m_rdist(0,1), m_hasher()
	{
		m_data.resize(size);

		// set used variable to false
		for(size_t ii=0; ii<m_data.size(); ii++) {
			m_data[ii].priority = 0;
		}

		m_used.clear();
		
		// now emplace the data
		for(auto it=first; it!=last; it++) {
			emplace(it.first, it.second);
		}
	};
	

	/**
	 * @brief Initializer list based constructor. This should be a list of 
	 * key/value pairs, that will be immediately added to the buffer upon
	 * construction.
	 *
	 * @param il	List of key/value pairs
	 * @param size	Size of underlying hash table (number of bins)
	 */
	unordered_buffer(std::initializer_list<std::pair<Key,T>> il, size_t size=1024) 
		: m_rng(time(NULL)), m_rdist(0,1), m_hasher()
	{
		m_data.resize(size);

		// set used variable to false
		for(size_t ii=0; ii<m_data.size(); ii++) {
			m_data[ii].priority = 0;
		}

		m_used.clear();
		
		// now emplace the data
		for(auto it=il.begin(); it!=il.end(); it++) {
			emplace(it.first, it.second);
		}
	};
	

	/**
	 * @brief Copy constructor,
	 *
	 * @param ump	Other buffer to copy
	 */
	unordered_buffer(const unordered_buffer& ump) : m_rng(time(NULL)), m_rdist(0,1), m_hasher()
	{
		m_data = ump.m_data;
		m_used = ump.m_used;
	};
	

	/**
	 * @brief Move constructor, old value will be left in inderminant state.
	 *
	 * @param ump	Other buffer to move from (will be left un-useable)
	 */
	unordered_buffer(unordered_buffer&& ump) : m_rng(time(NULL)), m_rdist(0,1), m_hasher()
	{
		m_data = std::move(ump.m_data);
		m_used = std::move(ump.m_used);
	};


	/**
	 * @brief Swaps the contents of two unordered buffers
	 *
	 * @param ump	Other buffer to swap contents with
	 */
	void swap(unordered_buffer& ump)
	{
		std::swap(ump.m_data, m_data);
		std::swap(ump.m_used, m_used);
	};


	/**
	 * @brief Set the content of the unorded_buffer from another
	 *
	 * @param ump	Other buffer
	 *
	 * @return 		A reference to ourselves.
	 */
	unordered_buffer& operator=(const unordered_buffer& ump)
	{
		m_data = ump.m_data;
		m_used = ump.m_used;

		return *this;
	};
	

	/**
	 * @brief Move equal operator, old value will be left in inderminant state.
	 *
	 * @param ump
	 *
	 * @return 
	 */
	unordered_buffer& operator=(unordered_buffer&& ump)
	{
		m_data = std::move(ump.m_data);
		m_used = std::move(ump.m_used);
		return *this;
	};
	
	/**
	 * @brief Assignment operator from an initializer list. E.g. 
	 * unordered_buffer<int,int> a = {{1,2}, {3,4}}
	 *
	 * @param il
	 *
	 * @return 
	 */
	unordered_buffer& operator=(std::initializer_list<std::pair<Key,T>>& il)
	{
		clear();
		// now emplace the data
		for(auto it=il.begin(); it!=il.end(); it++) {
			emplace(it.first, it.second);
		}
	};

	// destructor
	~unordered_buffer() {} ;

	/**************************************** 
	 * Const Information Functions 
	 ****************************************/
	
	/**
	 * @brief Does the buffer have any elements?
	 *
	 * @return whether the buffer is empty
	 */
	bool empty() const
	{
		return m_data.empty();
	};


	/**
	 * @brief Number of buffer elements stored, note that this will differ
	 * from the number of insertions, since insertions are probabilistic.
	 *
	 * @return Number of elements currently stored, this only goes up under
	 * normal circumstances.
	 */
	size_t size() const
	{
		return m_used.size();
	};

	
	/**
	 * @brief Get number of buckets.
	 *
	 * @return number of buckets
	 */
	size_t max_size() const
	{
		return m_data.size();
	};
	

	/**
	 * @brief Get Number of buckets.
	 *
	 * @return number of buckets
	 */
	size_t bucket_count() const 
	{
		return m_data.size();
	};

	/**************************************************************************
	 * Overall Settings/Changes
	 *************************************************************************/

	/**
	 * @brief Completely clears the buffer.
	 */
	void clear()
	{
		m_used.clear();

		// set used variable to false
		for(size_t ii=0; ii<m_data.size(); ii++) {
			m_data[ii].priority = 0;
		}
	};

	/**
	 * @brief Resize the hash table data structure to N buckets, and rehash 
	 * all the current elements.
	 *
	 * @param N
	 */
	void rehash(size_t N)
	{
		std::vector<Element> newdata(N);
		std::list<Element*> newused(N);

		for(auto it=m_used.begin(); it!=m_used.end(); it++) {
			size_t newbucket = m_hasher(std::get<0>(it->value))%N;
			newdata[newbucket].priority = it->priority;
			newdata[newbucket].value = std::move(it->value);

			newused.push_front(&newdata[newbucket]);
			newdata[newbucket].pos = newused.begin();
		}
		
		m_data = std::move(newdata);
		m_used = std::move(newused);
	};


	/**
	 * @brief if N is more than the current number of buckets, then rehash
	 * otherwise do nothing.
	 *
	 * @param N	Minimum number of buckets to be using.
	 */
	void reserve(size_t N)
	{
		if(N > m_data.size())
			rehash(N);
	};

	/**************************************************************************
	 * deletions
	 *************************************************************************/
	
	/**
	 * @brief Erase a bucket from the datastructure
	 *
	 * @param pos Position to erase. 
	 *
	 * @return Iterator pointing to the next position after the removed one
	 */
	iterator erase(iterator pos)
	{
		(*(pos.it))->priority = 0;
		pos.it = m_used.erase(pos.it);
		return pos;
	};
	
	/**
	 * @brief Erase a range of buckets from the datastructure.
	 *
	 * @param first Position of first element to erase
	 * @param last	Position of one past last element being erased.
	 *
	 * @return last
	 */
	iterator erase(iterator first, iterator last)
	{
		while(first.it != m_used.end() && first.it != last.it) {
			(*(first.it))->priority = 0;
			first.it = m_used.erase(first.it);
		}
		return first;
	};
	

	/**
	 * @brief Key-based erase. If a key/value pair matches then erase it. 
	 *
	 * @param key	Key to find and erase
	 *
	 * @return 		1 if erased, 0 otherwise
	 */
	size_t erase(const Key& key)
	{
		auto& data = m_data[bucket(key)];

		// if not found, just return 0
		if(data.priority <= 0)
			return 0;
		
		erase(data.pos);
		return 1;
	};


	/**************************************************************************
	 * insertions, these all trigger change in priority in the case of a hit
	 *************************************************************************/

	//////////////////////
	// Copy Insert
	//////////////////////
	
	/**
	 * @brief insert an element probabilistically, makes a copy of the input 
	 * values if the insertion is successful. The probability of insertion is
	 * 1 if no collision occurs, and exponentially decays with the number of
	 * re-uses of the given key. This will also decrease the 'hit' count of
	 * the given key so that if this value were repeated more than the 
	 * colliding key, it will eventually replace that one.
	 *
	 * @param value
	 *
	 * @return Iterator, insertion occured (bool) pair.
	 */
	std::pair<iterator, bool> insert(const std::pair<Key, T>& value)
	{

		auto& data = m_data[bucket(value.first)];

		/************************************
		 * Miss
		 ************************************/
		// if not yet used, set to used and copy key
		if(data.priority <= 0) {

			// set bin to used
			data.priority = 1;

			// copy into bin
			std::get<0>(data.value) = value.first;
			std::get<1>(data.value) = value.second;

			// add to list of used bins
			m_used.push_front(&data);
			data.pos = m_used.begin();
			
			return std::make_pair<iterator, bool>(m_used.begin(), true);
		} 
		/************************************
		 * Hit
		 ************************************/
		else if(std::get<0>(data.value) == value.first) {
			/*
			 * keys are equal, increase priority
			 */
			data.priority++;
			return std::make_pair<iterator, bool>(data.pos, false);
		} else {
			/*
			 * keys are different, probabilistically replace 
			 */
			
			// the higher the probability, the lower the odds of replacement
			if(m_rdist(m_rng) < pow(2,-data.priority)) {
				std::get<0>(data.value) = value.first;
				std::get<1>(data.value) = value.second;
				data.priority = 1;
				
				// return new 
				return std::make_pair<iterator, bool>(data.pos, true);
			} else {
				// return old
				return std::make_pair<iterator, bool>(data.pos, false);
			}
		}
	};
	
	/**
	 * @brief Insert an element probabilistically, identical to normal insertion,
	 * hints are not used.
	 *
	 * @param hint
	 * @param value
	 *
	 * @return 
	 */
	std::pair<iterator, bool> insert(const_iterator hint, 
			const std::pair<Key, T>& value)
	{
		// ignore the hint
		(void)(hint);
		return insert(value);
	};
	
	////////////////////////////
	// Move Insert 
	////////////////////////////
	
	/**
	 * @brief Identical to normal emplace, hints are not used.
	 *
	 * @param it
	 * @param key
	 * @param value
	 *
	 * @return 
	 */
	std::pair<iterator, bool> emplace_hint(const_iterator it, Key&& key, T&& value)
	{
		return emplace(key, value);
	};

	/**
	 * @brief Identical to insertion, but the key and value are left in an 
	 * inderminant state. Equivalent to moving the key and value
	 *
	 * @param key
	 * @param value
	 *
	 * @return 
	 */
	std::pair<iterator, bool> emplace(Key&& key, T&& value)
	{
		auto& data = m_data[bucket(key)];

		/************************************
		 * Miss
		 ************************************/
		// if not yet used, set to used and copy key
		if(data.priority <= 0) {

			// set bin to used
			data.priority = 1;

			// copy into bin
			std::get<0>(data.value) = key;
			std::get<1>(data.value) = value;

			// add to list of used bins
			m_used.push_front(&data);
			data.pos = m_used.begin();

			return std::make_pair<iterator, bool>(m_used.begin(), true);
		} 
		/************************************
		 * Hit
		 ************************************/
		else if(std::get<0>(data.value) == key) {
			/*
			 * keys are equal, increase priority
			 */
			data.priority++;
			return std::make_pair<iterator, bool>(data.pos, false);
		} else {
			/*
			 * keys are different, probabilistically replace 
			 */
			
			// the higher the probability, the lower the odds of replacement
			if(m_rdist(m_rng) < pow(2,-data.priority)) {
				std::get<0>(data.value) = key;
				std::get<1>(data.value) = value;
				data.priority = 1;
				
				// return new 
				return std::make_pair<iterator, bool>(data.pos, true);
			} else {
				// return old
				return std::make_pair<iterator, bool>(data.pos, false);
			}
		}
	};
	

	/**
	 * @brief Key/Value pair insertion. The pair is moved however leaving
	 * the given key and values in inderminant state. Good if you have created
	 * r-values for insertion
	 *
	 * @param value	Pair of Key/T to insert
	 *
	 * @return iterator of inserted value (or old value), bool indicating whether
	 * insertion occurred.
	 */
	std::pair<iterator, bool> insert(std::pair<Key, T>&& value)
	{
		auto& data = m_data[bucket(value.first)];
		
		/************************************
		 * Miss
		 ************************************/
		// if not yet used, set to used and copy key
		if(data.priority <= 0) {

			// set bin to used
			data.priority = 1;

			// copy into bin
			std::get<0>(data.value) = std::move(value.first);
			std::get<1>(data.value) = std::move(value.second);

			// add to list of used bins
			m_used.push_front(&data);
			data.pos = this->begin();

			return std::make_pair<iterator, bool>(this->begin(), true);
		} 
		/************************************
		 * Hit
		 ************************************/
		else if(std::get<0>(data.value) == value.first) {
			/*
			 * keys are equal, increase priority
			 */
			data.priority++;
			return std::make_pair(data.pos, false);
		} else {
			/*
			 * keys are different, probabilistically replace 
			 */
			
			// the higher the probability, the lower the odds of replacement
			if(m_rdist(m_rng) < pow(2,-data.priority)) {
				std::get<0>(data.value) = std::move(value.first);
				std::get<1>(data.value) = std::move(value.second);
				data.priority = 1;
				
				// return new 
				return std::make_pair(data.pos, true);
			} else {
				// return old
				return std::make_pair(data.pos, false);
			}
		}
	};
	

	/**
	 * @brief Identical to normal insertion, hint is unused.
	 *
	 * @param hint
	 * @param value
	 *
	 * @return 
	 */
	std::pair<iterator, bool> insert(const_iterator hint, 
			std::pair<Key, T>&& value)
	{
		(void)(hint);
		return insert(value);
	};
	

	/**
	 * @brief Multiple insertion, all values are emplaced.
	 *
	 * @tparam InputIterator	Iterator of pairs
	 * @param first	First value of pairs to insert
	 * @param last	One after last pair to insert
	 */
	template <class InputIterator>
	void insert(InputIterator first, InputIterator last)
	{
		for(auto it=first; it!=last; it++) {
			emplace(it.first, it.second);
		}
	};
	
	/**
	 * @brief Multiple insertion using initializer list.
	 * all values are emplaced.
	 *
	 * @param il array of values to insert
	 */
	void insert(std::initializer_list<std::pair<Key,T>> il) 
	{
		for(auto it=il.begin(); it!=il.end(); it++) {
			emplace(it.first, it.second);
		}
	};
	

	/**
	 * @brief Operator to get the current value, or insert a new value. If 
	 * the given value is a miss, this will create a new key/value pair and 
	 * return it, if it is a hit (same key already exists) then the hit count
	 * (priority) is incremented. If it is a collision, then a die is cast
	 * to determine which key wins out. Thus there will be a probabilistic 
	 * insertion.
	 *
	 * @param key Key to lookup, and insert/find
	 *
	 * @return Value matching given key
	 */
	T& operator[](const Key& key)
	{
		auto& data = m_data[bucket(key)];

		/************************************
		 * Miss
		 ************************************/
		// if not yet used, set to used and copy key
		if(data.priority <= 0) {

			// set bin to used
			data.priority = 1;

			// copy into bin
			std::get<0>(data.value) = key;
			std::get<1>(data.value) = T();

			// add to list of used bins
			m_used.push_front(&data);
			data.pos = m_used.begin();

			return std::get<1>(data.value);
		} 
		/************************************
		 * Hit
		 ************************************/
		else if(std::get<0>(data.value) == key) {
			/*
			 * keys are equal, increase priority
			 */
			data.priority++;
			return std::get<1>(data.value);
		} else {
			/*
			 * keys are different, probabilistically replace 
			 */
			
			// the higher the probability, the lower the odds of replacement
			if(m_rdist(m_rng) < pow(2,-data.priority)) {
				std::get<0>(data.value) = key;
				std::get<1>(data.value) = T();
				data.priority = 1;
				
				// return new 
				return std::get<1>(data.value);
			} else {
				// return old
				return std::get<1>(data.value);
			}
		}
	};
	
	/**
	 * @brief Operator to get the current value, or insert a new value. If 
	 * the given value is a miss, this will create a new key/value pair and 
	 * return it, if it is a hit (same key already exists) then the hit count
	 * (priority) is incremented. If it is a collision, then a die is cast
	 * to determine which key wins out. Thus there will be a probabilistic 
	 * insertion. In this case the Key will be destroyed.
	 *
	 * @param key Key to lookup, and insert/find
	 *
	 * @return Value matching given key
	 */
	T& operator[](Key&& key)
	{
		auto& data = m_data[bucket(key)];

		/************************************
		 * Miss
		 ************************************/
		// if not yet used, set to used and copy key
		if(data.priority <= 0) {

			// set bin to used
			data.priority = 1;

			// copy into bin
			std::get<0>(data.value) = std::move(key);
			std::get<1>(data.value) = T();

			// add to list of used bins
			m_used.push_front(&data);
			data.pos = m_used.begin();

			return std::get<1>(data.value);
		} 
		/************************************
		 * Hit
		 ************************************/
		else if(std::get<0>(data.value) == key) {
			/*
			 * keys are equal, increase priority
			 */
			data.priority++;
			return std::get<1>(data.value);
		} else {
			/*
			 * keys are different, probabilistically replace 
			 */
			
			// the higher the probability, the lower the odds of replacement
			if(m_rdist(m_rng) < pow(2,-data.priority)) {
				std::get<0>(data.value) = key;
				std::get<1>(data.value) = T();
				data.priority = 1;
				
				// return new 
				return std::get<1>(data.value);
			} else {
				// return old
				return std::get<1>(data.value);
			}
		}
	};

	/**************************************************************************
	 * Accessors that do not trigger change in priority
	 *************************************************************************/
	
	/**
	 * @brief Find a given key. Note that this is difference from insert/[] 
	 * operator in that the priority will not be adjusted. It is unlikely that
	 * you want this function in most buffer use cases.
	 *
	 * @param key	Key to search fort
	 *
	 * @return 		Iterator of located key
	 */
	iterator find(const Key& key)
	{
		auto& data = m_data[bucket(key)];
		
		/************************************
		 * Miss
		 ************************************/
		// if not yet used, set to used and copy key
		if(data.priority <= 0) {
			return this->end();
		} 
		/************************************
		 * Hit
		 ************************************/
		else if(std::get<0>(data.value) == key) {
			/*
			 * keys are equal, 
			 */
			return data.pos;
		} else {
			/*
			 * keys are different, 
			 */
			return this->end();
		}
	};
	
	/**
	 * @brief Find a given key. Note that this is difference from insert/[] 
	 * operator in that the priority will not be adjusted. It is unlikely that
	 * you want this function in most buffer use cases.
	 *
	 * @param key	Key to search fort
	 *
	 * @return 		Constant iterator of located key
	 */
	const_iterator find(const Key& key) const
	{
		auto& data = m_data[bucket(key)];
		
		/************************************
		 * Miss
		 ************************************/
		// if not yet used, set to used and copy key
		if(data.priority <= 0) {
			return this->cend();
		} 
		/************************************
		 * Hit
		 ************************************/
		else if(std::get<0>(data.value) == key) {
			/*
			 * keys are equal, 
			 */
			return data.pos;
		} else {
			/*
			 * keys are different, 
			 */
			return this->cend();
		}
	};


//	/**
//	 * @brief Like the [] operator, but won't adjust the priority or create a
//	 * new value if none exists. May throw out of range exception if key isn't 
//	 * found.
//	 *
//	 * @param key	Key to find, 
//	 *
//	 * @return Value from key/value pair
//	 */
//	T& at(const Key& key) 
//	{
//		auto& data = m_data[bucket(key)];
//
//		/************************************
//		 * Miss
//		 ************************************/
//		// if not yet used, set to used and copy key
//		if(data.priority <= 0) {
//			throw std::out_of_range("Key Not Found");
//			return T();
//		} 
//		/************************************
//		 * Bin Hit
//		 ************************************/
//		else if(std::get<0>(data.value) == key) {
//			/* keys are equal, Hit */
//			return data.value.second;
//		} else {
//		/************************************
//		 * Key Miss / Bin Hit
//		 ************************************/
//			/* keys are different, Miss */
//			throw std::out_of_range("Key Not Found");
//			return T();
//		}
//	};
	
	/**
	 * @brief Like the [] operator, but won't adjust the priority or create a
	 * new value if none exists. May throw out of range exception if key isn't 
	 * found.
	 *
	 * @param key	Key to find, 
	 *
	 * @return Value Constant value from key/value pair
	 */
	const T& at(const Key& key) const
	{
		auto& data = m_data[bucket(key)];

		/************************************
		 * Miss
		 ************************************/
		// if not yet used, set to used and copy key
		if(data.priority <= 0) {
			throw std::out_of_range("Key Not Found");
			return T();
		} 
		/************************************
		 * Bin Hit
		 ************************************/
		else if(std::get<0>(data.value) == key) {
			/* keys are equal, Hit */
			return data.value.second;
		} else {
		/************************************
		 * Key Miss / Bin Hit
		 ************************************/
			/* keys are different, Miss */
			throw std::out_of_range("Key Not Found");
			return T();
		}
	};

	/**
	 * @brief Which bucket a particular key is in, not very useful to the end
	 * user I don't believe.
	 *
	 * @param key	Key to search for
	 *
	 * @return 		Int indicating a bucket.
	 */
	size_t bucket(const Key& key)
	{
		return m_hasher(key)%m_data.size();
	};
	

	/**
	 * @brief Number of elements with matching key. Because only a single key
	 * can exist, this will return either 0 or 1, 1 indicating that the value
	 * exists in the data structure.
	 *
	 * @param key	Key to search for.
	 *
	 * @return 		0 if key not found, 1 if found
	 */
	size_t count(const Key& key) const
	{
		auto& data = m_data[bucket(key)];

		/************************************
		 * Miss
		 ************************************/
		// if not yet used, set to used and copy key
		if(data.priority <= 0) {
			/* Miss */
			return 0;
		} 
		/************************************
		 * Bin Hit
		 ************************************/
		else if(std::get<0>(data.value) == key) {
			/* keys are equal, Hit */
			return 1;
		} else {
		/************************************
		 * Key Miss / Bin Hit
		 ************************************/
			/* keys are different, Miss */
			return 0;
		}
	};

	/**
	 * @brief Since value can't be repeated this will always return either
	 * an end or two identical iterators. Use find, this is just to conform
	 * to other possible containers.
	 *
	 * @param key	Key to search for.
	 *
	 * @return 		pair of identical iterators
	 */
	std::pair<iterator,iterator> equal_range(const Key& key)
	{
		auto& data = m_data[bucket(key)];

		/************************************
		 * Miss
		 ************************************/
		// if not yet used, set to used and copy key
		if(data.priority <= 0) {
			return std::make_pair(end(), end());
		} 
		/************************************
		 * Bin Hit
		 ************************************/
		else if(std::get<0>(data.value) == key) {
			/* keys are equal, Hit */
			return std::make_pair(data.pos, data.pos);
		} else {
		/************************************
		 * Key Miss / Bin Hit
		 ************************************/
			/* keys are different, Miss */
			return std::make_pair(end(), end());
		}
	}

	/**
	 * @brief Since value can't be repeated this will always return either
	 * an end or two identical iterators. Use find, this is just to conform
	 * to other possible containers.
	 *
	 * @param key	Key to search for.
	 *
	 * @return 		pair of identical iterators
	 */
	std::pair<const_iterator,const_iterator> equal_range(const Key& key) const
	{
		auto& data = m_data[bucket(key)];

		/************************************
		 * Miss
		 ************************************/
		// if not yet used, set to used and copy key
		if(data.priority <= 0) {
			return std::make_pair(cend(), cend());
		} 
		/************************************
		 * Bin Hit
		 ************************************/
		else if(std::get<0>(data.value) == key) {
			/* keys are equal, Hit */
			auto tmp = (const_iterator)data.pos;
			return std::make_pair(tmp, tmp);
		} else {
		/************************************
		 * Key Miss / Bin Hit
		 ************************************/
			/* keys are different, Miss */
			return std::make_pair(cend(), cend());
		}
	}
};

#include "unordered_buffer.hpp"
#endif //UNORDERED_BUFFER_H
