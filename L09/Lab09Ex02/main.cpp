/******************************************************************************
 * Lab 09 - Exercise 2                                                        *
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
#define BUFSIZE 4096

BOOL CopyFileMod(LPTSTR inputFilename, LPTSTR outputFilename, DWORD fileSize)
{
    DWORD nRead, nWritten, totWritten;
    CHAR data[BUFSIZE];
    HANDLE inputFile, outputFile;

    // Create the files
    inputFile = CreateFile(inputFilename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    outputFile = CreateFile(outputFilename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    
    if (inputFile == INVALID_HANDLE_VALUE || outputFile == INVALID_HANDLE_VALUE)
    {
        _ftprintf(stderr, _T("Cannot open files.\n"));
        return FALSE;
    }

    // Write the file name
    if (!WriteFile(outputFile, inputFilename, _tcslen(inputFilename) * sizeof(TCHAR), &nWritten, NULL) ||
        nWritten != _tcslen(inputFilename) * sizeof(TCHAR))
    {
        _ftprintf(stderr, _T("Cannot write output file.\n"));
        CloseHandle(inputFile);
        CloseHandle(outputFile);
        return FALSE;
    }

    // Write the file size
    if (!WriteFile(outputFile, &fileSize, sizeof(DWORD), &nWritten, NULL) || nWritten != sizeof(DWORD))
    {
        _ftprintf(stderr, _T("Cannot write output file.\n"));
        CloseHandle(inputFile);
        CloseHandle(outputFile);
        return FALSE;
    }

    // Copy file contents
    totWritten = 0;
    while (totWritten < fileSize)
    {
        // Read input file
        if (!ReadFile(inputFile, data, BUFSIZE, &nRead, NULL))
        {
            _ftprintf(stderr, _T("Cannot read input file.\n"));
            CloseHandle(inputFile);
            CloseHandle(outputFile);
            return FALSE;
        }

        // Write output file
        if (!WriteFile(outputFile, data, nRead, &nWritten, NULL) || nWritten != nRead)
        {
            _ftprintf(stderr, _T("Cannot write output file.\n"));
            CloseHandle(inputFile);
            CloseHandle(outputFile);
            return FALSE;
        }

        totWritten = totWritten + nWritten;
    }

    // Close the files
    CloseHandle(inputFile);
    CloseHandle(outputFile);
    
    return TRUE;
}

BOOL CopyPath(LPTSTR inputPath, LPTSTR outputPath)
{
    TCHAR newPath[PATHLEN];
    HANDLE searchHandle;
    WIN32_FIND_DATA fileData;

    _tprintf(_T("Copying directory %s.\n"), inputPath);

    // Set the current directory in the input path
    if (!SetCurrentDirectory(inputPath))
    {
        _ftprintf(stderr, _T("Cannot open input path.\n"));
        return FALSE;
    }

    // Create the output directory
    if (!CreateDirectory(outputPath, NULL))
    {
        _ftprintf(stderr, _T("Cannot create output directory.\n"));
        return FALSE;
    }

    // Open the search handle
    searchHandle = FindFirstFile(_T("*"), &fileData);
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

        // Create output file name
        _stprintf(newPath, _T("%s\\%s"), outputPath, fileData.cFileName);

        // Check if the entry is a directory
        if (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            // Recur on the new directory
            CopyPath(fileData.cFileName, newPath);
            SetCurrentDirectory(_T(".."));
        }
        else
        {
            // Copy the file
            _tprintf(_T("Copying file %s.\n"), fileData.cFileName);
            if (!CopyFileMod(fileData.cFileName, newPath, fileData.nFileSizeLow))
            {
                _ftprintf(stderr, _T("Cannot copy the file.\n"));
                return FALSE;
            }
        }
    } while (FindNextFile(searchHandle, &fileData));

    // Close the search handle
    FindClose(searchHandle);

    return TRUE;
}

INT _tmain(INT argc, LPTSTR argv[])
{
    // Check input parameters
    if (argc < 3)
    {
        _ftprintf(stderr, _T("Not enough input parameters"));
        return -1;
    }

    // Copy the two paths
    CopyPath(argv[1], argv[2]);

    return 0;
}