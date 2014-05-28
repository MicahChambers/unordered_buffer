#include <chrono>
#include <random>
#include <vector>
#include <list>
#include <tuple>
#include <ctime>

template <class Key, class T, class Hash = std::hash<Key>>
class unordered_buffer
{
public:

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
			return &(*(*it));
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

		typename std::list<std::pair<Key,T>*>::iterator it;
	};
	
	class const_iterator {
	public:
		const_iterator(){ };

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
		
		typename std::list<std::pair<Key,T>*>::const_iterator it;
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

	/* 
	 * Constructors
	 */

	//constructor
	unordered_buffer(size_t size) : m_rng(time(NULL)), m_rdist(0,1), m_hasher()
	{
		m_data.resize(size);

		// set used variable to false
		for(size_t ii=0; ii<m_data.size(); ii++) {
			m_data[ii].priority = 0;
		}

		m_used.clear();
	};

	// destructor
	~unordered_buffer() {} ;

	/* 
	 * Const Information Functions 
	 */
	
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

	/*
	 * modifiers
	 */
	void clear()
	{
		m_used.clear();

		// set used variable to false
		for(size_t ii=0; ii<m_data.size(); ii++) {
			m_data[ii].priority = 0;
		}
	};

	// insert an element probabilistically, makes a copy of the input values
	std::pair<iterator, bool> insert(const std::pair<Key, T>& value, bool prob = true)
	{
		size_t hash = m_hasher(value.first);


		// if not yet used, set to used and copy key
		if(m_data[hash%m_data.size()].priority <= 0) {

			// set bin to used
			m_data[hash%m_data.size()].priority = 1;


			// copy into bin
			std::get<0>(m_data[hash%m_data.size()].value) = value.first;
			std::get<1>(m_data[hash%m_data.size()].value) = value.second;

			// add to list of used bins
			m_used.push_front(&m_data[hash%m_data.size()]);
			m_data[hash%m_data.size()].pos = m_used.begin();

			return std::make_pair<iterator, bool>(m_used.begin(), true);
		}

		auto& data = m_data[hash%m_data.size()];

		// if the value exists, and keys are equal, increase priority
		if(std::get<0>(data.value) == value.first) {
			data.priority++;
			return std::make_pair<iterator, bool>(data.pos, true);
		} else {
			bool doinsert = true;
			if(prob) {
				// probabilistic insert
				doinsert = false; 

				// the higher the probability, the lower the odds of replacement
				if(m_rdist(m_rng) < exp(-data.priority))
					doinsert = true;
			}

			if(doinsert) {
				// change the key, reinitialize data
				std::get<0>(data.value) = value.first;
				std::get<1>(data.value) = value.second;
			}
			return std::make_pair<iterator, bool>(data.pos, true);
		}
	};

	// moves input pair
	std::pair<iterator, bool> insert(std::pair<Key, T>&& value, bool prob = true)
	{
		size_t hash = m_hasher(value.first);

		// if not yet used, set to used and copy key
		if(m_data[hash%m_data.size()].priority <= 0) {

			// set bin to used
			m_data[hash%m_data.size()].priority = 1;


			// copy into bin
			std::get<0>(m_data[hash%m_data.size()].value) = std::move(value.first);
			std::get<1>(m_data[hash%m_data.size()].value) = std::move(value.second);

			// add to list of used bins
			m_used.push_front(&m_data[hash%m_data.size()].value);
			m_data[hash%m_data.size()].pos = this->begin();

			return std::make_pair(this->begin(), true);
		}

		auto& data = m_data[hash%m_data.size()];

		// if the value exists, and keys are equal, increase priority
		if(std::get<0>(data.value) == value.first) {
			if(data.priority < MAX_PRIORITY)
				data.priority++;
			return std::make_pair(data.pos, true);
		} else {
			bool doinsert = true;
			if(prob) {
				// probabilistic insert
				doinsert = false; 

				// the higher the probability, the lower the odds of replacement
				if(m_rdist(m_rng) < pow(2,-data.priority))
					doinsert = true;
			}

			if(doinsert) {
				// change the key, reinitialize data
				std::get<0>(data.value) = std::move(value.first);
				std::get<1>(data.value) = std::move(value.second);
			}
			return std::make_pair(data.pos, true);
		}
	};

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
	std::list<std::pair<Key, T>*> m_used;

	// it = m_used.begin();
	// *it is a pair
	// it->first

	const Hash m_hasher;
	std::default_random_engine m_rng;
	std::uniform_real_distribution<double> m_rdist;

	const int MAX_PRIORITY = 1000;
};

