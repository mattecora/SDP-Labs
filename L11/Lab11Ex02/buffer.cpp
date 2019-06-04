#include "buffer.h"

BOOL InitBuffer(LPBUFFER buf, DWORD n)
{
    buf->n = n;
    buf->readPtr = 0;
    buf->writePtr = 0;
    return (buf->array = (LPDWORD)malloc(n * sizeof(DWORD))) != NULL;
}

VOID PutBuffer(LPBUFFER buf, DWORD x)
{
    buf->array[buf->writePtr] = x;
    buf->writePtr = (buf->writePtr + 1) % (buf->n);
}

DWORD GetBuffer(LPBUFFER buf)
{
    DWORD x = buf->array[buf->readPtr];
    buf->readPtr = (buf->readPtr + 1) % (buf->n);
    return x;
}

VOID FreeBuffer(LPBUFFER buf)
{
    free(buf->array);
}