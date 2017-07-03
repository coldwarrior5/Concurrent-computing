#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#include "mpi.h"
#include "../include/main.h"
#include "../include/struct.h"
#include "../include/errorHandler.h"
#include "../include/IOHandler.h"
#include "../include/game.h"
#include "../include/board.h"

Communication comm;

void Play()
{
    bool over = CheckVictory();
    while(!terminated && !over)
    {
        printf("\nChoose column: ");
        bool valid = false;
        int choice = 0;
        while(!valid)
        {
            choice = UserChoice(B.width) - 1;
            if(terminated) 
                return;
            if(save)
            {
                char* fileName;
                printf("Enter the name of the file to save the board to: ");
                scanf("%s", fileName);
                bool success = SaveBoard(fileName, 0);
                if(success)
                    printf("Success\n");
                break;
            }
            bool *validMoves = CheckValidMoves(B);
            if(over)
                break;
            if(validMoves[choice] == true)
                valid = true;
            else
            {
                printf("\nThis column is full\n");
                valid = false;
            }
        }
        if(over)
            break;
        if(save)
        {
            save = false;
            continue;
        }
        DropToken(&B, choice, playerNum);
        PrintThePlayingField();
        over = CheckVictory(B);
        if(over)
            continue;

        //Computer turn
        int comChoice = ComputerChoice();
        DropToken(&B, comChoice, computerNum);
        SaveBoard(fileName, (char)1);
        PrintThePlayingField();
        over = CheckVictory(B);
        if(over)
            continue;
    }
}

int ComputerChoice()
{
    struct timespec tstart={0,0}, tend={0,0};
    double dResult, dBest;
	int iBestCol, iDepth = depth;
    
    clock_gettime(CLOCK_MONOTONIC, &tstart);
    Board temp;
    if(numProc == 1)
        temp = B;
    else
        temp = CopyBoard(B);

    bool* validMove = CheckValidMoves(temp);

    do
    {
		dBest = -1; iBestCol = -1;
        if(numProc == 1)
			dBest = EvaluateLocal(temp, 0, &iBestCol, iDepth);
        else
            dBest = EvaluateRemote(temp, 0, &iBestCol, iDepth, (int)(0.35 * (iDepth)));
        iDepth /= 2;
	}while(dBest == -1 && iDepth > 0);

    clock_gettime(CLOCK_MONOTONIC, &tend);
    printf("Computer took %.5f seconds to calculate the move\n",
            ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - 
            ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec));

    return iBestCol;
}

bool CheckVictory()
{
    int winner = 0;
    char victor[20];
    for(int i = 0; i < B.width; i++)
    {
        if(winner != 0)
            break;
        winner = Winner(B, i);

        switch(winner)
        {
            case 0:
                continue;
            case 1:
                sprintf(victor, "%s", "Player");
                break;
            case 2:
                sprintf(victor, "%s", "Computer");
                break;
            default:
                return false;
                break;
        };
    }
    if(winner != 0)
        printf("%s is victorious.\n", victor);

    return winner != 0;
}

//______________________________________________//
//                  COMMUNICATION               //
//______________________________________________//

void SetComm(int workers)
{
    comm.workers = workers;
    comm.numStates = 0;
}

void AddTaskComm(State* S, int depth)
{
    QueueState q;
    q.S = S;
    q.depth = depth;
    PushTheState(q, &comm.waitingStates, &comm.numStates);
    return;
}

void Wait()
{
    int busy = 0;
    do
    {
        CheckMessages();
        SendTask();
        busy = 0;
        for(int i = 0; i < comm.workers; i++)
        {
            busy += comm.task[i] > 0;
        }
    }while(busy > 0 || comm.numStates > 0);
}

void CheckMessages()
{
    int flag;
    MPI_Status status;
    MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
    while(flag)
    {
        if(status.MPI_TAG == MSG_READY)
        {
            int msg;
            MPI_Recv(&msg, 1, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);
            comm.task[status.MPI_SOURCE - 1] = 0;
            comm.state[status.MPI_SOURCE - 1] = NULL;
        }
        else if(status.MPI_TAG == MSG_RESULT)
        {
            double score;
            MPI_Recv(&score, 1, MPI_DOUBLE, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);
            comm.state[status.MPI_SOURCE - 1]->score = score;
        }
        MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
    }
}

void SendTask()
{
    for(int i = 0; i < comm.workers; i++)
    {
        if(comm.task[i] == 0)   // if worker i is available
        {
            QueueState task = PopTheState(&comm.waitingStates, &comm.numStates);
            MPI_Send(&task, sizeof(task), MPI_BYTE, i + 1, MSG_TASK, MPI_COMM_WORLD);
            comm.task[i] = 1;
            comm.state[i] = task.S;
        }
    }
}

void TellOthers()
{
    for(int i = 0; i < comm.workers; i++)
    {
        int stop = 1;
        MPI_Request req;
        MPI_Isend(&stop, 1, MPI_INT, i + 1, MSG_STOP, MPI_COMM_WORLD, &req);
    }
}

QueueState PopTheState(QueueState** queue, int* size)
{
    QueueState state;
    state = *queue[0];
    *size -= 1;
    QueueState* newQueue = (QueueState*) calloc(*size + 1, sizeof(QueueState));
    if(newQueue == NULL)
    {
        ErrorHandler(MALLOC, "Error whilst allocating new stack");
        return state;
    }
    for(int i = 0; i < *size; i++)
    {
        newQueue[i] = *queue[i + 1];
    }
    *queue = newQueue;

    return state;
}

void PushTheState(QueueState state, QueueState** queue, int* size)
{
    QueueState* newQueue = (QueueState*) realloc(*queue, (*size + 1) * sizeof(QueueState));
    if(newQueue == NULL)
    {
        ErrorHandler(MALLOC, "Error whilst allocating new stack");
        return;
    }

    newQueue[0] = state;
    for(int i = 1; i < *size - 1; i++)
    {
        newQueue[i] = *queue[i];
    }

    *queue = newQueue;

    return;
}