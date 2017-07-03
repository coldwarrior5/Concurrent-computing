#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpi.h"

#include "../include/main.h"
#include "../include/game.h"
#include "../include/board.h"
#include "../include/IOHandler.h"
#include "../include/errorHandler.h"

int score = 0;
const int emptyNum = 0;
const int playerNum = 1;
const int computerNum = 2;
Board B;

bool LoadBoard(char* fileName)
{
    ParseTokens();
    char* filePath = DefinePath(FILE_LOCATION, fileName, FILE_EXTENSION);
    bool success = ParseBoard(filePath);
    free(filePath);

    if(!success)
        return false;
    PrintThePlayingField();
    return true;
}

Board CopyBoard(Board input)
{
    Board temp;
    temp.width = input.width;
    temp.height = input.height;
    temp.matrix = calloc(temp.height, sizeof(int*) + 1);
    for(int i = 0; i < temp.height; i++) 
        temp.matrix[i] = calloc(temp.width, sizeof(int));
    temp.matrix[temp.height] = NULL;

    for(int i = 0; i < temp.height; i++) 
    {
        for(int j = 0; j < temp.width; j++)
            temp.matrix[i][j] = input.matrix[i][j];
    }

    temp.empty.height = temp.player.height = temp.computer.height = input.empty.height;
    temp.empty.width = temp.player.width = temp.computer.width = input.empty.width;
    int size = (temp.empty.width + 1) * temp.empty.height + 1;
    temp.empty.symbol = calloc(size, sizeof(char));
    temp.player.symbol = calloc(size, sizeof(char));
    temp.computer.symbol = calloc(size, sizeof(char));
    strncpy(temp.player.symbol, input.player.symbol, size);
    strncpy(temp.computer.symbol, input.computer.symbol, size);
    strncpy(temp.empty.symbol, input.empty.symbol, size);

    return temp;
}

bool SaveBoard(char* fileName, char overwrite)
{
    char* filePath = DefinePath(FILE_LOCATION, fileName, FILE_EXTENSION);
    bool success = TryToSave(filePath, overwrite);
    free(filePath);

    return success;
}

bool* CheckValidMoves(Board temp)
{
    bool *validMove = (bool*) malloc(temp.width * sizeof(bool));
    int valid = 0;
    for(int j = 0; j < temp.width; j++)
    {
        if(temp.matrix[0][j] == emptyNum)
        {
            *(validMove + j) = true;
            valid++;
        }
        else
            *(validMove + j) = false;
    }

    if(valid == 0)
        over = true;

    return validMove;
}

void DropToken(Board* temp, int column, int value)
{
    if(value != emptyNum && value != playerNum && value != computerNum)
        return;
    for(int i = 0; i < temp->height; i++)
    {
        if(temp->matrix[i][column] != emptyNum)
        {
            temp->matrix[i - 1][column] = value;
            return;
        }
        if(i == temp->height - 1)
            temp->matrix[i][column] = value;
    }
}

void UndoMove(Board* temp, int column)
{
    if(column < 0 || column > temp->width)
        return;
    for(int i = 0; i < temp->height; i++)
    {
        if(temp->matrix[i][column] != emptyNum)
        {
            temp->matrix[i][column] = emptyNum;
            return;
        }
    }
}

void PrintThePlayingField()
{
    int totalWidth = B.width * B.empty.width + 1;
    int totalHeight = B.height * B.empty.height;
    char* outputValue = malloc(totalWidth * totalHeight + 1);

    for(int i = 0; i < B.height; i++)
    {
        for(int j = 0; j < B.width; j++)
        {
            int value = B.matrix[i][j];
            char* currentToken;
            switch(value)
            {
                case emptyNum:
                    currentToken = B.empty.symbol;
                    break;
                case playerNum:
                    currentToken = B.player.symbol;
                    break;
                case computerNum:
                    currentToken = B.computer.symbol;
                    break;
                default:
                    ErrorHandler(MATRIXELEM, NULL);
                    return;
                    break;
            }
            for(int it = 0; it < B.empty.height; it++)
            {
                for(int jt = 0; jt < B.empty.width; jt++)
                    sprintf(&outputValue[(i * B.empty.height + it) * totalWidth + j * B.empty.width + jt], "%c", currentToken[it * B.empty.width + jt + it]); // Another it is because of newlines \n
                outputValue[(i * B.empty.height + it + 1) * totalWidth - 1] = '\n';
            }
        }
    }
    outputValue[totalWidth * totalHeight] = '\0';
    printf("\n%s\n", outputValue);
    free(outputValue);
    return;
}

