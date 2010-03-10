/* FileSystem.cpp : File System specific function for DFHL

DFHL - Duplicate File Hard Linker, a small tool to gather some space
    from duplicate files on your hard disk
Copyright (C) 2004, 2005 Jens Scheffler & Oliver Schneider

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "win_def.h"

#include "Util.h"
#include "FileSystem.h"


///============================================================================ UnbufferedFileStream
bool			UnbufferedFileStream::open() {
	Statistics::getInstance()->filesOpened++;
	PWSTR fileName = new WCHAR[MAX_PATH_LENGTH];
	file->copyName(cleanString(fileName));
	hFile = ::CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	delete	fileName;
	if (hFile == INVALID_HANDLE_VALUE)
		Statistics::getInstance()->fileOpenProblems++;
	return	hFile != INVALID_HANDLE_VALUE;
}

///====================================================================================== FileSystem
Path*			FileSystem::parsePath(PCWSTR pathStr) {
	CStrW	DirSpec(pathStr);

	// make the path absolute
	WinBuf<WCHAR>	absolutePath(MAX_PATH_LENGTH);
	PWSTR	somePointer;
	::GetFullPathName(pathStr, absolutePath.capacity(), absolutePath, &somePointer);

	// add the \\?\ prefix to support extra long paths
	WinBuf<WCHAR>	extendedPath(MAX_PATH_LENGTH + sizeofa(PATH_PREFIX));
	Copy(extendedPath, PATH_PREFIX, extendedPath.capacity());
	Cat(extendedPath, absolutePath);

	// get the long file name of the path
	::GetLongPathName(extendedPath, extendedPath, extendedPath.capacity());
	CharLastNotOf(extendedPath, L"\\ ")[1] = STR_END;		//erase tailing path separators
	if (!IsExist(DirSpec)) {
		return	NULL;
	}

	// check if we got something valid...
	Path*	Result = NULL;
	if (Empty(extendedPath)) {
		logError(L"Path \"%s\" is not existing or accessible!", pathStr);
	} else {
		// convert the string into a Path object
		Result = new Path(extendedPath, NULL);
		Statistics::getInstance()->directoriesFound++;
	}
	return	Result;
}
Collection*		FileSystem::getFolderContent(Path* folder) {
	WIN32_FIND_DATAW	findFileData;
	DWORD		dwError;
	Collection*	Result = new Collection;

	// prepare the folder name and append "\*"
	CStrW		dirSpec(MAX_PATH_LENGTH);
	folder->copyName((PWSTR)dirSpec.c_str());
	WinFS::AddSlash(dirSpec);
	dirSpec += L"*";

	HANDLE		hFind = INVALID_HANDLE_VALUE;
	hFind = ::FindFirstFileW(dirSpec, &findFileData);
	if (hFind == INVALID_HANDLE_VALUE) {
		logError(GetLastError(), L"Unable to read folder content.");
	} else {
		do {
			// check if this is a valid file and not a dummy like "." or ".."
			if (FileValidName(findFileData.cFileName)) {
				// check if we have found a file
				if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
					Statistics::getInstance()->filesFound++;

					// check for hidden file
					if (!hiddenFiles && (findFileData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)) {
						logDebug(L"ignoring hidden file \"%s\"", findFileData.cFileName);

						// check for system file
					} else if (!systemFiles && (findFileData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)) {
						logDebug(L"ignoring system file \"%s\"", findFileData.cFileName);

					} else {
						// add the path to the collection
						logFile(findFileData);

						File* item = new File(findFileData, folder);
						Result->append(item);
					}
				} else {
					// Okay we found a directory!
					Statistics::getInstance()->directoriesFound++;

					// check for junction
					if (!followJunctions && (findFileData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) {
						logDebug(L"ignoring junction \"%s\"", findFileData.cFileName);

					} else {
						// add the path to the collection
						logFile(findFileData);
						Result->append(new Path(findFileData.cFileName, folder));
					}
				}
			}
		} while (::FindNextFile(hFind, &findFileData) != 0);
		dwError = ::GetLastError();
		::FindClose(hFind);
		if (dwError != ERROR_NO_MORE_FILES) {
			PWSTR buffer = new WCHAR[MAX_PATH_LENGTH];
			_snwprintf(buffer, MAX_PATH_LENGTH, L"FindNextFile error. Error is %u\n", dwError);
			// clean up memory
			delete Result;
			throw buffer;
		}
	}
	return	Result;
}
bool			FileSystem::hardLinkFiles(const File* file1, const File* file2) {
	Statistics::getInstance()->hardLinks++;
	CStrW	file1Name(MAX_PATH_LENGTH);
	CStrW	file2Name(MAX_PATH_LENGTH);

	file1->copyName((PWSTR)file1Name.c_str());
	file2->copyName((PWSTR)file2Name.c_str());
	logInfo(L"Linking %s and %s", file1Name.c_str(), file2Name.c_str());

	// Step 1: precaution - rename original file
	CStrW	file2Backup(file2Name);
	file2Backup += L".backup";
	if (!WinFS::Move(file2Name, file2Backup)) {
		logError(L"Unable to move file to backup: %i", ::GetLastError());
		return	false;
	}

	// Step 2: create hard link
	if (!HardLink(file1Name, file2Name)) {
		logError(L"Unable to create hard link: %i", ::GetLastError());
		return	false;
	}

	// Step 3: remove backup file (orphan)
	if (!WinFS::DelFile(file2Backup)) {
		logError(L"Unable to delete file: %i", ::GetLastError());
		return	false;
	}

	Statistics::getInstance()->hardLinksSuccess++;
	return	true;
}
