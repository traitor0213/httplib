#define WIN32_LEAN_AND_MEAN

#define DEFAULT_BUFFER_SIZE 2048
#define DEFAULT_INNER_BUFFER_SIZE 256

#include "./url/url.h"
#include "./native/wrap.h"
#include "./network/http.h"

#include <malloc.h>
#include <windows.h>

#define BUFFER_SIZE 1024

typedef struct PROCEDURE_INFO
{
    SOCKET ResponseSocket;
    BOOL *MoveGarantie; 
} PROCEDURE_INFO, *LPPROCEDURE_INFO;

void HttpProcedure(PROCEDURE_INFO *info)
{
	SOCKET IOSocket = info->ResponseSocket;
	*info->MoveGarantie = TRUE;

	char *FirstLine = (char*)LocalAlloc(LPTR, BUFFER_SIZE);
	if(FirstLine == NULL)
	{
		printf("[WARNING] not enough memory!\n");
		CloseHttpSession(IOSocket);
		return;
	}

	ReadFirstHttpLine(IOSocket, FirstLine, BUFFER_SIZE);
	
	LPVOID RequestContent = ReadHttpContent(IOSocket);
	FreeHttpContent(RequestContent);

	char RequestedPath[260];
	char OpenPath[sizeof(RequestedPath)];

	GetPathFromHttpMethod(FirstLine, RequestedPath, sizeof(RequestedPath));

	OpenPath[0] = '.';
	memcpy(OpenPath + 1, RequestedPath, sizeof(RequestedPath) - 1);	

	printf("open: '%s'\n", OpenPath);

	HANDLE hFile = CreateFileA(OpenPath,
		GENERIC_READ, 
		FILE_SHARE_READ, 
		NULL, 
		OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL, 
		NULL);

	if(hFile == INVALID_HANDLE_VALUE)
	{
		printf("[WARNING] cannot open file!\n");
		CloseHttpSession(IOSocket);
		LocalFree(FirstLine);
		return;
	}

	int ContentSize = (int)GetFileSize(hFile, NULL);

	LPVOID Content = LocalAlloc(LPTR, ContentSize);
	if(Content == NULL)
	{
		printf("[WARNING] not enough memory!\n");
		CloseHttpSession(IOSocket);
		LocalFree(FirstLine);
		return;
	}

	DWORD Readed = 0;
	ReadFile(hFile, 
		Content, 
		ContentSize,
		&Readed,
		NULL);

	char *HttpResponse = (char*)LocalAlloc(LPTR, ContentSize + BUFFER_SIZE);
	if(HttpResponse == NULL)
	{
		printf("[WARNING] not enough memory!\n");
		CloseHttpSession(IOSocket);
		LocalFree(FirstLine);
		LocalFree(HttpResponse);
		return;
	}

	unsigned int ResponseSize = CreateHttpRaw(HttpResponse, 
		ContentSize + BUFFER_SIZE, 
		"HTTP/1.1 200 OK", 
		"Content-Length", ContentSize,
		"Content", Content);

	WriteHttpRaw(IOSocket, 
		HttpResponse,
		ResponseSize);

	CloseHttpSession(IOSocket);
	LocalFree(FirstLine);
	LocalFree(HttpResponse);
	CloseHandle(hFile);
	return;
}

void HttpServer(SOCKET *Socket)
{
	SOCKET OpenSocket = *Socket;
	char FirstLine[1024];

	SOCKET ClientSocket = 0;
	SOCKADDR_IN ClientInfo;

	for(;;)
	{
		if(*Socket == 0) break;

		if((ClientSocket = AcceptRequest(&ClientInfo, OpenSocket)) != SOCKET_ERROR)
		{
			BOOL Moved = FALSE;
			PROCEDURE_INFO info;
			info.MoveGarantie = &Moved;
			info.ResponseSocket = ClientSocket;
			
			CreateThread_((LPTHREAD_START_ROUTINE)HttpProcedure, &info);

			for(;*info.MoveGarantie != TRUE; Sleep(0));
		}
		else 
		{
			Sleep(0);
		}
	}

	return;
}


//main thread
int main()
{
    /*
    This function is using for initialize windows socket api function
    This function is call LoadLibraryA function 
    So, is not for linux OS, Only for windows
    */
    printf("[+] intialize tcp/ip library..\n");
    HMODULE SocketModule = InitializeTcpLibrary();
    printf("[+] %p\n", SocketModule);

    /*
    This WSAAPI function is using for initialize windows socket api module
    
    first argument is version number
    second argument is address of WSADATA variable
    */
    printf("[+] startup windows socket api\n");
    WSADATA wsa;
    WSAStartup_(MAKEWORD(2, 2), &wsa);

    const int HttpPort = 80;
    const int Backlog = 1024;

    SOCKET HttpSocket = OpenTcpPort(HttpPort, Backlog, TRUE);

    /*
    error check
    if socket value is small than zero, error case
    */

    if (HttpSocket < 0)
    {
        /*
        bind error || listen error
        
        defined "../network/tcp.h"

        #define ERROR_FROM_SOCKET 0x1
        #define ERROR_FROM_BIND 0x2
        #define ERROR_FROM_LISTEN 0x3
        #define ERROR_FROM_ACCEPT 0x4
        #define ERROR_FROM_CONNECT 0x5
        #define ERROR_FROM_IO 0x6
        */

        //free windows socket api
        WSACleanup_();
        //unmapping socket module
        FreeLibrary(SocketModule);

        return 0;
        //end of program
    }

    /*
    create server thread
    'hServer' variable is server thread handle 
    */

    HANDLE hHttpServer = CreateThread_((FUNCTION_ADDRESS)HttpServer, (LPVOID)&HttpSocket);

    getchar();

    printf("[+] close socket..\n");
    //if called closesocket function, server thread is return
    closesocket_(HttpSocket);
	HttpSocket = 0;
    //free windows socket api
    WSACleanup_();
    //unmapping socket module
    FreeLibrary(SocketModule);

    printf("[+] wait for thread..\n");
    /*
    wait for server thread return
    WaitForSingleObject function is blocking function 

    first argument is server thread handle
    second argument is wait time
    */
    WaitForSingleObject(hHttpServer, INFINITE);

    return 0;
}
