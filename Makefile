all : client server

#CXXFLAGS = -ggdb -Wall --std=c++11 -lboost_system -lpthread -lboost_program_options -lboost_regex
CXXFLAGS = -O2 --std=c++11 -lboost_system -lpthread -lboost_program_options -lboost_regex

OBJSERVER = server_main.o TcpServer.o UdpServer.o ServerController.o ClientContext.o commons.o mixer.o DatagramParser.o
OBJCLIENT = client_main.o Client.o  DatagramParser.o commons.o

%.o: %.cpp
	g++ -c $< $(CXXFLAGS)

server: $(OBJSERVER) 
	g++ $^ -o $@ $(CXXFLAGS)

client: $(OBJCLIENT)
	g++ $^ -o $@ $(CXXFLAGS)

sclean:
	rm -f $(OBJSERVER) server

cclean:
	rm -f $(OBJCLIENT) client

clean:
	rm -f *.o client server