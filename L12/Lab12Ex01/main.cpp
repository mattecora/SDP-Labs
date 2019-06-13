/******************************************************************************
 * Lab 12 - Exercise 1                                                        *
 * Matteo Corain - System and device programming - A.Y. 2018-19               *
 ******************************************************************************/

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

#define DATE_PREV -1
#define DATE_EQ 0
#define DATE_SUCC 1

typedef struct folder
{
    TCHAR path[MAX_PATH + 1];

    HANDLE mutex;
    CRITICAL_SECTION csReaders;
    DWORD nReaders;
} FOLDER, * LPFOLDER;

typedef struct record
{
    TCHAR ip[IP_LEN + 1], user[USER_LEN + 1], datetime[DATE_LEN + 1], length[TIME_LEN + 1];
} RECORD, * LPRECORD;

typedef struct shared_vars
{
    DWORD numberOfFolders;
    LPFOLDER folders;
} SHARED_VARS, * LPSHARED_VARS;

DWORD daysPerMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

DWORD GetSeconds(LPCTSTR time)
{
    DWORD hours = 0, minutes = 0, seconds = 0;

    _stscanf(time, _T("%d:%d:%d"), &hours, &minutes, &seconds);
    return hours * 3600 + minutes * 60 + seconds;
}

DWORD CompareDates(LPCTSTR date1, LPCTSTR date2)
{
    DWORD i, datetime[6][2];

    _stscanf(date1, _T("%d/%d/%d:%d:%d:%d"), &datetime[0][0], &datetime[0][1], &datetime[0][2], &datetime[0][3], &datetime[0][4], &datetime[0][5]);
    _stscanf(date2, _T("%d/%d/%d:%d:%d:%d"), &datetime[1][0], &datetime[1][1], &datetime[1][2], &datetime[1][3], &datetime[1][4], &datetime[1][5]);

    for (i = 0; i < 6; i++)
    {
        if (datetime[0][i] < datetime[1][i])
            return DATE_PREV;
        else if (datetime[0][i] > datetime[1][i])
            return DATE_SUCC;
    }

    return DATE_EQ;
}

VOID Reader(LPFOLDER folder)
{
    DWORD totTime = 0, nRead;
    TCHAR lastAccess[DATE_LEN + 1] = _T("");
    RECORD record;

    TCHAR searchPath[MAX_PATH + 3], filename[MAX_PATH + 256];
    HANDLE searchHandle, inFile;
    WIN32_FIND_DATA fileData;

    // Ingress protocol
    EnterCriticalSection(&folder->csReaders);
    folder->nReaders++;
    if (folder->nReaders == 1)
        WaitForSingleObject(folder->mutex, INFINITE);
    LeaveCriticalSection(&folder->csReaders);

    // Open the search handle
    _stprintf(searchPath, _T("%s\\*"), folder->path);
    searchHandle = FindFirstFile(searchPath, &fileData);
    if (searchHandle == INVALID_HANDLE_VALUE)
    {
        _ftprintf(stderr, _T("Thread %d - Cannot open directory %s.\n"), GetCurrentThreadId(), folder->path);
        return;
    }

    __try
    {
        // Loop through all files
        do
        {
            // Skip . and ..
            if (fileData.cFileName[0] == '.')
                continue;

            // Produce the file name
            _stprintf(filename, _T("%s\\%s"), folder->path, fileData.cFileName);
            
            // Open the file
            inFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (inFile == INVALID_HANDLE_VALUE)
            {
                _ftprintf(stderr, _T("Thread %d - Cannot open file %s.\n"), GetCurrentThreadId(), filename);
                __leave;
            }

            // Read each line of the file
            while (ReadFile(inFile, &record, sizeof(RECORD), &nRead, NULL) && nRead == sizeof(RECORD))
            {
                // Update connection time
                totTime += GetSeconds(record.length);

                // Update last access
                if (_tcscmp(lastAccess, _T("")) == 0 || CompareDates(lastAccess, record.datetime) == DATE_PREV)
                    memcpy(lastAccess, record.datetime, _tcslen(record.datetime) * sizeof(TCHAR));
            }

            // Close the file
            CloseHandle(inFile);
        } while (FindNextFile(searchHandle, &fileData));
    }
    __finally
    {
        // Print results
        _tprintf(_T("Thread %d - Total connection time: %d seconds\n"), GetCurrentThreadId(), totTime);
        _tprintf(_T("Thread %d - Last access time: %s\n"), GetCurrentThreadId(), lastAccess);

        // Close the search handle
        FindClose(searchHandle);

        // Egress protocol
        EnterCriticalSection(&folder->csReaders);
        folder->nReaders--;
        if (folder->nReaders == 0)
            SetEvent(folder->mutex);
        LeaveCriticalSection(&folder->csReaders);
    }
}

