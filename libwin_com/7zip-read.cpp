#include "7zip.h"
#include <libwin_net/exception.h>

#define UNKNOWN_IMPL_ITF(iid) \
	if (riid == IID_##iid) { *object = static_cast<iid*>(this); AddRef(); return S_OK; }

namespace SevenZip {
	///================================================================================ OpenCallback
	OpenCallback::~OpenCallback() {
	}

	OpenCallback::OpenCallback() {
	}

	ULONG OpenCallback::AddRef() {
		return UnknownImp::AddRef();
	}

	ULONG OpenCallback::Release() {
		return UnknownImp::Release();
	}

	HRESULT OpenCallback::QueryInterface(REFIID riid, void ** object) {
		UNKNOWN_IMPL_ITF(IArchiveOpenCallback)
		UNKNOWN_IMPL_ITF(ICryptoGetTextPassword)
		return UnknownImp::QueryInterface(riid, object);
	}

	HRESULT OpenCallback::SetTotal(const UInt64 */*files*/, const UInt64 */*bytes*/) {
		return S_OK;
	}

	HRESULT OpenCallback::SetCompleted(const UInt64 */*files*/, const UInt64 */*bytes*/) {
		return S_OK;
	}

	HRESULT OpenCallback::CryptoGetTextPassword(BSTR */*password*/) {
		if (Password.empty()) {
			// You can ask real password here from user
			// Password = GetPassword(OutStream);
			// PasswordIsDefined = true;
			return E_ABORT;
		}
		return S_OK;
	}

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

	ULONG ExtractCallback::AddRef() {
		return UnknownImp::AddRef();
	}

	ULONG ExtractCallback::Release() {
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

	HRESULT ExtractCallback::SetTotal(UInt64 /*size*/) {
//		return total size
//		printf(L"ArchiveExtractCallback::SetTotal(%d)\n", size);
//		FuncLogger();
		return S_OK;
	}

	HRESULT ExtractCallback::SetCompleted(const UInt64 */*completeValue*/) {
//		return processed size
//		if (completeValue) {
//			printf(L"ArchiveExtractCallback::SetCompleted(%d)\n", *completeValue);
//		}
//		FuncLogger();
		return S_OK;
	}

	HRESULT ExtractCallback::GetStream(UInt32 index, ISequentialOutStream ** outStream, Int32 askExtractMode) {
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

	HRESULT ExtractCallback::PrepareOperation(Int32 askExtractMode) {
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

	HRESULT ExtractCallback::SetOperationResult(Int32 operationResult) {
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

	HRESULT ExtractCallback::CryptoGetTextPassword(BSTR *pass) {
//		FuncLogger();
//		printf(L"ArchiveExtractCallback::CryptoGetTextPassword()\n");
		if (Password.empty()) {
			// You can ask real password here from user
			// PrintError("Password is not defined");
			//		return E_ABORT;
		}
		BStr(Password).detach(*pass);
		return S_OK;
	}

	///===================================================================================== Archive
	Archive::Archive(const Lib & lib, const ustring & path, flags_type flags):
		m_flags(flags) {
		open_archive(lib, path);
		init_props();
	}

	Archive::Archive(const Lib & lib, const ustring & path, const ustring & /*mask*/, flags_type flags):
		m_flags(flags) {
		open_archive(lib, path);
		init_props();
	}

	Archive::Archive(ComObject<IInArchive> arc, flags_type flags):
		m_arc(arc),
		m_flags(flags) {
		init_props();
	}

	void Archive::open_archive(const Lib & lib, const ustring & path) {
		ComObject<IInStream> stream(new FileReadStream(path.c_str()));
		ComObject<IArchiveOpenCallback> openCallback(new OpenCallback);
		for (Codecs::const_iterator it = lib.codecs().begin(); it != lib.codecs().end(); ++it) {
			CheckCom(lib.CreateObject(&it->second->guid, &IID_IInArchive, (PVOID*)&m_arc));
			CheckCom(stream->Seek(0, STREAM_SEEK_SET, nullptr));
			if (m_arc->Open(stream, &max_check_start_position, openCallback) == S_OK) {
				m_codec = it;
				return;
			}
		}
		CheckApiError(ERROR_INVALID_DATA);
	}

	void Archive::init_props() {
		CheckApiError(m_arc->GetNumberOfItems(&m_size));

		m_props.cache(m_arc);
	}

	const Codec & Archive::codec() const {
		return *(m_codec->second);
	}

	ComObject<IInArchive> Archive::operator->() const {
		return m_arc;
	}

	Archive::const_iterator Archive::begin() const {
		return const_iterator(*this);
	}

	Archive::const_iterator Archive::end() const {
		return const_iterator();
	}

	Archive::const_iterator Archive::at(size_t index) const {
		if (index >= (size_t)m_size)
			CheckCom(TYPE_E_OUTOFBOUNDS);
		return const_iterator(*this, index);
	}

	Archive::const_iterator Archive::operator[](int index) const {
		return const_iterator(*this, index);
	}

	bool Archive::empty() const {
		return m_size == 0;
	}

	size_t Archive::size() const {
		return m_size;
	}

	Archive::flags_type Archive::flags() const {
		return m_flags;
	}

	const Props & Archive::props() const {
		return m_props;
	}

	size_t Archive::test() const {
		ExtractCallback * callback(new ExtractCallback(*this));
		ComObject<IArchiveExtractCallback> extractCallback(callback);
		m_arc->Extract(nullptr, (UInt32)-1, true, extractCallback);
		return callback->failed_files.size();
	}

	void Archive::extract(const ustring & dest) const {
		ComObject<IArchiveExtractCallback> extractCallback(new ExtractCallback(*this, dest));
		CheckCom(m_arc->Extract(nullptr, (UInt32)-1, false, extractCallback));
	}

	Archive::operator ComObject<IInArchive>() const {
		return m_arc;
	}

	ComObject<IInArchive> Archive::open(const Lib & lib, PCWSTR path) {
		ComObject<IInArchive> arc;
		ComObject<IInStream> stream(new FileReadStream(path));
		ComObject<IArchiveOpenCallback> openCallback(new OpenCallback);
		for (Codecs::iterator it = lib.codecs().begin(); it != lib.codecs().end(); ++it) {
			CheckCom(lib.CreateObject(&it->second->guid, &IID_IInArchive, (PVOID*)&arc));
			CheckCom(stream->Seek(0, STREAM_SEEK_SET, nullptr));
			if (arc->Open(stream, &max_check_start_position, openCallback) == S_OK) {
				return arc;
			}
		}
		CheckApiError(ERROR_INVALID_DATA);
		return ComObject<IInArchive>();
	}

	ustring Archive::const_input_iterator::path() const {
		return get_prop(kpidPath).as_str();
	}

	uint64_t Archive::const_input_iterator::size() const {
		return get_prop(kpidSize).as_uint();
	}

	size_t Archive::const_input_iterator::attr() const {
		return get_prop(kpidAttrib).as_uint();
	}

	FILETIME Archive::const_input_iterator::mtime() const {
		return get_prop(kpidMTime).as_time();
	}

	bool Archive::const_input_iterator::is_file() const {
		return !is_dir();
	}

	bool Archive::const_input_iterator::is_dir() const {
		return get_prop(kpidIsDir).as_bool();
	}

	size_t Archive::const_input_iterator::get_props_count() const {
		UInt32 props = 0;
		m_seq->m_arc->GetNumberOfProperties(&props);
		return props;
	}

	bool Archive::const_input_iterator::get_prop_info(size_t index, ustring & name, PROPID & id) const {
		BStr m_nm;
		VARTYPE type;
		HRESULT err = m_seq->m_arc->GetPropertyInfo(index, &m_nm, &id, &type);
		if (err == S_OK && m_nm)
			name = m_nm.c_str();
		return err == S_OK;
	}

	PropVariant Archive::const_input_iterator::get_prop(PROPID id) const {
		PropVariant prop;
		m_seq->m_arc->GetProperty(m_index, id, prop.ref());
		return prop;
	}
}
