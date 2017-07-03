#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

#include "../include/main.h"
#include "../include/errorHandler.h"
#include "../include/IOHandler.h"

const char* const description = "miscelaneous/description.txt";

bool ProcessInput(int argc, char* argv[])
{
    if(argc == 2 && (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-?")))
        DisplayFile(description, 0);
    else if(argc != 3)
        ErrorHandler(ARGERROR, NULL);
    else
    {
        fileName = argv[1];
        depth = CharToInt(argv[2]);
        if(depth < 4)
        {
            ErrorHandler(ARGERROR, argv[2]);       
            return (int)ARGERROR;
        }
            
        return true;
    }

    return false;
}

int ReadFile(const char* const fileName, unsigned char** buffer, char binary)
{
    if (access(fileName, F_OK | R_OK) != 0)
    {
        ErrorHandler(NOFILE, fileName);
        *buffer = NULL;
        return 0;
    }
    
	int stringSize = 0, readSize = -1;
	FILE *handler = NULL;
    if(binary == 1)
	    handler = fopen(fileName, "rb");
    else
        handler = fopen(fileName, "r");

    if(handler)
    {
        fseek(handler, 0, SEEK_END);
        stringSize = ftell(handler);
        rewind(handler);

        unsigned char* tempBuffer = (unsigned char*)malloc(sizeof(unsigned char) * (stringSize + 1));
        readSize = fread(tempBuffer, sizeof(unsigned char), stringSize, handler);
        tempBuffer[stringSize] = '\0';
        *buffer = tempBuffer;
        fclose(handler);
    }

    if (stringSize != readSize)
    {  
        ErrorHandler(READERROR, fileName);
        free(*buffer);
        *buffer = NULL;
    }
    
    return readSize;;
}

void DisplayFile(const char* const fileName, char binary)
{
    unsigned char* buffer = NULL;
    int readSize = ReadFile(fileName, &buffer, binary);
    
    if (readSize != 0)
    {
        if(binary == 1)
            HexPrint(buffer, readSize);
        else
            printf("%s", buffer);
        free(buffer);
    }
    return;
}

bool SaveToFile(unsigned char* text, int length, const char* const destinationFile, char binary, char overwrite)
{
    
    if (access(destinationFile, F_OK) == 0 && !overwrite)
    {
        char input[5];
        fgets(input, 5, stdin);
        printf("Do you wish to overwrite existing file? [N/y] ");
        fgets(input, 5, stdin);
        if(input[0] != 'y')
            return false;
    }
    

    FILE *handler = NULL;
    if(binary == 1)
        handler = fopen(destinationFile, "wb");
    else
        handler = fopen(destinationFile, "w");

    if (handler)
    {
        fwrite(text, sizeof(unsigned char), length, handler);
        fclose(handler);
    }
    else
    {
        ErrorHandler(WRITEERROR, destinationFile);
        return false;
    }
    
    return true;
}

// Print string as hex string
void HexPrint(unsigned char* input, int length)
{
    unsigned char *p = input;
    if (NULL == input)
        printf("NULL");
    else
    {
        for(int i = 0; i < length; i++)
            printf("%02X ", *p++);
    }
    printf("\n");
}

char* DefinePath(const char* const folder, const char* const filename, const char* const extension)
{
    char* str = (char *) malloc(sizeof(char) * (sizeof(folder) + sizeof(filename) + sizeof(extension) + 2));
    strcpy(str, folder);
    strcat(str, "/");
    strcat(str, filename);
    strcat(str, extension);
    return str;
}

int UserChoice(int size)
{
    char input[10];
    char *endptr;
    int result = -1;

    scanf("%s", input);
    CheckTermination(input);
	long lResult = strtol(input, &endptr, BASE);
    
    while (!terminated && !save && (endptr == input || lResult > size || lResult <= 0)) 
    {
        printf("Incorrect choice, must be within 1 and %d\n", size);
        scanf("%s", input);
        CheckTermination(input);
        lResult = strtol(input, &endptr, BASE);
    }
    if(!terminated && !save)
        result = (int) lResult;
    return result;
}

void CheckTermination(char *input)
{
    if(!strcmp(input, "q") || !strcmp(input, "quit") || !strcmp(input, "stop") || !strcmp(input, "exit"))
        terminated = true;
    if(!strcmp(input, "save"))
        save = true;
}

int CharToInt(char* number) // Has to be char
{
    char *endptr;
    int result = -1;

	long lResult = strtol(number, &endptr, BASE);
    
    if(endptr == number || ((lResult <= INT_MIN || lResult >= INT_MAX) && errno == ERANGE))
        result = -1;
    else
        result = (int) lResult;

    return result;
}

// Int represented as ascii characters
char* IntToChar(int number, int* length)
{
    char* buffer = malloc(sizeof(char)*10); // Has to be char
    memset(buffer, 0, 10);
    snprintf(buffer, 10, "%d", number);
    for(int i = 0; i < 10; i++)
    {
        if(*(buffer + i) == 0)
        {
            *length = i;
            break;
        }
    }
    return buffer;
}

int GetWidth(char *text)
{
    char* firstLine;
    sscanf(text, "%s", firstLine);
    return strlen(firstLine);
}

int GetHeight(char *text)
{
    char* line;
    int size = 0;
    char buff[1024] = {0};
    char *pch = NULL;
    strcpy(buff, text);
    pch = strtok(buff, "\n");
    while(pch != NULL)
    {
        size++;
        pch = strtok(NULL, "\n");
    }
    
    return size;
}