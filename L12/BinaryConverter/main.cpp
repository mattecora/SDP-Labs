#define UNICODE
#define _UNICODE
#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <tchar.h>

#include <stdio.h>
#include <stdlib.h>

#define IP_LEN 15
#define USER_LEN 20
#define DATE_LEN 19
#define TIME_LEN 8

typedef struct record
{
    TCHAR ip[IP_LEN + 1], user[USER_LEN + 1], datetime[DATE_LEN + 1], length[TIME_LEN + 1];
} RECORD, * LPRECORD;

INT _tmain(INT argc, LPTSTR argv[])
{
    FILE* fIn, * fOut;
    RECORD record;

    if (argc < 3)
        return -1;

    fIn = _tfopen(argv[1], _T("r"));
    fOut = _tfopen(argv[2], _T("wb"));

    while (_ftscanf(fIn, _T("%s %s %s %s"), record.ip, record.user, record.datetime, record.length) == 4)
        fwrite(&record, sizeof(RECORD), 1, fOut);

    fclose(fIn);
    fclose(fOut);

    return 0;
}