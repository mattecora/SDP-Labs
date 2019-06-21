/******************************************************************************
 * Lab 13 - Exercise 1                                                        *
 * Matteo Corain - System and device programming - A.Y. 2018-19               *
 ******************************************************************************/

#define UNICODE
#define _UNICODE
#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <tchar.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_STAT 3
#define NUM_TRAIN 50
#define MAX_COMM 100
#define MAX_TRAIN 100

#define CLOCKWISE 0
#define COUNTERCLOCKWISE 1

#define S1 0
#define S2 1
#define S3 2

#define LINEA 0
#define LINEB 1

typedef struct train
{
    DWORD id = 0, passengers = 0, currentStation = 0, direction = 0, line = 0;
    BOOL suppress = FALSE;
    HANDLE trainThread = NULL;
    struct train* next = NULL;
} TRAIN, * LPTRAIN;

typedef struct station
{
    DWORD id = 0, commuters[2] = { 0, 0 }, countHigh[2] = { 0, 0 }, countLow[2] = { 0, 0 };
    HANDLE eventNewCommuters = NULL, binaries[2] = { NULL, NULL }, stationThread = NULL;
    LPTRAIN trains = NULL;
    CRITICAL_SECTION csCommuters;
} STATION, * LPSTATION;

typedef struct shared_vars
{
    DWORD time1 = 0, time2 = 0, time3 = 0;
    volatile DWORD trainCount = 0;
    DWORDLONG startTime = 0;
    HANDLE commutersThread = NULL, semNextCommuters = NULL;
    STATION stations[NUM_STAT];
} SHARED_VARS, * LPSHARED_VARS;

typedef struct thread_data
{
    DWORD stationId = 0, trainId = 0;
    LPSHARED_VARS sv = NULL;
} THREAD_DATA, * LPTHREAD_DATA;

DWORD WINAPI CommutersThread(LPVOID data);
DWORD WINAPI StationThread(LPVOID data);
DWORD WINAPI TrainThread(LPVOID data);

INT _tmain(INT argc, LPCTSTR argv[])
{
    DWORD i;
    SHARED_VARS sv;
    THREAD_DATA td[NUM_STAT];

    if (argc < 4)
    {
        _ftprintf(stderr, _T("Not enough input arguments.\n"));
        return -1;
    }

    // Set times
    sv.time1 = _ttoi(argv[1]);
    sv.time2 = _ttoi(argv[2]);
    sv.time3 = _ttoi(argv[3]);
    sv.startTime = time(NULL);

    // Create next commuters semaphore
    sv.semNextCommuters = CreateSemaphore(NULL, NUM_STAT, NUM_STAT, NULL);

    // Create stations
    for (i = 0; i < NUM_STAT; i++)
    {
        sv.stations[i].id = i;
        sv.stations[i].eventNewCommuters = CreateEvent(NULL, FALSE, FALSE, NULL);
        sv.stations[i].binaries[0] = CreateEvent(NULL, FALSE, TRUE, NULL);
        sv.stations[i].binaries[1] = CreateEvent(NULL, FALSE, TRUE, NULL);
        InitializeCriticalSection(&sv.stations[i].csCommuters);

        td[i].stationId = i;
        td[i].trainId = 0;
        td[i].sv = &sv;
        sv.stations[i].stationThread = CreateThread(NULL, 0, StationThread, td + i, 0, NULL);
    }

    // Create and join commuters thread
    sv.commutersThread = CreateThread(NULL, 0, CommutersThread, &sv, 0, NULL);
    WaitForSingleObject(sv.commutersThread, INFINITE);

    // Destroy the created structures
    for (i = 0; i < NUM_STAT; i++)
    {
        CloseHandle(sv.stations[i].stationThread);
        CloseHandle(sv.stations[i].eventNewCommuters);
        CloseHandle(sv.stations[i].binaries[0]);
        CloseHandle(sv.stations[i].binaries[1]);
        DeleteCriticalSection(&sv.stations[i].csCommuters);
    }

    CloseHandle(sv.semNextCommuters);

    return 0;
}

DWORD WINAPI CommutersThread(LPVOID data)
{
    DWORD i, j, commutersIn;
    LPSHARED_VARS sv = (LPSHARED_VARS)data;

    srand(GetCurrentThreadId());

    while (1)
    {
        // Wait for the stations
        for (i = 0; i < NUM_STAT; i++)
            WaitForSingleObject(sv->semNextCommuters, INFINITE);

        // Produce new commuters
        for (i = 0; i < NUM_STAT; i++)
        {
            // Get the critical section
            EnterCriticalSection(&sv->stations[i].csCommuters);

            // Update commuters
            for (j = 0; j < 2; j++)
            {
                commutersIn = rand() % MAX_COMM;
                sv->stations[i].commuters[j] += commutersIn;
                _tprintf(_T("T+%03lld: commuterThread station=%d direction=%d commutersIn=%d\n"), time(NULL) - sv->startTime, i, j, commutersIn);
            }

            // Release the critical section
            LeaveCriticalSection(&sv->stations[i].csCommuters);
        }

        // Unlock stations
        for (i = 0; i < NUM_STAT; i++)
            SetEvent(sv->stations[i].eventNewCommuters);

        // Sleep time1 seconds
        Sleep(sv->time1 * 1000);
    }

    return 0;
}

