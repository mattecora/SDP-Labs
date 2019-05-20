/******************************************************************************
 * Lab 08 - Exercise 3 (version c - using file locking)                       *
 * Matteo Corain - System and device programming - A.Y. 2018-19               *
 ******************************************************************************/

#define UNICODE
#define _UNICODE
#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <tchar.h>

#include <stdio.h>
#include <stdlib.h>

#define FLDLEN 30
#define BUFLEN 100

typedef struct student
{
	DWORD identifier;
	DWORDLONG registerNum;
	TCHAR surname[FLDLEN + 1], name[FLDLEN + 1];
	DWORD mark;
} STUDENT;

BOOL ReadStudent(HANDLE studentFile, DWORDLONG n, STUDENT* student)
{
	DWORD nIn;
	LARGE_INTEGER off;
	OVERLAPPED ov;

	// Setup the structure
	off.QuadPart = (n - 1) * sizeof(STUDENT);

	memset(&ov, 0, sizeof(OVERLAPPED));
	ov.Offset = off.LowPart;
	ov.OffsetHigh = off.HighPart;

	// Lock the student record
	if (!LockFileEx(studentFile, 0, 0, sizeof(STUDENT), 0, &ov))
	{
		_ftprintf(stderr, _T("Error while locking the file.\n"));
		return FALSE;
	}

	// Read the student record
	if (!ReadFile(studentFile, student, sizeof(STUDENT), &nIn, &ov) || nIn != sizeof(STUDENT))
	{
		_ftprintf(stderr, _T("Error while reading the file.\n"));
		UnlockFileEx(studentFile, 0, sizeof(STUDENT), 0, &ov);
		return FALSE;
	}

	UnlockFileEx(studentFile, 0, sizeof(STUDENT), 0, &ov);

	return TRUE;
}

BOOL WriteStudent(HANDLE studentFile, DWORDLONG n, STUDENT* student)
{
	DWORD nOut;
	LARGE_INTEGER off;
	OVERLAPPED ov;

	// Setup the structure
	off.QuadPart = (n - 1) * sizeof(STUDENT);

	memset(&ov, 0, sizeof(OVERLAPPED));
	ov.Offset = off.LowPart;
	ov.OffsetHigh = off.HighPart;

	// Lock the student record
	if (!LockFileEx(studentFile, LOCKFILE_EXCLUSIVE_LOCK, 0, sizeof(STUDENT), 0, &ov))
	{
		_ftprintf(stderr, _T("Error while locking the file.\n"));
		return FALSE;
	}

	// Write the student record
	if (!WriteFile(studentFile, student, sizeof(STUDENT), &nOut, &ov) || nOut != sizeof(STUDENT))
	{
		_ftprintf(stderr, _T("Error while writing the file.\n"));
		UnlockFileEx(studentFile, 0, sizeof(STUDENT), 0, &ov);
		return FALSE;
	}

	UnlockFileEx(studentFile, 0, sizeof(STUDENT), 0, &ov);

	return TRUE;
}

VOID Menu(HANDLE studentFile)
{
	DWORDLONG n;
	TCHAR buffer[BUFLEN];
	STUDENT student;

	while (1)
	{
		// Print the menu
		_tprintf(_T("Actions: R, W, E\n"));
		_tprintf(_T("Your choice: "));

		// Read user input
		memset(buffer, 0, BUFLEN);
		_fgetts(buffer, BUFLEN, stdin);

		// Decode command
		if (buffer[0] == 'R')
		{
			// Get the index
			if (_stscanf(buffer, _T("R %lld"), &n) != 1)
			{
				_tprintf(_T("Not a valid index.\n"));
				continue;
			}

			// Read the student
			if (!ReadStudent(studentFile, n, &student))
				continue;

			// Print the student
			_tprintf(_T("%d %lld %s %s %d\n"),
				student.identifier, student.registerNum, student.name, student.surname, student.mark);
		}
		else if (buffer[0] == 'W')
		{
			// Get the index
			if (_stscanf(buffer, _T("W %lld"), &n) != 1)
			{
				_tprintf(_T("Not a valid index.\n"));
				continue;
			}

			// Read student data
			_tprintf(_T("Input student data: "));

			memset(buffer, 0, BUFLEN);
			_fgetts(buffer, BUFLEN, stdin);

			// Parse student data
			if (_stscanf(buffer, _T("%d %lld %s %s %d\n"),
				&student.identifier, &student.registerNum, student.name, student.surname, &student.mark) != 5)
			{
				_tprintf(_T("Invalid student data.\n"));
				continue;
			}

			if (!WriteStudent(studentFile, n, &student))
				continue;
		}
		else if (buffer[0] == 'E')
			break;
		else
			_tprintf(_T("Not a valid command!\n"));
	}
}

INT _tmain(INT argc, LPTSTR argv[])
{
	HANDLE studentFile;

	// Check parameters
	if (argc < 2)
	{
		_ftprintf(stderr, _T("Not enough input arguments.\n"));
		return -1;
	}

	// Open student file
	studentFile = CreateFile(argv[1], GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (studentFile == INVALID_HANDLE_VALUE)
	{
		_ftprintf(stderr, _T("Cannot open input file.\n"));
		return -1;
	}

	// Show user menu
	Menu(studentFile);

	// Close output file
	CloseHandle(studentFile);

	return 0;
}