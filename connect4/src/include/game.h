#include "struct.h"
#ifndef GAME_H_
#define GAME_H_

void Play();
int ComputerChoice();
bool CheckVictory();
void SetComm(int workers);
void Wait();
void CheckMessages();
void SendTask();
void TellOthers();
QueueState PopTheState(QueueState** queue, int* size);
void PushTheState(QueueState state, QueueState** queue, int* size);

#endif