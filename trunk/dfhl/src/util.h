/* Util.h : Headers for Utility functions for DFHL

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

#ifndef UTIL_HPP
#define UTIL_HPP

// Global Definitions
// *******************************************
#define LOG_ERROR				1
#define LOG_INFO				0
#define LOG_VERBOSE				-1
#define LOG_DEBUG				-2

#define CLASS_TYPE_FILE         1
#define CLASS_TYPE_PATH         2

///====================================================================================== Statistics
/// Some global Variables for Statistics stuff...
struct		Statistics {
	/// Variable for statistic collection
	INT64		bytesRead;
	size_t		fileCompares;
	size_t		hashCompares;
	size_t		hashComparesHit1;
	size_t		hashComparesHit2;
	size_t		hashComparesHit3;
	size_t		fileMetaDataMismatch;
	size_t		fileAlreadyLinked;
	size_t		fileContentDifferFirstBlock;
	size_t		fileContentDifferLater;
	size_t		fileContentSame;
	size_t		fileCompareProblems;
	size_t		hashesCalculated;
	size_t		directoriesFound;
	size_t		filesFound;
	size_t		filesOpened;
	size_t		filesClosed;
	size_t		fileOpenProblems;
	size_t		pathObjCreated;
	size_t		pathObjDestroyed;
	size_t		fileObjCreated;
	size_t		fileObjDestroyed;
	size_t		unbufferedFileStreamObjCreated;
	size_t		unbufferedFileStreamObjDestroyed;
	size_t		fileSystemObjCreated;
	size_t		fileSystemObjDestroyed;
	size_t		hardLinks;
	size_t		hardLinksSuccess;
	size_t		collectionObjCreated;
	size_t		collectionObjDestroyed;
	size_t		itemObjCreated;
	size_t		itemObjDestroyed;
	size_t		referenceCounterObjCreated;
	size_t		referenceCounterObjDestroyed;
	size_t		sizeGroupObjCreated;
	size_t		sizeGroupObjDestroyed;
	size_t		sortedFileCollectionObjCreated;
	size_t		sortedFileCollectionObjDestroyed;
	size_t		duplicateEntryObjCreated;
	size_t		duplicateEntryObjDestroyed;
	size_t		duplicateFileCollectionObjCreated;
	size_t		duplicateFileCollectionObjDestroyed;
	size_t		duplicateFileHardLinkerObjCreated;
	size_t		duplicateFileHardLinkerObjDestroyed;
	size_t		fileTooSmall;

	static Statistics* getInstance() {
		static	Statistics instance;
		return	&instance;
	}
};

// Global Functions
// *******************************************

void	logError(PCWSTR message, ...);

void	logError(DWORD errNumber, PCWSTR message, ...);

void	logInfo(PCWSTR message, ...);

void	logVerbose(PCWSTR message, ...);

void	logDebug(PCWSTR message, ...);

void	setLogLevel(int newLevel);

void	logFile(WIN32_FIND_DATA FileData);

inline PWSTR cleanString(PWSTR str) {		// Cleans the provided String
	str[0] = 0;
	return	str;
}


///================================================================================ ReferenceCounter
///Abstract base class to implement "garbage collection" features by counting the number of references
class		ReferenceCounter {
	size_t		numberOfReferences;
protected:
	size_t		classType;	// signature of the class type for object following and correct destruction
public:
	~ReferenceCounter() {
		Statistics::getInstance()->referenceCounterObjDestroyed++;
	}
	ReferenceCounter(): numberOfReferences(1), classType(0) {
		Statistics::getInstance()->referenceCounterObjCreated++;
	}

	void			addReference() {	// Signals the object that a further reference is created
		numberOfReferences++;
	}
	void			removeReference();	// Signals the object that a reference is removed - the object needs to check whether it can be destroyed
};

///============================================================================================ Item
class		Item {		// Item of the collection. Each entry it of type Item
public:
	void*		data;
	Item*		next;

	~Item() {
		Statistics::getInstance()->itemObjDestroyed++;
	}
	Item(void* defaultData) {
		Statistics::getInstance()->itemObjCreated++;
		data = defaultData;
		next = NULL;
	}
};

///====================================================================================== Collection
/// Simple implementation for a collection
class		Collection {
	size_t		m_cnt;
	Item*		root;
	Item*		last;
	Item*		m_next;
public:
	~Collection();
	Collection(): m_cnt(0), root(NULL), last(NULL), m_next(NULL) {
		Statistics::getInstance()->collectionObjCreated++;
	}

	void		addAll(Collection* items);
	void		append(void* data);

	void		push(void* data) {
		append(data);
	}
	void*		pop();

	void*		next();

	void*		item(size_t index);

	size_t		capacity() const {
		return	m_cnt;
	}
};

#endif // UTIL_HPP
