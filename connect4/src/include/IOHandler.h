#ifndef IOHandler_H_
#define IOHandler_H_

bool ProcessInput(int argc, char* argv[]);
int ReadFile(const char* const fileName, unsigned char** buffer, char binary);
void DisplayFile(const char* const fileName, char binary);
bool SaveToFile(unsigned char* text, int length, const char* const destinationFile, char binary, char overwrite);
void HexPrint(unsigned char* input, int length);
char* DefinePath(const char* const folder, const char* const filename, const char* const extension);
int UserChoice(int size);
void CheckTermination(char *input);
int CharToInt(char* number);
char* IntToChar(int number, int* length);
int GetWidth(char *text);
int GetHeight(char *text);

#endif