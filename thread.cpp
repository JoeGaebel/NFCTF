#include "thread.h"
#include "unistd.h"
#include <iostream>

Thread::Thread(bool startOnCreation/*==false*/,int exitT/*=1000*/)
    : resumeSemaphore(0), pauseEvent(),stopEvent(), exitTimeout(exitT)
{
    // clock_gettime(CLOCK_REALTIME,&creation);
    if (startOnCreation)
        Start();
}

long Thread::NowInUSec(void) const
{
    timespec now;
    // clock_gettime(CLOCK_REALTIME,&now);
    return 100000*(now.tv_sec - creation.tv_sec)+(now.tv_nsec-creation.tv_nsec)/1000;
}

Thread::~Thread()
{
    Stop();
    FlexWait theEnd(1,&terminationEvent);
    if (theEnd.Wait(exitTimeout))
    {
        pthread_join(threadId,NULL);
    }
    else
        std::cout << "Th: Unable to properly terminate thread" << std::endl;
}

void * ThreadFunction(void * me)
{
    // The parameter was the "this" pointer, passed in Thread::Start.  It can
    // be safely cast back to Thread*
    Thread * theThread = (Thread*)me;
    void * returnValue = 0;

    try
    {
        void * returnValue = (void*) theThread->ThreadMain(); // Call a derived ThreadMain.
    }
    catch(TerminationException)
    {
        ;
    }
    theThread->terminationEvent.Trigger();
    return returnValue;
}

bool Thread::Start(void)
{
    // Create the thread through a system call.  Give the OS a pointer to this object
    bool error = pthread_create(&threadId,NULL,ThreadFunction,(void*)this);
    return error == 0;
}

void Thread::Stop(void)
{
    stopEvent.Trigger();
}

void Thread::Pause(void)
{
    pauseEvent.Trigger();
}

void Thread::Resume(void)
{
    resumeSemaphore.Signal();
}

bool Thread::BePolite(void)
{
    return Sleep(0);
}

bool Thread::Sleep(int milliseconds)
{
    FlexWait theSleeper(2,&stopEvent,&pauseEvent);
    Blockable * endCondition = theSleeper.Wait(milliseconds);

    // Sleep throws TerminationException if interrupted.  It should be caught by the derived class if
    // there is resource clean-up needed
    if (endCondition == &stopEvent)
        throw TerminationException(0);

    if (endCondition == &pauseEvent)
    {
        // Strictly speaking this is not real-time.  A really fast pause/resume
        // can short circuit a sleep as written.  To be fixed summer 2015.
        pauseEvent.Wait(); // Consume this pause
        resumeSemaphore.Wait(); // Wait for re-start
        return true;
    }

    return endCondition == 0;
}
