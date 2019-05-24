/******************************************************************************
 * Lab 09 - Exercise 3 (using thread synchronization and files)               *
 * Matteo Corain - System and device programming - A.Y. 2018-19               *
 ******************************************************************************/

#define UNICODE
#define _UNICODE
#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <tchar.h>

#include <stdio.h>
#include <stdlib.h>

#define PATHLEN 1024
#define LINELEN 4096

typedef struct shared_vars
{
    // Synchronization variables
    CRITICAL_SECTION csPrint;
    HANDLE eventStartPrint, eventEndPrint;

    // Shared global variables
    BOOL finished;
    DWORD currentTID;
    TCHAR producedLine[LINELEN];
} SHARED_VARS, *LPSHARED_VARS;

typedef struct visit_thread_data
{
    LPCTSTR path;
    LPSHARED_VARS sv;
} VISIT_THREAD_DATA, *LPVISIT_THREAD_DATA;

typedef struct print_thread_data
{
    DWORD nThreads;
    LPDWORD threadIds;
    LPSHARED_VARS sv;
} PRINT_THREAD_DATA, * LPPRINT_THREAD_DATA;

VOID EmitLine(LPSHARED_VARS sv, LPCTSTR format, ...)
{
    va_list args;

    va_start(args, format);

    // Go in mutual exclusion with other ThreadVisit
    EnterCriticalSection(&sv->csPrint);

    // Write the current line
    sv->finished = FALSE;
    sv->currentTID = GetCurrentThreadId();
    _vstprintf(sv->producedLine, LINELEN, format, args);

    // Signal to ThreadPrint and wait for completion
    SetEvent(sv->eventStartPrint);
    WaitForSingleObject(sv->eventEndPrint, INFINITE);

    // Exit critical section
    LeaveCriticalSection(&sv->csPrint);

    va_end(args);
}

VOID SignalTermination(LPSHARED_VARS sv)
{
    // Go in mutual exclusion with other ThreadVisit
    EnterCriticalSection(&sv->csPrint);

    // Set the finished flag
    sv->finished = TRUE;
    sv->currentTID = GetCurrentThreadId();

    // Signal to ThreadPrint and wait for completion
    SetEvent(sv->eventStartPrint);
    WaitForSingleObject(sv->eventEndPrint, INFINITE);

    // Exit critical section
    LeaveCriticalSection(&sv->csPrint);
}

BOOL VisitPath(LPSHARED_VARS sv, LPTSTR path, DWORD level)
{
    TCHAR searchPath[PATHLEN], newPath[PATHLEN];
    HANDLE searchHandle;
    WIN32_FIND_DATA fileData;

    // Open the search handle
    _stprintf(searchPath, _T("%s\\*"), path);
    searchHandle = FindFirstFile(searchPath, &fileData);

    if (searchHandle == INVALID_HANDLE_VALUE)
    {
        EmitLine(sv, _T("Cannot find the first file.\n"));
        return FALSE;
    }

    do
    {
        // Skip the current and the parent folder
        if (lstrcmp(fileData.cFileName, _T(".")) == 0 || lstrcmp(fileData.cFileName, _T("..")) == 0)
            continue;

        _stprintf(newPath, _T("%s\\%s"), path, fileData.cFileName);

        // Check if the entry is a directory
        if (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            // List the directory
            EmitLine(sv, _T("%*c %d - Dir: %s\n"), level, ' ', GetCurrentThreadId(), newPath);
            
            // Recur on the new directory
            VisitPath(sv, newPath, level + 1);
        }
        else
        {
            // List the file
            EmitLine(sv, _T("%*c %d - File: %s\n"), level, ' ', GetCurrentThreadId(), newPath);
        }
    } while (FindNextFile(searchHandle, &fileData));

    // Close the search handle
    FindClose(searchHandle);

    return TRUE;
}

DWORD WINAPI ThreadVisit(LPVOID data)
{
    TCHAR cwd[PATHLEN], absolutePath[PATHLEN];
    LPVISIT_THREAD_DATA vtd = (LPVISIT_THREAD_DATA)data;

    // Unwrap relative paths
    if (_tcslen(vtd->path) < 2 || (vtd->path)[1] != ':')
    {
        GetCurrentDirectory(PATHLEN, cwd);
        _stprintf(absolutePath, _T("%s\\%s"), cwd, vtd->path);
    }
    else
    {
        _stprintf(absolutePath, _T("%s"), vtd->path);
    }

    EmitLine(vtd->sv, _T("%d - Path: %s\n"), GetCurrentThreadId(), absolutePath);

    // Visit the given path
    VisitPath(vtd->sv, absolutePath, 1);

    // Inform ThreadPrint about the termination
    SignalTermination(vtd->sv);

    return 0;
}

