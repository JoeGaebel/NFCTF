#ifndef THREAD_H
#define THREAD_H

#include <stdlib.h>
#include <time.h>
#include <list>
#include <iostream>
#include <string>
#include <pthread.h>
#include "Blockable.h"
void * ThreadFunction(void * me);
class Thread
{
    friend void * ThreadFunction(void * me);
private:
    pthread_t threadId;
    timespec creation;    
    ThreadSem resumeSemaphore;
    Event pauseEvent;
    Event stopEvent;
    int exitTimeout;

protected:
    Event terminationEvent;

private:
    void Finalize(void);
    long NowInUSec(void) const;
    Thread(Thread const &){}
    Thread & operator=(Thread const &){}
public:
    Thread(bool startOnCreation = false, int exitTimeout = 1000);
    virtual ~Thread();

    // Override this function in your threads
    virtual long ThreadMain(void) = 0;

    // Use these functions to control the thread from outside
    bool Start(void); // Start the thread
    void Stop(void);  // Kill the thread
    void Pause(void); // Make the thread stop temporarily
    void Resume(void); // Undo the effect of the previous Pause()
    bool IsRunning(void); // Is the thread currently active?

protected:
    // Your thread can use these to be polite with respect to Stop() and Pause()
    bool Sleep(int milliseconds);   // Thread::Sleep will return false if interrupted
    bool BePolite(void);            // Thread::BePolite is equivalent to Sleep(0)
};

#endif // THREAD_H
