#include "7zip.h"
#include <libwin_net/exception.h>

#define UNKNOWN_IMPL_ITF(iid) \
	if (riid == IID_##iid) { *object = static_cast<iid*>(this); AddRef(); return S_OK; }

namespace SevenZip {
	///============================================================================= ExtractCallback
	struct ExtractCallback::CurrItem {
		ustring path;
		ComObject<FileWriteStream> stream;
		Archive::iterator item;
		Int32 mode;

		CurrItem(Int32 m, const ustring & p, Archive::iterator i):
			path(p + i.path()),
			item(i),
			mode(m) {
		}
	};

	ULONG WINAPI ExtractCallback::AddRef() {
		return UnknownImp::AddRef();
	}

	ULONG WINAPI ExtractCallback::Release() {
		return UnknownImp::Release();
	}

	HRESULT WINAPI ExtractCallback::QueryInterface(REFIID riid, void** object) {
//		printf(L"ArchiveExtractCallback::QueryInterface()\n");
		UNKNOWN_IMPL_ITF(IArchiveExtractCallback)
		UNKNOWN_IMPL_ITF(ICryptoGetTextPassword)
		return UnknownImp::QueryInterface(riid, object);
	}

	ExtractCallback::~ExtractCallback() {
//		printf(L"ArchiveExtractCallback::~ArchiveExtractCallback()\n");
	}

	ExtractCallback::ExtractCallback(const Archive & arc, const ustring & dest_path, const ustring & pass):
		Password(pass),
		m_wa(arc),
		m_dest(MakeGoodPath(dest_path)) {
//		printf(L"ArchiveExtractCallback::ArchiveExtractCallback(%s)\n", dest_path.c_str());
		ensure_end_path_separator(m_dest);
	}

	HRESULT WINAPI ExtractCallback::SetTotal(UInt64 /*size*/) {
//		return total size
//		printf(L"ArchiveExtractCallback::SetTotal(%d)\n", size);
//		FuncLogger();
		return S_OK;
	}

	HRESULT WINAPI ExtractCallback::SetCompleted(const UInt64 */*completeValue*/) {
//		return processed size
//		if (completeValue) {
//			printf(L"ArchiveExtractCallback::SetCompleted(%d)\n", *completeValue);
//		}
//		FuncLogger();
		return S_OK;
	}

	HRESULT WINAPI ExtractCallback::GetStream(UInt32 index, ISequentialOutStream ** outStream, Int32 askExtractMode) {
//		FuncLogger();
//		printf(L"ArchiveExtractCallback::GetStream(%d, %d)\n", index, askExtractMode);
		*outStream = nullptr;

		m_curr.reset(new CurrItem(askExtractMode, m_dest, m_wa.at(index)));

		if (askExtractMode != NArchive::NExtract::NAskMode::kExtract)
			return S_OK;

		try {
			if (m_curr->item.is_dir()) {
				Directory::create_full(m_curr->path);
			} else {
				// Create folders for file
				size_t pos = m_curr->path.find_last_of(PATH_SEPARATORS);
				if (pos != ustring::npos) {
					Directory::create_full(m_curr->path.substr(0, pos));
				}

				if (File::is_exist(m_curr->path)) {
					File::del(m_curr->path);
				}

				FileWriteStream * tmp(new FileWriteStream(m_curr->path, true));
				ComObject<FileWriteStream> stream(tmp);
				m_curr->stream = stream;
				stream.detach(tmp);
				*outStream = tmp;
			}
		} catch (WinError & e) {
			return E_ABORT;
		}
		return S_OK;
	}

	HRESULT WINAPI ExtractCallback::PrepareOperation(Int32 askExtractMode) {
//		FuncLogger();
//		printf(L"ArchiveExtractCallback::PrepareOperation(%d)\n", askExtractMode);
		switch (askExtractMode) {
			case NArchive::NExtract::NAskMode::kExtract:
				printf(L"Extract: %s\n", m_curr->item.path().c_str());
				break;
			case NArchive::NExtract::NAskMode::kTest:
				printf(L"Test: %s\n", m_curr->item.path().c_str());
				break;
			case NArchive::NExtract::NAskMode::kSkip:
				printf(L"Skip: %s\n", m_curr->item.path().c_str());
				break;
		};
		return S_OK;
	}

	HRESULT WINAPI ExtractCallback::SetOperationResult(Int32 operationResult) {
//		FuncLogger();
//		printf(L"ArchiveExtractCallback::SetOperationResult(%d)\n", operationResult);

		if (operationResult != NArchive::NExtract::NOperationResult::kOK) {
			failed_files.push_back(FailedFile(m_curr->item.path(), operationResult));
		} else {
			if (m_curr->mode == NArchive::NExtract::NAskMode::kExtract) {
				FS::set_attr(m_curr->path, m_curr->item.attr());
				if (m_curr->stream) {
					m_curr->stream->set_mtime(m_curr->item.mtime());
				}
			}
		}
		return S_OK;
	}

	HRESULT WINAPI ExtractCallback::CryptoGetTextPassword(BSTR *pass) {
		printf(L"%S\n", __PRETTY_FUNCTION__);
		if (Password.empty()) {
			// You can ask real password here from user
			// PrintError("Password is not defined");
			//		return E_ABORT;
		}
		BStr(Password).detach(*pass);
		return S_OK;
	}
}
