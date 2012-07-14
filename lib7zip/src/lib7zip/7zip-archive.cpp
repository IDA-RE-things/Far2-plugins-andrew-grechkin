#include <lib7zip/7zip.hpp>
#include <libext/exception.hpp>

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
			if (m_arc->Open(stream, &max_check_size, openCallback) == S_OK) {
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

	ComObject<IInArchive> Archive::operator ->() const {
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


	Archive::const_input_iterator::this_type & Archive::const_input_iterator::operator ++() {
		flags_type flags = m_seq->flags();
		while (true) {
			if (++m_index >= m_seq->m_size) {
				m_end = true;
				break;
			}

			if ((flags & skipHidden) && ((attr() & FILE_ATTRIBUTE_HIDDEN) == FILE_ATTRIBUTE_HIDDEN)) {
				continue;
			}

			if ((flags & skipDirs) && is_dir()) {
				continue;
			}

			if ((flags & skipFiles) && is_file()) {
				continue;
			}
			break;
		}
		return *this;
	}

	Archive::const_input_iterator::this_type Archive::const_input_iterator::operator ++(int) {
		this_type ret(*this);
		operator ++();
		return ret;
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

	bool Archive::const_input_iterator::operator ==(const this_type & rhs) const {
		if (m_end && rhs.m_end)
			return true;
		return m_seq == rhs.m_seq && m_index == rhs.m_index;
	}

	bool Archive::const_input_iterator::operator !=(const this_type & rhs) const {
		return !operator==(rhs);
	}

	Archive::const_input_iterator::const_input_iterator():
		m_seq(nullptr),
		m_index(0),
		m_end(true) {
	}

	Archive::const_input_iterator::const_input_iterator(const Archive & seq):
		m_seq((Archive*)&seq),
		m_index(0),
		m_end(!m_seq->m_size) {
	}

	Archive::const_input_iterator::const_input_iterator(const Archive & seq, UInt32 index):
		m_seq((Archive*)&seq),
		m_index(index),
		m_end(!m_seq->m_size || index >= m_seq->m_size) {
//		printf(L"\tconst_input_iterator::const_input_iterator(%d, %d)", m_seq, index);
	}
}
