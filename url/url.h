#undef UNICODE
#include <windows.h>

#define TRUE 1
#define FALSE 0

int IsUrlReservedCharacter(const char Source)
{
    int A = 'A';
    int Z = 'Z';

    int a = 'a';
    int z = 'z';

    int WhiteList[] = {'/', '\\', '&', ';'};
    int CountOfWhiteList = sizeof(WhiteList) / sizeof(WhiteList);

    int flag = FALSE;

    if (A <= Source && Z >= Source)
    {
        flag = TRUE;
    }

    if (a <= Source && z >= Source)
    {
        flag = TRUE;
    }

    for (int j = 0; j != CountOfWhiteList; j++)
    {
        if (Source == WhiteList[j])
        {
            flag = TRUE;
            break;
        }
    }

    return flag;
}

#include "./decode.c"
#include "./encode.c"
