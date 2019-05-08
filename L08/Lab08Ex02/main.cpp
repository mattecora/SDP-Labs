/******************************************************************************
 * Lab 08 - Exercise 2                                                        *
 * Matteo Corain - System and device programming - A.Y. 2018-19               *
 ******************************************************************************/

#define UNICODE
#define _UNICODE
#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <tchar.h>

#include <stdio.h>
#include <stdlib.h>

#define MAXLEN 30

typedef struct student
{
	INT identifier;
	LONGLONG registerNum;
	TCHAR surname[MAXLEN + 1], name[MAXLEN + 1];
	INT mark;
} STUDENT;

INT _tmain(INT argc, LPTSTR argv[])
{
	DWORD nOut;
	LARGE_INTEGER off, retOff;
	STUDENT student;
	FILE* inFile;
	HANDLE outFile;

	// Check parameters
	if (argc < 3)
	{
		_ftprintf(stderr, _T("Not enough input arguments."));
		return -1;
	}

	// Open input file (standard library)
	inFile = _tfopen(argv[1], _T("r"));

	// Open output file (Windows API)
	outFile = CreateFile(argv[2], GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	// Read all the input file
	while (_ftscanf(inFile, _T("%d %lld %s %s %d"), &student.identifier, &student.registerNum, student.name, student.surname, &student.mark) != EOF)
	{
		// Store the student in the output file
		if (!WriteFile(outFile, &student, sizeof(STUDENT), &nOut, NULL) || nOut != sizeof(STUDENT))
		{
			_ftprintf(stderr, _T("Error while writing the output file!"));
			return -1;
		}
	}

	// Close input file
	fclose(inFile);

	// Reset file pointer at the beginning of the file
	off.QuadPart = 0;
	if (!SetFilePointerEx(outFile, off, &retOff, FILE_BEGIN) || off.QuadPart != retOff.QuadPart)
	{
		_ftprintf(stderr, _T("Error while resetting the file pointer!"));
		return -1;
	}

	// Read all the output file
	while (ReadFile(outFile, &student, sizeof(STUDENT), &nOut, NULL) && nOut == sizeof(STUDENT))
	{
		_tprintf(_T("%d %lld %s %s %d\n"), student.identifier, student.registerNum, student.name, student.surname, student.mark);
	}

	// Close output file
	CloseHandle(outFile);

	return 0;
}