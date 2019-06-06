/******************************************************************************
 * Lab 11 - Exercise 3                                                        *
 * Matteo Corain - System and device programming - A.Y. 2018-19               *
 ******************************************************************************/

#define UNICODE
#define _UNICODE
#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <tchar.h>

#include <stdio.h>
#include <stdlib.h>

typedef struct shared_vars
{
    DWORD nL2R, nR2L;
    CRITICAL_SECTION meL2R, meR2L;
    HANDLE busy;
} SHARED_VARS, * LPSHARED_VARS;

typedef struct thread_data
{
    DWORD timeA, timeT, carsNumber;
    LPSHARED_VARS sv;
} THREAD_DATA, * LPTHREAD_DATA;

typedef struct car_thread_data
{
    DWORD id, timeT;
    LPSHARED_VARS sv;
} CAR_THREAD_DATA, * LPCAR_THREAD_DATA;

DWORD WINAPI CarLeftToRightThread(LPVOID data)
{
    LPCAR_THREAD_DATA td = (LPCAR_THREAD_DATA)data;

    // Access protocol
    EnterCriticalSection(&(td->sv->meL2R));
    td->sv->nL2R++;
    if (td->sv->nL2R == 1)
        WaitForSingleObject(td->sv->busy, INFINITE);
    LeaveCriticalSection(&(td->sv->meL2R));

    // Car traverses the bridge
    _tprintf(_T("Car %dL traversing bridge left to right (L2R: %d, R2L: %d).\n"), td->id, td->sv->nL2R, td->sv->nR2L);
    Sleep(td->timeT * 1000);
    _tprintf(_T("Car %dL traversed bridge left to right (L2R: %d, R2L: %d).\n"), td->id, td->sv->nL2R, td->sv->nR2L);

    // Release protocol
    EnterCriticalSection(&(td->sv->meL2R));
    td->sv->nL2R--;
    if (td->sv->nL2R == 0)
        SetEvent(td->sv->busy);
    LeaveCriticalSection(&(td->sv->meL2R));

    return 0;
}

DWORD WINAPI CarRightToLeftThread(LPVOID data)
{
    LPCAR_THREAD_DATA td = (LPCAR_THREAD_DATA)data;

    // Access protocol
    EnterCriticalSection(&(td->sv->meR2L));
    td->sv->nR2L++;
    if (td->sv->nR2L == 1)
        WaitForSingleObject(td->sv->busy, INFINITE);
    LeaveCriticalSection(&(td->sv->meR2L));

    // Car traverses the bridge
    _tprintf(_T("Car %dR traversing bridge right to left (L2R: %d, R2L: %d).\n"), td->id, td->sv->nL2R, td->sv->nR2L);
    Sleep(td->timeT * 1000);
    _tprintf(_T("Car %dR traversed bridge right to left (L2R: %d, R2L: %d).\n"), td->id, td->sv->nL2R, td->sv->nR2L);

    // Release protocol
    EnterCriticalSection(&(td->sv->meR2L));
    td->sv->nR2L--;
    if (td->sv->nR2L == 0)
        SetEvent(td->sv->busy);
    LeaveCriticalSection(&(td->sv->meR2L));

    return 0;
}

DWORD WINAPI LeftToRightThread(LPVOID data)
{
    DWORD i;
    LPHANDLE cars;
    LPCAR_THREAD_DATA ctd;
    LPTHREAD_DATA td = (LPTHREAD_DATA)data;

    // Initialize random seed
    srand(GetCurrentThreadId());

    // Allocate memory
    cars = (LPHANDLE)malloc(td->carsNumber * sizeof(HANDLE));
    ctd = (LPCAR_THREAD_DATA)malloc(td->carsNumber * sizeof(CAR_THREAD_DATA));
    if (cars == NULL || ctd == NULL)
    {
        _ftprintf(stderr, _T("Cannot allocate memory.\n"));
        return -1;
    }

    // Create cars
    for (i = 0; i < td->carsNumber; i++)
    {
        // Wait for a car to arrive
        Sleep((rand() % td->timeA) * 1000);

        // Set parameters for the car
        ctd[i].id = i;
        ctd[i].timeT = td->timeT;
        ctd[i].sv = td->sv;

        // Create a car
        cars[i] = CreateThread(NULL, 0, CarLeftToRightThread, &ctd[i], 0, NULL);
        if (cars[i] == NULL)
        {
            _ftprintf(stderr, _T("Cannot create car thread.\n"));
            return -1;
        }
    }

    // Wait for all cars to traverse
    WaitForMultipleObjects(td->carsNumber, cars, TRUE, INFINITE);

    // Close handles
    for (i = 0; i < td->carsNumber; i++)
        CloseHandle(cars[i]);

    // Release memory
    free(cars);
    free(ctd);

    return 0;
}

