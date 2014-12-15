Client : Client.o socket.o Blockable.o thread.o
	g++ -o Client Client.o socket.o thread.o Blockable.o -pthread 

Client.o : Client.cpp SharedObject.h Semaphore.h thread.h
	g++ -c Client.cpp 

Server : Server.o thread.o socket.o socketserver.o Blockable.o
	g++ -o Server Server.o thread.o socket.o socketserver.o Blockable.o -pthread

Blockable.o : Blockable.h Blockable.cpp
	g++ -c Blockable.cpp

Server.o : Server.cpp thread.h
	g++ -c Server.cpp

thread.o : thread.cpp thread.h
	g++ -c thread.cpp

socket.o : socket.cpp socket.h
	g++ -c socket.cpp

socketserver.o : socketserver.cpp socket.h socketserver.h
	g++ -c socketserver.cpp
