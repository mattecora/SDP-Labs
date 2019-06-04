/******************************************************************************
 * Lab 11 - Exercise 1 (version b - using critical sections)                  *
 * Matteo Corain - System and device programming - A.Y. 2018-19               *
 ******************************************************************************/

#define UNICODE
#define _UNICODE
#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <tchar.h>

#include <stdio.h>
#include <stdlib.h>

#define DEBUG
#define MAXLEN 30

typedef struct line
{
    DWORD id;
    DWORDLONG accountNumber;
    TCHAR surname[MAXLEN + 1], name[MAXLEN + 1];
    INT amount;
} LINE, * LPLINE;

typedef struct thread_data
{
    HANDLE accountFile;
    LPCTSTR operationsFilename;
    LPCRITICAL_SECTION cs;
} THREAD_DATA, * LPTHREAD_DATA;

VOID PrintLine(LPLINE line)
{
    _tprintf(_T("%u %llu %s %s %d\n"), line->id, line->accountNumber, line->surname, line->name, line->amount);
}

VOID PrintFile(LPCTSTR filename)
{
    DWORD n;
    LINE line;
    HANDLE file;

    // Open file for reading
    file = CreateFile(filename, GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE)
        return;

    // Read to the end and print line by line
    while (ReadFile(file, &line, sizeof(LINE), &n, NULL) && n == sizeof(LINE))
        PrintLine(&line);

    // Close the file
    CloseHandle(file);
}