DWORD WINAPI RightToLeftThread(LPVOID data)
{
    DWORD i;
    LPHANDLE cars;
    LPCAR_THREAD_DATA ctd;
    LPTHREAD_DATA td = (LPTHREAD_DATA)data;

    // Initialize random seed
    srand(GetCurrentThreadId());

    // Allocate memory
    cars = (LPHANDLE)malloc(td->carsNumber * sizeof(HANDLE));
    ctd = (LPCAR_THREAD_DATA)malloc(td->carsNumber * sizeof(CAR_THREAD_DATA));
    if (cars == NULL || ctd == NULL)
    {
        _ftprintf(stderr, _T("Cannot allocate memory.\n"));
        return -1;
    }

    // Create cars
    for (i = 0; i < td->carsNumber; i++)
    {
        // Wait for a car to arrive
        Sleep((rand() % td->timeA) * 1000);

        // Set parameters for the car
        ctd[i].id = i;
        ctd[i].timeT = td->timeT;
        ctd[i].sv = td->sv;

        // Create a car
        cars[i] = CreateThread(NULL, 0, CarRightToLeftThread, &ctd[i], 0, NULL);
        if (cars[i] == NULL)
        {
            _ftprintf(stderr, _T("Cannot create car thread.\n"));
            return -1;
        }
    }

    // Wait for all cars to traverse
    WaitForMultipleObjects(td->carsNumber, cars, TRUE, INFINITE);

    // Close handles
    for (i = 0; i < td->carsNumber; i++)
        CloseHandle(cars[i]);

    // Release memory
    free(cars);
    free(ctd);

    return 0;
}

INT _tmain(INT argc, LPTSTR argv[])
{
    DWORD i;
    HANDLE threadL2R, threadR2L;
    SHARED_VARS sv;
    THREAD_DATA tdL2R, tdR2L;

    // Check input parameters
    if (argc < 7)
    {
        _ftprintf(stderr, _T("Not enough input parameters.\n"));
        return -1;
    }

    tdL2R.timeA = _ttoi(argv[1]);
    tdR2L.timeA = _ttoi(argv[2]);
    tdL2R.timeT = _ttoi(argv[3]);
    tdR2L.timeT = _ttoi(argv[4]);
    tdL2R.carsNumber = _ttoi(argv[5]);
    tdR2L.carsNumber = _ttoi(argv[6]);

    // Initialize synchronization vars (no mutex since they are released when thread terminates)
    sv.busy = CreateEvent(NULL, FALSE, TRUE, NULL);
    InitializeCriticalSection(&sv.meL2R);
    InitializeCriticalSection(&sv.meR2L);

    sv.nL2R = 0;
    sv.nR2L = 0;

    if (sv.busy == NULL)
    {
        _ftprintf(stderr, _T("Cannot create synchronization vars.\n"));
        return -1;
    }

    tdL2R.sv = &sv;
    tdR2L.sv = &sv;

    // Create threads
    threadL2R = CreateThread(NULL, 0, LeftToRightThread, &tdL2R, 0, NULL);
    threadR2L = CreateThread(NULL, 0, RightToLeftThread, &tdR2L, 0, NULL);

    if (threadL2R == NULL || threadR2L == NULL)
    {
        _ftprintf(stderr, _T("Cannot create threads.\n"));
        return -1;
    }

    // Join threads
    WaitForSingleObject(threadL2R, INFINITE);
    WaitForSingleObject(threadR2L, INFINITE);

    // Close handles
    CloseHandle(threadL2R);
    CloseHandle(threadR2L);

    // Delete synchronization variables
    CloseHandle(sv.busy);
    DeleteCriticalSection(&sv.meL2R);
    DeleteCriticalSection(&sv.meR2L);

    return 0;
}