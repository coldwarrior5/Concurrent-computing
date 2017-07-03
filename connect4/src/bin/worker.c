#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi.h"
#include "../include/main.h"
#include "../include/worker.h"
#include "../include/game.h"
#include "../include/board.h"

bool haveTask = false;
QueueState q;

void Calculation()
{
    MPI_Status status;
    SendReady();
    while(true)
    {
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        if(status.MPI_TAG == MSG_STOP)
            break;
        
        if(!haveTask)
            MPI_Probe(MPI_ANY_SOURCE, MSG_TASK, MPI_COMM_WORLD, &status);

        if(status.MPI_TAG == MSG_TASK)
        {
            char* msg =  (char*) calloc(sizeof(QueueState), sizeof(char));
            MPI_Recv(msg, sizeof(QueueState), MPI_BYTE, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);

            memcpy(&q, msg, sizeof(QueueState));
            free(msg);
            haveTask = true;
        }
        Calculate();
    }
}

void Calculate()
{
    if(haveTask)
    {
        int lastMover = -1;
        int lastCol = 0;
        if(q.S->lastRow == -1 || q.S->lastColumn == -1)
        {
            lastMover = 0;
            lastCol = -1;
        }
        if(lastMover == -1)
            lastMover = (int) q.S->B.matrix[q.S->lastRow][q.S->lastColumn];
        double result = EvaluateLocal((q.S)->B, lastMover, &lastCol, q.depth);
        MPI_Send(&result, 1, MPI_DOUBLE, 0, MSG_RESULT, MPI_COMM_WORLD);
        SendReady();
        haveTask = false;
    }
}

void SendReady()
{
    int ready = 1;
    MPI_Send(&ready, 1, MPI_INT, 0, MSG_READY, MPI_COMM_WORLD);
}