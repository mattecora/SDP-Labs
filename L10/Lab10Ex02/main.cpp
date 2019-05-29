/******************************************************************************
 * Lab 10 - Exercise 2                                                        *
 * Matteo Corain - System and device programming - A.Y. 2018-19               *
 ******************************************************************************/

#define UNICODE
#define _UNICODE
#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <tchar.h>

#include <stdio.h>
#include <stdlib.h>

#define PATHLEN 4096
#define LINELEN 1024

typedef struct shared_vars
{
    // Synchronization variables
    HANDLE semFileFound;
    LPHANDLE eventsFileOk;

    // Shared global variables
    LPTSTR* filenames;
    LPBOOL finished;
    BOOL kill;
} SHARED_VARS, * LPSHARED_VARS;

typedef struct visit_thread_data
{
    DWORD id;
    LPCTSTR path;
    LPSHARED_VARS sv;
} VISIT_THREAD_DATA, * LPVISIT_THREAD_DATA;

typedef struct compare_thread_data
{
    DWORD nThreads;
    LPHANDLE threads;
    LPSHARED_VARS sv;
} COMPARE_THREAD_DATA, * LPCOMPARE_THREAD_DATA;

BOOL VisitPath(LPVISIT_THREAD_DATA vtd, LPTSTR absolutePath, LPTSTR relativePath)
{
    LPTSTR searchPath, newAbsolutePath, newRelativePath;
    HANDLE searchHandle;
    WIN32_FIND_DATA fileData;

    // Allocate strings
    searchPath = (LPTSTR)malloc(PATHLEN * sizeof(TCHAR));
    if (searchPath == NULL)
    {
        _ftprintf(stderr, _T("Cannot allocate memory.\n"));
        return FALSE;
    }

    // Open the search handle
    _stprintf(searchPath, _T("%s\\*"), absolutePath);
    searchHandle = FindFirstFile(searchPath, &fileData);
    free(searchPath);

    if (searchHandle == INVALID_HANDLE_VALUE)
    {
        _ftprintf(stderr, _T("Cannot find the first file.\n"));
        return FALSE;
    }

    do
    {
        // Skip the current and the parent folder
        if (lstrcmp(fileData.cFileName, _T(".")) == 0 || lstrcmp(fileData.cFileName, _T("..")) == 0)
            continue;

        // List the entry for comparison
        _stprintf(vtd->sv->filenames[vtd->id], _T("%s\\%s"), relativePath, fileData.cFileName);

        // Signal to compare thread and wait for completion
        ReleaseSemaphore(vtd->sv->semFileFound, 1, NULL);
        WaitForSingleObject(vtd->sv->eventsFileOk[vtd->id], INFINITE);

        // Check if we need to stop searching (paths not equal)
        if (vtd->sv->kill)
        {
            FindClose(searchHandle);
            return FALSE;
        }

        // Check if the entry is a directory
        if (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            // Allocate strings
            newAbsolutePath = (LPTSTR)malloc(PATHLEN * sizeof(TCHAR));
            newRelativePath = (LPTSTR)malloc(PATHLEN * sizeof(TCHAR));

            if (newAbsolutePath == NULL || newRelativePath == NULL)
            {
                _ftprintf(stderr, _T("Cannot allocate memory.\n"));
                return FALSE;
            }

            // Recur on the new directory
            _stprintf(newAbsolutePath, _T("%s\\%s"), absolutePath, fileData.cFileName);
            _stprintf(newRelativePath, _T("%s\\%s"), relativePath, fileData.cFileName);
            VisitPath(vtd, newAbsolutePath, newRelativePath);

            // Free strings
            free(newAbsolutePath);
            free(newRelativePath);
        }
    } while (FindNextFile(searchHandle, &fileData));

    // Close the search handle and release memory
    FindClose(searchHandle);

    return TRUE;
}

DWORD WINAPI ThreadVisit(LPVOID data)
{
    LPTSTR cwd, absolutePath;
    LPVISIT_THREAD_DATA vtd = (LPVISIT_THREAD_DATA)data;

    // Allocate strings
    cwd = (LPTSTR)malloc(PATHLEN * sizeof(TCHAR));
    absolutePath = (LPTSTR)malloc(PATHLEN * sizeof(TCHAR));
    vtd->sv->filenames[vtd->id] = (LPTSTR)malloc(LINELEN * sizeof(TCHAR));

    if (cwd == NULL || absolutePath == NULL || vtd->sv->filenames[vtd->id] == NULL)
    {
        _ftprintf(stderr, _T("Cannot allocate memory.\n"));
        return -1;
    }

    // Create event
    vtd->sv->eventsFileOk[vtd->id] = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (vtd->sv->eventsFileOk[vtd->id] == NULL)
    {
        free(cwd);
        free(absolutePath);
        free(vtd->sv->filenames[vtd->id]);
        _ftprintf(stderr, _T("Cannot create event.\n"));
        return -1;
    }

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

    // Unset the termination flag
    vtd->sv->finished[vtd->id] = FALSE;

    // Visit the given path
    VisitPath(vtd, absolutePath, (LPTSTR) _T("."));

    // Release strings
    free(cwd);
    free(absolutePath);
    free(vtd->sv->filenames[vtd->id]);

    // Destroy event
    CloseHandle(vtd->sv->eventsFileOk[vtd->id]);

    // Set the termination flag and signal
    vtd->sv->finished[vtd->id] = TRUE;
    ReleaseSemaphore(vtd->sv->semFileFound, 1, NULL);

    return 0;
}

