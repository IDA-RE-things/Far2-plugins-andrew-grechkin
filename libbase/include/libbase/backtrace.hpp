#ifndef _LIBBASE_BACKTRACE_HPP_
#define	_LIBBASE_BACKTRACE_HPP_

#include <libbase/std.hpp>
#include <dbghelp.h>

#include <iosfwd>
#include <vector>

namespace Base {

	struct FrameInfo {
		~FrameInfo();

		FrameInfo(size_t frame);

		FrameInfo(const FrameInfo & right);

		FrameInfo(FrameInfo && right);

		FrameInfo & operator = (const FrameInfo & right);

		FrameInfo & operator = (FrameInfo && right);

		void swap(FrameInfo & right);

		size_t frame() const {return m_frame;}

		ustring source() const;

		ustring func() const;

		ustring module() const;

		size_t addr() const;

		size_t offset() const;

		size_t line() const;

		ustring to_str() const;

	private:
		void InitData() const;

		size_t m_frame;

		struct Data;
		mutable Data * m_data;
	};

	struct Backtrace: private std::vector<FrameInfo> {
		~Backtrace();

		Backtrace(size_t depth = MAX_DEPTH);

		using std::vector<FrameInfo>::begin;
		using std::vector<FrameInfo>::end;
		using std::vector<FrameInfo>::empty;
		using std::vector<FrameInfo>::size;

	private:
		static const size_t MAX_DEPTH = 128;
	};

}

#endif
