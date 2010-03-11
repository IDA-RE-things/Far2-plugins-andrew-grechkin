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
#ifndef __HARDLINKER__HPP
#define __HARDLINKER__HPP

#include "win_def.h"

#include <Wincrypt.h>

enum	LogLevel {
	LOG_DEBUG =	-2,
	LOG_VERBOSE,
	LOG_INFO,
	LOG_ERROR,
};

void		setLogLevel(LogLevel newLevel);
void		logDebug(PCWSTR message, ...);
void		logVerbose(PCWSTR message, ...);
void		logInfo(PCWSTR message, ...);
void		logError(PCWSTR message, ...);
void		logError(DWORD errNumber, PCWSTR message, ...);
void		logFile(WIN32_FIND_DATA FileData);

///====================================================================================== Statistics
struct		Statistics {
	size_t		FoundDirs;
	size_t		FoundJuncs;
	size_t		FoundFiles;
	size_t		filesFoundUnique;
	size_t		IgnoredHidden;
	size_t		IgnoredSystem;
	size_t		IgnoredSmall;
	size_t		IgnoredJunc;
	size_t		filesOnDifferentVolumes;

	uint64_t	bytesRead;
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

	static	Statistics*  getInstance() {
		static	Statistics instance;
		return	&instance;
	}
};

///==================================================================================== WinCryptProv
class		WinCryptProv: public WinErrorCheck {
	HCRYPTPROV	m_hnd;

public:
	~WinCryptProv() {
		if (m_hnd) {
			::CryptReleaseContext(m_hnd, 0);
		}
	}
	// type = (PROV_RSA_FULL, PROV_RSA_AES)
	WinCryptProv(PCWSTR prov = NULL, DWORD type = PROV_RSA_FULL): m_hnd(NULL) {
		if (!ChkSucc(::CryptAcquireContextW(&m_hnd, L"MY", prov, type, 0))) {
			ChkSucc(::CryptAcquireContextW(&m_hnd, L"MY", prov, type, CRYPT_NEWKEYSET));
		}
	}
	operator		HCRYPTPROV() const {
		return	m_hnd;
	}
};

///==================================================================================== WinCryptHash
class		WinCryptHash: public WinErrorCheck {
	HCRYPTHASH		m_handle;

	bool			Close() {
		if (m_handle) {
			::CryptDestroyHash(m_handle);
			return	true;
		}
		return	false;
	}
public:
	~WinCryptHash() {
		Close();
	}
	// alg = (CALG_MD5, CALG_SHA1, CALG_SHA_512)
	WinCryptHash(HCRYPTPROV prov, ALG_ID alg): m_handle(NULL) {
		ChkSucc(::CryptCreateHash(prov, alg, 0, 0, &m_handle));
	}
	operator		HCRYPTHASH() const {
		return	m_handle;
	}
	bool			Hash(const PBYTE buf, size_t size) {
		return	ChkSucc(::CryptHashData(m_handle, buf, size, 0));
	}
	bool			Hash(PCWSTR path) {
		FileMap	file(path);
		if (file.IsOK())
			return	Hash(file);
		err(file.err());
		return	false;
	}
	bool			Hash(FileMap &file) {
		bool	Result;
		file.Home();
		while (file.Next())
			Result = Hash((PBYTE)file.data(), file.size());
		return	Result;
	}
	size_t			GetHashSize() const {
		DWORD	Result = 0;
		::CryptGetHashParam(m_handle, HP_HASHVAL, NULL, &Result, 0);
		return	Result;
	}
	ALG_ID			GetHashAlg() const {
		DWORD	Result = 0;
		::CryptGetHashParam(m_handle, HP_ALGID, NULL, &Result, 0);
		return	Result;
	}
	bool			GetHash(PBYTE buf, DWORD size) const {
		return	ChkSucc(::CryptGetHashParam(m_handle, HP_HASHVAL, buf, &size, 0));
	}
};


///============================================================================================ Path
class		Path {
	Shared_ptr<Path>	parent;
	CStrW				name;
public:
	~Path() {
		Statistics::getInstance()->pathObjDestroyed++;
	}
	Path(Shared_ptr<Path> newParent, PCWSTR newName): parent(newParent), name(newName) {
		Statistics::getInstance()->pathObjCreated++;
	}

	void		copyName(PWSTR buf) const {
		if (parent) {
			parent->copyName(buf);
			Cat(buf, PATH_SEPARATOR);
		}
		Cat(buf, name);
	}
	bool		equals(Path* otherPath)  const {
		if (!otherPath || !Eqi(name, otherPath->name)) {
			return	false;
		}
		if (parent != NULL) {
			return	parent->equals(otherPath->parent);
		}
		return	true;
	}
};

