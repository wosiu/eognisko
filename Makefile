all : tmp server

#CXXFLAGS = -ggdb -Wall --std=c++11 -lboost_system -lpthread -lboost_program_options -lboost_regex
CXXFLAGS = -O2 --std=c++11 -lboost_system -lpthread -lboost_program_options -lboost_regex

tmp :
	g++ tmp.cpp -o tmp $(CXXFLAGS) 

server : server_main.cpp tcpserver.o udpserver.o servercontroller.o clientcontext.o commons.o mixer.o datagramparser.o
	g++ server_main.cpp tcpserver.o udpserver.o servercontroller.o clientcontext.o commons.o mixer.o datagramparser.o -o server $(CXXFLAGS)

tcpserver.o : TcpServer.cpp TcpServer.hpp
	g++ -c TcpServer.cpp -o tcpserver.o $(CXXFLAGS) 

servercontroller.o : ServerController.cpp ServerController.hpp 
	g++ -c ServerController.cpp -o servercontroller.o  $(CXXFLAGS)

clientcontext.o : ClientContext.cpp ClientContext.hpp 
	g++ -c ClientContext.cpp -o clientcontext.o $(CXXFLAGS) 

commons.o : commons.hpp commons.cpp
	g++ -c commons.cpp $(CXXFLAGS) 

mixer.o : mixer.hpp mixer.cpp
	g++ -c mixer.cpp -o mixer.o $(CXXFLAGS) 

udpserver.o : UdpServer.cpp UdpServer.hpp
	g++ -c UdpServer.cpp -o udpserver.o $(CXXFLAGS) 

datagramparser.o : DatagramParser.cpp DatagramParser.hpp
	g++ -c DatagramParser.cpp -o datagramparser.o $(CXXFLAGS) 

clean:
	rm -f *.o
	rm server