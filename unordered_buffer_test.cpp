#include <utility>
#include <iostream>
#include "unordered_buffer.h"

using std::cerr;
using std::endl;

int main()
{
	unordered_buffer<int, double> buff(10);
	
	int which = 0;
	size_t NUM = 3;
	double KEYS[NUM] = {28, 9, 0};
	double VALUES[NUM] = {.32, 1.8, 23};

	int key = 0;
	double value = 0;
	
	// test by reusing 3 values regularly, but the rest randomly

	for(size_t ii=0 ; ii<1000; ii++) {
		// use one of the 3

		key = KEYS[which];
		value = VALUES[which];
		auto ret = buff.insert(std::make_pair(key, value));
		which = (which+1)%NUM;
		if(ret.second) {
			if(ret.first->first != key || ret.first->second != value) {
				cerr << "Error, mismatch of key or value" << endl;
				return -1;
			} 
			cerr << "@";
		} else {
			cerr << "x";
		}

		// use a ranodm key/value
		key = rand();
		value = rand()/(double)RAND_MAX;
		ret = buff.insert(std::make_pair(key, value));
		
		// return whether this was a hit or not
		if(ret.second) {
			cerr << "$";
		} else {
			cerr << "%";
		}
	}
}
