/* FileSystem.h : Headers for File System specific functions for DFHL

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

#ifndef FILESYSTEM_HPP
#define FILESYSTEM_HPP

///===================================================================================== FolderEntry
/// Abstract base class for all entries of a folder in the file system
class		FolderEntry : public ReferenceCounter {
protected:
	bool		file;
public:
	bool		isFile() const {	/// Checks if this object represents a file
		return	file;
	}
	bool		isFolder() const {	/// Checks if this object represents a file
		return	!file;
	}
};

///============================================================================================ Path
/// Container class which represents a folder in the file system and creates the directory hierarchy in the file system
class		Path : public FolderEntry {
	Path*		parent;
	PWSTR		name;
public:
	~Path() {
		Statistics::getInstance()->pathObjDestroyed++;
		delete	name;
		if (parent != NULL) {
			parent->removeReference();
		}
	}
	Path(PWSTR newName, Path* newParent) {
		Statistics::getInstance()->pathObjCreated++;
		classType = CLASS_TYPE_PATH;
		file = false;
		name = new WCHAR[Len(newName) + 1];
		Copy(name, newName);
		parent = newParent;
		if (newParent != NULL) {
			parent->addReference();
		}
	}

	void		copyName(PWSTR buffer) {
		/// Recursively copies the name of this path into the specified buffer.
		/// If this entry has a parent, all parents up to the root of the file system are copied to buffer
		/// @param buffer temporary buffer variable where the path should be copied to
		/// recursively copy the parent path
		if (parent != NULL) {
			parent->copyName(buffer);
			Cat(buffer, PATH_SEPARATOR);
		}
		Cat(buffer, name);
	}

	bool		equals(Path* otherPath) {
		/// Checks if a file reference equals an other file reference
		if (otherPath == NULL || !Eqi(name, otherPath->name)) {
			return	false;
		}
		if (parent != NULL) {
			return	parent->equals(otherPath->parent);
		}
		return	true;
	}
};

///============================================================================================ File
/// Container class which represents a file in the file system
class		File : public FolderEntry {
	Path*		parent;
	PWSTR		name;
	DWORD		attributes;
	FILETIME	lastModifyTime;
	uint64_t	size;
	uint64_t	hash;
	uint64_t	node;
	bool		hashAvailable;
public:
	~File() {
		Statistics::getInstance()->fileObjDestroyed++;
		delete[]	name;
		parent->removeReference();
	}
	File(WIN32_FIND_DATAW info, Path* newParent) {
		Statistics::getInstance()->fileObjCreated++;
		classType = CLASS_TYPE_FILE;
		file = true;
		name = new WCHAR[Len(info.cFileName) + 1];
		Copy(name, info.cFileName);
		parent = newParent;
		parent->addReference();
		size = MyUI64(info.nFileSizeLow, info.nFileSizeHigh);
		attributes = info.dwFileAttributes;
		lastModifyTime = info.ftLastWriteTime;
		hash = 0ull;
		node = 0ull;
		hashAvailable = false;
	}

	PWSTR			getName() const {
		return	name;
	}
	DWORD			getAttributes() const {
		return	attributes;
	}
	FILETIME		getLastModifyTime() const {
		return	lastModifyTime;
	}
	bool			isHashAvailable() const {
		return	hashAvailable;
	}
	uint64_t		getSize() const {
		return	size;
	}
	uint64_t		getHash() const {
		return	hash;
	}
	uint64_t		getNode() const {
		return	node;
	}
	void			setHash(uint64_t newHash) {
		hashAvailable = true;
		hash = newHash;
	}

	void			copyName(PWSTR buffer) const {
		/// copies the absolute file system name of this file into the specified buffer.
		/// @param buffer temporary buffer variable where the path should be copied to
		/// recursively copy the parent path
		if (parent != NULL) {
			parent->copyName(buffer);
			Cat(buffer, PATH_SEPARATOR);
		}
		Cat(buffer, name);
	}

	bool			equals(File* otherFile) const {
		/// Checks if a file reference equals an other file reference
		if (otherFile == NULL) {
			return	false;
		}
		if (!Eqi(name, otherFile->name)) {
			return	false;
		}
		return	parent->equals(otherFile->parent);
	}
};

///============================================================================ UnbufferedFileStream
/// class implementation for a byte stream operations
class		UnbufferedFileStream {
	File*		file;
	HANDLE		hFile;

public:
	~UnbufferedFileStream() {
		Statistics::getInstance()->unbufferedFileStreamObjDestroyed++;
	}
	UnbufferedFileStream(File* newFile) {
		Statistics::getInstance()->unbufferedFileStreamObjCreated++;
		file = newFile;
	}
	bool		open();												// Open the file for reading
	void		close() {											// Closes the file stream
		Statistics::getInstance()->filesClosed++;
		::CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}
	bool		getFileDetails(BY_HANDLE_FILE_INFORMATION* info) {	// Retrieves further file information from the opened file stream
		return	::GetFileInformationByHandle(hFile, info) == TRUE;
	}
	size_t		read(LPVOID buffer, size_t bytesToRead) {			// Reads data from the file
		DWORD	Result;
		::ReadFile(hFile, buffer, bytesToRead, &Result, NULL);
		Statistics::getInstance()->bytesRead += Result;
		return	Result;
	}
};

///====================================================================================== FileSystem
/// logic implementation for abstraction of the file system
class		FileSystem {
	bool		followJunctions;
	bool		hiddenFiles;
	bool		systemFiles;
public:
	~FileSystem() {
		Statistics::getInstance()->fileSystemObjDestroyed++;
	}
	FileSystem(): followJunctions(false), hiddenFiles(false), systemFiles(false) {
		Statistics::getInstance()->fileSystemObjCreated++;
	}

	/**
	 * Parses a path string and returns an appropriate path object
	 * @return Path object, NULL if the path is not existing
	 */
	Path*		parsePath(PCWSTR pathStr);

	/**
	 * Retrievs a list of items in the given folder
	 * @return Collection of FolderEntry Objects
	 * @throws LPWSTR If the file finding generates a unexpected error (Not access denied)
	 */
	Collection*	getFolderContent(Path* folder);

	/**
	 * Links two files on hard disk by deletiong one and linking the name of this to the other
	 * @param file1 File name of the first file
	 * @param file2 File name of the second file to link to
	 * @return boolean value if operation was successful
	 */
	bool		hardLinkFiles(const File* file1, const File* file2);

	void		setFollowJunctions(bool newValue) {			// Sets the configuration option for following junctions
		followJunctions = newValue;
	}
	void		setHiddenFiles(bool newValue) {				// Sets the configuration option for using hidden files and directories
		hiddenFiles = newValue;
	}
	void		setSystemFiles(bool newValue) {				// Sets the configuration option for using system files and directories
		systemFiles = newValue;
	}
};

#endif // FILESYSTEM_HPP
