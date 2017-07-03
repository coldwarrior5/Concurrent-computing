#ifndef ERRORHANDLER_H_
#define ERRORHANDLER_H_

enum ErrorState
{
    ARGERROR = 1,
    MALLOC,
    INTCONVERSION,
    NOFILE,
    READERROR,
    WRITEERROR,
    USERTERMINATION,
    PROCNUM,
    MATRIXELEM
};

extern int errorCode;

void ErrorHandler(enum ErrorState error, const char *description);

#endif