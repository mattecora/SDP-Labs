/******************************************************************************
 * Lab 11 - Exercise 2                                                        *
 * Matteo Corain - System and device programming - A.Y. 2018-19               *
 ******************************************************************************/

#define UNICODE
#define _UNICODE
#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <tchar.h>

#include <stdio.h>
#include <stdlib.h>

#include "buffer.h"

#define PRODNO 50
#define SENTINEL 0xFFFFFFFF

typedef struct synchronization_vars
{
    HANDLE mep, mec, full, empty;
} SYNCHRONIZATION_VARS, * LPSYNCHRONIZATION_VARS;

typedef struct thread_data
{
    DWORD t;
    LPBUFFER buffer;
    LPSYNCHRONIZATION_VARS sv;
} THREAD_DATA, * LPTHREAD_DATA;

DWORD WINAPI ProducerThread(LPVOID data)
{
    DWORD i, x;
    LPTHREAD_DATA td = (LPTHREAD_DATA)data;

    // Initialize random seed
    srand(GetCurrentThreadId());

    for (i = 0; i < PRODNO; i++)
    {
        // Sleep for a random number of seconds
        Sleep(1000 * (rand() % td->t));

        // Produce an object (not the sentinel)
        x = rand() % SENTINEL;

        // Put the object in the buffer
        WaitForSingleObject(td->sv->empty, INFINITE);
        WaitForSingleObject(td->sv->mep, INFINITE);
        PutBuffer(td->buffer, x);
        _tprintf(_T("Producer %d put %d.\n"), GetCurrentThreadId(), x);
        ReleaseMutex(td->sv->mep);
        ReleaseSemaphore(td->sv->full, 1, NULL);
    }

    _tprintf(_T("Producer %d terminated.\n"), GetCurrentThreadId());

    return 0;
}

DWORD WINAPI ConsumerThread(LPVOID data)
{
    DWORD x;
    LPTHREAD_DATA td = (LPTHREAD_DATA)data;

    do
    {
        // Sleep for a random number of seconds
        Sleep(1000 * (rand() % td->t));

        // Read an object from the buffer
        WaitForSingleObject(td->sv->full, INFINITE);
        WaitForSingleObject(td->sv->mec, INFINITE);
        x = GetBuffer(td->buffer);
        _tprintf(_T("Consumer %d got %d.\n"), GetCurrentThreadId(), x);
        ReleaseMutex(td->sv->mec);
        ReleaseSemaphore(td->sv->empty, 1, NULL);
    } while (x != SENTINEL);

    _tprintf(_T("Consumer %d terminated.\n"), GetCurrentThreadId());

    return 0;
}

INT _tmain(INT argc, LPTSTR argv[])
{
    DWORD i, p, c, n, t;
    LPHANDLE producers, consumers;
    BUFFER buffer;
    SYNCHRONIZATION_VARS sv;
    THREAD_DATA td;

    // Check input parameters
    if (argc < 5)
    {
        _ftprintf(stderr, _T("Not enough input parameters.\n"));
        return -1;
    }

    p = _ttoi(argv[1]);
    c = _ttoi(argv[2]);
    n = _ttoi(argv[3]);
    t = _ttoi(argv[4]);

    // Initialize buffer
    InitBuffer(&buffer, n);

    // Initialize synchronization vars
    sv.empty = CreateSemaphore(NULL, n, n, NULL);
    sv.full = CreateSemaphore(NULL, 0, n, NULL);
    sv.mec = CreateMutex(NULL, FALSE, NULL);
    sv.mep = CreateMutex(NULL, FALSE, NULL);

    if (sv.empty == NULL || sv.full == NULL || sv.mec == NULL || sv.mep == NULL)
    {
        _ftprintf(stderr, _T("Cannot create synchronization vars.\n"));
        return -1;
    }

    // Allocate dynamic memory
    producers = (LPHANDLE)malloc(p * sizeof(HANDLE));
    consumers = (LPHANDLE)malloc(c * sizeof(HANDLE));

    if (producers == NULL || consumers == NULL)
    {
        _ftprintf(stderr, _T("Cannot allocate memory.\n"));
        return -1;
    }

    // Create thread data
    td.t = t;
    td.buffer = &buffer;
    td.sv = &sv;

    // Create producers
    for (i = 0; i < p; i++)
    {
        producers[i] = CreateThread(NULL, 0, ProducerThread, &td, 0, NULL);
        if (producers[i] == NULL)
        {
            _ftprintf(stderr, _T("Cannot create threads.\n"));
            return -1;
        }
    }

    // Create consumers
    for (i = 0; i < c; i++)
    {
        consumers[i] = CreateThread(NULL, 0, ConsumerThread, &td, 0, NULL);
        if (consumers[i] == NULL)
        {
            _ftprintf(stderr, _T("Cannot create threads.\n"));
            return -1;
        }
    }

    // Join producers
    WaitForMultipleObjects(p, producers, TRUE, INFINITE);

    // Insert sentinel values in the buffer
    for (i = 0; i < c; i++)
    {
        WaitForSingleObject(sv.empty, INFINITE);
        PutBuffer(&buffer, SENTINEL);
        ReleaseSemaphore(sv.full, 1, NULL);
    }

    // Close producers handles
    for (i = 0; i < p; i++)
        CloseHandle(producers[i]);

    // Join consumers
    WaitForMultipleObjects(c, consumers, TRUE, INFINITE);

    // Close consumers handles
    for (i = 0; i < c; i++)
        CloseHandle(consumers[i]);

    // Delete synchronization variables
    CloseHandle(sv.empty);
    CloseHandle(sv.full);
    CloseHandle(sv.mec);
    CloseHandle(sv.mep);

    // Free dynamic memory
    free(producers);
    free(consumers);

    // Free buffer
    FreeBuffer(&buffer);

    return 0;
}