BOOL CheckTermination(LPCOMPARE_THREAD_DATA ctd, BOOL all)
{
    DWORD i;

    for (i = 0; i < ctd->nThreads - 1; i++)
    {
        if (ctd->sv->finished[i] == FALSE && all == TRUE)
            return FALSE;
        else if (ctd->sv->finished[i] == TRUE && all == FALSE)
            return TRUE;
    }

    return all;
}

BOOL CheckFilenames(LPCOMPARE_THREAD_DATA ctd)
{
    DWORD i;

    for (i = 0; i < ctd->nThreads - 2; i++)
    {
        if (_tcscmp(ctd->sv->filenames[i], ctd->sv->filenames[i + 1]) != 0)
            return FALSE;
    }

    return TRUE;
}

DWORD WINAPI ThreadCompare(LPVOID data)
{
    DWORD i;
    LPCOMPARE_THREAD_DATA ctd = (LPCOMPARE_THREAD_DATA)data;

    ctd->sv->kill = FALSE;

    while (ctd->sv->kill == FALSE)
    {
        // Wait for all visit threads to complete
        for (i = 0; i < ctd->nThreads - 1; i++)
            WaitForSingleObject(ctd->sv->semFileFound, INFINITE);

        // If all terminated, then the two trees are equal
        if (CheckTermination(ctd, TRUE))
        {
            _tprintf(_T("The given trees are equal.\n"));
            ctd->sv->kill = TRUE;
        }

        // If only one terminated, the two trees are partially equal
        else if (CheckTermination(ctd, FALSE))
        {
            _tprintf(_T("The given trees are partially equal.\n"));
            ctd->sv->kill = TRUE;
        }

        // Check all produced names
        else if (!CheckFilenames(ctd))
        {
            _tprintf(_T("The given trees are not equal.\n"));
            ctd->sv->kill = TRUE;
        }

        if (ctd->sv->kill == FALSE)
            _tprintf(_T("Entry %s is equal.\n"), ctd->sv->filenames[0]);

        // Unlock all visit threads
        for (i = 0; i < ctd->nThreads - 1; i++)
            SetEvent(ctd->sv->eventsFileOk[i]);
    }

    return 0;
}

INT _tmain(INT argc, LPTSTR argv[])
{
    DWORD i, threadId, nThreads;
    LPHANDLE threads;
    SHARED_VARS sv;
    LPVISIT_THREAD_DATA vtd;
    COMPARE_THREAD_DATA ctd;
    
    // Check input parameters
    if (argc < 2)
    {
        _ftprintf(stderr, _T("Not enough input parameters.\n"));
        return -1;
    }

    // Allocate dynamic memory
    nThreads = argc;
    threads = (LPHANDLE)malloc(nThreads * sizeof(HANDLE));
    vtd = (LPVISIT_THREAD_DATA)malloc((nThreads - 1) * sizeof(VISIT_THREAD_DATA));
    sv.filenames = (LPTSTR*)malloc((nThreads - 1) * sizeof(LPTSTR));
    sv.finished = (LPBOOL)malloc((nThreads - 1) * sizeof(BOOL));
    sv.eventsFileOk = (LPHANDLE)malloc((nThreads - 1) * sizeof(HANDLE));

    if (threads == NULL || vtd == NULL || sv.filenames == NULL || sv.finished == NULL || sv.eventsFileOk == NULL)
    {
        _ftprintf(stderr, _T("Cannot allocate memory.\n"));
        return -1;
    }

    // Create synchronization objects
    sv.semFileFound = CreateSemaphore(NULL, 0, nThreads - 1, NULL);
    if (sv.semFileFound == NULL)
    {
        _ftprintf(stderr, _T("Cannot create semaphore.\n"));
        return -1;
    }

    // Create threads
    for (i = 0; i < nThreads - 1; i++)
    {
        vtd[i].id = i;
        vtd[i].path = argv[i + 1];
        vtd[i].sv = &sv;

        threads[i] = CreateThread(NULL, 0, ThreadVisit, &vtd[i], 0, &threadId);
        if (threads[i] == NULL)
        {
            _ftprintf(stderr, _T("Cannot create threads.\n"));
            return -1;
        }

        _tprintf(_T("Thread %d visiting path: %s\n"), threadId, argv[i + 1]);
    }

    // Start compare thread
    ctd.nThreads = nThreads;
    ctd.threads = threads;
    ctd.sv = &sv;
    
    threads[nThreads - 1] = CreateThread(NULL, 0, ThreadCompare, &ctd, 0, NULL);
    if (threads[nThreads - 1] == NULL)
    {
        _ftprintf(stderr, _T("Cannot create threads.\n"));
        return -1;
    }

    // Join threads
    WaitForMultipleObjects(nThreads, threads, TRUE, INFINITE);

    // Close thread handles
    for (i = 0; i < nThreads; i++)
        CloseHandle(threads[i]);

    // Destroy semaphore
    CloseHandle(sv.semFileFound);

    // Free dynamic memory
    free(threads);
    free(vtd);
    free(sv.filenames);
    free(sv.finished);
    free(sv.eventsFileOk);

    return 0;
}