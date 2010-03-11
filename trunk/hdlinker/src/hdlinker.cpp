/**
	hdlinker console tool
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

#include "hdlinker.h"
#include "version.h"

#include <stdio.h>

#include <list>
#include <algorithm>
using namespace std;

typedef		list<Shared_ptr<File> >	FilesList;
typedef		FilesList::iterator		FilesListIt;

#define FIRST_BLOCK_SIZE		65536 // Smaller block size
#define MIN_FILE_SIZE			1024 // Minimum file size so that hard linking will be checked...

bool		showStatistics = true;
int			logLevel = LOG_INFO;

///========================================================================================= Logging
void		setLogLevel(LogLevel newLevel) {
	logLevel = newLevel;
}
void		logError(PCWSTR message, ...) {
	va_list argp;
	fwprintf(stderr, L"ERROR: ");
	va_start(argp, message);
	vfwprintf(stderr, message, argp);
	va_end(argp);
	fwprintf(stderr, L"\n");
}
void		logError(DWORD errNumber, PCWSTR message, ...) {
	va_list argp;
	fwprintf(stderr, L"ERROR: ");
	va_start(argp, message);
	vfwprintf(stderr, message, argp);
	va_end(argp);
	LPWSTR msgBuffer = NULL;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		errNumber,
		GetSystemDefaultLangID(),
		(LPWSTR)(&msgBuffer),
		0,
		NULL);
	fwprintf(stderr, L" -> [%i] %s\n", errNumber, msgBuffer);
	LocalFree(msgBuffer);
}
void		logInfo(PCWSTR message, ...) {
	if (logLevel <= LOG_INFO) {
		va_list argp;
		va_start(argp, message);
		vwprintf(message, argp);
		va_end(argp);
		wprintf(L"\n");
	}
}
void		logVerbose(PCWSTR message, ...) {
	if (logLevel <= LOG_VERBOSE) {
		va_list argp;
		wprintf(L"  ");
		va_start(argp, message);
		vwprintf(message, argp);
		va_end(argp);
		wprintf(L"\n");
	}
}
void		logDebug(PCWSTR message, ...) {
	if (logLevel <= LOG_DEBUG) {
		va_list argp;
		wprintf(L"    ");
		va_start(argp, message);
		vwprintf(message, argp);
		va_end(argp);
		wprintf(L"\n");
	}
}
void		logFile(WIN32_FIND_DATA info) {
	uint64_t	size = MyUI64(info.nFileSizeLow, info.nFileSizeHigh);
	logDebug(L"%s   found: \"%s\" (Size=%I64i,%s%s%s%s%s%s%s%s%s%s%s)",
			 FILE_ATTRIBUTE_DIRECTORY    &info.dwFileAttributes ? L"Dir " : L"File",
			 info.cFileName,
			 size,
			 FILE_ATTRIBUTE_ARCHIVE      &info.dwFileAttributes ? L"ARCHIVE " : L"",
			 FILE_ATTRIBUTE_COMPRESSED   &info.dwFileAttributes ? L"COMPRESSED " : L"",
			 FILE_ATTRIBUTE_ENCRYPTED    &info.dwFileAttributes ? L"ENCRYPTED " : L"",
			 FILE_ATTRIBUTE_HIDDEN       &info.dwFileAttributes ? L"HIDDEN " : L"",
			 FILE_ATTRIBUTE_NORMAL       &info.dwFileAttributes ? L"NORMAL " : L"",
			 FILE_ATTRIBUTE_OFFLINE      &info.dwFileAttributes ? L"OFFLINE " : L"",
			 FILE_ATTRIBUTE_READONLY     &info.dwFileAttributes ? L"READONLY " : L"",
			 FILE_ATTRIBUTE_REPARSE_POINT&info.dwFileAttributes ? L"REPARSE_POINT " : L"",
			 FILE_ATTRIBUTE_SPARSE_FILE  &info.dwFileAttributes ? L"SPARSE " : L"",
			 FILE_ATTRIBUTE_SYSTEM       &info.dwFileAttributes ? L"SYSTEM " : L"",
			 FILE_ATTRIBUTE_TEMPORARY    &info.dwFileAttributes ? L"TEMP " : L"");
}

///====================================================================================== FileSystem
class		FileSystem {
	FilesList	data;
	list<Shared_ptr<Path> > paths;

	bool		recursive;
	bool		hardlink;
	bool		AttrMustMatch;
	bool		TimeMustMatch;
	bool		LinkSmall;
	bool		SkipJunct;
	bool		SkipHidden;
	bool		SkipSystem;

	Path*		parsePath(const CStrW &path) {
		CStrW	extendedPath(PATH_PREFIX);

		CStrW	absolutePath(MAX_PATH_LENGTH);
		::GetFullPathName(path, absolutePath.capacity(), absolutePath.buffer(), NULL);
		extendedPath += absolutePath;

		::GetLongPathName(extendedPath, extendedPath.buffer(), extendedPath.capacity());
		CharLastNotOf(extendedPath, L"\\ ")[1] = STR_END;		//erase tailing path separators
		if (!IsExist(extendedPath)) {
			return	NULL;
		}

		Path*	Result = NULL;
		if (Empty(extendedPath)) {
			logError(L"Path \"%s\" is not existing or accessible!", extendedPath.c_str());
		} else {
			Result = new Path(NULL, extendedPath);
			logInfo(L"Parsing directory: \"%s\"", path.c_str());
		}
		return	Result;
	}
	bool		getFolderContent(Shared_ptr<Path> folder) {
		DWORD	dwError = 0;
		CStrW	root(MAX_PATH_LENGTH);
		folder->copyName(root.buffer());
		WinFS::AddSlash(root);
		root += L"*";

		WIN32_FIND_DATAW	info;
		HANDLE	hFind = ::FindFirstFileW(root, &info);
		if (hFind == INVALID_HANDLE_VALUE) {
			logError(::GetLastError(), L"Unable to read folder content.");
		} else {
			do {
				if (FileValidName(info.cFileName)) {
					if (!(info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
						++Statistics::getInstance()->FoundFiles;
						if (SkipHidden && (info.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)) {
							++Statistics::getInstance()->IgnoredHidden;
							logVerbose(L"File ignored: \"%s\" [hidden]", info.cFileName);
						} else if (SkipSystem && (info.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)) {
							++Statistics::getInstance()->IgnoredSystem;
							logVerbose(L"File ignored: \"%s\" [system]", info.cFileName);
						} else if (!LinkSmall && (MyUI64(info.nFileSizeLow, info.nFileSizeHigh) < MIN_FILE_SIZE)) {
							++Statistics::getInstance()->IgnoredSmall;
							logVerbose(L"File ignored: \"%s\" [small]", info.cFileName);
						} else {
							logFile(info);
							data.push_back(new File(folder, info));
						}
					} else {
						if (SkipJunct && info.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
							++Statistics::getInstance()->IgnoredJunc;
							logDebug(L"Dir  ignored: \"%s\" [junction]", info.cFileName);
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
				logInfo(L"FindNextFile error. Error is %s\n", Err(dwError).c_str());
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
		logInfo(L"Search duplicate files and make hardlinks.");
		logInfo(L"(C) 2010 %s\thttp://code.google.com/p/andrew-grechkin/", L"Andrew Grechkin");
		logInfo(L"");
		logInfo(L"NOTE: Use this tool on your own risk!");
		logInfo(L"");
		logInfo(L"Usage:");
		logInfo(L"\thardlinker [options] [path] [path] [...]");
		logInfo(L"Options:");
		logInfo(L"/?\tShows this help screen");
		logInfo(L"/l\tHardlink files, if not specified, tool will just read search duplicates");
		logInfo(L"/a\tFile attributes must match for linking");
		logInfo(L"/t\tTime + Date of files must match");
		logInfo(L"/h\tSkip hidden files");
		logInfo(L"/s\tSkip system files");
		logInfo(L"/j\tSkip junctions (reparse points)");
		logInfo(L"/m\tLink small files <1024 bytes");
		logInfo(L"/r\tRuns recursively through the given folder list");
		logInfo(L"/q\tQuiet mode");
		logInfo(L"/v\tVerbose mode");
		logInfo(L"/d\tDebug mode");
	}
	bool		ParseCommandLine(int argc, PWSTR argv[]) {
		bool	pathAdded = false;

		if (argc == 1) {
			logInfo(argv[0]);
			PrintHelp();
			return	false;
		}

		for (int i = 1; i < argc; ++i) {
			// first check if command line option
			if (argv[i][0] == L'-' || argv[i][0] == L'/') {
				if (Len(argv[i]) == 2) {
					switch (argv[i][1]) {
						case L'?':
							logInfo(argv[0]);
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
							logError(L"Illegal Command line option! Use /? to see valid options!");
							return	false;
					}
				} else {
					logError(L"Illegal Command line option! Use /? to see valid options!");
					return	false;
				}
			} else {
				Path* dir = parsePath(argv[i]);
				if (dir) {
					paths.push_back(dir);
					pathAdded = true;
				}
			}
		}

		if (!pathAdded) {
			logError(L"You need to specify at least one folder to process!\nUse /? to see valid options!");
			return	false;
		}
		return	true;
	}
	void		Process() {
		list<Shared_ptr<Path> >::iterator it = paths.begin();
		while (it != paths.end()) {
			getFolderContent(*it);
			++it;
		}
		logInfo(L"Found files: %u", data.size());
		logInfo(L"Processing...");

		logDebug(L"");
		data.sort(CompareBySize);
		pair<FilesListIt, FilesListIt>	bounds;
		FilesListIt	srch = data.begin();
		size_t	ctr = 1;
		WCHAR	ctr_txt[20];
		while (srch != data.end()) {
			if (logLevel == LOG_INFO) {
				Num2Str(ctr_txt, ctr);
				consoleout(ctr_txt);
				consoleout(L"\r");
			}
			bounds = equal_range(srch, data.end(), *srch, CompareBySize);
			if (distance(bounds.first, bounds.second) == 1) {
				++ctr;
				++Statistics::getInstance()->filesFoundUnique;
				data.erase(bounds.first);
			} else {
				while (srch != bounds.second) {
					FilesListIt it = srch;
					++it;
					Shared_ptr<File>& f1 = *srch;
					while (it != bounds.second) {
						++ctr;
						Shared_ptr<File>& f2 = *it;
						logDebug(L"Comparing files \"%s\" and \"%s\"", f1->name().c_str(), f2->name().c_str());
						++Statistics::getInstance()->fileCompares;
						f1->LoadInode();
						f2->LoadInode();
						if (AttrMustMatch && f1->attr() != f2->attr()) {
							logDebug(L"Attributes of files do not match, skipping.");
							Statistics::getInstance()->fileMetaDataMismatch++;
							++it;
							break;
						}
						if (TimeMustMatch && f1->time() != f2->time()) {
							logDebug(L"Modification timestamps of files do not match, skipping.");
							Statistics::getInstance()->fileMetaDataMismatch++;
							++it;
							break;
						}
						if (!isSameVolume(f1, f2)) {
							logDebug(L"Files ignored - on different volumes");
							++Statistics::getInstance()->filesOnDifferentVolumes;
							++it;
							break;
						}
						if (f1 == f2) {
							++Statistics::getInstance()->fileAlreadyLinked;
							logDebug(L"Files ignored - already linked");
							++it;
							break;
						}
						if (isIdentical(f1, f2)) {
							++Statistics::getInstance()->fileContentSame;
							if (logLevel == LOG_VERBOSE) {
								CStrW	buf1(MAX_PATH_LENGTH);
								CStrW	buf2(MAX_PATH_LENGTH);
								f1->copyName(buf1.buffer());
								f2->copyName(buf2.buffer());
								logVerbose(L"\"%s\" \"%s\"", buf1.c_str(), buf2.c_str());
								logVerbose(L"Files are equal, hard link possible.");
							} else
								logDebug(L"Files are equal, hard link possible.");
							if (hardlink)
								f1->hardlink(f2);
							it = data.erase(it);
						} else {
							logDebug(L"Files differ in content (hash).");
							Statistics::getInstance()->hashComparesHit1++;
							++it;
						}
					}
					srch = data.erase(srch);
				}
			}
			srch = bounds.second;
		}
	}
};

/// =========================================================================================== main
int			main() {
	{
		int		argc = 0;
		PWSTR	*argv = ::CommandLineToArgvW(::GetCommandLine(), &argc);
		FileSystem fs;
		if (fs.ParseCommandLine(argc, argv))
			fs.Process();
		::LocalFree(argv); // do not replace
	}

	if (showStatistics) {
		Statistics*	s = Statistics::getInstance();
		logInfo(L"");
		logInfo(L"Processing statistics:");
		logInfo(L"  Files     found: %i", s->FoundFiles);
		logInfo(L"  Folders   found: %i", s->FoundDirs);
		logInfo(L"  Junctions found: %i", s->FoundJuncs);
		logInfo(L"");
		logInfo(L"  Files unique by size and skipped: %i", s->filesFoundUnique);
		logInfo(L"  Files were already linked: %i", s->fileAlreadyLinked);
		logInfo(L"  Files   which were on different volumes: %i", s->filesOnDifferentVolumes);
		logInfo(L"  Files   which were filtered by size: %i", s->IgnoredSmall);
		logInfo(L"  Files   which were filtered by system attribute: %i", s->IgnoredSystem);
		logInfo(L"  Files   which were filtered by hidden attribute: %i", s->IgnoredHidden);
		logInfo(L"  Folders which were filtered as junction: %i", s->IgnoredJunc);
		logInfo(L"");
		logInfo(L"  Number of file hashes calculated: %i", s->hashesCalculated);
		logInfo(L"  Files compared: %i", s->fileCompares);
		logInfo(L"  Files compared using a hash: %i", s->hashCompares);
		logInfo(L"  Files content is same: %i", s->fileContentSame);
		logInfo(L"  Link generations: %i", s->hardLinksSuccess);

		logDebug(L"Path objects created: %i", s->pathObjCreated);
		logDebug(L"Path objects destroyed: %i", s->pathObjDestroyed);
		logDebug(L"File objects created: %i", s->fileObjCreated);
		logDebug(L"File objects destroyed: %i", s->fileObjDestroyed);
		/*
				logInfo(L"Files content differed in first %i bytes: %i", FIRST_BLOCK_SIZE, s->fileContentDifferFirstBlock);
				logInfo(L"Files content differ after %i bytes: %i", FIRST_BLOCK_SIZE, s->fileContentDifferLater);
				logInfo(L"File compare problems: %i", s->fileCompareProblems);
				logInfo(L"Number of bytes read from disk: %I64i", s->bytesRead);
				logInfo(L"Number of files opened: %i", s->filesOpened);
				logInfo(L"Number of files closed: %i", s->filesClosed);
				logInfo(L"Number of file open problems: %i", s->fileOpenProblems);
		*/
	}
	return	0;
}
