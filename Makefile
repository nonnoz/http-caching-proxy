all: proxy

proxy: proxy.cpp client.hpp client.cpp tcp.hpp
	g++ -g  -o proxy proxy.cpp client.cpp -lpthread

.PHONY:
	clean
clean:
	rm -rf *.o proxy