DWORD WINAPI StationThread(LPVOID data)
{
    DWORD dir;
    LPSTATION station;
    LPTRAIN x, train;
    LPTHREAD_DATA td = (LPTHREAD_DATA)data, tdTrain;

    srand(GetCurrentThreadId());

    // Retrieve current station
    station = td->sv->stations + td->stationId;

    while (1)
    {
        // Wait for new commuters
        WaitForSingleObject(station->eventNewCommuters, INFINITE);

        // Check both directions
        for (dir = CLOCKWISE; dir <= COUNTERCLOCKWISE; dir++)
        {
            // Get the critical section
            EnterCriticalSection(&station->csCommuters);

            _tprintf(_T("T+%03lld: stationThread=%d direction=%d commuters=%d\n"), time(NULL) - td->sv->startTime, station->id, dir, station->commuters[dir]);

            // Check if number of commuters is high
            if (station->commuters[dir] > 75)
                station->countHigh[dir]++;
            else
                station->countHigh[dir] = 0;

            // Check if number of commuters is low
            if (station->commuters[dir] < 30)
                station->countLow[dir]++;
            else
                station->countLow[dir] = 0;

            // Release the critical section
            LeaveCriticalSection(&station->csCommuters);

            // Check if a train should be created
            if (station->countHigh[dir] >= 3)
            {
                // Create a train structure
                train = (LPTRAIN)malloc(sizeof(TRAIN));

                // Set fields
                train->id = InterlockedIncrement(&td->sv->trainCount) - 1;
                train->currentStation = td->stationId;
                train->direction = dir;
                train->suppress = FALSE;
                train->next = NULL;

                // Set train line
                if (station->id == S1) train->line = LINEA;
                else if (station->id == S2) train->line = rand() % 2;
                else if (station->id == S3) train->line = LINEB;

                // Update passengers
                train->passengers = min(MAX_TRAIN, station->commuters[dir]);
                station->commuters[dir] -= train->passengers;
                if (station->commuters[dir] <= 75)
                    station->countHigh[dir] = 0;

                // Create thread data structure
                tdTrain = (LPTHREAD_DATA)malloc(sizeof(THREAD_DATA));
                tdTrain->stationId = station->id;
                tdTrain->trainId = train->id;
                tdTrain->sv = td->sv;
                train->trainThread = CreateThread(NULL, 0, TrainThread, tdTrain, 0, NULL);

                // Attach structure to the end of the list
                if (station->trains == NULL)
                    station->trains = train;
                else
                {
                    for (x = station->trains; x->next != NULL; x = x->next);
                    x->next = train;
                }

                _tprintf(_T("T+%03lld: stationThread=%d direction=%d train-create=%d\n"), time(NULL) - td->sv->startTime, station->id, dir, train->id);
            }

            // Check if a train should be suppressed
            if (station->countLow[dir] >= 3 && station->trains != NULL)
            {
                // Retrieve the train
                train = station->trains;

                // Update the list
                station->trains = station->trains->next;

                // Set the suppress flag
                train->suppress = TRUE;

                // Wait for thread completion
                WaitForSingleObject(train->trainThread, INFINITE);
                CloseHandle(train->trainThread);

                // Free the train
                free(train);

                _tprintf(_T("T+%03lld: stationThread=%d direction=%d train-suppress=%d\n"), time(NULL) - td->sv->startTime, station->id, dir, train->id);
            }
        }

        // Request new commuters
        ReleaseSemaphore(td->sv->semNextCommuters, 1, NULL);
    }

    return 0;
}

DWORD WINAPI TrainThread(LPVOID data)
{
    DWORD nextStationId, commutersIn, commutersOut;
    LPTHREAD_DATA td = (LPTHREAD_DATA)data;
    LPSTATION station;
    LPTRAIN train;
    
    srand(GetCurrentThreadId());

    // Retrieve current train
    station = &(td->sv->stations[td->stationId]);
    for (train = station->trains; train->id != td->trainId; train = train->next);

    while (!train->suppress)
    {
        // Select the next station
        if (train->currentStation == S1)
        {
            // S1 -> S2
            nextStationId = S2;
        }
        else if (train->currentStation == S2)
        {
            if (train->line == LINEA)
            {
                // S2 -> S1
                nextStationId = S1;
            }
            else if (train->line == LINEB)
            {
                // S2 -> S3
                nextStationId = S3;
            }
        }
        else
        {
            // S3 -> S2
            nextStationId = S2;
        }

        // Go to the station
        Sleep(td->sv->time2 * 1000);
        train->currentStation = nextStationId;
        station = &td->sv->stations[nextStationId];

        // Wait to enter in the station
        WaitForSingleObject(station->binaries[train->direction], INFINITE);
        _tprintf(_T("T+%03lld: trainThread=%d passengers=%d stationIn=%d\n"), time(NULL) - td->sv->startTime, train->id, train->passengers, train->currentStation);

        // Get the critical section
        EnterCriticalSection(&station->csCommuters);

        // Update commuters leaving the train
        commutersOut = train->passengers > 0 ? rand() % train->passengers : 0;
        train->passengers -= commutersOut;
        _tprintf(_T("T+%03lld: trainThread=%d commutersOut=%d\n"), time(NULL) - td->sv->startTime, train->id, commutersOut);

        // Update commuters entering the train
        commutersIn = min(MAX_TRAIN - train->passengers, station->commuters[train->direction]) > 0 ? rand() % min(MAX_TRAIN - train->passengers, station->commuters[train->direction]) : 0;
        train->passengers += commutersIn;
        station->commuters[train->direction] -= commutersIn;
        _tprintf(_T("T+%03lld: trainThread=%d commutersIn=%d\n"), time(NULL) - td->sv->startTime, train->id, commutersIn);

        // Release the critical section
        LeaveCriticalSection(&station->csCommuters);

        // Stay in the station
        Sleep(td->sv->time3 * 1000);

        // Release the station
        SetEvent(station->binaries[train->direction]);
        _tprintf(_T("T+%03lld: trainThread=%d passengers=%d stationOut=%d\n"), time(NULL) - td->sv->startTime, train->id, train->passengers, train->currentStation);
    }

    // Free dynamic memory
    free(data);
    return 0;
}