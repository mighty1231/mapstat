#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>
#pragma comment(lib, "User32.lib")

#define MAX_RECURSIVE_DEPTH 10
#define STR_USAGE "USAGE: mapstat folderpath\n"

typedef int (__stdcall* FUNCTYPE_SFileOpenArchive)(char *, int, int, void *);
typedef int (__stdcall* FUNCTYPE_SFileCloseArchive)(int);
typedef int (__stdcall* FUNCTYPE_SFileGetArchiveName)(void *, char *, int);
typedef int (__stdcall* FUNCTYPE_SFileOpenFile)(int, int);
typedef int (__stdcall* FUNCTYPE_SFileOpenFileEx)(int, char *, int, void *);
typedef int (__stdcall* FUNCTYPE_SFileCloseFile)(int);
typedef int (__stdcall* FUNCTYPE_SFileGetFileSize)(int, void *);
typedef int (__stdcall* FUNCTYPE_SFileGetFileArchive)(void *, void *);
typedef int (__stdcall* FUNCTYPE_SFileGetFileName)(void *, char *, int);
typedef int (__stdcall* FUNCTYPE_SFileSetFilePointer)(void *, int, int, int);
typedef int (__stdcall* FUNCTYPE_SFileReadFile)(int,void *,int,void *,int);
typedef int (__stdcall* FUNCTYPE_SFileSetLocale)(int);
typedef int (__stdcall* FUNCTYPE_SFileGetBasePath)(char *, int);
typedef int (__stdcall* FUNCTYPE_SFileSetBasePath)(char *);
typedef int (__stdcall* FUNCTYPE_SFileDestroy)();

FUNCTYPE_SFileCloseArchive    SFileCloseArchive;
FUNCTYPE_SFileCloseFile       SFileCloseFile;
FUNCTYPE_SFileDestroy         SFileDestroy;
FUNCTYPE_SFileGetFileArchive  SFileGetFileArchive;
FUNCTYPE_SFileGetFileSize     SFileGetFileSize;
FUNCTYPE_SFileOpenArchive     SFileOpenArchive;
FUNCTYPE_SFileOpenFile        SFileOpenFile;
FUNCTYPE_SFileOpenFileEx      SFileOpenFileEx;
FUNCTYPE_SFileReadFile        SFileReadFile;
FUNCTYPE_SFileSetBasePath     SFileSetBasePath;
FUNCTYPE_SFileSetFilePointer  SFileSetFilePointer;
FUNCTYPE_SFileSetLocale       SFileSetLocale;
FUNCTYPE_SFileGetBasePath     SFileGetBasePath;
FUNCTYPE_SFileGetArchiveName  SFileGetArchiveName;
FUNCTYPE_SFileGetFileName     SFileGetFileName;

void searchFolder(TCHAR *szFolder, int dwLevel);

int _tmain(int argc, TCHAR *argv[]) {
	if (argc != 2) {
		printf(STR_USAGE);
		return -1;
	}
	HANDLE storm = LoadLibrary("storm.dll");
	{
		
		SFileCloseArchive     = (FUNCTYPE_SFileCloseArchive) GetProcAddress(storm, MAKEINTRESOURCE(252));
		SFileCloseFile        = (FUNCTYPE_SFileCloseFile) GetProcAddress(storm, MAKEINTRESOURCE(253));
		SFileDestroy          = (FUNCTYPE_SFileDestroy) GetProcAddress(storm, MAKEINTRESOURCE(262));
		SFileGetFileArchive   = (FUNCTYPE_SFileGetFileArchive) GetProcAddress(storm, MAKEINTRESOURCE(264));
		SFileGetFileSize      = (FUNCTYPE_SFileGetFileSize) GetProcAddress(storm, MAKEINTRESOURCE(265));
		SFileOpenArchive      = (FUNCTYPE_SFileOpenArchive) GetProcAddress(storm, MAKEINTRESOURCE(266));
		SFileOpenFile         = (FUNCTYPE_SFileOpenFile) GetProcAddress(storm, MAKEINTRESOURCE(267));
		SFileOpenFileEx       = (FUNCTYPE_SFileOpenFileEx) GetProcAddress(storm, MAKEINTRESOURCE(268));
		SFileReadFile         = (FUNCTYPE_SFileReadFile) GetProcAddress(storm, MAKEINTRESOURCE(269));
		SFileSetBasePath      = (FUNCTYPE_SFileSetBasePath) GetProcAddress(storm, MAKEINTRESOURCE(270));
		SFileSetFilePointer   = (FUNCTYPE_SFileSetFilePointer) GetProcAddress(storm, MAKEINTRESOURCE(271));
		SFileSetLocale        = (FUNCTYPE_SFileSetLocale) GetProcAddress(storm, MAKEINTRESOURCE(272));
		SFileGetBasePath      = (FUNCTYPE_SFileGetBasePath) GetProcAddress(storm, MAKEINTRESOURCE(273));
		SFileGetArchiveName   = (FUNCTYPE_SFileGetArchiveName) GetProcAddress(storm, MAKEINTRESOURCE(275));
		SFileGetFileName      = (FUNCTYPE_SFileGetFileName) GetProcAddress(storm, MAKEINTRESOURCE(276));
		SFileSetLocale(0x409);
	}
	
	FILE *fp = freopen("result.txt", "w", stdout);
	searchFolder(argv[1], 0);
	fclose(fp);
	FreeLibrary(storm);
	return 1;
}

