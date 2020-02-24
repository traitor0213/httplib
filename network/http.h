#ifndef HTTP_H
#define HTTP_H

#include "./tcp.h"
#include "./str.h"

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

			if (ContentLength > 0)
			{
				memcpy(buffer + GetStringLength(buffer), *(parameters + 1), ContentLength);
			}

			for(; buffer[BufferLength] != 0; BufferLength++);

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

#endif