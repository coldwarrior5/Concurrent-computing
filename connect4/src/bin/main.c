#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "mpi.h"
#include "../include/main.h"
#include "../include/board.h"
#include "../include/IOHandler.h"
#include "../include/errorHandler.h"
#include "../include/worker.h"
#include "../include/game.h"

int myId;
int numProc;

int numOfTasks = 7;

int errorCode = USERTERMINATION;
bool terminated = false;
bool over = false;
bool save = false;

char* fileName;
const char* const FILE_LOCATION = "settings";
const char* const TOKEN_LOCATION = "miscelaneous";
const char* const FILE_EXTENSION = ".txt";
const int BASE = 10;

const int MSG_READY = 1;
const int MSG_TASK = 2;
const int MSG_RESULT = 3;
const int MSG_STOP = 4;

int depth;

int main(int argc, char* argv[])
{
    Init(&argc, &argv);

    if(myId == 0)
    {
        bool success = ProcessInput(argc, argv);
        if(!success)
            return errorCode;
        success = LoadBoard(fileName);
        if(!success)
            return errorCode;
        SetComm(numProc - 1);
        Play();
        TellOthers();
    }
    else
    {
        Calculation();
    }
    
    MPI_Finalize();
    return 0;
}

void Init(int* argc, char*** argv)
{
    sigset(SIGINT, Terminated);
    MPI_Init (argc, argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &myId);
    MPI_Comm_size (MPI_COMM_WORLD, &numProc);

    if(numProc < 1)
    {
        ErrorHandler(PROCNUM, "Too little processes, n should be at least 1");
        Terminated();
    }
}

void Terminated()
{
    TellOthers();
    MPI_Finalize();
    exit(errorCode);
}