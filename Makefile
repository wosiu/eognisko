all : tmp server

CXXFLAGS = -ggdb --std=c++11 -lboost_system -lpthread -lboost_program_options

tmp :
	g++ -std=c++11 -Wall -ggdb tmp.cpp -o tmp

server : server_main.cpp tcpserver.o udpserver.o servercontroller.o clientcontext.o commons.o
	g++ server_main.cpp tcpserver.o udpserver.o servercontroller.o clientcontext.o commons.o -o server $(CXXFLAGS)

tcpserver.o : TcpServer.cpp TcpServer.hpp servercontroller.o commons.o
	g++ -c TcpServer.cpp -o tcpserver.o $(CXXFLAGS) 

servercontroller.o : ServerController.cpp ServerController.hpp clientcontext.o commons.o
	g++ -c ServerController.cpp -o servercontroller.o  $(CXXFLAGS)

clientcontext.o : ClientContext.cpp ClientContext.hpp commons.o
	g++ -c ClientContext.cpp -o clientcontext.o $(CXXFLAGS) 

commons.o : commons.hpp commons.cpp
	g++ -c commons.cpp $(CXXFLAGS) 

mixer.o : mixer.hpp mixer.cpp
	g++ -c mixer.cpp -o mixer.o $(CXXFLAGS) 

udpserver.o : UdpServer.cpp UdpServer.hpp servercontroller.o commons.o mixer.o
	g++ -c UdpServer.cpp -o udpserver.o $(CXXFLAGS) 

clean:
	rm -f *.o