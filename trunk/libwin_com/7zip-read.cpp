#include "7zip.h"

#define UNKNOWN_IMPL_ITF(iid) \
	if (riid == IID_##iid) { *object = static_cast<iid*>(this); AddRef(); return S_OK; }

///============================================================================= ArchiveOpenCallback
ArchiveOpenCallback::~ArchiveOpenCallback() {
}

ArchiveOpenCallback::ArchiveOpenCallback() {
}

HRESULT ArchiveOpenCallback::QueryInterface(REFIID riid, void** object) {
	UNKNOWN_IMPL_ITF(IArchiveOpenCallback)
	UNKNOWN_IMPL_ITF(ICryptoGetTextPassword)
	return UnknownImp::QueryInterface(riid, object);
}

HRESULT ArchiveOpenCallback::SetTotal(const UInt64 */*files*/, const UInt64 */*bytes*/) {
	return S_OK;
}

HRESULT ArchiveOpenCallback::SetCompleted(const UInt64 */*files*/, const UInt64 */*bytes*/) {
	return S_OK;
}

HRESULT ArchiveOpenCallback::CryptoGetTextPassword(BSTR */*password*/) {
	if (Password.empty()) {
		// You can ask real password here from user
		// Password = GetPassword(OutStream);
		// PasswordIsDefined = true;
		return E_ABORT;
	}
	return S_OK;
}

///========================================================================== ArchiveExtractCallback
HRESULT WINAPI ArchiveExtractCallback::QueryInterface(REFIID riid, void** object) {
//	printf(L"ArchiveExtractCallback::QueryInterface()\n");
	UNKNOWN_IMPL_ITF(IArchiveExtractCallback)
	UNKNOWN_IMPL_ITF(ICryptoGetTextPassword)
	return UnknownImp::QueryInterface(riid, object);
}

ArchiveExtractCallback::~ArchiveExtractCallback() {
//	printf(L"ArchiveExtractCallback::~ArchiveExtractCallback()\n");
}

ArchiveExtractCallback::ArchiveExtractCallback(const WinArchive & arc, const ustring & dest_path, const ustring & pass):
	NumErrors(0),
	Password(pass),
	m_wa(arc),
	m_dest(MakeGoodPath(dest_path)) {
	printf(L"ArchiveExtractCallback::ArchiveExtractCallback(%s)\n", dest_path.c_str());
	ensure_end_path_separator(m_dest);
}

STDMETHODIMP ArchiveExtractCallback::SetTotal(UInt64 /*size*/) {
	// return total size
//	printf(L"ArchiveExtractCallback::SetTotal(%d)\n", size);
	return S_OK;
}

STDMETHODIMP ArchiveExtractCallback::SetCompleted(const UInt64 */*completeValue*/) {
	// return processed size
//	if (completeValue) {
//		printf(L"ArchiveExtractCallback::SetCompleted(%d)\n", *completeValue);
//	}
	return S_OK;
}

STDMETHODIMP ArchiveExtractCallback::GetStream(UInt32 index, ISequentialOutStream **outStream, Int32 askExtractMode) {
	printf(L"ArchiveExtractCallback::GetStream(%d, %d)\n", index, askExtractMode);
	*outStream = nullptr;
	m_index = index;

	if (askExtractMode != NArchive::NExtract::NAskMode::kExtract)
		return S_OK;

	m_diskpath = m_dest + m_wa.at(m_index).path();
	printf(L"ArchiveExtractCallback::GetStream(%s)\n", m_diskpath.c_str());

	try {
		if (m_wa[m_index].is_dir()) {
			Directory::create_full(m_diskpath);
		} else {
			// Create folders for file
			size_t pos = m_diskpath.find_last_of(PATH_SEPARATORS);
			if (pos != ustring::npos) {
				Directory::create_full(m_diskpath.substr(0, pos));
			}

			if (File::is_exists(m_diskpath)) {
				File::del(m_diskpath);
	//			PrintString(ustring(kCantDeleteOutputFile) + m_diskpath);
			}

			FileWriteStream * tmp(new FileWriteStream(m_diskpath, true));
			ComObject<FileWriteStream> stream(tmp);
			m_stream = stream;
			stream.detach(tmp);
			*outStream = tmp;
		}
	} catch (WinError & e) {
		return E_ABORT;
	}
	return S_OK;
}

STDMETHODIMP ArchiveExtractCallback::PrepareOperation(Int32 askExtractMode) {
//	printf(L"ArchiveExtractCallback::PrepareOperation(%d)\n", askExtractMode);
	ExtractMode = askExtractMode;

	switch (askExtractMode) {
		case NArchive::NExtract::NAskMode::kExtract:
//			PrintString(kExtractingString);
			break;
		case NArchive::NExtract::NAskMode::kTest:
//			PrintString(kTestingString);
			break;
		case NArchive::NExtract::NAskMode::kSkip:
//			PrintString(kSkippingString);
			break;
	};

//	PrintString(_processedFileInfo.path);
	return S_OK;
}

STDMETHODIMP ArchiveExtractCallback::SetOperationResult(Int32 operationResult) {
	printf(L"ArchiveExtractCallback::SetOperationResult(%d)\n", operationResult);
	switch (operationResult) {
		case NArchive::NExtract::NOperationResult::kOK:
			break;
		default: {
			NumErrors++;
//			PrintString("     ");
			switch (operationResult) {
				case NArchive::NExtract::NOperationResult::kUnSupportedMethod:
//					PrintString(kUnsupportedMethod);
					break;
				case NArchive::NExtract::NOperationResult::kCRCError:
//					PrintString(kCRCFailed);
					break;
				case NArchive::NExtract::NOperationResult::kDataError:
//					PrintString(kDataError);
					break;
				default:
					;
//					PrintString(kUnknownError);
			}
			break;
		}
	}

	if (ExtractMode == NArchive::NExtract::NAskMode::kExtract) {
		FS::set_attr(m_diskpath, m_wa[m_index].attr());
		if (m_stream) {
			m_stream->set_mtime(m_wa[m_index].mtime());
		}
	}
	m_stream.Release();
//	PrintString(L"\n");
	return S_OK;
}

STDMETHODIMP ArchiveExtractCallback::CryptoGetTextPassword(BSTR *pass) {
//	printf(L"ArchiveExtractCallback::CryptoGetTextPassword()\n");
	if (Password.empty()) {
		// You can ask real password here from user
		// PrintError("Password is not defined");
//		return E_ABORT;
	}
	BStr(Password).detach(*pass);
	return S_OK;
}

///====================================================================================== WinArchive
WinArchive::WinArchive(const SevenZipLib &lib, const ustring &path, flags_type flags):
	m_path(path),
	m_mask(L"*"),
	m_flags(flags) {
	InitArc(lib);
	InitProps();
}

WinArchive::WinArchive(const SevenZipLib &lib, const ustring &path, const ustring &mask, flags_type flags):
	m_path(path),
	m_mask(mask),
	m_flags(flags) {
	InitArc(lib);
	InitProps();
}

WinArchive::WinArchive(ComObject<IInArchive> arc, flags_type flags):
	m_mask(L"*"),
	m_flags(flags),
	m_arc(arc) {
	InitProps();
}

void WinArchive::InitArc(const SevenZipLib &lib) {
	ComObject<IInArchive> arc;
	ComObject<IInStream> stream(new FileReadStream(m_path.c_str()));
	ComObject<IArchiveOpenCallback> openCallback(new ArchiveOpenCallback);
	for (ArcCodecs::const_iterator it = lib.codecs().begin(); it != lib.codecs().end(); ++it) {
		CheckCom(lib.CreateObject(&it->second->guid, &IID_IInArchive, (PVOID*)&arc));
		CheckCom(stream->Seek(0, STREAM_SEEK_SET, nullptr));
		if (arc->Open(stream, &max_check_start_position, openCallback) == S_OK) {
			m_codec = it;
			m_arc = arc;
			return;
		}
	}
	CheckApiError(ERROR_INVALID_DATA);
}

void WinArchive::InitProps() {
	m_arc->GetNumberOfItems(&m_size);
	m_arc->GetNumberOfArchiveProperties(&m_num_props);
}

const ArcCodec & WinArchive::codec() const {
	return *(m_codec->second);
}

ComObject<IInArchive> WinArchive::operator->() const {
	return m_arc;
}


WinArchive::const_iterator WinArchive::begin() const {
	return const_iterator(*this);
}

WinArchive::const_iterator WinArchive::end() const {
	return const_iterator();
}

WinArchive::const_iterator WinArchive::at(size_t index) const {
	if (index >= (size_t)m_size)
		CheckCom(TYPE_E_OUTOFBOUNDS);
	return const_iterator(*this, index);
}

WinArchive::const_iterator WinArchive::operator[](int index) const {
	return const_iterator(*this, index);
}

bool WinArchive::empty() const {
	return m_size == 0;
}

size_t WinArchive::size() const {
	return m_size;
}

ustring WinArchive::path() const {
	return m_path;
}

ustring WinArchive::mask() const {
	return m_mask;
}

WinArchive::flags_type WinArchive::flags() const {
	return m_flags;
}

size_t WinArchive::get_num_props() const {
	return m_num_props;
}

size_t WinArchive::get_num_item_props() const {
	UInt32 props = 0;
	m_arc->GetNumberOfProperties(&props);
	return props;
}

bool WinArchive::get_prop_info(size_t index, ustring &name, PROPID &id) const {
	BStr m_nm;
	VARTYPE type;
	HRESULT err = m_arc->GetArchivePropertyInfo(index, &m_nm, &id, &type);
	if (err == S_OK && m_nm)
		name = m_nm.c_str();
	return err == S_OK;
}

PropVariant WinArchive::get_prop(PROPID id) const {
	PropVariant prop;
	m_arc->GetArchiveProperty(id, prop.ref());
	return prop;
}

size_t WinArchive::test() const {
	ArchiveExtractCallback *callback(new ArchiveExtractCallback(*this, L""));
	ComObject<IArchiveExtractCallback> extractCallback(callback);
	m_arc->Extract(nullptr, (UInt32) - 1, true, extractCallback);
	return callback->NumErrors;
}

void WinArchive::extract(const ustring & dest) const {
	ComObject<IArchiveExtractCallback> extractCallback(new ArchiveExtractCallback(*this, dest));
	CheckCom(m_arc->Extract(nullptr, (UInt32)-1, false, extractCallback));
}

WinArchive::operator ComObject<IInArchive>() const {
	return m_arc;
}

ComObject<IInArchive> WinArchive::open(const SevenZipLib &lib, PCWSTR path) {
	ComObject<IInArchive> arc;
	ComObject<IInStream> stream(new FileReadStream(path));
	ComObject<IArchiveOpenCallback> openCallback(new ArchiveOpenCallback);
	for (ArcCodecs::iterator it = lib.codecs().begin(); it != lib.codecs().end(); ++it) {
		CheckCom(lib.CreateObject(&it->second->guid, &IID_IInArchive, (PVOID*)&arc));
		CheckCom(stream->Seek(0, STREAM_SEEK_SET, nullptr));
		if (arc->Open(stream, &max_check_start_position, openCallback) == S_OK) {
			return arc;
		}
	}
	CheckApiError(ERROR_INVALID_DATA);
	return ComObject<IInArchive>();
}

ustring WinArchive::const_input_iterator::path() const {
	return get_prop(kpidPath).as_str();
}

uint64_t WinArchive::const_input_iterator::size() const {
	return get_prop(kpidSize).as_uint();
}

size_t WinArchive::const_input_iterator::attr() const {
	return get_prop(kpidAttrib).as_uint();
}

FILETIME WinArchive::const_input_iterator::mtime() const {
	return get_prop(kpidMTime).as_time();
}

bool WinArchive::const_input_iterator::is_file() const {
	return !is_dir();
}

bool WinArchive::const_input_iterator::is_dir() const {
	return get_prop(kpidIsDir).as_bool();
}

bool WinArchive::const_input_iterator::get_prop_info(size_t index, ustring &name, PROPID &id) const {
	BStr m_nm;
	VARTYPE type;
	HRESULT err = m_seq->m_arc->GetPropertyInfo(index, &m_nm, &id, &type);
	if (err == S_OK && m_nm)
		name = m_nm.c_str();
	return err == S_OK;
}

PropVariant WinArchive::const_input_iterator::get_prop(PROPID id) const {
	PropVariant prop;
	m_seq->m_arc->GetProperty(m_index, id, prop.ref());
	return prop;
}
