#define UNICODE
#define _UNICODE
#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <tchar.h>

#include <stdio.h>
#include <stdlib.h>

INT _tmain(INT argc, LPTSTR argv[])
{
    DWORD n;
    TCHAR line[MAX_PATH + 1];
    FILE* fIn, * fOut;

    if (argc < 3)
        return -1;

    fIn = _tfopen(argv[1], _T("r"));
    fOut = _tfopen(argv[2], _T("wb"));

    _ftscanf(fIn, _T("%d"), &n);
    fwrite(&n, sizeof(DWORD), 1, fOut);

    while (_ftscanf(fIn, _T("%s"), line) == 1)
        fwrite(line, (MAX_PATH + 1) * sizeof(TCHAR), 1, fOut);

    fclose(fIn);
    fclose(fOut);

    return 0;
}