#ifndef STRUCT_H_
#define STRUCT_H_

typedef struct
{
    int width;
    int height;
    char* symbol;
} Token;

typedef struct
{
    int width;
    int height;
    int** matrix;
    Token empty;
    Token player;
    Token computer;
    
} Board;

typedef struct
{
    Board B;
    double score;
    int lastRow;
    int lastColumn;
} State;

typedef struct
{
    State* S;
    int depth;
} QueueState;

typedef struct
{
    int workers;
    char* task;
    State** state;

    int numStates;
    QueueState* waitingStates;

} Communication;

#endif