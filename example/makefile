# test:test.cpp
# 	g++ -o $@ $^ -std=c++11 -ljsoncpp
test:test.cpp bundle.cpp
	g++ -o $@ $^ -std=c++11 -lpthread

.PHONY:clean
clean:
	rm test