VOID Writer(LPFOLDER folder)
{
    DWORD nRead, year, month, day;
    RECORD record;
    OVERLAPPED ov = { 0, 0, 0, 0, NULL };

    TCHAR searchPath[MAX_PATH + 3], filename[MAX_PATH + 256];
    HANDLE searchHandle, inFile;
    WIN32_FIND_DATA fileData;

    // Ingress protocol
    WaitForSingleObject(folder->mutex, INFINITE);

    // Open the search handle
    _stprintf(searchPath, _T("%s\\*"), folder->path);
    searchHandle = FindFirstFile(searchPath, &fileData);
    if (searchHandle == INVALID_HANDLE_VALUE)
    {
        _ftprintf(stderr, _T("Thread %d - Cannot open directory %s.\n"), GetCurrentThreadId(), folder->path);
        return;
    }

    __try
    {
        // Loop through all files
        do
        {
            // Skip . and ..
            if (fileData.cFileName[0] == '.')
                continue;

            // Produce the file name
            _stprintf(filename, _T("%s\\%s"), folder->path, fileData.cFileName);

            // Open the file
            inFile = CreateFile(filename, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (inFile == INVALID_HANDLE_VALUE)
            {
                _ftprintf(stderr, _T("Thread %d - Cannot open file %s.\n"), GetCurrentThreadId(), filename);
                __leave;
            }

            ov.Offset = 0;

            // Read each line of the file
            while (ReadFile(inFile, &record, sizeof(RECORD), &nRead, NULL) && nRead == sizeof(RECORD))
            {
                // Randomize connection time
                _stprintf(record.length, _T("%02d:%02d:%02d"), rand() % 24, rand() % 60, rand() % 60);

                // Randomize access time
                year = 2000 + rand() % 20;
                month = 1 + rand() % 12;
                day = 1 + rand() % daysPerMonth[month - 1];
                _stprintf(record.datetime, _T("%04d/%02d/%02d:%02d:%02d:%02d"), year, month, day, rand() % 24, rand() % 60, rand() % 60);

                // Write the updated line
                if (!WriteFile(inFile, &record, sizeof(RECORD), &nRead, &ov) || nRead != sizeof(RECORD))
                {
                    _ftprintf(stderr, _T("Thread %d - Cannot write file %s.\n"), GetCurrentThreadId(), filename);
                    __leave;
                }

                // Setup the overlapped structure
                ov.Offset += sizeof(RECORD);
            }

            // Close the file
            CloseHandle(inFile);
        } while (FindNextFile(searchHandle, &fileData));
    }
    __finally
    {
        // Close the search handle
        FindClose(searchHandle);

        // Egress protocol
        SetEvent(folder->mutex);
    }
}

DWORD WINAPI ThreadFunction(LPVOID data)
{
    DWORD n1, n2, n3;
    LPFOLDER folder;
    LPSHARED_VARS sv = (LPSHARED_VARS)data;

    // Initialize random seed
    srand(GetCurrentThreadId());

    while (1)
    {
        // Generate three random numbers
        n1 = rand() % 100;
        n2 = rand() % 100;
        n3 = rand() % 100;

        // Sleep n1 seconds
        _tprintf(_T("Thread %d - Sleeping for %d seconds.\n"), GetCurrentThreadId(), n1);
        Sleep(n1 * 1000);

        // Select the folder on which to operate
        folder = &sv->folders[n3 * sv->numberOfFolders / 100];

        // Decide the behavior
        if (n2 < 50)
        {
            // Act as a reader
            _tprintf(_T("Thread %d - Working as reader on %s.\n"), GetCurrentThreadId(), folder->path);
            Reader(folder);
        }
        else
        {
            // Act as a writer
            _tprintf(_T("Thread %d - Working as writer on %s.\n"), GetCurrentThreadId(), folder->path);
            Writer(folder);
        }
    }

    return 0;
}

INT _tmain(INT argc, LPTSTR argv[])
{
    DWORD i, nIn, nThreads;
    HANDLE inFile;
    LPHANDLE threads;
    SHARED_VARS sv;

    if (argc < 3)
    {
        _ftprintf(stderr, _T("Not enough input arguments.\n"));
        return -1;
    }

    // Open the input file
    inFile = CreateFile(argv[1], GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (inFile == INVALID_HANDLE_VALUE)
    {
        _ftprintf(stderr, _T("Cannot open input file.\n"));
        return -1;
    }

    // Read the number of folders
    if (!ReadFile(inFile, &sv.numberOfFolders, sizeof(DWORD), &nIn, NULL) || nIn != sizeof(DWORD))
    {
        _ftprintf(stderr, _T("Cannot read input file.\n"));
        CloseHandle(inFile);
        return -1;
    }

    // Allocate the folders array
    sv.folders = (LPFOLDER)malloc(sv.numberOfFolders * sizeof(FOLDER));
    if (sv.folders == NULL)
    {
        _ftprintf(stderr, _T("Cannot allocate memory.\n"));
        CloseHandle(inFile);
        return -1;
    }

    // Fill the folders array
    for (i = 0; i < sv.numberOfFolders; i++)
    {
        // Read the folder path
        if (!ReadFile(inFile, sv.folders[i].path, (MAX_PATH + 1) * sizeof(TCHAR), &nIn, NULL) || nIn != (MAX_PATH + 1) * sizeof(TCHAR))
        {
            _ftprintf(stderr, _T("Cannot read input file.\n"));
            CloseHandle(inFile);
            free(sv.folders);
            return -1;
        }

        // Create the event handle
        sv.folders[i].mutex = CreateEvent(NULL, FALSE, TRUE, NULL);
        if (sv.folders[i].mutex == NULL)
        {
            _ftprintf(stderr, _T("Cannot create event.\n"));
            CloseHandle(inFile);
            free(sv.folders);
            return -1;
        }

        // Create the critical section
        InitializeCriticalSection(&sv.folders[i].csReaders);

        // Initialize the count variable
        sv.folders[i].nReaders = 0;
    }

    // Close the file
    CloseHandle(inFile);

    // Allocate thread handles
    nThreads = _ttoi(argv[2]);
    threads = (LPHANDLE)malloc(nThreads * sizeof(HANDLE));
    if (threads == NULL)
    {
        _ftprintf(stderr, _T("Cannot allocate memory.\n"));
        free(sv.folders);
        return -1;
    }

    // Create threads
    for (i = 0; i < nThreads; i++)
    {
        threads[i] = CreateThread(NULL, 0, ThreadFunction, &sv, 0, NULL);
        if (threads[i] == NULL)
        {
            _ftprintf(stderr, _T("Cannot create threads.\n"));
            free(sv.folders);
            free(threads);
            return -1;
        }
    }

    // Join threads
    WaitForMultipleObjects(nThreads, threads, TRUE, INFINITE);

    // Destroy synchronization variables
    for (i = 0; i < nThreads; i++)
    {
        DeleteCriticalSection(&sv.folders[i].csReaders);
        CloseHandle(&sv.folders[i].mutex);
    }

    // Free dynamic memory
    free(sv.folders);
    free(threads);

    return 0;
}