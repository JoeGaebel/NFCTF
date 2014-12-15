#include "SharedObject.h"
#include "Semaphore.h"
#include "thread.h"
#include "socketserver.h"
#include <stdlib.h>
#include <time.h>
#include <list>
#include <pthread.h>
#include <vector>
#include "game.h"


class PlayerThread : public Thread
{
private:
    Socket theSocket;
    Game * game;
public:
    string playerName;
    int playerID;
    bool hasFlag;
    int points;
    PlayerThread(Socket const & p, int pID, Game * g)
        : Thread(true),theSocket(p)
    { 
        playerID = pID;
        game = g;
    }
    long ThreadMain(void)
    {
        ByteArray bytes;
        cout << "Created a socket thread!" << endl;
        for(;;) //Reading and writing to socket
        {
            int read = theSocket.Read(bytes);
            if (read == -1)
            {
                cout << "Error in socket detected" << endl;
                break;
            }
            else if (read == 0)
            {
                cout << "Socket closed at remote end" << endl;
                break;
            }
            else
            {
                string theString = bytes.ToString();
                cout << "Received: " << theString << endl;

                ///Assign name to Player on name: command
                if(theString.find("name:") == 0) {
                    theString.replace(0,5,"");
                    playerName = theString;
                }
                if(theString.find("stole")){
                    game->setFlag(playerID);
                 }
                // bytes.v[0]='R';
                // theSocket.Write(bytes);
                
                
            }
        }
        cout << "Thread is gracefully ending" << endl;
    }
    ~PlayerThread(void)
    {
        theSocket.Write(ByteArray("done"));
        terminationEvent.Wait();
        theSocket.Close();
    }
};


class ControlThread : public Thread
{
private:
    vector<PlayerThread> * players;
public:
    ControlThread(vector<PlayerThread> * p)
        : Thread(true)
    { 
       players = p;
    }
    long ThreadMain(void)
    {
    
    }
    ~ControlThread(void)
    {

    }
};