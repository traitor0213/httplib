
int StrToHex(const char *s)
{
    int total = 0;

    int r = 0;
    int m = 0;

    int length = 0;
    for(; s[length] != 0; length+=1);

    int t = 0;

    for(int i = length - 1; i >= 0; i -= 1)
    {
        t = s[i];

        if(t >= 'a' && t <= 'f')
        {
            t -= 32 + 7;
        }
        
        if(t >= 'A' && t <= 'F')
        {
            t -= 7;
        }

        t -= 48;

        if(t * m == 0)
        {
            r = t + m;
        }
        else 
        {
            r = t * m;
        }

        total += r;

        if(m == 0)
        {
            m = 16;
        }
        else 
        {
            m *= 16;
        }
    }

    return total; 
}

int DecodeUrl(char *buffer, int size, const char *s)
{
    const int HexRange = 2;
    const int Format = '%';

    char hex[2 + 1];

    int x = 0;

    int r = 0;
    for(; s[r] != 0 && r <= size; r += 1)
    {
        if(s[r] == Format)
        {
            for(int i = 0; i != HexRange; i+=1)
            {
                hex[i] = s[r + i + 1];
            }
            hex[HexRange] = 0;

            int c = StrToHex(hex);
            r += 2;

            buffer[x] = c;
            buffer[x + 1] = 0;
            x += 1;
        }
        else 
        {
            buffer[x] = s[r];
            buffer[x + 1] = 0;

            x += 1;
        }
    }

    int k = 0;
    for(k = 0; buffer[k] != 0; k++);

    return k;
}