DWORD WINAPI WorkerThread(LPVOID data)
{
    DWORD n;
    HANDLE operationsFile;
    LARGE_INTEGER offset;
    OVERLAPPED ov;
    LINE account, operation;
    LPTHREAD_DATA td = (LPTHREAD_DATA)data;

    // Open operations file
    operationsFile = CreateFile(td->operationsFilename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (operationsFile == INVALID_HANDLE_VALUE)
    {
        _ftprintf(stderr, _T("Cannot open operations file.\n"));
        return -1;
    }

    // Read operations to the end of the file
    while (ReadFile(operationsFile, &operation, sizeof(LINE), &n, NULL) && n == sizeof(LINE))
    {
        // Compute account file offset
        offset.QuadPart = (DWORDLONG)(operation.id - 1) * sizeof(LINE);

        // Enter the critical section
        EnterCriticalSection(td->cs);
        
        _tprintf(_T("Thread %d entering CS\n"), GetCurrentThreadId());

        // Read the account file
        ov.hEvent = 0;
        ov.Offset = offset.LowPart;
        ov.OffsetHigh = offset.HighPart;

        if (!ReadFile(td->accountFile, &account, sizeof(LINE), &n, &ov) || n != sizeof(LINE))
        {
            _ftprintf(stderr, _T("Cannot read accounts file.\n"));
            LeaveCriticalSection(td->cs);
            return -1;
        }

        // Execute operation
        account.amount += operation.amount;

        // Update the account file
        ov.hEvent = 0;
        ov.Offset = offset.LowPart;
        ov.OffsetHigh = offset.HighPart;

        if (!WriteFile(td->accountFile, &account, sizeof(LINE), &n, &ov) || n != sizeof(LINE))
        {
            _ftprintf(stderr, _T("Cannot write accounts file.\n"));
            LeaveCriticalSection(td->cs);
            return -1;
        }

        _tprintf(_T("Thread %d leaving CS\n"), GetCurrentThreadId());

        // Leave critical section
        LeaveCriticalSection(td->cs);
    }

    // Close operations file
    CloseHandle(operationsFile);

    return 0;
}

VOID CreateDebugFiles(LPTSTR* argv)
{
    HANDLE file;

    LINE accounts[] = {
        {1, 100000, _T("Romano"), _T("Antonio"), 1250},
        {2, 150000, _T("Fabrizi"), _T("Aldo"), 2245},
        {3, 200000, _T("Verdi"), _T("Giacomo"), 11115},
        {4, 250000, _T("Rossi"), _T("Luigi"), 13630}
    };

    LINE operations1[] = {
        {1, 100000, _T("Romano"), _T("Antonio"), 50},
        {3, 200000, _T("Verdi"), _T("Giacomo"), -90},
        {1, 100000, _T("Romano"), _T("Antonio"), -130},
        {1, 100000, _T("Romano"), _T("Antonio"), -180},
        {2, 150000, _T("Fabrizi"), _T("Aldo"), 320}
    };

    LINE operations2[] = {
        {3, 200000, _T("Verdi"), _T("Giacomo"), 30},
        {2, 150000, _T("Fabrizi"), _T("Aldo"), 60},
        {4, 250000, _T("Rossi"), _T("Luigi"), 10},
        {1, 100000, _T("Romano"), _T("Antonio"), -100},
        {3, 200000, _T("Verdi"), _T("Giacomo"), -670}
    };

    // Create accounts file
    file = CreateFile(argv[1], GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (file == INVALID_HANDLE_VALUE)
        return;

    WriteFile(file, &accounts, 4 * sizeof(LINE), NULL, NULL);
    CloseHandle(file);

    // Create first operations file
    file = CreateFile(argv[2], GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (file == INVALID_HANDLE_VALUE)
        return;

    WriteFile(file, &operations1, 5 * sizeof(LINE), NULL, NULL);
    CloseHandle(file);

    // Create second operations file
    file = CreateFile(argv[3], GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (file == INVALID_HANDLE_VALUE)
        return;

    WriteFile(file, &operations2, 5 * sizeof(LINE), NULL, NULL);
    CloseHandle(file);
}

INT _tmain(INT argc, LPTSTR argv[])
{
    DWORD i, nThreads;
    HANDLE accountFile;
    LPHANDLE threads;
    LPTHREAD_DATA td;
    CRITICAL_SECTION cs;

    // Check input parameters
    if (argc < 3)
    {
        _ftprintf(stderr, _T("Not enough input parameters.\n"));
        return -1;
    }

#ifdef DEBUG
    CreateDebugFiles(argv);
#endif

    // Allocate dynamic memory
    nThreads = argc - 2;
    threads = (LPHANDLE)malloc(nThreads * sizeof(HANDLE));
    td = (LPTHREAD_DATA)malloc(nThreads * sizeof(THREAD_DATA));

    if (threads == NULL || td == NULL)
    {
        _ftprintf(stderr, _T("Cannot allocate memory.\n"));
        return -1;
    }

    // Open account file
    accountFile = CreateFile(argv[1], GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (accountFile == INVALID_HANDLE_VALUE)
    {
        _ftprintf(stderr, _T("Cannot open account file.\n"));
        return -1;
    }

    // Print accounts file
    _tprintf(_T("Contents of file %s before updating:\n"), argv[1]);
    PrintFile(argv[1]);

    // Initialize critical section
    InitializeCriticalSection(&cs);

    // Create threads
    for (i = 0; i < nThreads; i++)
    {
        td[i].accountFile = accountFile;
        td[i].operationsFilename = argv[i + 2];
        td[i].cs = &cs;

        threads[i] = CreateThread(NULL, 0, WorkerThread, &td[i], 0, NULL);
        if (threads[i] == NULL)
        {
            _ftprintf(stderr, _T("Cannot create threads.\n"));
            return -1;
        }
    }

    // Join threads
    WaitForMultipleObjects(nThreads, threads, TRUE, INFINITE);

    // Print accounts file
    _tprintf(_T("Contents of file %s after updating:\n"), argv[1]);
    PrintFile(argv[1]);

    // Close thread handles
    for (i = 0; i < nThreads; i++)
        CloseHandle(threads[i]);

    // Delete critical section
    DeleteCriticalSection(&cs);

    // Close account file
    CloseHandle(accountFile);

    // Free dynamic memory
    free(threads);
    free(td);

    return 0;
}