bool ParseBoard(char *filePath)
{
    char* text;
    char buff[1024] = {0};
    char *pch = NULL;
    int i = 0;
    int j = 0;

    int readSize = ReadFile(filePath, (unsigned char**)&text, 0);
    if(readSize == 0)
        return false;
    
    int readThings = sscanf(text, "%d %d", &B.height, &B.width);
    if(readThings != 2 || B.width != 7)
        return false;

    B.matrix = calloc(B.height, sizeof(int*) + 1);
     for(int i = 0; i < B.height; i++) {
        B.matrix[i] = calloc(B.width, sizeof(int));
    }
    B.matrix[B.height] = NULL;

    strcpy(buff, text);
    pch = strtok(buff, "\r\n");
    if(pch != NULL)
        pch = strtok(NULL, "\r\n");
    while(pch != NULL)
    {
        char tBuff[100] = {0};
        char *tPch = NULL;
        strcpy(tBuff, pch);
        tPch = strtok(tBuff, " ");
        while(tPch != NULL)
        {
            int read = sscanf(tPch, "%d", &B.matrix[i][j]);
            if(read != 1)
                return false;
            tPch = strtok(NULL, " ");
            j++;
        }
        pch = strtok(pch+ strlen (pch) + 1, "\r\n");
        i++;
        j = 0;
    }
    return true;
}

bool TryToSave(char* filePath, char overwrite)
{
    int size = 6 + ((B.width + 2) * B.height) * 3 + 1;
    char* buffer = calloc(size, sizeof(char));

    sprintf(buffer, "%d %d\n", B.height, B.width);

    for(int i = 0; i < B.height; i++)
    {
        sprintf(buffer  + strlen(buffer), "%s", " ");
        for(int j = 0; j < B.width; j++)
        {
            sprintf(buffer  + strlen(buffer), " %d ", B.matrix[i][j]);
        }
        sprintf(buffer  + strlen(buffer), "%s", "\n");
    }
    buffer[size] = '\0';

    bool success = SaveToFile((unsigned char*) buffer, size, filePath, 0, overwrite);
    free(buffer);
    return success;
}

void ParseTokens()
{
    char* playerFile = "player";
    char* computerFile = "computer";
    char* emptyFile = "empty";
    Token player;
    Token computer;
    Token empty;
    char* tokenPath = DefinePath(TOKEN_LOCATION, playerFile, FILE_EXTENSION);
    ReadFile(tokenPath, (unsigned char**) &player.symbol, 0);
    tokenPath = DefinePath(TOKEN_LOCATION, computerFile, FILE_EXTENSION);
    ReadFile(tokenPath, (unsigned char**) &computer.symbol, 0);
    tokenPath = DefinePath(TOKEN_LOCATION, emptyFile, FILE_EXTENSION);
    ReadFile(tokenPath, (unsigned char**) &empty.symbol, 0);
    player.width = computer.width = empty.width = GetWidth(empty.symbol);
    player.height = computer.height = empty.height = GetHeight(empty.symbol);
    B.player = player;
    B.computer = computer;
    B.empty = empty;
    return;
}

