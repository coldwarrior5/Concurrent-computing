#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>
#include "mpi.h"
#include <signal.h>
#include <stddef.h>

#define SECOND 1000000
#define MAXSECOND 9
#define MINSECOND 2
#define LIMIT 2

int myId;
int numProc;
char indentation = '\t';
char *preamble;
bool terminate = false;

typedef struct fork_s {
    bool mine;
    bool request;
    bool clean;
} forks;

forks left;
forks right;

void Init();
void Eating(int howLong);
void Thinking(int howLong);
void *CheckRequests(void *args);
void CheckAnyRequests();
int NeedQuit(pthread_mutex_t *mtx);
void Terminated();

int main(int argc,char *argv[])
{
    int seconds;
    int iter = 0;
    MPI_Status status;

    srand(time(NULL) ^ getpid());
    MPI_Init (&argc, &argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &myId);
    MPI_Comm_size (MPI_COMM_WORLD, &numProc);

    if(numProc < 2)
    {
        fprintf(stderr, "Requires at least two proceses.\n");
        exit(-1);
    }

    Init();

    while(iter++ < LIMIT)
    {
        seconds = rand() % MAXSECOND + MINSECOND;
        Thinking(seconds);

        while(right.mine == false || left.mine == false)
        {
            if(right.mine == false)
            {
                int req = 0;    // 0 - left
                int receiver = (myId + 1) % numProc;    // to my right
                MPI_Send(&req, 1, MPI_INT, receiver, 0, MPI_COMM_WORLD); // 0 - request
                //printf("P %d requesting from %d: %d\n", myId + 1, receiver + 1, req);
                right.request = false;
            }
            else if(left.mine == false)
            {
                int req = 1;    // 1 - right
                int receiver = (myId - 1 + numProc) % numProc;  // to my left
                MPI_Send(&req, 1, MPI_INT, receiver, 0, MPI_COMM_WORLD); // 0 - request
                //printf("P %d requesting from %d: %d\n", myId + 1, receiver + 1, req);
                left.request = false;
            }
            while(true)
            {
                int response;
                MPI_Recv(&response, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                /*
                int receiver = (response == 1) ? (myId - 1 + numProc) % numProc : (myId + 1) % numProc;
                printf("P %d response from %d: %d\n", myId + 1, receiver + 1, response);
                */
                if(status.MPI_TAG == 1)
                {
                    if(response == 0)
                    {
                        right.clean = true;
                        right.mine = true;
                        break;
                    }
                    if(response == 1)
                    {
                        left.clean = true;
                        left.mine = true;
                        break;
                    }
                }
                else
                {
                    int receiver = (response == 0) ? (myId - 1 + numProc) % numProc : (myId + 1) % numProc;
                    if(response == 0)
                    {
                        if(!left.clean)
                        {
                            left.clean = true;
                            left.mine = false;
                            MPI_Send(&response, 1, MPI_INT, receiver, 1, MPI_COMM_WORLD); // 1 - reply
                            //printf("P %d reply to %d: %d\n", myId + 1, receiver + 1, response);
                        }
                        else
                            left.request = true;
                    }
                    if(response == 1)
                    {
                        if(!right.clean)
                        {
                            right.clean = true;
                            right.mine = false;
                            MPI_Send(&response, 1, MPI_INT, receiver, 1, MPI_COMM_WORLD); // 1 - reply
                            //printf("P %d reply to %d: %d\n", myId + 1, receiver + 1, response);
                        }
                        else
                            right.request = true;
                    }
                }
            }
        }
        seconds = rand() % MAXSECOND + MINSECOND;
        Eating(seconds);
        CheckAnyRequests();
    }

    Thinking(MAXSECOND);
    free(preamble);
    MPI_Finalize();
    return 0;
}
void Init()
{
    sigset(SIGINT, Terminated);
    preamble = (char *)calloc(myId + 1, sizeof(char));
    for(int i = 0; i < myId; i++)
        strcat(preamble, &indentation);
    
    right.mine = true;
    right.request = false;
    right.clean = false;
    left.mine = false;
    left.request = true;
    left.clean = false;
    if(myId == 0)
    {
        left.mine = true;
        left.request = false;
    }
    if(myId == numProc - 1)
    {
        right.mine = false;
        right.request = true;
    }
}

void Eating(int howLong)
{
    printf("%sPhilosopher %d: I am eating\n", preamble, myId + 1);
    usleep(howLong * SECOND);
    left.clean = false;
    right.clean = false;
    printf("%sPhilosopher %d: I am done with eating\n", preamble, myId + 1);
}

void Thinking(int howLong)
{
    pthread_t asyncTask;
    pthread_mutex_t endTask;
    pthread_mutex_init(&endTask, NULL);
    pthread_mutex_lock(&endTask);
    pthread_create(&asyncTask, NULL, CheckRequests, &endTask);

    printf("%sPhilosopher %d: I am thinking\n", preamble, myId + 1);

    usleep(howLong * SECOND);
    pthread_mutex_unlock(&endTask);
    pthread_join(asyncTask, NULL);

    printf("%sPhilosopher %d: I am done with thinking\n", preamble, myId + 1);
}

void CheckAnyRequests()
{
    for(int i = 0; i < numProc; i++)
    {
        int response;
        int flag;
        MPI_Status status;
        MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
        if(flag && status.MPI_TAG == 0)
        {
            MPI_Recv(&response, 1, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);
            if(response == 0)
                left.request = true;
            
            if(response == 1)
                right.request = true;
            /*
            int receiver = (response == 0) ? (myId - 1 + numProc) % numProc : (myId + 1) % numProc;
            printf("P %d response from %d: %d\n", myId + 1, receiver + 1, response);
            */
        }
    }

    if(right.request)
    {
        int returnValue = 1;
        int receiver = (myId + 1) % numProc;  // to my right
        MPI_Send(&returnValue, 1, MPI_INT, receiver, 1, MPI_COMM_WORLD); // 1 - reply
        //printf("P %d reply to %d: %d\n", myId + 1, receiver + 1, returnValue);
        right.clean = true;
        right.mine = false;
    }
       
    if(left.request)
    {
        int returnValue = 0;
        int receiver = (myId - 1 + numProc) % numProc;  // to my left
        MPI_Send(&returnValue, 1, MPI_INT, receiver, 1, MPI_COMM_WORLD); // 1 - reply
        //printf("P %d reply to %d: %d\n", myId + 1, receiver + 1, returnValue);
        left.clean = true;
        left.mine = false;
    }
}

void *CheckRequests(void *args)
{
    pthread_mutex_t *mx = args;
    MPI_Status status;
    while(!NeedQuit(mx))
    {
        int flag;
        int response;
        MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
        if(flag && status.MPI_TAG == 0)
        {
            MPI_Recv(&response, 1, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);
            if(response == 0)
            {
                left.clean = true;
                left.mine = false;
            }
            if(response == 1)
            {
                right.clean = true;
                right.mine = false;
            }
            int receiver = (response == 0) ? (myId - 1 + numProc) % numProc : (myId + 1) % numProc;
            //printf("P %d response from %d: %d\n", myId + 1, receiver + 1, response);
            MPI_Send(&response, 1, MPI_INT, receiver, 1, MPI_COMM_WORLD); // 1 - reply
            //printf("P %d reply to %d: %d\n", myId + 1, receiver + 1, response);
        }
    }
    return NULL;
}

int NeedQuit(pthread_mutex_t *mtx)
{
    switch(pthread_mutex_trylock(mtx))
    {
        case 0:
            pthread_mutex_unlock(mtx);
            return 1;
        case EBUSY:
            return 0;
        default:
            return 1;
    }
}

void Terminated()
{
    free(preamble);
    exit(1);
}