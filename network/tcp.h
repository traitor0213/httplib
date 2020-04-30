
#ifndef TCP_H
#define TCP_H

#undef UNICDOE

#include <stdio.h>
#include <stdlib.h>

//errorcode
#define ERROR_FROM_SOCKET 0x1
#define ERROR_FROM_BIND 0x2
#define ERROR_FROM_LISTEN 0x3
#define ERROR_FROM_ACCEPT 0x4
#define ERROR_FROM_CONNECT 0x5
#define ERROR_FROM_IO 0x6
//only use windows
#define ERROR_FROM_INIT 0x7

//windows flatform
#ifdef _WIN32 

//windows socket api module name
#define SOCKET_MODULE_NAME "WS2_32.DLL"

//windows header
#include <windows.h>
#include <WinSock2.h>

//get export function in the module
LPVOID GetExportAddress(const CHAR *ModuleName, const CHAR *ExportName)
{
    HMODULE Library = NULL;

    Library = GetModuleHandle(ModuleName);
    if (Library == NULL)
    {
        Library = LoadLibrary(ModuleName);
    }

    if (Library == NULL)
    {
        return NULL;
    }

    return GetProcAddress(Library, ExportName);
}

#undef SOCKET
#define SOCKET unsigned int

//function types
typedef int (WSAAPI *_WSAStartup)(WORD, LPWSADATA);
typedef int (WSAAPI *_WSACleanup)();
typedef int (WSAAPI *_WSAGetLastError)();

typedef int (WSAAPI *_socket)(int, int, int);
typedef int (WSAAPI *_ioctlsocket) (SOCKET, long, u_long *);
typedef int (WSAAPI *_shutdown) (SOCKET, int);
typedef int (WSAAPI *_closesocket)(SOCKET);

typedef int (WSAAPI *_connect) (SOCKET, SOCKADDR*, int);

typedef int (WSAAPI *_bind) (SOCKET, SOCKADDR*, int);
typedef int (WSAAPI *_listen) (SOCKET, int);

typedef int (WSAAPI *_accept) (SOCKET, SOCKADDR*, int *);

typedef int (WSAAPI *_recv) (
    IN SOCKET s,
    char FAR * buf,
    IN int len,
    IN int flags
    );

typedef int (WSAAPI *_send) (
    IN SOCKET s,
    const char FAR * buf,
    IN int len,
    IN int flags
    );

typedef unsigned long (PASCAL *_inet_addr) (const char *);
typedef struct hostent* (PASCAL *_gethostbyname) (const char *);

//global function
_WSAStartup WSAStartup_;
_WSACleanup WSACleanup_;
_WSAGetLastError WSAGetLastError_;

_socket socket_;
_ioctlsocket ioctlsocket_;
_shutdown shutdown_;
_closesocket closesocket_;

_connect connect_;

_bind bind_;
_listen listen_;
_accept accept_;

_recv recv_;
_send send_;

_inet_addr inet_addr_; 
_gethostbyname gethostbyname_;

//linux flatform
#else

//linux socket header
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

//define for universal code
#define socket_ socket
#define ioctlsocket_ ioctlsocket;
#define shutdown_ shutdown
#define closesocket_ close

#define connect_ connect

#define bind_ bind
#define listen_ listen
#define accept_ accept

#define recv_ recv
#define send_ send

#define SOCKET_ERROR -1

#endif


int htonl_(int h);
//little-endian to big-endian
unsigned short htons_(unsigned short x);

int CreateTcpSocket();
int OpenTcpPort(unsigned short Port, int BackLog, int Flag);
int AcceptRequest(SOCKADDR_IN *AcceptInfo, int OpenSocket);

//htonl re-creation

/*
11223344 to 44332211
*/
int htonl_(int h)
{
    int r = 0;

    int MainIndex = 0;
    int SubIndex = sizeof(h);

    int i = 0;

    for (;SubIndex != 0;)
    {
        ((unsigned char*)&r)[--SubIndex] = ((unsigned char*)&h)[i++];
    }

    return r;
}

//htons re-creation
/*
1122 to 2211
*/
unsigned short htons_(unsigned short x)
{
    unsigned short r = 0;
    int l = sizeof(r) - 1;
    int i = 0;

    for (;;)
    {
        ((unsigned char *)&r)[l--] = ((unsigned char *)&x)[i++];

        if (i == sizeof(x))
        {
            break;
        }
    }

    return r;
}

