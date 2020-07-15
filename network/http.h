#ifndef HTTP_H
#define HTTP_H

#include "./tcp.h"
#include "./str.h"

#define HTTP_LINE_SIZE 1024

int RecvLine(int hSocket, char *buffer, int len)
{
    int index = 0;
    for(;;)
    {
        if(index >= len)
        {
            index = SOCKET_ERROR;
            break;
        }

        if(recv_(hSocket, (char *)&buffer[index], sizeof(buffer[index]), 0) == SOCKET_ERROR)
        {
            if(WSAGetLastError_() != WSAEWOULDBLOCK)
            {
				index = SOCKET_ERROR;

                break;
            }
        }
        else 
        {
            if(buffer[index] == '\n')
            {
                break;
            }

            index++;
        }
    }

    return index;
}

int CreateHttpRaw(char* buffer, int size, const char* method, const char* headers, ...)
{
	ZeroMemory(buffer, size);

	int MethodStringLength = strlen(method);
	if (MethodStringLength > size)
	{
		return 0;
	}

	const char* Crlf = "\r\n";
	const int CrlfLength = GetStringLength(Crlf);

	memcpy(buffer, method, MethodStringLength);
	
	const char** parameters = &headers;

	const char* ContentLengthHeader = "Content-Length";
	const char* ContentHeader = "Content";

	const char* Colon = ":";
	const int ColonLength = GetStringLength(Colon);

	const char* Space = " ";
	const int SpaceLength = GetStringLength(Space);

	int ContentLength = 0;
	int BufferLength = 0;

	for (;;)
	{
		if (StringCompare(*parameters, ContentHeader) == 0)
		{
			memcpy(buffer + GetStringLength(buffer), Crlf, CrlfLength + 1);
			memcpy(buffer + GetStringLength(buffer), Crlf, CrlfLength + 1);

            BufferLength = GetStringLength(buffer);

            if (BufferLength >= size || BufferLength == 0 || ContentLength >= size)
	        {
		        return 0;   
	        }

			if (ContentLength > 0)
			{
				memcpy(buffer + BufferLength, *(parameters + 1), ContentLength);
			}

			break;
		}

		if (StringCompare(*parameters, ContentLengthHeader) == 0)
		{
			ContentLength = (int)*(parameters + 1);

			_sprintf(buffer, size, "%s\r\n%s%s%s%d",
				buffer,
				*parameters, Colon, Space, (int)*(parameters + 1));
		}
		else
		{
			_sprintf(buffer, size, "%s\r\n%s%s%s%s",
				buffer,
				*parameters, Colon, Space, *(parameters + 1));
		}

		parameters += 2;
	}

	return ContentLength + BufferLength;
}

int ReadFirstHttpLine(SOCKET hSocket, char *lpBuffer, int BufferSize)
{
    int RecvedLineIndexLocation = RecvLine(hSocket, lpBuffer, BufferSize);
    if(RecvedLineIndexLocation <= 1) return -1;

    lpBuffer[RecvedLineIndexLocation] = 0;

    return 0;
}

//return request content return buffer from malloc function
LPVOID ReadHttpContent(SOCKET hSocket)
{
    unsigned int ContentLength = 0;

    char HttpLineBuffer[HTTP_LINE_SIZE];
    int RecvedLineIndexLocation = 0;

    for(;;)
    {
        RecvedLineIndexLocation = RecvLine(hSocket, HttpLineBuffer, sizeof(HttpLineBuffer));
        if(RecvedLineIndexLocation <= 1) break;
        HttpLineBuffer[RecvedLineIndexLocation] = 0;
        
        if(FindString(HttpLineBuffer, "Content-Length") != NULL)
        {
            for(int index = 15; ContentLength != 0; index ++)
            {
                //2,147,483,647
                if(index == 15 + 10) break;

                if( HttpLineBuffer[index] == '\r' || 
                    HttpLineBuffer[index] == '\n' ||
                    HttpLineBuffer[index] == 0) 
                {
                    break;
                }


                ContentLength = atoi(HttpLineBuffer + index);
            }
        }
    }

    if(RecvedLineIndexLocation == SOCKET_ERROR) return NULL;

    char *lpContentBuffer = NULL;

    if(ContentLength != 0) 
    {
        lpContentBuffer = (char *)malloc(ContentLength);
        if(lpContentBuffer == NULL) 
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return NULL;
        }
    }

    recv_(hSocket, lpContentBuffer, ContentLength, 0);

    return lpContentBuffer;
}

//free allocated from malloc function
void FreeHttpContent(LPVOID lpBuffer)
{
    free(lpBuffer);
}

void CloseHttpSession(SOCKET hSocket)
{
	shutdown_(hSocket, SD_BOTH);
	closesocket_(hSocket);
}

int WriteHttpRaw(SOCKET hSocket, const char *lpHttpRaw, unsigned int HttpRawSize)
{
    return send_(hSocket, lpHttpRaw, HttpRawSize, 0);
}

int GetPathFromHttpMethod(const char *lpFirstLineOfHttpHeader, char *RequestedPath, int PathSize)
{
    //lpFirstLineOfHttpHeader

    int StringLength = GetStringLength(lpFirstLineOfHttpHeader);
    if (StringLength == 0)
        return -1;

    int FirstSpace = GetChrLocation(lpFirstLineOfHttpHeader, ' ');
    int SecondSpace = GetChrLocation(lpFirstLineOfHttpHeader + FirstSpace, ' ');

    char DecodedUrlString[HTTP_LINE_SIZE];
    int r = DecodeUrl(DecodedUrlString, sizeof(DecodedUrlString), lpFirstLineOfHttpHeader);
    
    int x = 0;
    for(x = r; x != 0; x--) 
        if(DecodedUrlString[x] == ' ')
            break;
    DecodedUrlString[x] = 0;

    memcpy(RequestedPath, DecodedUrlString + FirstSpace, r);

    return 0;
}

int SeparateParamFromHttpMethod(char *HttpPath, char *RequestedParam, int ParamSize)
{
    //lpFirstLineOfHttpHeader

    int StringLength = GetStringLength(HttpPath);
    if (StringLength == 0)
        return -1;

    int Ask = GetChrLocation(HttpPath, '?');
    if(Ask == -1) 
        return -1;

    if (Ask == StringLength)
        return -1;

    HttpPath[Ask - 1] = 0;
    HttpPath += Ask;

    RequestedParam[0] = 0;
    memcpy(RequestedParam, HttpPath, StringLength - Ask);
    RequestedParam[StringLength - Ask] = 0;

    return 0;
}


void GetHttpParamMember(const char* Param, const char* MemberName, char* MemberRaw, int MemberSize)
{
    int MemberNameLength = GetStringLength(MemberName);
    int ParamLength = GetStringLength(Param);

    const char* ptr = Kmp(Param, ParamLength, MemberName, MemberNameLength);
    if (ptr == NULL) return;

    ptr += MemberNameLength;

    for (int i = 0; i != ParamLength; i++)
    {
        if (MemberSize == i + 1) break;

        MemberRaw[i] = ptr[i];

        if (ptr[i] == '&')
        {
            MemberRaw[i] = 0;
            break;
        }
    }

    return;
}


#endif