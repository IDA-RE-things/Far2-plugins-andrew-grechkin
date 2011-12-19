#include "7zip.h"
#include <libwin_net/exception.h>

#define UNKNOWN_IMPL_ITF(iid) \
	if (riid == IID_##iid) { *object = static_cast<iid*>(this); AddRef(); return S_OK; }

namespace SevenZip {
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