///======================================================================================== FileHash
class		FileHash {
	BYTE	m_hash[20];
	BYTE	m_hash2[12];
	bool mutable m_avail;
public:
	FileHash(): m_avail(false) {
		WinMem::Zero(m_hash2, sizeofa(m_hash2));
	}
	PBYTE	hash(const PBYTE hash, size_t sz) const {
		WinMem::Copy((PVOID)m_hash, (PCVOID)hash, Min(size(), sz));
		return	(PBYTE)&m_hash;
	}
	PBYTE	hash() const {
		return	(PBYTE)&m_hash;
	}
	size_t	size() const {
		return	sizeofa(m_hash);
	}
	bool	avail(bool in) const {
		return	m_avail = in;
	}
	bool	avail() const {
		return	m_avail;
	}
	bool	operator==(const FileHash &rhs) const {
		return	WinMem::Cmp(m_hash, rhs.m_hash, size());
	}
};

///============================================================================================ File
class		File {
	FileHash	m_hash;
	WinFileId	m_inode;
	Shared_ptr<Path>	parent;
	CStrW		m_name;
	DWORD		m_attr;
	uint64_t	m_size;
	uint64_t	m_time;
public:
	~File() {
		++Statistics::getInstance()->fileObjDestroyed;
	}
	File(Shared_ptr<Path> newParent, const WIN32_FIND_DATAW &info): parent(newParent),
			m_name(info.cFileName),
			m_attr(info.dwFileAttributes),
			m_size(MyUI64(info.nFileSizeLow, info.nFileSizeHigh)),
			m_time(MyUI64(info.ftLastWriteTime.dwLowDateTime, info.ftLastWriteTime.dwHighDateTime)) {
		Statistics::getInstance()->fileObjCreated++;
	}

	CStrW		name() const {
		return	m_name;
	}
	DWORD		attr() const {
		return	m_attr;
	}
	uint64_t	time() const {
		return	m_time;
	}
	const FileHash&	hash() const {
		return	m_hash;
	}
	bool		isHashAvailable() const {
		return	m_hash.avail();
	}
	uint64_t	size() const {
		return	m_size;
	}
	const WinFileId&	inode() const {
		return	m_inode;
	}
	bool		operator<(const File &rhs) const {
		return	m_size < rhs.m_size;
	}
	bool		LoadHash() const {
		static WinCryptProv	hCryptProv(NULL, PROV_RSA_AES);
		DWORD	err = hCryptProv.err();
		if (hCryptProv.IsOK()) {
			WinCryptHash	hSHA(hCryptProv, CALG_MD5);
			CStrW	buf(MAX_PATH_LENGTH);
			copyName(buf.buffer());
			if (hSHA.Hash(buf)) {
				Statistics::getInstance()->hashesCalculated++;
				m_hash.avail(hSHA.GetHash(m_hash.hash(), m_hash.size()));
				return	m_hash.avail();
			}
			err = hSHA.err();
		}
		logError(L"Unable to count hash: %s", Err(err).c_str());
		return	false;
	}
	bool		LoadInode() {
		if (m_inode.IsOK())
			return	true;
		CStrW	buf(MAX_PATH_LENGTH);
		copyName(buf.buffer());
		bool	Result = m_inode.Load(buf);
		if (!Result)
			logError(L"Unable to load file inode info: %s", Err().c_str());
		return	Result;
	}

	bool		hardlink(const Shared_ptr<File> &rhs) const {
		++Statistics::getInstance()->hardLinks;
		CStrW	file1Name(MAX_PATH_LENGTH);
		CStrW	file2Name(MAX_PATH_LENGTH);

		this->copyName(file1Name.buffer());
		rhs->copyName(file2Name.buffer());
		logVerbose(L"Linking %s and %s", file1Name.c_str(), file2Name.c_str());

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
	void		copyName(PWSTR buf) const {
		parent->copyName(buf);
		Cat(buf, PATH_SEPARATOR);
		Cat(buf, m_name);
	}
	bool		equals(File* otherFile) const {
		if (!otherFile || !Eqi(m_name, otherFile->m_name)) {
			return	false;
		}
		return	parent->equals(otherFile->parent);
	}

};


bool		operator==(const Shared_ptr<File> &lhs, const Shared_ptr<File> &rhs) {
	if (lhs->size() != rhs->size())
		return	false;
	if (lhs->inode() == rhs->inode())
		return	true;
	return	false;
}
bool		isIdentical(const Shared_ptr<File> &lhs, const Shared_ptr<File> &rhs) {
	if (!lhs->isHashAvailable() || !rhs->isHashAvailable()) {
		if (!lhs->isHashAvailable())
			if (!lhs->LoadHash())
				return	false;
		if (!rhs->isHashAvailable())
			if (!rhs->LoadHash())
				return	false;
	}
	Statistics::getInstance()->hashCompares++;
	return	lhs->hash() == rhs->hash();
}
bool		isSameVolume(const Shared_ptr<File> &lhs, const Shared_ptr<File> &rhs) {
	return	lhs->inode().vol_sn() == rhs->inode().vol_sn();
}
bool		CompareBySize(const Shared_ptr<File> &f1, const Shared_ptr<File> &f2) {
	return	f1->size() < f2->size();
}

#endif
