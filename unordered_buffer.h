#include <chrono>
#include <random>
#include <vector>
#include <list>
#include <tuple>

template <class Key, class T, class Hash = std::hash<Key>>
class unordered_buffer
{
public:
//	typedef typename std::list<std::tuple<size_t, Key, T>*>::iterator iterator;
//	typedef typename std::list<std::tuple<size_t, Key, T>*>::const_iterator const_iterator;

	//constructor
	unordered_buffer(size_t size);

	// destructor
	~unordered_buffer();

	// capacity
	bool empty() const;
	size_t size() const;
	size_t max_size() const;

	// modifiers
	void clear();

	std::pair<iterator, bool> insert(const std::pair<Key, T>& value, bool prob = true);
	std::pair<iterator, bool> insert(std::pair<Key, T>&& value, bool prob = true);

private:
	
	struct Element
	{
		int priority;					//
		std::list<std::pair<Key, T>*> pos; 	//position in used list
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
	const std::uniform_real_distribution<double> m_rdist;

	const int MAX_PRIORITY = 1000;
};

// constructor 
template <class Key, class T, class Hash>
unordered_buffer<Key,T,Hash>::unordered_buffer(size_t size) :
	m_rng(std::chrono::system_clock::now().time_since_epoch().count()),  
	m_rdist(0,1)
{
	m_data.resize(size);
	
	// set used variable to false
	for(size_t ii=0; ii<m_data.size(); ii++) {
		m_data[ii].priority = 0;
	}

	m_used.clear();
}

// destructor
template <class Key, class T, class Hash>
unordered_buffer<Key,T,Hash>::~unordered_buffer()
{
}

//////////////////////////////
// capacity
//////////////////////////////

// return true if there are no members
template <class Key, class T, class Hash>
bool unordered_buffer<Key,T,Hash>::empty() const
{
	return m_data.empty();
}

// return number of elements
template <class Key, class T, class Hash>
size_t unordered_buffer<Key,T,Hash>::size() const
{
	return m_used.size();
}

// return maximum number of elements
template <class Key, class T, class Hash>
size_t unordered_buffer<Key,T,Hash>::max_size() const
{
	return m_data.size();
}

// remove all the elements
template <class Key, class T, class Hash>
void unordered_buffer<Key,T,Hash>::clear()
{
	m_used.clear();
	
	// set used variable to false
	for(size_t ii=0; ii<m_data.size(); ii++) {
		m_data[ii].priority = 0;
	}
}

// add/access element
template <class Key, class T, class Hash>
std::pair<typename unordered_buffer<Key,T,Hash>::iterator, bool> 
unordered_buffer<Key,T,Hash>::insert(const std::pair<Key, T>& value, bool prob)
{
	size_t hash = m_hasher(value.first);


	// if not yet used, set to used and copy key
	if(m_data[hash%m_data.size()].priority <= 0) {

		// set bin to used
		m_data[hash%m_data.size()].priority = 1;

		
		// copy into bin
		std::get<1>(m_data[hash%m_data.size()].value) = value.first;
		std::get<2>(m_data[hash%m_data.size()].value) = value.second;

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
}

// add/access element
template <class Key, class T, class Hash>
std::pair<typename unordered_buffer<Key,T,Hash>::iterator, bool> 
unordered_buffer<Key,T,Hash>::insert(std::pair<Key, T>&& value, bool prob)
{
	size_t hash = m_hasher(value.first);

	// if not yet used, set to used and copy key
	if(m_data[hash%m_data.size()].priority <= 0) {

		// set bin to used
		m_data[hash%m_data.size()].priority = 1;

		
		// copy into bin
		std::get<1>(m_data[hash%m_data.size()].value) = std::move(value.first);
		std::get<2>(m_data[hash%m_data.size()].value) = std::move(value.second);

		// add to list of used bins
		m_used.push_front(&m_data[hash%m_data.size()]);
		m_data[hash%m_data.size()].pos = m_used.begin();

		return std::make_pair<iterator, bool>(m_used.begin(), true);
	}

	auto& data = m_data[hash%m_data.size()];

	// if the value exists, and keys are equal, increase priority
	if(std::get<0>(data.value) == value.first) {
		if(data.priority < MAX_PRIORITY)
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
			std::get<0>(data.value) = std::move(value.first);
			std::get<1>(data.value) = std::move(value.second);
		}
		return std::make_pair<iterator, bool>(data.pos, true);
	}
}

