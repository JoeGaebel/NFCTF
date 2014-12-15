#include "SharedObject.h"
#include "Semaphore.h"
#include "socket.h"
#include "thread.h"
#include <iostream>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
using namespace std;

class CommThread : public Thread
{
private:
    Socket theSocket;
public:
    bool shouldDie; //a hack but a small one
    CommThread(Socket const & p)
    : Thread(true),theSocket(p), shouldDie(false)
    {
        ;
    }
    long ThreadMain(void)
    {
        ByteArray bytes;

        for(;;)
        {
            ByteArray bytes;
            int read = theSocket.Read(bytes);
            if (read == -1)
            {
                std::cout << "Error in socket detected" << std::endl;
                break;
            }
            else if (read == 0)
            {
                std::cout << "Socket closed at remote end" << std::endl;
                break;
            }
            else
            {
                std::string theString = bytes.ToString();
                // cout <<"**********88888888888888888888**********"<<theString<<endl;
                if(theString.find("flag:") == 0){
                    theString.erase(0,5);
                    std::cout << theString << " has the flag!" << std::endl;
                }

                else if(theString.find("score:") == 0){
                    theString.erase(0,6);
                    std::cout << "Your score is now " << theString << std::endl;
                }

                else if(theString.find("paused") == 0){
                    std::cout << "Game is paused until more players join!"<<endl;
                }

                else if (theString.find("done") == 0){shouldDie = true; break;}

            }
        }
        std::cout << "Thread is gracefully ending" << std::endl;
    }
    ~CommThread(void)
    {   
        theSocket.Write(ByteArray("done"));
        terminationEvent.Wait();
        theSocket.Close();
    }
};




int main(void)
{
    try
    {
        Socket theSocket("127.0.0.1", 2000);
        theSocket.Open();
        std::string entry = "";

        //create printing thread
        CommThread *pointsThread = new CommThread(theSocket);

        //begin listening for input
        std::string input = "";
        int stage = 0;
        cout <<"Please enter your name: "<<endl;
        while (input != "done" && !pointsThread->shouldDie)
        {
            
            FlexWait waiter(2,&theSocket,&cinWatcher);
            Blockable * result = waiter.Wait();
            if (result == &cinWatcher){
            if(stage == 0){
                cin >> input;
                std::cout << "Name entered as: " << input << std::endl;
                ByteArray ba("name:" + input);
                theSocket.Write(ba);
                stage = 1;
                cout << "type steal to steal the flag"<<endl;
              }
            else if (stage == 1) {
            cout << "type steal to steal the flag"<<endl;
             cin >> input;
            }
            

            if (input == "steal") {
                std::cout << "stealing . . ." << std::endl;
                ByteArray ba("steal");
                theSocket.Write(ba);
            }

        }
    }

    std::cout << "Sleep now" << std::endl;
    delete pointsThread;


}
catch(std::string s)
{
    std::cout << s << std::endl;
}
catch(...)
{
    std::cout << "Caught unexpected exception" << std::endl;
}

}
