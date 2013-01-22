#ifndef _LIBBASE_IO_HPP_
#define _LIBBASE_IO_HPP_

#include <libbase/std.hpp>

namespace Base {

	struct file_map_t {
		typedef uint64_t size_type;

		~file_map_t();

		file_map_t(PCWSTR path, size_type size = (size_type)-1, bool write = false);

		size_type size() const;

		PVOID data() const;

		bool is_writeble() const;

		bool is_ok() const;

	private:
		uint64_t get_size(HANDLE file) const;

		PVOID m_data;
		size_type m_size;
		bool m_write;
	};

}

#endif
