#include <windows.h>

#define FUNCTION_ADDRESS LPTHREAD_START_ROUTINE 

//CreateThread
HANDLE CreateThread_(FUNCTION_ADDRESS function, LPVOID arguments)
{
    return CreateThread(NULL, 0, function, arguments, 0, NULL);
}