//get windows socket api function
LPVOID InitializeTcpLibrary()
{
    #ifdef WIN32

    WSAStartup_ = (_WSAStartup)GetExportAddress(SOCKET_MODULE_NAME, "WSAStartup");
    WSACleanup_ = (_WSACleanup)GetExportAddress(SOCKET_MODULE_NAME, "WSACleanup");
    WSAGetLastError_ = (_WSAGetLastError)GetExportAddress(SOCKET_MODULE_NAME, "WSAGetLastError");

    socket_ = (_socket)GetExportAddress(SOCKET_MODULE_NAME, "socket");
    ioctlsocket_ = (_ioctlsocket)GetExportAddress(SOCKET_MODULE_NAME, "ioctlsocket");
    shutdown_ = (_shutdown)GetExportAddress(SOCKET_MODULE_NAME, "shutdown");
    closesocket_ = (_closesocket)GetExportAddress(SOCKET_MODULE_NAME, "closesocket");

    connect_ = (_connect)GetExportAddress(SOCKET_MODULE_NAME, "connect");

    bind_ = (_bind)GetExportAddress(SOCKET_MODULE_NAME, "bind");
    listen_ = (_listen)GetExportAddress(SOCKET_MODULE_NAME, "listen");
    accept_ = (_accept)GetExportAddress(SOCKET_MODULE_NAME, "accept");

    recv_ = (_recv)GetExportAddress(SOCKET_MODULE_NAME, "recv");
    send_ = (_send)GetExportAddress(SOCKET_MODULE_NAME, "send");

    inet_addr_ = (_inet_addr)GetExportAddress(SOCKET_MODULE_NAME, "inet_addr");
    gethostbyname_ = (_gethostbyname)GetExportAddress(SOCKET_MODULE_NAME, "gethostbyname");

    
    return GetModuleHandleA(SOCKET_MODULE_NAME);

    #else 
    return NULL;

    #endif
}

int SetNonBlockingMode(SOCKET hSocket, BOOL NonBlockingFg)
{
    return ioctlsocket_(hSocket, FIONBIO, (DWORD*)&NonBlockingFg);
}

int CreateTcpSocket()
{   
    return socket_(PF_INET, SOCK_STREAM, 0);
}

int OpenTcpPort(unsigned short Port, int BackLog, int IoCtlFlag)
{
    int fd = CreateTcpSocket();
    if (fd == SOCKET_ERROR)
    {
        return ERROR_FROM_SOCKET;
    }

    if(IoCtlFlag == TRUE)
    {
        if(SetNonBlockingMode(fd, TRUE) == SOCKET_ERROR)
        {
            closesocket_(fd);
            return ERROR_FROM_SOCKET;
        }
    }

    SOCKADDR_IN info;
    info.sin_addr.s_addr = htonl_(INADDR_ANY);
    info.sin_port = htons_(Port);
    info.sin_family = AF_INET;

    int SocketError = 0;

    SocketError = bind_(fd, (SOCKADDR *)&info, sizeof(info));
    if (SocketError != FALSE)
    {
        return ERROR_FROM_BIND;
    }

    SocketError = listen_(fd, BackLog);
    if (SocketError != FALSE)
    {
        return ERROR_FROM_LISTEN;
    }

    return fd;
}

int AcceptRequest(SOCKADDR_IN *AcceptInfo, int OpenSocket)
{
    int size = sizeof(SOCKADDR_IN);
    return accept_(OpenSocket, (SOCKADDR *)AcceptInfo, &size);
}


SOCKADDR_IN _GetHostName(const char *name)
{
    HOSTENT *myent = NULL;
    struct in_addr myen;
    int *add;

    SOCKADDR_IN info = {
        0,
    };

    myent = (HOSTENT*)gethostbyname_(name);
    if (myent == NULL)
    {
        return info;
    }
    else
    {
        while (*myent->h_addr_list != NULL)
        {
            add = (int *)*myent->h_addr_list;
            myen.s_addr = *add;

            info.sin_addr = myen;

            myent->h_addr_list++;
        }
    }

    return info;
}

int Connecter(SOCKET hSocket, const char *ip, const char *port)
{
    SOCKADDR_IN AccepterInfo = {
        0,
    };

    if (inet_addr_(ip) != -1)
    {
        AccepterInfo.sin_addr.s_addr = inet_addr_(ip);
    }
    else
    {
        AccepterInfo = _GetHostName(ip);
    }

    AccepterInfo.sin_family = AF_INET;
    AccepterInfo.sin_port = htons_(atoi(port));

    if (connect_(hSocket, (SOCKADDR *)&AccepterInfo, sizeof(AccepterInfo)) == SOCKET_ERROR)
    {
        return SOCKET_ERROR;
    }
    else
    {
        return TRUE;
    }
}


int Recv(SOCKET hSocket, LPVOID Buff, int ReqSize)
{
    int AccAddBytes = 0, RecvBytes;

    for (;;)
    {
        if ((RecvBytes = recv_(hSocket, (LPSTR)Buff + AccAddBytes, ReqSize - AccAddBytes, 0)) > 0)
        {
            AccAddBytes += RecvBytes;
            if (AccAddBytes >= ReqSize)
                return 0;
        }
        else
        {
            int r = WSAGetLastError_();

            if (r != WSAEWOULDBLOCK)
            {
                AccAddBytes = SOCKET_ERROR;
                break;
            }
            Sleep(1);
        }
    }

    return AccAddBytes;
}


int Send(SOCKET hSocket, LPCVOID buffer, int ReqSize)
{
    int AccAddBytes = 0, SendBytes;

    for (;;)
    {
        if ((SendBytes = send_(hSocket, (LPSTR)buffer + AccAddBytes, ReqSize - AccAddBytes, 0)) > 0)
        {
            AccAddBytes += SendBytes;
            if (AccAddBytes >= ReqSize)
                return 0;
        }
        else
        {
            int r = WSAGetLastError_();

            if (r != WSAEWOULDBLOCK)
            {
                AccAddBytes = SOCKET_ERROR;
                break;
            }
            Sleep(1);
        }
    }

    return AccAddBytes;
}

#endif