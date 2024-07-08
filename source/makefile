# cloud::cloud.cpp bundle.cpp
# 	g++ -o $@ $^ -std=c++11 -lstdc++fs -ljsoncpp

.PHONY:cloud
cloud::cloud.cpp
	g++ -o $@ $^ -L./lib -std=c++11 -lstdc++fs -ljsoncpp -lbundle -lpthread -g

.PHONY:clean
clean:
	rm cloud
	rm -rf backdir/*
	rm -rf packdir/*
	rm cloud.dat