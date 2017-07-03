#include <stdio.h>

#include "../include/errorHandler.h"

void ErrorHandler(enum ErrorState error, const char *description)
{
    errorCode = (int) error;
    switch(error)
    {
        case ARGERROR:
            if(description == NULL)
                printf("\nArguments were not correct, try --help argument.\n");
            else
                printf("\nArguments %s is not correct.\n", description);
            break;
        case MALLOC:
            printf("\nAn error occured while allocating memory. %s.\n", description);
            break;
        case INTCONVERSION:
            printf("\nInt conversion error. %s.\n", description);
            break;
        case NOFILE:
            printf("\nThe file %s does not exist.\n", description);
            break;
        case READERROR:
            printf("\nAn error occured whilst reading file: %s.\n", description);
            break;    
        case WRITEERROR:
            printf("\nAn error occured whilst saving file: %s.\n", description);
            break;
        case PROCNUM:
            printf("\nProcess error. %s.\n", description);
            break;
        case MATRIXELEM:
            printf("\nA matrix element is incorrect, values should be only 0, 1 or 2.\n");
            break;
        default:
            printf("\nUnknown error occured.\n");
            break;
    }
    return;
}