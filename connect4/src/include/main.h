#ifndef bool
#define bool char
#define false ((bool)0)
#define true  ((bool)1)
#endif

#ifndef MAIN_H_
#define MAIN_H_

void Init();
void Terminated();

extern int myId;
extern int numProc;
extern int numOfTasks;
extern bool terminated;
extern bool over;
extern bool save;

extern char* fileName;
extern const char* const FILE_LOCATION;
extern const char* const TOKEN_LOCATION;
extern const char* const FILE_EXTENSION;
extern const int BASE;
extern int depth;

extern const int MSG_READY;
extern const int MSG_TASK;
extern const int MSG_RESULT;
extern const int MSG_STOP;

#endif