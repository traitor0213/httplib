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

//copycat of sprintf in stdlib.h
void _sprintf(char* buffer, int size, const char* format, ...)
{
	const char** args = &format;
	int FormatLength = GetStringLength(format);

	int CopyIndex = 0;

	int f = 0;

	for (int r = 0; r != size; r++)
	{
		if (format[r] == 0) break;

		if (r > 0)
		{
			if (format[r - 1] == '%')
			{
				//%%
				if (format[r] == '%')
				{
					buffer[f] = format[r];
					buffer[f + 1] = 0;
					f++;
				}
				
				//%~
				if(format[r] != '%')
				{
					if (format[r] == 'd')
					{
						args += 1;

						int length = DecimalToString(buffer + f, (int)*args);
						f += length;

						buffer[f + 1] = 0;
					}

					if (format[r] == 's')
					{
						args += 1;

						int length = GetStringLength(*args);

						memcpy(buffer + f, *args, length);
						f += length;
						buffer[f] = 0;
					}
				}
			}

			if (format[r - 1] != '%' && format[r] != '%')
			{
				buffer[f] = format[r];
				buffer[f + 1] = 0;
				f++;
			}
		}
		else
		{
			if (format[r] != '%')
			{
				buffer[f] = format[r];
				buffer[f + 1] = 0;
				f++;
			}
		}
	}
}

int warraper(char* buffer, int size, const char* method, const char* headers, ...)
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
	buffer += MethodStringLength;

	const char** parameters = &headers;

	const char* ContentLengthHeader = "Content-Length";
	const char* EndOfHeaders = "Content";

	const char* Colon = ":";
	const int ColonLength = GetStringLength(Colon);

	const char* Space = " ";
	const int SpaceLength = GetStringLength(Space);

	int ContentLength = 0;

	int p = 0;

	int HeaderLength = 0;

	for (;;)
	{
		if (StringCompare(*parameters, ContentLengthHeader) == 0)
		{
			//buffer += r;

			ContentLength = (int)* (parameters + 1);

			_sprintf(buffer, size, "%s\r\n%s%s%s%d", buffer, *parameters, Colon, Space, (int)*(parameters + 1));

			parameters += 2;
		}
		else 
		{
			_sprintf(buffer, size, "%s\r\n%s%s%s%s", buffer, *parameters, Colon, Space, * (parameters + 1));
			parameters += 2;
		}

		if (StringCompare(*parameters, EndOfHeaders) == 0)
		{
			memcpy(buffer + GetStringLength(buffer), Crlf, CrlfLength + 1);
			memcpy(buffer + GetStringLength(buffer), Crlf, CrlfLength + 1);
			
			HeaderLength = GetStringLength(buffer);

			if(ContentLength != 0)
			{
				parameters += 1;
				memcpy(buffer + HeaderLength, *parameters, ContentLength);
			}
			
			break;
		}
		
		p += 1;
	}

	return HeaderLength;
}