#include "file.h"

file_map_t::~file_map_t() {
	if (m_data)
		::UnmapViewOfFile(m_data);
}

file_map_t::file_map_t(PCWSTR path, size_type size, bool write):
	m_data(nullptr),
	m_size(0),
	m_write(write) {
	ACCESS_MASK access = write ? GENERIC_READ | GENERIC_WRITE : GENERIC_READ;
	DWORD share = write ? 0 : FILE_SHARE_DELETE | FILE_SHARE_READ;
	HANDLE file = ::CreateFileW(path, access, share, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (file == INVALID_HANDLE_VALUE)
		return;
	m_size = std::min(get_size(file), size);
	DWORD protect = write ? PAGE_READWRITE : PAGE_READONLY;
	HANDLE m_map = ::CreateFileMappingW(file, nullptr, protect, HighPart64(m_size), LowPart64(m_size), nullptr);
	if (m_map) {
		ACCESS_MASK access = m_write ? FILE_MAP_WRITE : FILE_MAP_READ;
		m_data = ::MapViewOfFile(m_map, access, 0, 0, size);
		::CloseHandle(m_map);
	}
	::CloseHandle(file);
}

file_map_t::size_type file_map_t::size() const {
	return m_size;
}

PVOID file_map_t::data() const {
	return m_data;
}

bool file_map_t::is_writeble() const {
	return m_write;
}

bool file_map_t::is_ok() const {
	return m_data;
}

uint64_t file_map_t::get_size(HANDLE file) {
	LARGE_INTEGER size;
	size.QuadPart = 0ll;
	::GetFileSizeEx(file, &size);
	return size.QuadPart;
}
