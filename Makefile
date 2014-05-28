
#FLAGS=-ggdb 
FLAGS=-DNDEBUG -O3 -Wall

unordered_buffer_test: unordered_buffer_test.o
	c++ $< -o $@  -std=c++11 ${FLAGS}

unordered_buffer_test.o: unordered_buffer_test.cpp unordered_buffer.h
	c++ $< -o $@ -c -std=c++11 ${FLAGS}

clean:
	rm unordered_buffer_test unordered_buffer_test.o
