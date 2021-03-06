/******************************************************************************
 * Lab 09 - Exercise 3 (using files)                                          *
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

BOOL VisitPath(LPTSTR path, DWORD level, FILE* out)
{
    TCHAR searchPath[PATHLEN], newPath[PATHLEN];
    HANDLE searchHandle;
    WIN32_FIND_DATA fileData;
    
    // Open the search handle
    _stprintf(searchPath, _T("%s\\*"), path);
    searchHandle = FindFirstFile(searchPath, &fileData);
    
    if (searchHandle == INVALID_HANDLE_VALUE)
    {
        _ftprintf(out, _T("Cannot find the first file.\n"));
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
            _ftprintf(out, _T("%*c %d - Dir: %s\n"), level, ' ', GetCurrentThreadId(), newPath);
            
            // Recur on the new directory
            VisitPath(newPath, level + 1, out);
        }
        else
        {
            // List the file
            _ftprintf(out, _T("%*c %d - File: %s\n"), level, ' ', GetCurrentThreadId(), newPath);
        }
    } while (FindNextFile(searchHandle, &fileData));

    // Close the search handle
    FindClose(searchHandle);

    return TRUE;
}

DWORD WINAPI ThreadVisit(LPVOID data)
{
    TCHAR tempFilename[PATHLEN], cwd[PATHLEN], absolutePath[PATHLEN];
    FILE* fp;

    // Unwrap relative paths
    if (_tcslen((LPTSTR)data) < 2 || ((LPTSTR)data)[1] != ':')
    {
        GetCurrentDirectory(PATHLEN, cwd);
        _stprintf(absolutePath, _T("%s\\%s"), cwd, (LPTSTR)data);
    }
    else
    {
        _stprintf(absolutePath, _T("%s"), (LPTSTR)data);
    }

    // Open the file threadID.txt
    _stprintf(tempFilename, _T("%d.txt"), GetCurrentThreadId());
    fp = _tfopen(tempFilename, _T("w"));

    if (fp == NULL)
    {
        _ftprintf(stderr, _T("Cannot create output file.\n"));
        return -1;
    }

    _ftprintf(fp, _T("%d - Path: %s\n"), GetCurrentThreadId(), absolutePath);

    // Visit the given path
    VisitPath(absolutePath, 1, fp);
    
    // Close the file
    fclose(fp);
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

INT _tmain(INT argc, LPTSTR argv[])
{
    DWORD i, nThreads;
    LPDWORD threadIds;
    LPHANDLE threads;

    // Check input parameters
    if (argc < 2)
    {
        _ftprintf(stderr, _T("Not enough input parameters.\n"));
        return -1;
    }

    // Allocate dynamic memory
    nThreads = argc - 1;
    threads = (LPHANDLE)malloc(nThreads * sizeof(HANDLE));
    threadIds = (LPDWORD)malloc(nThreads * sizeof(DWORD));
    if (threads == NULL || threadIds == NULL)
    {
        _ftprintf(stderr, _T("Cannot allocate memory.\n"));
        return -1;
    }

    // Create threads
    for (i = 0; i < nThreads; i++)
        threads[i] = CreateThread(NULL, 0, ThreadVisit, argv[i + 1], 0, &threadIds[i]);

    // Join threads
    WaitForMultipleObjects(nThreads, threads, TRUE, INFINITE);

    // Print results
    for (i = 0; i < nThreads; i++)
        PrintResults(threadIds[i]);

    // Free dynamic memory
    free(threads);
    free(threadIds);
    return 0;
}