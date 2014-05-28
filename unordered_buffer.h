#include <chrono>
#include <random>
#include <vector>
#include <list>
#include <tuple>
#include <ctime>

template <class Key, class T, class Hash = std::hash<Key>>
class unordered_buffer
{

/******************************************************************************
 *
 * Data
 *
 ******************************************************************************/
private:
	
	struct Element
	{
		int priority;
		typename unordered_buffer::iterator pos; 	//position in used list
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
	bool loud;
#endif //NDEBUG
	/* 
	 * Iterators
	 */
	class iterator {
	public:
		iterator(){ };

		iterator(const iterator& other){
			it = other.it;
		};
		
		std::pair<Key,T>& operator*(){
			return *(*it);
		};
		
		std::pair<Key,T>* operator->(){
			return &((*it)->value);
		};
		
		// movement
		std::pair<Key,T> operator++(int){
			iterator tmp = *this;
			++*this;
			return tmp;
		};
		
		std::pair<Key,T>& operator++(){
			it++;
			return *this;
		};
	
		std::pair<Key,T> operator--(int){
			iterator tmp = *this;
			--*this;
			return tmp;
		};
		
		std::pair<Key,T>& operator--(){
			it--;
			return *this;
		};

	private:
		friend class unordered_buffer<Key,T,Hash>;

		typename std::list<Element*>::iterator it;
	};
	
	class const_iterator {
	public:
		const_iterator(){ };
		
		const_iterator(const iterator& other){
			it = other.it;
		};

		const_iterator(const const_iterator& other){
			it = other.it;
		};
		
		const std::pair<Key,T>& operator*() const {
			return *(*it);
		};
		
		const std::pair<Key,T>* operator->(){
			return &(*(*it));
		};
		
		
		// movement
		std::pair<Key,T> operator++(int){
			iterator tmp = *this;
			++*this;
			return tmp;
		};
		
		const std::pair<Key,T>& operator++(){
			it++;
			return *this;
		};
		
		std::pair<Key,T> operator--(int){
			iterator tmp = *this;
			--*this;
			return tmp;
		};
		
		const std::pair<Key,T>& operator--(){
			--it;
			return *this;
		};

	private:
		friend class unordered_buffer<Key,T,Hash>;
		
		typename std::list<Element*>::const_iterator it;
	};

	// iterator functions
	iterator begin()
	{
		iterator tmp;
		tmp.it = m_used.begin();
		return tmp;
	};
	
	iterator end()
	{
		iterator tmp;
		tmp.it = m_used.end();
		return tmp; 
	};
	
	const_iterator cbegin()
	{
		const_iterator tmp;
		tmp.it = m_used.cbegin();
		return tmp;
	};
	
	const_iterator cend()
	{
		const_iterator tmp;
		tmp.it = m_used.cend();
		return tmp;
	};

	/**************************************************************************
	 * Constructors
	**************************************************************************/

	//basic constructor
	unordered_buffer(size_t size = 1024) : m_rng(time(NULL)), m_rdist(0,1), m_hasher()
	{
		m_data.resize(size);

		// set used variable to false
		for(size_t ii=0; ii<m_data.size(); ii++) {
			m_data[ii].priority = 0;
		}

		m_used.clear();
	};

	//range constructor
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
	
	//initializer list constructor
	template<class InputIterator>
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
	
	//copy constructor
	unordered_buffer(const unordered_buffer& ump) : m_rng(time(NULL)), m_rdist(0,1), m_hasher()
	{
		m_data = ump.m_data;
		m_used = ump.m_used;
	};
	
	//move constructor
	unordered_buffer(unordered_buffer&& ump) : m_rng(time(NULL)), m_rdist(0,1), m_hasher()
	{
		m_data = std::move(ump.m_data);
		m_used = std::move(ump.m_used);
	};

	// swap
	void swap(unordered_buffer& ump)
	{
		std::swap(ump.m_data, m_data);
		std::swap(ump.m_used, m_used);
	};

	// assignment
	unordered_buffer& operator=(const unordered_buffer& ump)
	{
		m_data = ump.m_data;
		m_used = ump.m_used;
	};
	
	unordered_buffer& operator=(unordered_buffer&& ump)
	{
		m_data = std::move(ump.m_data);
		m_used = std::move(ump.m_used);
	};
	
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
	// capacity
	bool empty() const
	{
		return m_data.empty();
	};

	// number of elements currently stored
	size_t size() const
	{
		return m_used.size();
	};

	// maximum number of elements
	size_t max_size() const
	{
		return m_data.size();
	};
	
	// maximum number of elements
	size_t bucket_count() const 
	{
		return m_data.size();
	};

	/**************************************************************************
	 * Overall Settings/Changes
	 *************************************************************************/
	void clear()
	{
		m_used.clear();

		// set used variable to false
		for(size_t ii=0; ii<m_data.size(); ii++) {
			m_data[ii].priority = 0;
		}
	};

	void rehash(size_t n)
	{
		std::vector<Element> newdata(n);
		std::list<Element*> newused(n);

		for(auto it=m_used.begin(); it!=m_used.end(); it++) {
			size_t newbucket = m_hasher(std::get<0>(it->value))%n;
			newdata[newbucket].priority = it->priority;
			newdata[newbucket].value = std::move(it->value);

			newused.push_front(&newdata[newbucket]);
			newdata[newbucket].pos = newused.begin();
		}
		
		m_data = std::move(newdata);
		m_used = std::move(newused);
	};

	void reserve(size_t n)
	{
		if(n > m_data.size())
			rehash(n);
		return 0;
	};

	/**************************************************************************
	 * deletions
	 *************************************************************************/
	iterator erase(const_iterator pos)
	{
		iterator out = pos;
		out->priority = 0;
		m_used.erase(out->pos.it);
		return out;
	};
	
	iterator erase(const_iterator first, const_iterator last)
	{
		iterator out = first;
		for(; out != cend() && out != last; ++out) {
			out->priority = 0;
			m_used.erase(out->pos.it);
		}
		return out;
	};
	
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
	
	// insert an element probabilistically, makes a copy of the input values
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
	
	// insert an element probabilistically, makes a copy of the input values
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
	
	// insert an element probabilistically, makes a copy of the input values
	std::pair<iterator, bool> emplace_hint(const_iterator it, Key&& key, T&& value)
	{
		return emplace(key, value);
	};

	// insert an element probabilistically, makes a copy of the input values
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
	
	std::pair<iterator, bool> insert(const_iterator hint, 
			std::pair<Key, T>&& value)
	{
		(void)(hint);
		return insert(value);
	};
	
	template <class InputIterator>
	void insert(InputIterator first, InputIterator last)
	{
		for(auto it=first; it!=last; it++) {
			emplace(it.first, it.second);
		}
	};
	
	void insert(std::initializer_list<std::pair<Key,T>> il) 
	{
		for(auto it=il.begin(); it!=il.end(); it++) {
			emplace(it.first, it.second);
		}
	};
	
	// bracket operator
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
	
	// bracket operator
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

	T& at(const Key& key)
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

	size_t bucket(const Key& key)
	{
		return m_hasher(key)%m_data.size();
	};
	
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

