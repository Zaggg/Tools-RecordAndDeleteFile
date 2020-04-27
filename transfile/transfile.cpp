//

#include <windows.h>
#include <tchar.h> 
#include <stdio.h>
#include <strsafe.h>
#include <string>
#include <vector>
#include <fstream>
#pragma comment(lib, "User32.lib")

class PathModel
{

public:
	explicit PathModel(std::wstring path, DWORD size) {
		this->sPath = path;
		this->iSize = size;
	}

	std::wstring sPath;
	DWORD iSize;
};

int FindFileInPath(LPCTSTR lpDirectory, std::vector<PathModel*> &pathList)
{
	HANDLE hFind;
	LARGE_INTEGER filesize;
	WIN32_FIND_DATA FindFileData;
	LPCTSTR lpFileName = NULL;
	DWORD dwError = 0;
	TCHAR tFileName[MAX_PATH];

	StringCchCopy(tFileName, sizeof(tFileName) / sizeof(TCHAR), lpDirectory);
	StringCchCat(tFileName, sizeof(tFileName) / sizeof(TCHAR), _T("\\*"));
	lpFileName = tFileName;
	hFind = FindFirstFile(lpFileName, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		printf("FindFirstFile failed (%d)\n", GetLastError());
		return 0;
	}

	do
	{
		TCHAR tFileName_t[MAX_PATH];
		StringCchCopy(tFileName_t, sizeof(tFileName_t) / sizeof(TCHAR), lpDirectory);
		StringCchCat(tFileName_t, sizeof(tFileName_t) / sizeof(TCHAR), _T("\\"));
		StringCchCat(tFileName_t, sizeof(tFileName_t) / sizeof(TCHAR), FindFileData.cFileName);
		BOOL bValidFileName = FALSE;
		if (wcscmp(FindFileData.cFileName, L".") != 0 && wcscmp(FindFileData.cFileName, L"..") != 0)
		{
			bValidFileName = TRUE;
			//_tprintf(TEXT("The first file found is %s\n"),
			//	FindFileData.cFileName);
		}

		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			//_tprintf(TEXT("  %s   <DIR>\n"), FindFileData.cFileName);
			if (bValidFileName)
			{
				FindFileInPath(tFileName_t, pathList);
			}
			else
			{
				continue;
			}
		}
		else
		{
			filesize.LowPart = FindFileData.nFileSizeLow;
			filesize.HighPart = FindFileData.nFileSizeHigh;
			_tprintf(TEXT("%s\t\t"),
				tFileName_t);
			_tprintf(TEXT("%ld bytes\n"), filesize.QuadPart);

			if (bValidFileName)
			{
				PathModel *data = new PathModel(tFileName_t, filesize.QuadPart);
				pathList.push_back(data);
			}
		}
	} while (FindNextFile(hFind, &FindFileData) != 0);

	dwError = GetLastError();
	if (dwError != ERROR_NO_MORE_FILES)
	{
		printf("error code %d", dwError);
	}

	FindClose(hFind);
	return dwError;
}

void DeleteAndReplaceWithTXTFile(std::wstring filePath)
{
	TCHAR tNewFileName[MAX_PATH];
	DWORD dwBytesToWrite, dwBytesWritten;
	if (DeleteFile(filePath.c_str()) != 0)
	{
		_tprintf(L"DeleteFile Success (%s) \n", filePath.c_str());
		auto index = filePath.find_last_of(L".");
		if (index >= 0)
		{
			StringCchCopy(tNewFileName, sizeof(tNewFileName) / sizeof(TCHAR), filePath.substr(0, index).c_str());
			StringCchCat(tNewFileName, sizeof(tNewFileName) / sizeof(TCHAR), _T(".txt"));

			//std::ofstream fileStream(tNewFileName);
			//fileStream.close();
			HANDLE hFile = CreateFile(tNewFileName, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hFile == INVALID_HANDLE_VALUE)
			{
				_tprintf(TEXT("Terminal failure: Unable to open file \"%s\" for write.\n"), filePath);
				return;
				//CloseHandle(hFile);
			}
			dwBytesToWrite = wcslen(filePath.c_str()) * sizeof(WCHAR);

			// 1
			//char buffer[MAX_PATH];
			//setlocale(LC_ALL, ".936");
			//size_t tBytesWritten = 0;
			//wcstombs_s(&tBytesWritten, buffer, sizeof(buffer), filePath.c_str(), sizeof(buffer));
			//BOOL bErrorFlag = WriteFile(hFile, buffer, sizeof(buffer), &dwBytesWritten, NULL);
			//setlocale(LC_ALL, "C");

			// 2 utf-16 with BOM
			/*WORD unicodeHead = 0xFEFF;
			WriteFile(hFile, &unicodeHead, sizeof(unicodeHead), &dwBytesWritten, NULL);
			BOOL bErrorFlag = WriteFile(hFile, filePath.c_str(), dwBytesToWrite, &dwBytesWritten, NULL);*/
			
			// 3 utf-8
			char buffer[MAX_PATH];
			dwBytesToWrite = WideCharToMultiByte(CP_UTF8, 0, filePath.c_str(), -1, NULL, 0, NULL, NULL);
			WideCharToMultiByte(CP_UTF8, 0, filePath.c_str(), -1, buffer, dwBytesToWrite, NULL, NULL);
			// drop terminating null
			dwBytesToWrite -= 1;
			BOOL bErrorFlag = WriteFile(hFile, buffer, dwBytesToWrite, &dwBytesWritten, NULL);

			if (FALSE == bErrorFlag)
			{
				printf("Terminal failure: Unable to write to file.\n");
			}
			else
			{
				//if (dwBytesWritten != dwBytesToWrite)
				//{
				//	printf("Error: dwBytesWritten != dwBytesToWrite\n");
				//}
				//else
				//{
				_tprintf(TEXT("Wrote %d bytes to %s successfully.\n"), dwBytesWritten, tNewFileName);
				//}
			}
			CloseHandle(hFile);

		}
	}
	else
	{
		printf("DeleteFile failed (%d)\n", GetLastError());
	}
}

int _tmain(int argc, TCHAR* argv[])
{
	size_t length_of_arg;
	TCHAR szDir[MAX_PATH] = L"D:\\test";
	DWORD fileMaxSize = 100;
	//LPCTSTR lpFileName = L"D:\\test";

#ifdef RELEASE
	if (argc < 3)
	{
		_tprintf(TEXT("\nUsage: %s <directory name> <file size threshold>\n"), argv[0]);
		return (-1);
	}

	if (argc == 2)
	{
		fileMaxSize = 100;
	}
	else
	{
		fileMaxSize = _wtoi(argv[2]);
	}

	StringCchLength(argv[1], MAX_PATH, &length_of_arg);

	if (length_of_arg > (MAX_PATH - 3))
	{
		_tprintf(TEXT("\nDirectory path is too long.\n"));
		return (-1);
	}

	_tprintf(TEXT("\nTarget directory is %s\n\n"), argv[1]);

	StringCchCopy(szDir, MAX_PATH, argv[1]);
	//StringCchCat(szDir, MAX_PATH, TEXT("\\*"));

#endif
	fileMaxSize *= 1024 * 1000;
	std::vector<PathModel*> filePathList;
	FindFileInPath(szDir, filePathList);

	for (auto data : filePathList)
	{
		if (data->iSize > fileMaxSize)
		{
			DeleteAndReplaceWithTXTFile(data->sPath);
		}
	}

	for (auto itor = filePathList.begin(); itor != filePathList.end(); )
	{
		auto value = *itor;
		itor = filePathList.erase(itor);
		delete value;
	}
}