VOID PrintResults(DWORD threadId)
{
    TCHAR filename[PATHLEN], line[LINELEN];
    FILE* fp;

    // Open the file
    _stprintf(filename, _T("%d.txt"), threadId);
    fp = _tfopen(filename, _T("r"));

    if (fp == NULL)
    {
        _ftprintf(stderr, _T("Cannot open output file.\n"));
        return;
    }

    // Read the file line by line
    while (_fgetts(line, LINELEN, fp))
        _fputts(line, stdout);
    _tprintf(_T("\n"));

    // Close and delete the file
    fclose(fp);
    DeleteFile(filename);
}

DWORD WINAPI ThreadPrint(LPVOID data)
{
    DWORD i, finishedThreads = 0;
    TCHAR filename[PATHLEN];
    LPPRINT_THREAD_DATA ptd = (LPPRINT_THREAD_DATA)data;
    FILE** fps;

    // Allocate vector of file pointers
    fps = (FILE**) malloc((ptd->nThreads - 1) * sizeof(FILE*));
    if (fps == NULL)
    {
        _ftprintf(stderr, _T("Cannot allocate memory.\n"));
        exit(-1);
    }

    for (i = 0; i < ptd->nThreads - 1; i++)
    {
        _stprintf(filename, _T("%d.txt"), ptd->threadIds[i]);
        
        fps[i] = _tfopen(filename, _T("w"));
        if (fps[i] == NULL)
        {
            _ftprintf(stderr, _T("Cannot create output file.\n"));
            return -1;
        }
    }
    
    // Print results when threads finish
    while (finishedThreads < ptd->nThreads - 1)
    {
        WaitForSingleObject(ptd->sv->eventStartPrint, INFINITE);
        
        // Search the file pointer for the current thread
        for (i = 0; i < ptd->nThreads; i++)
        {
            if (ptd->threadIds[i] == ptd->sv->currentTID)
            {
                if (ptd->sv->finished)
                {
                    // Thread finished, close file
                    fclose(fps[i]);

                    // Print results and delete file
                    PrintResults(ptd->sv->currentTID);

                    // Increment finished threads
                    finishedThreads++;
                }
                else
                {
                    // A thread has produced some data, write them
                    _fputts(ptd->sv->producedLine, fps[i]);
                }
            }
        }
        
        SetEvent(ptd->sv->eventEndPrint);
    }

    // Free memory
    free(fps);
        
    return 0;
}

INT _tmain(INT argc, LPTSTR argv[])
{
    DWORD i, nThreads;
    LPDWORD threadIds;
    LPHANDLE threads;
    
    SHARED_VARS sv;
    LPVISIT_THREAD_DATA vtd;
    PRINT_THREAD_DATA ptd;

    // Check input parameters
    if (argc < 2)
    {
        _ftprintf(stderr, _T("Not enough input parameters.\n"));
        return -1;
    }

    // Allocate dynamic memory
    nThreads = argc;
    threads = (LPHANDLE)malloc(nThreads * sizeof(HANDLE));
    threadIds = (LPDWORD)malloc(nThreads * sizeof(DWORD));
    vtd = (LPVISIT_THREAD_DATA)malloc((nThreads - 1) * sizeof(VISIT_THREAD_DATA));
    
    if (threads == NULL || threadIds == NULL || vtd == NULL)
    {
        _ftprintf(stderr, _T("Cannot allocate memory.\n"));
        return -1;
    }

    // Create synchronization objects
    InitializeCriticalSection(&sv.csPrint);
    sv.eventStartPrint = CreateEvent(NULL, FALSE, FALSE, NULL);
    sv.eventEndPrint = CreateEvent(NULL, FALSE, FALSE, NULL);

    if (sv.eventStartPrint == NULL || sv.eventEndPrint == NULL)
    {
        _ftprintf(stderr, _T("Cannot create mutexes.\n"));
        return -1;
    }

    // Create threads
    for (i = 0; i < nThreads - 1; i++)
    {
        vtd[i].path = argv[i + 1];
        vtd[i].sv = &sv;
        threads[i] = CreateThread(NULL, 0, ThreadVisit, &vtd[i], 0, &threadIds[i]);
    }

    ptd.nThreads = nThreads;
    ptd.threadIds = threadIds;
    ptd.sv = &sv;
    threads[nThreads - 1] = CreateThread(NULL, 0, ThreadPrint, &ptd, 0, &threadIds[nThreads - 1]);

    // Join threads
    WaitForMultipleObjects(nThreads, threads, TRUE, INFINITE);

    // Destroy synchronization objects
    DeleteCriticalSection(&sv.csPrint);
    CloseHandle(sv.eventStartPrint);
    CloseHandle(sv.eventEndPrint);

    // Free dynamic memory
    free(threads);
    free(threadIds);
    free(vtd);
    return 0;
}