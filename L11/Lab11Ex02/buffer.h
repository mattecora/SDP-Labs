#ifndef BUFFER_H
#define BUFFER_H

#include <Windows.h>
#include <tchar.h>

#include <stdio.h>
#include <stdlib.h>

typedef struct buffer
{
    DWORD n, readPtr, writePtr;
    LPDWORD array;
} BUFFER, * LPBUFFER;

BOOL InitBuffer(LPBUFFER buf, DWORD n);
VOID PutBuffer(LPBUFFER buf, DWORD x);
DWORD GetBuffer(LPBUFFER buf);
VOID FreeBuffer(LPBUFFER buf);

#endif