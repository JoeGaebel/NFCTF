#include "SharedObject.h"
#include "Semaphore.h"
#include "thread.h"
#include "socketserver.h"
#include <stdlib.h>
#include <time.h>
#include <list>
#include <pthread.h>
#include <vector>
#include <sstream>


using namespace std;

class Game;

class PlayerThread : public Thread
{
private:
    Socket theSocket;
public:
    string playerName;
    int playerID;
    bool hasFlag;
    int points;
    Game * game;

    PlayerThread(Socket const & p, int pID, Game *g)
    : Thread(true),theSocket(p)
    { 
        playerID = pID;
        game = g;
        points = 0;
        hasFlag = false;
        playerName = "";
        //BETATODO: Make player name required during socket connection
    }

    void pushScore(){ //Used to send the updated score information to client
       stringstream ss;
       ss << points;
       string pointString;
       ss >> pointString;
       string toSend = "score:"+pointString;
       ByteArray ba(toSend);
       int written = theSocket.Write(ba);
       if ( written != ba.v.size())
       {
         cout << "Problem sending Score info, Wrote: " << written << endl;
         cout << "The socket appears to have been closed suddenly" << endl;
     }

 }  

    void pushGamePaused(){ //Used to send the updated score information to client
       string toSend = "paused";
       ByteArray ba(toSend);
       int written = theSocket.Write(ba);
   }  


    void pushFlagHolder(string name){//Used to tell client who has the flag
       // cout <<"I'm about to send flag info"<<endl;
        ByteArray ba("flag:"+name);
        int written = theSocket.Write(ba);
        if ( written != ba.v.size())
        {  
         cout << "Problem sending Flag info, Wrote: " << written << endl;
         cout << "The socket appears to have been closed suddenly" << endl;
     }

 }

 long ThreadMain(void);


 ~PlayerThread(void)
 {
    cout <<playerName<<" has left"<<endl;
    theSocket.Write(ByteArray("done"));
    theSocket.Close();
    terminationEvent.Wait();

}
};






class Game : public Thread
{
public:
    vector<PlayerThread*> players;
    int gameID;
    bool gameStarted;
    string flagHolderName;
    int flagHolderID; //ID of player with the flag

    Game(int gID)
    : Thread(true)
    { 
        gameID = gID;
        flagHolderID = -1; //No-one has flag yet
        flagHolderName = "";
        gameStarted = false;
    }

    void addPlayer(PlayerThread * player){
        players.push_back(player);
        if (players.size() == 2)
            gameStarted = true;
    }


    long ThreadMain(void) //Runs every two seconds to update scores
    {
        while(1){
            if (gameStarted){
        //Loop to increment points based on who has flag
                for (int i = 0; i< players.size(); i++){
                 if (players[i]->hasFlag) {
                  players[i]->points+=5;
                  players[i]->pushScore();
                   }

             }


             }
      Sleep(5000);
  }

}
~Game(void)
{

}

int findPlayerFromID(int playerID){
    for (int i = 0; i<players.size(); i++)
        if (players[i]->playerID == playerID)
            return i;
        throw string("Player Not Found");
    }

    void updateClientFlag(string name){
        for (int i=0; i<players.size(); i++){
            if (players[i]->playerName == name)
                 //Update the client who owns the flag, who has it? You do!
                players[i]->pushFlagHolder("you");

            else
                //Tell everyone else the name of who has the flag
                players[i]->pushFlagHolder(name);
        }


    }

    void updateClientPaused(){
        for (int i=0; i<players.size(); i++){
         players[i]->pushGamePaused();
     }


 }

 void killPlayer(int playerID){
    int index = findPlayerFromID(playerID);
    PlayerThread* toDelete = players[index];
    if (toDelete->hasFlag) {
        flagHolderID = -1; //No one has flag
        updateClientFlag("No-one");
    }
    sleep(1);
    players.erase(players.begin()+index);
    if (players.size() < 2){
        gameStarted = false;
        updateClientPaused();
    }

    delete toDelete;
}


void setFlag(int playerID){
    int index;
    //Remove flag from the current flag holder
        if(flagHolderID > -1) { //Has the flag been assigned yet?
          index = findPlayerFromID(flagHolderID);
          players[index]->hasFlag = false;
      }

    //Assign the flag to the specified player id
      index = findPlayerFromID(playerID);
      players[index]->hasFlag = true;
      flagHolderName = players[index]->playerName;
      flagHolderID = playerID;

    //Update the client's information on who has the flag
      updateClientFlag(flagHolderName);
  }



};


//Must be outside in order to compile
long PlayerThread::ThreadMain(void){
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
               // cout << "Received: " << theString << endl;

                ///Assign name to Player with name:Joe
                if(theString.find("name:") == 0) {
                    theString.replace(0,5,"");
                    playerName = theString;
                    cout <<"PlayerID: "<<playerID<<" now has the name: "<<playerName<<endl;

                }
                //Assign name to 
                if(theString.find("steal") == 0){
                  game->setFlag(playerID);
                  cout <<"Game"<<game->gameID<<": "<<playerName<<" has stolen the flag!"<<endl;
              }

              if(theString.find("done") == 0){
                theSocket.Write(ByteArray("done"));
                game->killPlayer(playerID);
                cout <<playerName<<" has left the game!"<<endl;
            }
                // bytes.v[0]='R';
                // theSocket.Write(bytes);


        }
    }
    cout << "Thread is gracefully ending" << endl;
}




int main(void)
{
    cout << "I am a socket server.  Type 'done' to exit" << endl;
    SocketServer theServer(2000);
    vector<PlayerThread *> threads;
    int g=0; //Number of games -> Used for gameID
    vector<Game*> games;
    int playerCount = 0; //Number of players -> Used for playerID
    for(;;)
    {
        try
        {
            FlexWait waiter(2,&theServer,&cinWatcher);
            Blockable * result = waiter.Wait();
            if (result == &cinWatcher)
            {
                string s;
                cin >> s;
                if (s=="done")
                {   
                    for(int i=0;i<games.size();i++)
                        games[i]->gameStarted = false;
                    // No need to call SocketServer::Shutdown.  It isn't active.
                    cout << "Should be dieing"<<endl;

                    break;
                }
                else
                    continue;
            }
            // Accept should not now block.
            Socket newSocket = theServer.Accept();
            cout << "Received a socket connection!" << endl;
            if (games.size() <= playerCount/3) { //3 players per game
                games.push_back(new Game(g++));
                cout <<"Game "<<g-1<<" created!"<<endl;
            }
            threads.push_back(new PlayerThread(newSocket, playerCount++, games.back()));
            games.back()->addPlayer(threads.back());
        }
        catch(TerminationException e)
        {
            cout << "The socket server is no longer listening. Exiting now." << endl;
            break;
        }
        catch(string s)
        {
            cout << "thrown " << s << endl;
            break;
        }
        catch(...)
        {
            cout << "caught  unknown exception" << endl;
            break;
        }
    }
    cout<<"About to delete threads"<<endl;
    for (int i=0;i<threads.size();i++){
        threads[i]->Stop();
        delete threads[i];
    }

    for (int i=0;i<games.size();i++){
        delete games[i];
    }

    cout << "Sleep now" << endl;
    sleep(1);
    cout << "End of main" << endl;

}