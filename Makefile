
unordered_buffer_test: unordered_buffer_test.o
	c++ $< -o $@  -std=c++11 -DNDEBUG

unordered_buffer_test.o: unordered_buffer_test.cpp unordered_buffer.h
	c++ $< -o $@ -c -std=c++11 -DNDEBUG

clean:
	rm unordered_buffer_test unordered_buffer_test.o
