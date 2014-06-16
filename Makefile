
#FLAGS=-ggdb 
#FLAGS=-DNDEBUG -O3 -Wall
FLAGS=-Wall
CPP=clang++
DOX=doxygen

unordered_buffer_test: unordered_buffer_test.o
	${CPP} $< -o $@  -std=c++11 ${FLAGS}

unordered_buffer_test.o: unordered_buffer_test.cpp unordered_buffer.h
	${CPP} $< -o $@ -c -std=c++11 ${FLAGS}

doxygen:
	${DOX} dox.conf

clean:
	rm -fr unordered_buffer_test unordered_buffer_test.o html/ latex/
