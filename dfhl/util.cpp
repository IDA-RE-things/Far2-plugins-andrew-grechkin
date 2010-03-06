/* Util.cpp : Utility function for DFHL

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

#include <stdio.h>
#include "Util.h"
#include "FileSystem.h"

/// Logging Level for the console output
int			logLevel = LOG_INFO;

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

void		setLogLevel(int newLevel) {
	logLevel = newLevel;
}

void		logFile(WIN32_FIND_DATA FileData) {
	INT64 size = FileData.nFileSizeLow + ((INT64)MAXDWORD + 1) * FileData.nFileSizeHigh;
	logDebug(L"Found file \"%s\" (Size=%I64i,%s%s%s%s%s%s%s%s%s%s%s%s)",
			 FileData.cFileName,
			 size,
			 FILE_ATTRIBUTE_ARCHIVE      &FileData.dwFileAttributes ? L"ARCHIVE " : L"",
			 FILE_ATTRIBUTE_COMPRESSED   &FileData.dwFileAttributes ? L"COMPRESSED " : L"",
			 FILE_ATTRIBUTE_DIRECTORY    &FileData.dwFileAttributes ? L"DIRECTORY " : L"",
			 FILE_ATTRIBUTE_ENCRYPTED    &FileData.dwFileAttributes ? L"ENCRYPTED " : L"",
			 FILE_ATTRIBUTE_HIDDEN       &FileData.dwFileAttributes ? L"HIDDEN " : L"",
			 FILE_ATTRIBUTE_NORMAL       &FileData.dwFileAttributes ? L"NORMAL " : L"",
			 FILE_ATTRIBUTE_OFFLINE      &FileData.dwFileAttributes ? L"OFFLINE " : L"",
			 FILE_ATTRIBUTE_READONLY     &FileData.dwFileAttributes ? L"READONLY " : L"",
			 FILE_ATTRIBUTE_REPARSE_POINT&FileData.dwFileAttributes ? L"REPARSE_POINT " : L"",
			 FILE_ATTRIBUTE_SPARSE_FILE  &FileData.dwFileAttributes ? L"SPARSE " : L"",
			 FILE_ATTRIBUTE_SYSTEM       &FileData.dwFileAttributes ? L"SYSTEM " : L"",
			 FILE_ATTRIBUTE_TEMPORARY    &FileData.dwFileAttributes ? L"TEMP " : L"");
}

///================================================================================ ReferenceCounter
void		ReferenceCounter::removeReference() {
	--numberOfReferences;
	// Delete the object if no further reference exists
	if (numberOfReferences < 1) {
		switch (classType) {
			case CLASS_TYPE_FILE:
				delete(File*)this;
				break;
			case CLASS_TYPE_PATH:
				delete(Path*)this;
				break;
			default:
				delete	this;
				break;
		}
	}
}

///====================================================================================== Collection
Collection::~Collection() {
	Statistics::getInstance()->collectionObjDestroyed++;
	while (m_cnt > 0)
		pop();
}

void		Collection::append(void* data) {
	Item*	newItem = new Item(data);
	if (root != NULL) {
		last->next = newItem;
		last = newItem;
	} else {
		root = last = newItem;
	}
	++m_cnt;
	m_next = root;
}

void		Collection::addAll(Collection* items) {
	if (items == NULL) {
		return;
	}
	void*	item;
	while ((item = items->pop())) {
		append(item);
	}
}

void*		Collection::pop() {
	if (root != NULL) {
		Item*	temp = root;
		m_next = root = root->next;
		if (last == temp) {
			last = root;
		}
		--m_cnt;
		void*	Result = temp->data;
		delete	temp;
		return	Result;
	} else {
		return	NULL;
	}
}

void*		Collection::next() {
	if (m_next != NULL) {
		Item*	temp = m_next;
		m_next = m_next->next;
		return	temp->data;
	} else {
		return	NULL;
	}
}

void*		Collection::item(size_t index) {
	if (index < m_cnt) {
		Item*	temp = root;
		for (size_t i = 0; i < index; i++) {
			temp = temp->next;
		}
		m_next = temp->next;
		return	temp->data;
	} else {
		return	NULL;
	}
}

