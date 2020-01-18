int EncodeUrl(char *Destination, int SizeOfDestination, const char *Source)
{
    int SourceLength = lstrlen(Source);
    int DestinationLength = 0;

    char HexString[32];

    int SourceIndex = 0;

    int error = FALSE;

    if (SourceLength == 0)
    {
        error = TRUE;
        return error;
    }

    for (; SourceIndex <= SourceLength - 1; SourceIndex++)
    {
        if (IsUrlReservedCharacter(Source[SourceIndex]) == TRUE)
        {
            if (SizeOfDestination <= DestinationLength)
            {
                SetLastError(ERROR_BUFFER_OVERFLOW);
                error = TRUE;
                break;
            }

            Destination[DestinationLength++] = Source[SourceIndex];
        }
        else
        {
            wsprintf(HexString, "%%%02X", Source[SourceIndex]);
            int HexStringLength = lstrlen(HexString);

            if (SizeOfDestination <= DestinationLength + HexStringLength)
            {
                SetLastError(ERROR_BUFFER_OVERFLOW);
                error = TRUE;
                break;
            }

            for (int x = 0; x != DestinationLength; x++)
            {
                Destination[DestinationLength + x] = HexString[x];
            }

            DestinationLength += HexStringLength;
        }
    }

    Destination[DestinationLength] = 0;

    return error;
}
