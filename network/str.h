
#ifndef STR_H
#define STR_H

#include <stdio.h>
#include <Windows.h>

int _log10(int n)
{
	int r = 0;

	for (;;)
	{
		if (n / 10 == 0) break;

		n /= 10;
		r += 1;
	}

	return r;
}

//get string length
int GetStringLength(const char* s)
{
	if (s == NULL) return 0;

	int r = 0;
	for (; *s != 0; s++) r++;

	return r;
}

//string compare (not affect uppercase and lowercase)
int StringCompare(const char* s1, const char* s2)
{
	int f = 0;

	int t1 = 0;
	int t2 = 0;

	if (s1 == NULL || s2 == NULL) return 1;

	for (; *s1 != 0; s1++)
	{
		if (*s2 == 0)
		{
			f = 1;
			break;
		}

		//next word
		t1 = *s1;
		t2 = *s2++;

		//alphabet uppercase to lowercase
		//example: A to a, B to b ...

		if (t1 >= 'A' && t1 <= 'Z')
		{
			t1 += 32;
		}

		if (t2 >= 'A' && t2 <= 'Z')
		{
			t2 += 32;
		}

		//compare word
		if (t1 != t2)
		{
			f = 1;
			break;
		}
	}

	return f;
}

//"1234" to 1234
int StringToDecimal(const char* s)
{
	int scope = 1;
	BYTE Array[10];

	int DecimalLength = 0;
	for (;;)
	{
		if ( (s[DecimalLength] >= '0' && s[DecimalLength] <= '9') == FALSE)
		{
			break;
		}

		DecimalLength++;
	}

	int r = 0;
	int j = 1;
	for (int i = 0; i != DecimalLength; i++) j *= 10;

	for (int i = 0; i <= DecimalLength - 1; i++)
	{
		r += (s[i] - '0') * j;
		j /= 10;
	}
	
	return r / 10;
}

//1234 to "1234"
int DecimalToString(char* buf, int dec)
{
	/*
	log10(12345) = 4
	log10(1234) = 3
	log10(123) = 2
	...
	*/

	int len = (int)_log10(dec) + 1;

	int a = len - 1;
	int i = 0;
	for (; i != len; i++)
	{
		int temp = dec % 10;
		dec /= 10;

		buf[a--] = temp + 0x30;
	}

	buf[i] = 0;

	return len;
}


int GetChrLocation(const char* String, char chr)
{
	int StringLength = GetStringLength(String);

	int b = 0;
	int i = 0;
	for (; i != StringLength; i++)
	{
		if (String[i] == chr)
		{
			return i + 1;
		}
	}

	return -1;
}

//kmp algorithm
const char* Kmp(const char* Source, int SourceSize, const char* Destination, int DestinationSize)
{
	if (DestinationSize > SourceSize)
	{
		return NULL;
	}

	int IsError = 0;
	const char* ret = NULL;

	for (int x = 0; x != SourceSize; x++)
	{
		if (x + DestinationSize > SourceSize)
		{
			ret = NULL;
			break;
		}

		IsError = 0;

		for (int y = 0; y != DestinationSize; y++)
		{
			if (Source[y + x] != Destination[y])
			{
				IsError = 1;
				break;
			}
		}

		if (IsError == 0)
		{
			ret = &Source[x];
			break;
		}
	}

	return ret;
}

const char* FindString(const char* Source, const char* Destination)
{
	int SourceSize = GetStringLength(Source);
	int DestinationSize = GetStringLength(Destination);

	if (DestinationSize > SourceSize)
	{
		return NULL;
	}

	int IsError = 0;
	const char* ret = NULL;

	for (int x = 0; x != SourceSize; x++)
	{
		if (x + DestinationSize > SourceSize)
		{
			ret = NULL;
			break;
		}

		IsError = 0;

		for (int y = 0; y != DestinationSize; y++)
		{
			if (Source[y + x] != Destination[y])
			{
				IsError = 1;
				break;
			}
		}

		if (IsError == 0)
		{
			ret = &Source[x];
			break;
		}
	}

	return ret;
}

int SeparateString(char* buffer, int size, const char* s, const char* f)
{
	int r = 0;
	int FindLength = GetStringLength(f);

	const char* p = Kmp(s, GetStringLength(s), f, FindLength);

	if (p != NULL)
	{
		r = p - s;

		int l = GetStringLength(s + r);

		for (int i = 0; i != l; i += 1)
		{
			buffer[i] = s[r + i + FindLength];
		}
		buffer[l] = 0;
	}
	else
	{
		r = -1;
		buffer[0] = 0;
	}

	return r;
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
				if (format[r] != '%')
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


BOOL FindIntData(int *f, const char *HttpLineBuffer, const char *Find)
{
    BOOL data = FALSE;

    if (FindString(HttpLineBuffer, Find) != NULL)
    {
        int length = GetStringLength(Find);

        int index = length;
        for (;; index++)
        {
            //2,147,483,647
            if (index == length + 10)
                break;

            if (HttpLineBuffer[index] == '\r' ||
                HttpLineBuffer[index] == '\n' ||
                HttpLineBuffer[index] == 0)
            {
                break;
            }

            *f = StringToDecimal(HttpLineBuffer + index);
            if (*f != 0)
            {
                data = TRUE;
                break;
            }
        }
    }

    return data;
}

#endif