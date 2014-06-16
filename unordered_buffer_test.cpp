#include <utility>
#include <unordered_map>
#include <iostream>
#include "unordered_buffer.h"

using std::cerr;
using std::endl;

int main()
{
	size_t OUTERCOUNT = 50;
	size_t INNNERCOUNT = 1000;

	std::unordered_map<int, double> truestore;
	
	const size_t NUM = 3;
	double KEYS[NUM] = {28, 9, 0};
	double VALUES[NUM] = {.32, 1.8, 23};
	
	unordered_buffer<int, double> buff(INNNERCOUNT+NUM);

	int key = 0;
	double value = 0;
	
	// test by reusing 3 values once every thousand, 
	// but the rest randomly, 2000 buffer size should
	// allow for a built up of use cases for the 3
	for(size_t jj=0 ; jj<OUTERCOUNT; jj++) {
		buff.loud = false;
		for(size_t ii=0 ; ii<INNNERCOUNT; ii++) {

			// use a ranodm key/value
			key = rand();
			value = rand()/(double)RAND_MAX;
			buff.insert(std::make_pair(key, value));
			truestore.insert(std::make_pair(key, value));
		}
			
		// use one of the 3
		buff.loud = true;
		for(size_t ii=0; ii<NUM; ii++) {
			key = KEYS[ii];
			value = VALUES[ii];
			
			auto bret = buff.insert(std::make_pair(key, value));
			auto tret = truestore.insert(std::make_pair(key, value));
			// return whether this was a hit or not
			if(tret.second) {
				if(bret.second) {
					// both inserted
					cerr << "b";
				} else {
					// true inserted only
					cerr << "t";
				}
			} else {
				if(bret.second) {
					// buffer inserted only
					cerr << "m";
				} else {
					// Neither Inserted
					cerr << "n";
				}
			}
		}
	}
	cerr << endl;

	for(size_t ii=0; ii<NUM; ii++) {
		key = KEYS[ii];
		value = VALUES[ii];

		auto bret = buff.insert(std::make_pair(key, value));
		if(!bret.second) {
			cerr << "Buffered: " << bret.first->first << ":" 
				<< bret.first->second << " vs " <<  key 
				<< ":" << value << endl;
		} else {
			// true inserted only
			cerr << "Unbuffered: " << key << ":" 
				<< value << endl;
		}
	}
	
}
