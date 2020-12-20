#define WIN32_LEAN_AND_MEAN

#include "../url/url.h"
#include "../native/wrap.h"
#include "../network/http.h"

/*
server thread
*/

void server(int *fd)
{
    SOCKADDR_IN info = {
        0,
    };
    
    const char *Crlf = "\r\n\r\n";
    const int CrlfLength = GetStringLength(Crlf);

    const char *Space = " ";
    const char SpaceWord = ' ';
    const int SpaceLength = GetStringLength(Space);

    const char *Parameter = "?";
    const char ParameterWord = '?';
    const int ParameterLength = GetStringLength(Parameter);

    const char *And = "*&";
    const char AndWord = '&';
    const int AndLength = GetStringLength(And);

    const char *FullPathParameter = "path=";
    const int FullPathParameterLength = GetStringLength(FullPathParameter);

    //http header
    char header[4096];

    //request buffer
    char EncodeString[1024];
    char DecodeString[1024];

    char RequestPath[1024];
    char RequestParameters[1024];

    char temp[1024];
    char OtherPath[1024];
    char RootPath[1024];
    char RootDrive[3];

    char ext[_MAX_EXT];

    //accept loop
    for (;;)
    {
        Sleep(1);

        int ResponseSocket = AcceptRequest(&info, *fd);
        if (ResponseSocket < 0)
        {
            //case of socket error

            if (WSAGetLastError_() != WSAEWOULDBLOCK)
            {
                //thread exit
                break;
            }
        }
        else
        {
            ZeroMemory(RequestPath, sizeof(RequestPath));
            ZeroMemory(RequestParameters, sizeof(RequestParameters));

            /*
            Recv first line of request header
            
            (example request header)
            GET /windows/system32/cmd.exe?download HTTP/1.1\r\n
            User-Agent: ...
            */

            //recv
            ZeroMemory(header, sizeof(header));
            RecvLine(ResponseSocket, header, sizeof(header));

            printf("\n\n\n");
            printf("%s\n", header);

            /*
            find ' '(Space)

            before              = " /windows/system32/cmd.exe?download HTTP/1.1\r\n"
            after(EncodeString) = "/windows/system32/cmd.exe?download HTTP/1.1\r\n"
            */

            int r = SeparateString(EncodeString, sizeof(EncodeString), header, " ");
            if(r == -1)
            {
                goto RESPONSE;
            }


            /*
            find ' '(Space)

            before              = "/windows/system32/cmd.exe?download HTTP/1.1\r\n"
            after(EncodeString) = "/windows/system32/cmd.exe?download"
            */

            int EncodeStringLength = GetStringLength(EncodeString);
            
            for(int x = 0; x <= EncodeStringLength && x <= sizeof(EncodeString); x += 1)
            {
                if(EncodeString[x] == 0) break;

                if(EncodeString[x] == SpaceWord) 
                {
                    EncodeString[x] = 0;
                    break;
                }
            }

            /*
            replace url encode to url decode

            before                  = "/users/hello%20world?dir"
            after(DecodeString)     = "/users/hello world?dir"
            */

            DecodeUrl(DecodeString, sizeof(DecodeString), EncodeString);

            /*
            find request path

            before = "/windows/system32/cmd.exe?download"
            after(RequestPath) = "/windows/system32/cmd.exe"
            */

            int DecodeStringLength = GetStringLength(DecodeString);
            
            for(int x = 0; x <= DecodeStringLength && x <= sizeof(DecodeString); x += 1)
            {
                if(DecodeString[x] == 0) break;
                if(DecodeString[x] == ParameterWord)
                {
                    RequestPath[x] = 0;
                    break;
                }
                
                RequestPath[x] = DecodeString[x];
            }
        
            /*
            find request path

            before = "/windows/system32/cmd.exe?download"
            after(RequestParameters) = "?download"
            */
            
            int i = 0;
            for(int x = 0; x <= DecodeStringLength && x <= sizeof(RequestParameters); x += 1)
            {
                if(DecodeString[x] == 0) break;

                if(DecodeString[x - 1] == ParameterWord)
                {
                    for(int y = 0; ; y += 1)
                    {
                        RequestParameters[i] = DecodeString[x + y];
                        if(DecodeString[x + y] == 0) break;
                        i += 1;
                    }
                }
            }

            /*
            find 'path' command

            before = "path=C:/windows/system32/cmd.exe&value=1234..."
            after  = "C:/windows/system32/cmd.exe"
            */
            
            int RequestParametersLength = GetStringLength(RequestParameters);
            const char *FullPath = Kmp(RequestParameters, RequestParametersLength, FullPathParameter, FullPathParameterLength);
            
            int AndLocation = GetStringLength(FullPath);
            if(FullPath != NULL)
            {
                FullPath += FullPathParameterLength;

                for(int x = 0; ; x += 1)
                {
                    if(FullPath[x] == 0) break;

                    if(FullPath[x] == AndWord)
                    {
                        if (x > 0)
                        {
                            AndLocation = x;
                        }

                        break;
                    }
                }

                memcpy(RequestPath, FullPath, AndLocation);

            }
            

            //read other http request header
            for (;;)
            {
                //fill zero
                ZeroMemory(header, sizeof(header));

                //RecvLine function is return index of line feed ( CR 'LF' )
                int CrlfIndex = RecvLine(ResponseSocket, header, sizeof(header));

                if (CrlfIndex < 2)
                {
                    /*
                    CRLF or LF is first word
                    situation of index is 0, 1 or error
                    */

                    break;
                }
            }

RESPONSE:;

            if(Kmp(RequestPath, sizeof(RequestPath), "index.html", 11) == NULL) 
            {
                sprintf(OtherPath, "%s%s", RootPath, RequestPath);
                memcpy(RequestPath, OtherPath, sizeof(OtherPath));

                printf("Open file = %s\n", RequestPath);
            }

            char *ResponseContent = NULL;
            DWORD ResponseContentSize = 0;

            int IsCreateFileError = FALSE;

            HANDLE hRequestFile = CreateFile(RequestPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if(hRequestFile != INVALID_HANDLE_VALUE)
            { 
                ResponseContentSize = GetFileSize(hRequestFile, NULL);
                ResponseContent = (char *)GlobalAlloc(GPTR, ResponseContentSize);

                DWORD ReadBuffer = 0;

                ReadFile(hRequestFile, ResponseContent, ResponseContentSize, &ReadBuffer, NULL);
                CloseHandle(hRequestFile);          
            }
            else 
            {
                //request file is not exist
                IsCreateFileError = TRUE;
            }

            const int DefualtSize = 1024;
            char *response = GlobalAlloc(GPTR, DefualtSize + ResponseContentSize);
            if(response != NULL) 
            {
                CreateHttpRaw(response, DefualtSize + ResponseContentSize,
                        "HTTP/1.1 200 OK",
                        "Content-Disposition", "inline",
                        "Content-Length", ResponseContentSize,
                        "Content", ResponseContent);

                const char *ptr = Kmp(response, DefualtSize, Crlf, CrlfLength);
                const int ResponseLength = (ptr - response) + ResponseContentSize + CrlfLength;

                send_(ResponseSocket, response, ResponseLength, 0);

                if(IsCreateFileError == FALSE)
                {
                    GlobalFree(ResponseContent);
                }

                GlobalFree(response);
            }

            //end of connection
            shutdown_(ResponseSocket, SD_BOTH);
            closesocket_(ResponseSocket);

            printf("Request File = '%s'\n", RequestPath);
            printf("Request Params = '%s'\n", RequestParameters);
            
            if(Kmp(RequestPath, sizeof(RequestPath), "index.html", 11) != NULL) 
            {
                _splitpath(RequestPath, RootDrive, temp, NULL, NULL);

                sprintf(RootPath, "%s%s", RootDrive, temp);

                printf("Setting Root = %s\n", RootPath);
            }
        }
    }
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

    /*
    This WSAAPI function is using for initialize windows socket api module
    
    first argument is version number
    second argument is address of WSADATA variable
    */
    printf("[+] startup windows socket api\n");
    WSADATA wsa;
    WSAStartup_(MAKEWORD(2, 2), &wsa);

    /*
    first argument is port number, (using for bind function)
    second argument is backlog, (using for listen function)
    third argument is blocking mode

    return value is opened socket 
    */

    const int Port = 8080;
    const int Backlog = 1024;
    printf("[+] open port=%d; backlog=%d\n", Port, Backlog);
    int ServerSocket = OpenTcpPort(Port, Backlog, TRUE);

    /*
    error check
    if socket value is small than zero, error case
    */

    if (ServerSocket < 0)
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

        if (ServerSocket == ERROR_FROM_BIND || ServerSocket == ERROR_FROM_LISTEN)
        {
            printf("error from port open\n");
            closesocket_(ServerSocket);
        }

        WSACleanup_();
        return 0;

        //end of program
    }

    /*
    create server thread
    'hServer' variable is server thread handle 
    */

    printf("[+] create server thread..\n");
    HANDLE hServer = CreateThread_((FUNCTION_ADDRESS)server, (LPVOID)&ServerSocket);

    //user shell
    char input[126];
    for (;;)
    {
        Sleep(0);
        //get stdin stream
        fgets(input, sizeof(input) - 1, stdin);

        //separate CRLF
        strtok(input, "\r\n");

        //check 'exit' command
        if (stricmp(input, "exit") == 0)
        {
            //exit server
            break;
        }
    }

    //if called closesocket function, server thread is return
    closesocket_(ServerSocket);
    //free windows socket api
    WSACleanup_();
    //unmapping socket module
    FreeLibrary(SocketModule);

    /*
    wait for server thread return
    WaitForSingleObject function is blocking function 

    first argument is server thread handle
    second argument is wait time
    */

    WaitForSingleObject(hServer, INFINITE);

    //end of program

    return 0;
}
