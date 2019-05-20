/******************************************************************************
 * Lab 09 - Exercise 1                                                        *
 * Matteo Corain - System and device programming - A.Y. 2018-19               *
 ******************************************************************************/

#define UNICODE
#define _UNICODE
#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <tchar.h>

#include <stdio.h>
#include <stdlib.h>

typedef struct thread_data
{
    LPTSTR filename;
    DWORD length;
    LPDWORD vector;
} THREAD_DATA, *LPTHREAD_DATA;

VOID PrintVector(LPDWORD vector, DWORD length)
{
    DWORD i;

    for (i = 0; i < length; i++)
        _tprintf(_T("%d "), vector[i]);
    _tprintf(_T("\n"));
}

VOID QuickSort(LPDWORD vector, INT left, INT right)
{
    INT i, j, pivot, temp;

    if (left >= right)
        return;

    pivot = left;
    i = left;
    j = right;

    while (i < j)
    {
        while (vector[i] <= vector[pivot] && i < right)
            i++;
        while (vector[j] > vector[pivot])
            j--;
        
        if (i < j)
        {
            temp = vector[i];
            vector[i] = vector[j];
            vector[j] = temp;
        }
    }

    temp = vector[pivot];
    vector[pivot] = vector[j];
    vector[j] = temp;

    QuickSort(vector, left, j - 1);
    QuickSort(vector, j + 1, right);
}

VOID Merge(LPDWORD first, DWORD firstLen, LPDWORD second, DWORD secondLen, LPDWORD result)
{
    DWORD i, j, k;

    for (i = 0, j = 0, k = 0; k < firstLen + secondLen; k++)
    {
        if (i >= firstLen)
            result[k] = second[j++];
        else if (j >= secondLen)
            result[k] = first[i++];
        else if (first[i] <= second[j])
            result[k] = first[i++];
        else
            result[k] = second[j++];
    }
}

DWORD WINAPI Sorter(LPVOID data)
{
    HANDLE inputFile;
    DWORD nRead;
    LPTHREAD_DATA tData = (THREAD_DATA*) data;

    // Open the input file
    inputFile = CreateFile(tData->filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (inputFile == INVALID_HANDLE_VALUE)
    {
        _ftprintf(stderr, _T("Cannot open input file.\n"));
        return FALSE;
    }

    // Read the length
    if (!ReadFile(inputFile, &(tData->length), sizeof(DWORD), &nRead, NULL) || nRead != sizeof(DWORD))
    {
        _ftprintf(stderr, _T("Cannot read length from input file.\n"));
        CloseHandle(inputFile);
        return FALSE;
    }

    // Allocate the vector
    tData->vector = (LPDWORD) malloc(tData->length * sizeof(DWORD));
    if (tData->vector == NULL)
    {
        _ftprintf(stderr, _T("Cannot allocate memory.\n"));
        CloseHandle(inputFile);
        return FALSE;
    }

    // Fill in the vector
    if (!ReadFile(inputFile, tData->vector, tData->length * sizeof(DWORD), &nRead, NULL) || 
        nRead != tData->length * sizeof(DWORD))
    {
        _ftprintf(stderr, _T("Cannot read input file.\n"));
        CloseHandle(inputFile);
        return FALSE;
    }

    // Close the file
    CloseHandle(inputFile);

    // Sort the vector
    QuickSort(tData->vector, 0, tData->length - 1);

    return TRUE;
}

DWORD Merger(DWORD nThreads, LPTHREAD_DATA threadData, LPTSTR outputFilename)
{
    DWORD i, lastLen, nWritten;
    LPDWORD result, lastResult;
    HANDLE outputFile;

    lastLen = 0;
    lastResult = NULL;

    for (i = 0; i < nThreads; i++)
    {
        // Allocate the partial results vector
        result = (LPDWORD) malloc((lastLen + threadData[i].length) * sizeof(DWORD));
        if (result == NULL)
        {
            _ftprintf(stderr, _T("Cannot allocate memory.\n"));
            return FALSE;
        }

        // Merge two partial results
        Merge(lastResult, lastLen, threadData[i].vector, threadData[i].length, result);

        // Free old result and copy new one
        free(lastResult);
        lastResult = result;
        lastLen = lastLen + threadData[i].length;
    }

    // Print the sorted vector
    _tprintf(_T("Sorted vector: "));
    PrintVector(lastResult, lastLen);

    // Open the output file
    outputFile = CreateFile(outputFilename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (outputFile == INVALID_HANDLE_VALUE)
    {
        _ftprintf(stderr, _T("Cannot open output file.\n"));
        return FALSE;
    }

    // Write the output length
    if (!WriteFile(outputFile, &lastLen, sizeof(DWORD), &nWritten, NULL) ||
        nWritten != sizeof(DWORD))
    {
        _ftprintf(stderr, _T("Cannot write length on output file.\n"));
        CloseHandle(outputFile);
        return FALSE;
    }

    // Write the output data
    if (!WriteFile(outputFile, lastResult, lastLen * sizeof(DWORD), &nWritten, NULL) ||
        nWritten != lastLen * sizeof(DWORD))
    {
        _ftprintf(stderr, _T("Cannot write output file.\n"));
        CloseHandle(outputFile);
        return FALSE;
    }

    // Close the output file
    CloseHandle(outputFile);

    // Release dynamic memory
    free(lastResult);
    return TRUE;
}

INT _tmain(INT argc, LPTSTR argv[])
{
    DWORD i, nThreads;
    LPHANDLE threads;
    LPTHREAD_DATA threadData;

    // Check input parameters
    if (argc < 3)
    {
        _ftprintf(stderr, _T("Not enough input parameters"));
        return -1;
    }

    // Allocate memory
    nThreads = argc - 2;
    threads = (LPHANDLE) malloc(nThreads * sizeof(HANDLE));
    threadData = (LPTHREAD_DATA) malloc(nThreads * sizeof(THREAD_DATA));
    if (threads == NULL || threadData == NULL)
    {
        _ftprintf(stderr, _T("Cannot allocate memory.\n"));
        return -1;
    }

    // Create threads
    for (i = 0; i < nThreads; i++)
    {
        threadData[i].filename = argv[i + 1];
        threads[i] = CreateThread(NULL, 0, Sorter, &threadData[i], 0, NULL);
        if (threads[i] == NULL)
        {
            _ftprintf(stderr, _T("Cannot create threads.\n"));
            return -1;
        }
    }

    // Join threads
    WaitForMultipleObjects(nThreads, threads, TRUE, INFINITE);
    
    // Merge partial results
    Merger(nThreads, threadData, argv[argc - 1]);

    // Release dynamic memory
    for (i = 0; i < nThreads; i++)
        free(threadData[i].vector);
    free(threadData);
    free(threads);

    return 0;
}