double EvaluateLocal(Board temp, int LastMover, int* iLastCol, int iDepth)
{
    double dBest = -1; 
    double iBestCol = -1;

	double dResult, dTotal;
	int newMover;
	bool bAllLose = true, bAllWin = true;
	int iMoves;
    bool* validMove = CheckValidMoves(temp);
	
    int winner = 0;
    if(*iLastCol != -1)
        winner = Winner(temp, *iLastCol);
    else
        winner = CheckEveryColumn(temp);

	if(winner != 0)	// igra gotova?
    {
		if(winner == computerNum)
            return 1;	// pobjeda
		else            //if(LastMover == HUMAN)
            return -1;	// poraz
    }
	// nije gotovo, idemo u sljedecu razinu
	if(iDepth == 0)
		return 0;

	if(LastMover == computerNum)	// tko je na potezu
		newMover = playerNum;
	else if(LastMover == playerNum)
		newMover = computerNum;
    else
        newMover = computerNum;

	dTotal = 0;
	iMoves = 0;	// broj mogucih poteza u ovoj razini
	for(int iCol=0; iCol < temp.width; iCol++)
	{	
        if(validMove[iCol])	// jel moze u stupac iCol
		{	
            iMoves++;
			DropToken(&temp, iCol, newMover);
			dResult = EvaluateLocal(temp, newMover, &iCol, iDepth - 1);
			UndoMove(&temp, iCol);

            if(iDepth == depth)
            {	
                if(dResult > dBest || (dResult == dBest && rand()%2 == 0))
                {
                    dBest = dResult;
                    iBestCol = iCol;
                }
            }
            else
            {
                if(dResult > -1)
                    bAllLose = false;
                if(dResult != 1)
                    bAllWin = false;
                if(dResult == 1 && newMover == computerNum)
                    return 1;	// ako svojim potezom mogu doci do pobjede (pravilo 1)
                if(dResult == -1 && newMover == playerNum)
                    return -1;	// ako protivnik moze potezom doci do pobjede (pravilo 2)
                //if(dResult > 0)	// izgleda da je bolje ako se u racunanju uzimaju i porazi ...
                    dTotal += dResult;
            }
		}
	}
    if(iDepth != depth)
    {
        if(bAllWin == true)	// ispitivanje za pravilo 3.
            return 1;
        if(bAllLose == true)
            return -1;
        dTotal /= iMoves;	// dijelimo ocjenu sa brojem mogucih poteza iz zadanog stanja
	    return dTotal;
    }
    else
    {
        *iLastCol = iBestCol;
        return dBest;
    }
}

double EvaluateRemote(Board temp, int LastMover, int* iLastCol, int iDepth, int localDepth)
{
    State* tree;
    State** leaves;
    int* children = (int*)malloc(1 * sizeof(int));
    int childrenSize = 0;

    State S;
    S.B = CopyBoard(temp);
    S.lastRow = -1;
    S.lastColumn = -1;
    S.score = 0;
    GetTree(&tree, &children, &childrenSize, &S, depth - localDepth);


    return 0;
}

int CheckEveryColumn(Board temp)
{
    int winner = 0;

    for(int i = 0; i < temp.width; i++)
    {
        if(winner != 0)
            break;
        winner = Winner(temp, i);
    }
    return winner;
}

int Winner(Board temp, int lastColumn)
{
    int lastPlayer = 0;
    int lastRow = -1;
    int winner = 0;

    for(int i = 0; i < temp.height; i++)
    {
        if(temp.matrix[i][lastColumn] != emptyNum)
        {
            lastPlayer = temp.matrix[i][lastColumn];
            lastRow = i;
            break;
        }
    }
    if(lastRow == -1)
        return emptyNum;
    winner = CheckVertical(temp, lastColumn, lastPlayer);
    if(winner != 0)
        return winner;
    winner = CheckHorizontal(temp, lastRow, lastColumn, lastPlayer);
    if(winner != 0)
        return winner;
    winner = CheckRightDiagonal(temp, lastRow, lastColumn, lastPlayer);
    if (winner != 0)
        return winner;
    winner = CheckLeftDiagonal(temp, lastRow, lastColumn, lastPlayer);
    
    return winner;
}

int CheckVertical(Board temp, int lastColumn, int lastPlayer)
{
    int count = 0;
    for (int i = 0; i < temp.height; i++)
    {
        if (temp.matrix[i][lastColumn] == lastPlayer)
        {
            count++;
            if (count == 4)
                return lastPlayer;
        }
        else
        {
            count = 0;
        }
    }
    return 0;
}

