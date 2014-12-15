#ifndef SOCKETSERVER_H
#define SOCKETSERVER_H
#include "socket.h"
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>

class SocketServer : public Blockable
{
private:
    int pipeFD[2];
    Event terminator;
    sockaddr_in socketDescriptor;
public:
    SocketServer(int port);
    ~SocketServer();
    Socket Accept(void);
    void Shutdown(void);
};

#endif // SOCKETSERVER_H