// recursive searching
void searchFolder(TCHAR *cPath, int dwLevel) {	
	int hMpq, hFile;
	int dwFileSize, dwReadLeft, dwRead, dwReadTotal;

	WIN32_FIND_DATA ffd;
	TCHAR szLocal[MAX_PATH];
	TCHAR szDir[MAX_PATH];
	size_t path_length;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	TCHAR indentation[MAX_RECURSIVE_DEPTH*3+1];
	{
		if (dwLevel > MAX_RECURSIVE_DEPTH)
			dwLevel = MAX_RECURSIVE_DEPTH;
		memset(indentation, ' ', dwLevel*3);
		indentation[dwLevel*3] = '\0';
	}
	
	StringCchLength(cPath, MAX_PATH, &path_length);
	_tprintf(TEXT("%sSearching folder %s\n"), indentation, cPath);
	if (path_length > (MAX_PATH - 3)) {
		_tprintf(TEXT("%sPath is too long\n"), indentation);
		return;
	}
	StringCchCopy(szDir, MAX_PATH, cPath);
	StringCchCat(szDir, MAX_PATH, TEXT("\\*"));
	hFind = FindFirstFile(szDir, &ffd);
	if (hFind == INVALID_HANDLE_VALUE) {
		_tprintf(TEXT("%sError: FindFirstFile()\n"), indentation);
		return;
	}

	// initialize variables used for implementation of recursion.
	StringCchCopy(szLocal, MAX_PATH, cPath);
	szLocal[path_length++] = '/';

	do {
		SFileDestroy();
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			// do not consider "." or ".." cases
			if (ffd.cFileName[0] == '.' && (ffd.cFileName[1] == '\0' || (ffd.cFileName[1] == '.' && ffd.cFileName[2] == '\0')))
				continue;
			StringCchCopy(szLocal + path_length, MAX_PATH - path_length, ffd.cFileName);
			searchFolder(szLocal, dwLevel+1);
		} else {
			StringCchCopy(szDir, MAX_PATH, cPath);
			StringCchCat(szDir, MAX_PATH, TEXT("\\"));
			StringCchCat(szDir, MAX_PATH, ffd.cFileName);
			if (SFileOpenArchive(szDir, 0, 0, &hMpq)) {
				if (SFileOpenFileEx(hMpq, "staredit\\scenario.chk", 0, &hFile)) {
					dwReadLeft = dwFileSize = SFileGetFileSize(hFile, 0);
					dwReadTotal = 0;
					char *buf = (char *) malloc(dwFileSize);
					{
						// just read chk
						while (dwReadLeft > 0) {
							if (!SFileReadFile(hFile, buf+dwReadTotal, dwReadLeft, &dwRead, 0) || dwRead == 0) {
								break;
							}
							dwReadLeft -= dwRead;
							dwReadTotal += dwRead;
						}

						// main logic for chk
						char *curp = buf;
						int dwTrigSize;
						_tprintf(TEXT("%s Map[%s]\n"), indentation, ffd.cFileName);
						while (curp >= buf && curp < buf + dwReadTotal) {
							if (*(int *)curp == 0x47495254 /* TRIG */) {
								dwTrigSize = *(int *)(curp+4);
								_tprintf(TEXT("%s  TRIG found: %d\n"), indentation, dwTrigSize / 2400);
							}
							curp += (*(int *)(curp + 4) + 8);
						}
					}
					free(buf);
					SFileCloseFile(hFile);
				}
				else {
					_tprintf(TEXT("%s Not scenario file (type 2): %s\n"), indentation, ffd.cFileName);
				}
				SFileCloseArchive(hMpq);
			}
			else {
				_tprintf(TEXT("%s Not scenario file (type 1): %s\n"), indentation, ffd.cFileName);
			}
		}
	} while (FindNextFile(hFind, &ffd) != 0);
	FindClose(hFind);
}
