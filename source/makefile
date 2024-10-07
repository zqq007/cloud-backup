# cloud::cloud.cpp bundle.cpp
# 	g++ -o $@ $^ -std=c++11 -lstdc++fs -ljsoncpp

.PHONY:all
all:cloud transfer

cloud:cloud.cpp
	g++ -o $@ $^ -L./lib -std=c++11 -lstdc++fs -ljsoncpp -lbundle -lpthread -g -lssl -lcrypto #-DCPPHTTPLIB_OPENSSL_SUPPORT
	
transfer:transfer.cpp
	g++ -o $@ $^ -lssl -lcrypto -DCPPHTTPLIB_OPENSSL_SUPPORT -ljsoncpp
#	g++ transfer.cpp -o transfer -lssl -lcrypto -DCPPHTTPLIB_OPENSSL_SUPPORT -ljsoncpp

.PHONY:clean
clean:
	rm cloud
	# rm -rf backdir/*
	# rm -rf packdir/*
	# rm cloud.dat