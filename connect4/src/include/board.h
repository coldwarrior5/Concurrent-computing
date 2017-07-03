#include "struct.h"
#ifndef max
	#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#endif

#ifndef min
	#define min( a, b ) ( ((a) < (b)) ? (a) : (b) )
#endif

#ifndef BOARD_H_
#define BOARD_H_



extern Board B;
bool LoadBoard(char* fileName);
Board CopyBoard(Board input);
bool SaveBoard(char* fileName, char overwrite);
bool* CheckValidMoves(Board temp);
void DropToken(Board* temp, int column, int value);
void UndoMove(Board* temp, int column);
void PrintThePlayingField();
bool TryToSave(char* filePath, char overwrite);
bool ParseBoard(char *filePath);
void ParseTokens();
double EvaluateLocal(Board temp, int LastMover, int* iLastCol, int iDepth);
double EvaluateRemote(Board temp, int LastMover, int* iLastCol, int iDepth, int localDepth);
int CheckEveryColumn(Board temp);
int Winner(Board temp, int column);
int CheckVertical(Board temp, int lastColumn, int lastPlayer);
int CheckHorizontal(Board temp, int lastRow, int lastColumn, int lastPlayer);
int CheckRightDiagonal(Board temp, int lastRow, int lastColumn, int lastPlayer);
int CheckLeftDiagonal(Board temp, int lastRow, int lastColumn, int lastPlayer);
void PushBack(State state, State** states, int* size);
void GetTree(State** tree, int** children, int* childrenSize, State* state, const int targetDepth);
extern const int emptyNum;
extern const int playerNum;
extern const int computerNum;

#endif