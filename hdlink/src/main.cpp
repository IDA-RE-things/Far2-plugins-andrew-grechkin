/**
	hdlink console tool
	Search duplicate files and make hardlinks

	© 2010 Andrew Grechkin

	Some code was adopted from:
	DFHL - Duplicate File Hard Linker, a small tool to gather some space
	from duplicate files on your hard disk
	Copyright (C) 2004, 2005 Jens Scheffler & Oliver Schneider
	http://www.jensscheffler.de/dfhl

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/
#include "win_def.h"

#include "main.h"
#include "version.h"

#include <stdio.h>

#include <list>
#include <algorithm>
using namespace std;

typedef		list<Shared_ptr<Path> >	PathsList;
typedef		list<Shared_ptr<File> >	FilesList;
typedef		FilesList::iterator		FilesListIt;
typedef		PathsList::iterator		PathListIt;

const size_t	MIN_FILE_SIZE = 1024;	// Minimum file size so that hard linking will be checked...

bool		showStatistics = true;

///====================================================================================== FileSystem
class		FileSystem {
	PathsList	paths;
	FilesList	data;

	bool		recursive;
	bool		hardlink;
	bool		AttrMustMatch;
	bool		TimeMustMatch;
	bool		LinkSmall;
	bool		SkipJunct;
	bool		SkipHidden;
	bool		SkipSystem;

	Path*		parsePath(PCWSTR path) {
		WinBuf<WCHAR>	extendedPath(MAX_PATH_LEN);
		Copy(extendedPath, PATH_PREFIX_NT, extendedPath.capacity());
		Cat(extendedPath, FullPath(path).c_str(), extendedPath.capacity());
		::GetLongPathName(extendedPath, extendedPath, extendedPath.capacity());
		PWSTR	tmp = (PWSTR)find_last_not_of((PCWSTR)extendedPath, L"\\ ");
		if (tmp && (tmp - extendedPath) < (ssize_t)extendedPath.capacity()) {
			tmp[1] = STR_END;		//erase tailing path separators
		}

		Path*	Result = nullptr;
		if (Empty(extendedPath) || !file_exists(extendedPath)) {
			logError(L"Path \"%s\" is not existing or accessible!\n", extendedPath.data());
		} else {
			{
				ConsoleColor	col(FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN);
				logInfo(L"Adding directory: ");
			}
			logInfo(L"\"%s\"\n", path);
			Result = new Path(nullptr, extendedPath);
		}
		return	Result;
	}
	bool		getFolderContent(Shared_ptr<Path> folder) {
		DWORD	dwError = 0;
		WinBuf<WCHAR> path(MAX_PATH_LEN);
		folder->copyName(path);
		AutoUTF	mask(MakePath(path, L"*"));

		WIN32_FIND_DATAW	info;
		HANDLE	hFind = ::FindFirstFileW(mask.c_str(), &info);
		if (hFind == INVALID_HANDLE_VALUE) {
			logError(::GetLastError(), L"Unable to read folder \"%s\" content.\n", path.data());
		} else {
			do {
				if (is_valid_filename(info.cFileName)) {
					if (!(info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
						uintmax_t	filesize = MyUI64(info.nFileSizeLow, info.nFileSizeHigh);
						++Statistics::getInstance()->FoundFiles;
						Statistics::getInstance()->FoundFilesSize += filesize;
						if (SkipHidden && (info.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)) {
							++Statistics::getInstance()->IgnoredHidden;
							{
								ConsoleColor	col(FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN);
								logDebug(L"File ignored [hidden]: ");
							}
							logDebug(L"\"%s\"\n", info.cFileName);
						} else if (SkipSystem && (info.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)) {
							++Statistics::getInstance()->IgnoredSystem;
							{
								ConsoleColor	col(FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN);
								logDebug(L"File ignored [system]: ");
							}
							logDebug(L"\"%s\"\n", info.cFileName);
						} else if (filesize == 0LL) {
							++Statistics::getInstance()->IgnoredZero;
							{
								ConsoleColor	col(FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN);
								logDebug(L"File ignored [zero]: ");
							}
							logDebug(L"\"%s\"\n", info.cFileName);
						} else if (!LinkSmall && (filesize < MIN_FILE_SIZE)) {
							++Statistics::getInstance()->IgnoredSmall;
							{
								ConsoleColor	col(FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN);
								logDebug(L"File ignored [small]: ");
							}
							logDebug(L"\"%s\"\n", info.cFileName);
						} else {
							logFile(info);
							data.push_back(new File(folder, info));
						}
					} else {
						if (SkipJunct && info.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
							++Statistics::getInstance()->IgnoredJunc;
							{
								ConsoleColor	col(FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN);
								logDebug(L"Dir  ignored [junction]: ");
							}
							logDebug(L"\"%s\"\n", info.cFileName);
						} else {
							if (info.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
								++Statistics::getInstance()->FoundJuncs;
							else
								++Statistics::getInstance()->FoundDirs;
							logFile(info);
							if (recursive) {
								bool	Result = getFolderContent(new Path(folder, info.cFileName));
								if (!Result)
									return	false;
							}
						}
					}
				}
			} while (::FindNextFile(hFind, &info) != 0);
			dwError = ::GetLastError();
			::FindClose(hFind);
			if (dwError != ERROR_NO_MORE_FILES) {
				logError(dwError, L"FindNextFile error\n");
				return	false;
			}
		}
		return	true;
	}

public:
	~FileSystem() {
	}
	FileSystem() {
		recursive = hardlink = false;
		AttrMustMatch = TimeMustMatch = false;
		LinkSmall = SkipJunct =	SkipHidden = SkipSystem = false;
	}

	void		PrintHelp() {
		showStatistics = false;
		logInfo(L"Search duplicate files and make hardlinks\n");
		logInfo(L"© 2010 Andrew Grechkin, http://code.google.com/p/andrew-grechkin/\n");
		{
			ConsoleColor	col(FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE);
			logInfo(L"NOTE:\n\tUse this tool on your own risk!\n");
		}
		{
			ConsoleColor	col(FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN);
			logInfo(L"Usage:\n");
		}
		logInfo(L"\thdlink [options] [path] [path] [...]\n");
		{
			ConsoleColor	col(FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN);
			logInfo(L"Options:\n");
		}
		logInfo(L"\t/?\tShows this help screen\n");
		logInfo(L"\t/l\tHardlink files, if not specified, tool will just search duplicates\n");
		logInfo(L"\t/a\tFile attributes must match for linking\n");
		logInfo(L"\t/t\tTime + Date of files must match\n");
		logInfo(L"\t/h\tSkip hidden files\n");
		logInfo(L"\t/s\tSkip system files\n");
		logInfo(L"\t/j\tSkip junctions (reparse points)\n");
		logInfo(L"\t/m\tLink small files <1024 bytes\n");
		logInfo(L"\t/r\tRuns recursively through the given folder list\n");
		logInfo(L"\t/q\tQuiet mode\n");
		logInfo(L"\t/v\tVerbose mode\n");
		logInfo(L"\t/d\tDebug mode\n");
	}
	bool		ParseCommandLine(int argc, PWSTR argv[]) {
		bool	pathAdded = false;

		if (argc == 1) {
			logInfo(L"%s\n", argv[0]);
			PrintHelp();
			return	false;
		}

		for (int i = 1; i < argc; ++i) {
			// first check if command line option
			if ((argv[i][0] == L'-' || argv[i][0] == L'/')) {
				if (Len(argv[i]) == 2) {
					switch (argv[i][1]) {
						case L'?':
							logInfo(L"%s\n", argv[0]);
							PrintHelp();
							return	false;
							break;
						case L'a':
							AttrMustMatch = true;
							break;
						case L'd':
							setLogLevel(LOG_DEBUG);
							break;
						case L'h':
							SkipHidden = true;
							break;
						case L'j':
							SkipJunct = true;
							break;
						case L'l':
							hardlink = true;
							break;
						case L'm':
							LinkSmall = true;
							break;
						case L'q':
							setLogLevel(LOG_ERROR);
							showStatistics = false;
							break;
						case L'r':
							recursive = true;
							break;
						case L's':
							SkipSystem = true;
							break;
						case L't':
							TimeMustMatch = true;
							break;
						case L'v':
							setLogLevel(LOG_VERBOSE);
							break;
						case L'w':
							showStatistics = true;
							break;
						default:
							logError(L"Illegal Command line option! Use /? to see valid options!\n");
							return	false;
					}
				} else {
					logError(L"Illegal Command line option! Use /? to see valid options!\n");
					return	false;
				}
			} else {
				Path* dir = parsePath(argv[i]);
				if (dir) {
					AddPath(dir);
					pathAdded = true;
				}
			}
		}

		if (!pathAdded) {
			logError(L"You need to specify at least one folder to process!\nUse /? to see valid options!\n");
			return	false;
		}
		return	true;
	}
	void		Process() {
		PathListIt it = paths.begin();
		while (it != paths.end()) {
			getFolderContent(*it);
			++it;
		}
		logInfo(L"Files to process:\t%8llu\n", data.size());

		logDebug(L"");
		data.sort(CompareBySizeAndTime);
		pair<FilesListIt, FilesListIt>	bounds;
		FilesListIt	srch = data.begin();
		WinBuf<WCHAR>	buf1(MAX_PATH_LEN);
		WinBuf<WCHAR>	buf2(MAX_PATH_LEN);
		while (srch != data.end()) {
			logCounter(L"Files left:\t%8llu", distance(srch, data.end()));
			bounds = equal_range(srch, data.end(), *srch, CompareBySize);
			if (distance(bounds.first, bounds.second) == 1) {
				++Statistics::getInstance()->filesFoundUnique;
				data.erase(bounds.first);
			} else {
				while (srch != bounds.second) {
					FilesListIt it = srch;
					++it;
					Shared_ptr<File>& f1 = *srch;
					while (it != bounds.second) {
						Shared_ptr<File>& f2 = *it;
						f1->copyName(buf1);
						f2->copyName(buf2);
						{
							ConsoleColor	col(FOREGROUND_INTENSITY | FOREGROUND_BLUE | FOREGROUND_GREEN);
							logDebug(L"Comparing files [size = %I64u]:\n", f1->size());
						}
						logDebug(L"  %s\n", buf1.data());
						logDebug(L"  %s\n", buf2.data());
						++Statistics::getInstance()->fileCompares;
						f1->LoadInode();
						f2->LoadInode();
						if (AttrMustMatch && f1->attr() != f2->attr()) {
							{
								ConsoleColor	col(FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN);
								logDebug(L"  Attributes of files do not match, skipping\n");
							}
							Statistics::getInstance()->fileMetaDataMismatch++;
							++it;
							break;
						}
						if (TimeMustMatch && f1->time() != f2->time()) {
							{
								ConsoleColor	col(FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN);
								logDebug(L"  Modification timestamps of files do not match, skipping\n");
							}
							Statistics::getInstance()->fileMetaDataMismatch++;
							++it;
							break;
						}
						if (!isSameVolume(f1, f2)) {
							{
								ConsoleColor	col(FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN);
								logDebug(L"  Files ignored - on different volumes\n");
							}
							++Statistics::getInstance()->filesOnDifferentVolumes;
							++it;
							break;
						}
						if (f1 == f2) {
							++Statistics::getInstance()->fileAlreadyLinked;
							{
								ConsoleColor	col(FOREGROUND_INTENSITY | FOREGROUND_GREEN);
								logDebug(L"  Files ignored - already linked\n");
							}
							++it;
							break;
						}
						if (f1->size() > FirstBlock) {
							if (!f1->LoadHashMini()) {
								break;
							}
							if (!f2->LoadHashMini()) {
								it = data.erase(it);
								continue;
							}
						} else {
							if (!f1->LoadHashFull()) {
								break;
							}
							if (!f2->LoadHashFull()) {
								it = data.erase(it);
								continue;
							}
						}
						if (isIdentical(f1, f2)) {
							++Statistics::getInstance()->fileContentSame;
							if (logLevel == LOG_VERBOSE) {
								{
									ConsoleColor	col(FOREGROUND_INTENSITY | FOREGROUND_BLUE | FOREGROUND_GREEN);
									logVerbose(L"Comparing files [size = %I64u]:\n", f1->size());
								}
								logVerbose(L"  %s\n", buf1.data());
								logVerbose(L"  %s\n", buf2.data());
							}
							{
								ConsoleColor	col(FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN);
								logVerbose(L"  Files are equal, hard link possible\n");
							}
							if (hardlink) {
								f1->hardlink(f2);
							}
							Statistics::getInstance()->FreeSpaceIncrease += f1->size();
							it = data.erase(it);
						} else {
							{
								ConsoleColor	col(FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN);
								logDebug(L"  Files differ in content (hash)\n");
							}
							Statistics::getInstance()->hashComparesHit1++;
							++it;
						}
					}
					srch = data.erase(srch);
				}
			}
			srch = bounds.second;
		}
		logCounter(L"                              ");
	}
	void		AddPath(Path* p) {
		paths.push_back(p);
	}
};

/// =========================================================================================== main
int			main() {
	{
		int		argc = 0;
		PWSTR	*argv = ::CommandLineToArgvW(::GetCommandLine(), &argc);
		FileSystem	fs;
		if (fs.ParseCommandLine(argc, argv)) {
			fs.Process();
		} else {
			showStatistics = false;
		}
		::LocalFree(argv); // do not replace
	}

	if (showStatistics) {
		Statistics*	s = Statistics::getInstance();
		logInfo(L"\n");
		{
			ConsoleColor	col(FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN);
			logInfo(L"Processing statistics:\n");
		}
		logInfo(L"  Files     found: %i\n", s->FoundFiles);
		logInfo(L"  Folders   found: %i\n", s->FoundDirs);
		logInfo(L"  Junctions found: %i\n", s->FoundJuncs);
		logInfo(L"\n");
		logInfo(L"  Files unique by size and skipped: %i\n", s->filesFoundUnique);
		logInfo(L"  Files were already linked: %i\n", s->fileAlreadyLinked);
		logInfo(L"  Files   which were on different volumes: %i\n", s->filesOnDifferentVolumes);
		logInfo(L"  Files   which were ignored by zero size: %i\n", s->IgnoredZero);
		logInfo(L"  Files   which were filtered by size: %i\n", s->IgnoredSmall);
		logInfo(L"  Files   which were filtered by system attribute: %i\n", s->IgnoredSystem);
		logInfo(L"  Files   which were filtered by hidden attribute: %i\n", s->IgnoredHidden);
		logInfo(L"  Folders which were filtered as junction: %i\n", s->IgnoredJunc);
		logInfo(L"\n");
		logInfo(L"  Number of file hashes calculated: %i\n", s->hashesCalculated);
		logInfo(L"  Files compared: %i\n", s->fileCompares);
		logInfo(L"  Files differ in first 65535 bytes: %i\n", s->fileContentDifferFirstBlock);
		logInfo(L"  Files compared using a hash: %i\n", s->hashCompares);
		logInfo(L"  Files content is same: %i\n", s->fileContentSame);
		logInfo(L"  Link generations: %i\n", s->hardLinksSuccess);
		logInfo(L"  Increase of free space: %I64i\n", s->FreeSpaceIncrease);
		logInfo(L"  Summary size of files: %I64i\n", s->FoundFilesSize);

		logDebug(L"\n");
		logDebug(L"  Path objects created: %i\n", s->pathObjCreated);
		logDebug(L"  Path objects destroyed: %i\n", s->pathObjDestroyed);
		logDebug(L"  File objects created: %i\n", s->fileObjCreated);
		logDebug(L"  File objects destroyed: %i\n", s->fileObjDestroyed);

		/*
		logInfo(L"  Files content differed in first 64 KiB: %i", s->fileContentDifferFirstBlock);
				logInfo(L"Files content differ after %i bytes: %i", FIRST_BLOCK_SIZE, s->fileContentDifferLater);
				logInfo(L"File compare problems: %i", s->fileCompareProblems);
				logInfo(L"Number of files opened: %i", s->filesOpened);
				logInfo(L"Number of files closed: %i", s->filesClosed);
				logInfo(L"Number of file open problems: %i", s->fileOpenProblems);
		*/
	}

	return	0;
}