int CheckHorizontal(Board temp, int lastRow, int lastColumn, int lastPlayer)
{
    int counter = 0;
    int startCol = -1;
    int endCol = -1;

    for (int i = 0; i < temp.width; i++)
    {
        if (temp.matrix[lastRow][i] == lastPlayer)
        {
            if(counter == 0)
                startCol = i;
            counter++;
            endCol = i;
            if (counter >= 4 && lastColumn >= startCol && lastColumn <= endCol)
                return lastPlayer;
        }
        else
        {
            counter = 0;
        }
    }
    return 0;
}

int CheckRightDiagonal(Board temp, int lastRow, int lastColumn, int lastPlayer)
{
    int startRow = lastRow;
    int startCol = lastColumn;
    int counter = 0;

    for (int i = 0; i < max(temp.height, temp.width); i++)
    {
        if (lastColumn + i >= temp.width || lastRow - i < 0)
            break;
        startCol = lastColumn + i;
        startRow = lastRow - i;
    }

    for (int i = 0; i < max(temp.height, temp.width); i++)
    {
        if (startCol - i < 0 || startRow + i >= temp.height)
            break;
        if (temp.matrix[startRow + i][startCol - i] == lastPlayer)
        {
            counter++;

            if (counter == 4)
                return lastPlayer;
        }
        else
        {
            counter = 0;
        }
    }
    return 0;
}

int CheckLeftDiagonal(Board temp, int lastRow, int lastColumn, int lastPlayer)
{
    int startRow = lastRow;
    int startCol = lastColumn;
    int counter = 0;

    for (int i = 0; i < max(temp.height, temp.width); i++)
    {
        if (lastColumn - i < 0 || lastRow - i < 0)
            break;
        startCol = lastColumn - i;
        startRow = lastRow - i;
    }

    for (int i = 0; i < max(temp.height, temp.width); i++)
    {
        if (startCol + i >= temp.width || startRow + i >= temp.height)
            break;
        if (B.matrix[startRow + i][startCol + i] == lastPlayer)
        {
            counter++;

            if (counter == 4)
                return lastPlayer;
        }
        else
        {
            counter = 0;
        }
    }
    return 0;
}

void GetTree(State** tree, int** children, int* childrenSize, State* state, const int targetDepth)
{
    int qSize = 0;
    int tSize = 0;
    QueueState* q = calloc(1, sizeof(QueueState));

    QueueState qState;
    qState.S = state;
    qState.depth = 0;

    PushTheState(qState, &q, &qSize);
    PushBack(*state, tree, &tSize);
    
    int width = state->B.width;
    while(qSize)
    {
        QueueState tempQState = PopTheState(&q, &qSize);;
        State* tempState = tempQState.S;
        int tempDepth = tempQState.depth;
        if(tempDepth >= targetDepth)
            continue;
        int nextPlayer = 0;
        if(tempQState.S->lastColumn == -1 || tempQState.S->lastRow)
            nextPlayer = computerNum;
        if(nextPlayer == 0)
        {
            int lastPlayer = tempQState.S->B.matrix[tempQState.S->lastRow][tempQState.S->lastColumn];
            nextPlayer = (lastPlayer == playerNum)? computerNum: playerNum; 
        }
        *childrenSize += 1;
        int* newChildren = realloc(*children, *childrenSize * sizeof(int));
        newChildren[*childrenSize] = tSize;

        for(int i = 0; i < width; i++)
        {
            State Child;
            Child.B = CopyBoard(state->B);
            Child.lastColumn = state->lastColumn;

        }
    }

}

void PushBack(State state, State** states, int* size)
{
    State* newStates = (State*) realloc(*states, (*size + 1) * sizeof(QueueState));
    if(newStates == NULL)
    {
        ErrorHandler(MALLOC, "Error whilst allocating new stack");
        return;
    }

    newStates[*size] = state;
    *states = newStates;

    return;
}