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

	for (; *s1 != 0; s1++)
	{
		if (*s2 == 0) break;
		
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
	int i = 0;

	int r = 0;
	int scope = 1;

	i = 0;
	for (;;)
	{
		if (s[i] >= '0' && s[i] <= '9')
		{
			r += (s[i] - '0') * scope;
		}
		else
		{
			break;
		}

		scope *= 10;

		i++;
	}

	//if argument = 1024, 'r' value is 4021
	int l = 0;
	int x = (int)_log10(r) + 1;
	scope = 10;

	int z = 1;

	//4021 to 1204

	for (int y = 1; y != x; y++)
	{
		scope *= 10;
	}

	for (; x != 0; x--)
	{
		scope /= 10;
		l += r / scope * z;
		r %= scope;
		z *= 10;
	}

	return l;
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

//kmp algorithm
const char* Kmp(const char* Source, int SourceSize, const char* Destination, int DestinationSize)
{
	if(DestinationSize > SourceSize)
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


int SeparateString(char *buffer, int size, const char* s, const char *f)
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