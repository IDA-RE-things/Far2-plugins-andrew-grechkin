#include <libbase/backtrace.hpp>

#include <libbase/err.hpp>
#include <libbase/logger.hpp>
#include <libbase/str.hpp>

#include <string>

#if !defined(_AMD64_) && defined(__GNUC__)
#include <bfd.h>
#include <cxxabi.h>

struct bfd_ctx {
	bfd * handle;
	asymbol ** symbol;
};

struct bfd_set {
	PCWSTR name;
	struct bfd_ctx * bc;
	struct bfd_set *next;
};

struct find_info {
	asymbol **symbol;
	bfd_vma counter;
	const char *file;
	const char *func;
	unsigned line;
};

void close_bfd_ctx(bfd_ctx * bc)
{
	if (bc) {
		free(bc->symbol);
		bfd_close(bc->handle);
	}
}

void release_set(bfd_set * set)
{
	while(set) {
		bfd_set * temp = set->next;
		free((void*)set->name);
		close_bfd_ctx(set->bc);
		free(set);
		set = temp;
	}
}

int init_bfd_ctx(bfd_ctx * bc, PCWSTR procname)
{
	LogTrace();
	bc->handle = NULL;
	bc->symbol = NULL;

	LogTrace();
	bfd *b = bfd_openr(Base::w2cp(procname, CP_OEMCP).c_str(), 0);
	if (!b) {
		LogFatal(L"Failed to open bfd from (%s)\n" , procname);
		return 1;
	}

	int r1 = bfd_check_format(b, bfd_object);
	int r2 = bfd_check_format_matches(b, bfd_object, NULL);
	int r3 = bfd_get_file_flags(b) & HAS_SYMS;

	if (!(r1 && r2 && r3)) {
		bfd_close(b);
		LogFatal(L"Failed to init bfd from (%s)\n", procname);
		return 1;
	}

	void * symbol_table;
	unsigned dummy = 0;
	if (bfd_read_minisymbols(b, FALSE, &symbol_table, &dummy) == 0) {
		if (bfd_read_minisymbols(b, TRUE, &symbol_table, &dummy) < 0) {
			free(symbol_table);
			bfd_close(b);
			LogFatal(L"Failed to read symbols from (%s)\n", procname);
			return 1;
		}
	}

	bc->handle = b;
	bc->symbol = (asymbol**)symbol_table;

	return 0;
}

bfd_ctx * get_bc(bfd_set * set , PCWSTR procname)
{
	LogTrace();
	while(set->name) {
		if (wcscmp(set->name , procname) == 0)
			return set->bc;
		set = set->next;
	}

	bfd_ctx bc;
	if (init_bfd_ctx(&bc, procname )) {
		return NULL;
	}

	LogTrace();
	set->next = (bfd_set*)calloc(1, sizeof(*set));
	set->bc = (bfd_ctx*)malloc(sizeof(bfd_ctx));
	memcpy(set->bc, &bc, sizeof(bc));
	set->name = wcsdup(procname);

	return set->bc;
}

void lookup_section(bfd * abfd, asection * sec, void * opaque_data)
{
	find_info * data = (find_info*)opaque_data;

	if (data->func)
		return;

	if (!(bfd_get_section_flags(abfd, sec) & SEC_ALLOC))
		return;

	bfd_vma vma = bfd_get_section_vma(abfd, sec);
	if (data->counter < vma || vma + bfd_get_section_size(sec) <= data->counter)
		return;

	bfd_find_nearest_line(abfd, sec, data->symbol, data->counter - vma, &(data->file), &(data->func), &(data->line));
}

void find(bfd_ctx * b, DWORD offset, const char *& file, const char *& func, size_t & line)
{
	LogTrace();
	find_info data;
	data.symbol = b->symbol;
	data.counter = offset;
	data.file = NULL;
	data.func = NULL;
	data.line = 0;

	bfd_map_over_sections(b->handle, &lookup_section, &data);
	file = data.file;
	func = data.func;
	line = data.line;
}
#endif

namespace Base
{
	struct FrameInfo::Data
	{
		Data(size_t frame);

		bool LoadFromPDB(size_t frame);

		bool LoadFromSymbols(size_t frame);

		bool LoadFromMap(size_t frame);

		size_t addr;
		size_t offset;
		size_t module_base;
		size_t line;
		ustring source;
		ustring func;
		ustring module;
		ustring image;
	};

	FrameInfo::Data::Data(size_t frame):
		addr(0),
		offset(0),
		module_base(0),
		line(0),
		func(L"?")
	{
		IMAGEHLP_MODULEW64 modinfo;
		modinfo.SizeOfStruct = sizeof(modinfo);
		if (SymGetModuleInfoW64(GetCurrentProcess(), frame, &modinfo) != FALSE) {
			module = modinfo.ModuleName;
			module_base = modinfo.BaseOfImage;
			image = modinfo.ImageName;
			LogDebug(L"ModuleName: %s\n", modinfo.ModuleName);
			LogDebug(L"ImageName: %s\n", modinfo.ImageName);
			LogDebug(L"LoadedImageName: %s\n", modinfo.LoadedImageName);
			LogDebug(L"LoadedPdbName: %s\n", modinfo.LoadedPdbName);
			LogDebug(L"SymType: %d\n", (int)modinfo.SymType);
		}

		(modinfo.SymType && LoadFromPDB(frame)) || LoadFromSymbols(frame) || LoadFromMap(frame);
	}

	bool FrameInfo::Data::LoadFromPDB(size_t frame)
	{
		bool ret = false;

		{
			size_t size = sizeof(SYMBOL_INFOW) + MAX_SYM_NAME * sizeof(wchar_t);
			SYMBOL_INFOW * info = (SYMBOL_INFOW*)malloc(size);
			info->SizeOfStruct = sizeof(*info);
			info->MaxNameLen = MAX_SYM_NAME;

			DWORD64 displacement;
			if (SymFromAddrW(GetCurrentProcess(), frame, &displacement, info) != FALSE) {
				addr = (size_t)info->Address;
				func = info->Name;
				offset = (size_t)displacement;
				ret = true;
			}
			free(info);
		}

		{
			DWORD dummy;
			IMAGEHLP_LINE64 info;
			info.SizeOfStruct = sizeof(info);
			if (SymGetLineFromAddr64(GetCurrentProcess(), frame, &dummy, &info)) {
				line = info.LineNumber;
				source = cp2w(filename_only(info.FileName), CP_ACP);
			}
		}

		return ret;
	}

	bool FrameInfo::Data::LoadFromSymbols(size_t frame)
	{
		addr = frame;
#if !defined(_AMD64_) && defined(__GNUC__)
		LogTrace();
		bfd_set * set = (bfd_set*)malloc(sizeof(*set));
		memset(set, 0, sizeof(*set));

		bfd_ctx * bc = get_bc(set, image.c_str());
		if (bc) {
			const char * file = NULL;
			const char * fun = NULL;

			find(bc, frame, file, fun, line);
			LogDebug(L"file: '%S'\n", file);
			LogDebug(L"func: '%S'\n", fun);
			LogDebug(L"line: %d\n", line);

			if (file)
				source = cp2w(filename_only(filename_only(file), '/'), CP_OEMCP);
			if (fun)
			{
				char buf[MAX_PATH];
				size_t size = sizeof(buf);
				int st = 0;
				abi::__cxa_demangle(fun, buf, &size, &st);
				func = cp2w(st ? fun : buf, CP_OEMCP);
			}
		}
		LogTrace();
		release_set(set);

		LogTrace();
#endif
		return false;
	}

	bool FrameInfo::Data::LoadFromMap(size_t /*frame*/)
	{
		return false;
	}

	FrameInfo::~FrameInfo()
	{
		delete m_data;
	}

	FrameInfo::FrameInfo(size_t frame):
		m_frame(frame),
		m_data(nullptr)
	{
	}

	FrameInfo::FrameInfo(const FrameInfo & right):
		m_frame(right.m_frame),
		m_data(nullptr)
	{
	}

	FrameInfo::FrameInfo(FrameInfo && right):
		m_frame(right.m_frame),
		m_data(right.m_data)
	{
		right.m_data = nullptr;
	}

	FrameInfo & FrameInfo::operator = (const FrameInfo & right)
	{
		if (this != &right)
			FrameInfo(right).swap(*this);
		return *this;
	}

	FrameInfo & FrameInfo::operator = (FrameInfo && right)
	{
		if (this != &right)
			FrameInfo(std::move(right)).swap(*this);
		return *this;
	}

	void FrameInfo::swap(FrameInfo & right)
	{
		using std::swap;
		swap(m_frame, right.m_frame);
		swap(m_data, right.m_data);
	}

	ustring FrameInfo::source() const
	{
		InitData();
		return m_data->source;
	}

	ustring FrameInfo::func() const
	{
		InitData();
		return m_data->func;
	}

	ustring FrameInfo::module() const
	{
		InitData();
		return m_data->module;
	}

	size_t FrameInfo::addr() const
	{
		InitData();
		return m_data->addr;
	}

	size_t FrameInfo::offset() const
	{
		InitData();
		return m_data->offset;
	}

	size_t FrameInfo::line() const
	{
		InitData();
		return m_data->line;
	}

	void FrameInfo::InitData() const
	{
		if (!m_data)
			m_data = new Data(m_frame);
	}

	ustring FrameInfo::AsStr() const
	{
		wchar_t buf[MAX_PATH];
		if (line())
			_snwprintf(buf, Base::lengthof(buf), L"[%s] (%p) %s:0x%Ix {%s:%Iu}", module().c_str(), addr(), func().c_str(), offset(), source().c_str(), line());
		else
			_snwprintf(buf, Base::lengthof(buf), L"[%s] (%p) %s:0x%Ix", module().c_str(), addr(), func().c_str(), offset());
		return ustring(buf);
	}

	///=============================================================================================
	struct SymbolInit
	{
		static SymbolInit & inst(PCWSTR path = nullptr)
		{
			static SymbolInit instance(path);
			return instance;
		}

	private:
		~SymbolInit()
		{
			SymCleanup(GetCurrentProcess());
		}

		SymbolInit(PCWSTR path)
		{
			SymInitializeW(GetCurrentProcess(), path, TRUE);
			SymSetOptions(SymGetOptions() | SYMOPT_FAIL_CRITICAL_ERRORS | SYMOPT_LOAD_LINES);

#if !defined(_AMD64_) && defined(__GNUC__)
			bfd_init();
#endif

		}
	};

	///=============================================================================================
	Backtrace::~Backtrace()
	{
		LogTrace();
	}

	Backtrace::Backtrace(size_t depth)
	{
		LogTrace();
		SymbolInit::inst();

		CONTEXT ctx = {0};
		ctx.ContextFlags = CONTEXT_FULL;
		RtlCaptureContext(&ctx);

		/* � ��� � Release-������������ ���������� ��������� ���� /Oy- (�� �������� ��������� �� ������), ����� ����� ������.
		 * http://www.bytetalk.net/2011/06/why-rtlcapturecontext-crashes-on.html
		 */
		STACKFRAME64 sf;
		memset(&sf, 0, sizeof(sf));
		DWORD machine = 0;
#ifdef _AMD64_
		sf.AddrPC.Offset = ctx.Rip;
		sf.AddrPC.Mode = AddrModeFlat;
		sf.AddrFrame.Offset = ctx.Rbp;
		sf.AddrFrame.Mode = AddrModeFlat;
		sf.AddrStack.Offset = ctx.Rsp;
		sf.AddrStack.Mode = AddrModeFlat;
		machine = IMAGE_FILE_MACHINE_AMD64;
#else
		sf.AddrPC.Offset = ctx.Eip;
		sf.AddrPC.Mode = AddrModeFlat;
		sf.AddrFrame.Offset = ctx.Ebp;
		sf.AddrFrame.Mode = AddrModeFlat;
		sf.AddrStack.Offset = ctx.Esp;
		sf.AddrStack.Mode = AddrModeFlat;
		machine = IMAGE_FILE_MACHINE_I386;
#endif
		while (depth-- > 0) {
			BOOL res = StackWalk64(machine, GetCurrentProcess(), GetCurrentThread(), &sf, (void*)&ctx, nullptr, &SymFunctionTableAccess64, &SymGetModuleBase64, nullptr);
#ifdef _AMD64_
			if (!res || sf.AddrReturn.Offset == 0)
				break;
			emplace_back(sf.AddrReturn.Offset);
#else
			if (!res || sf.AddrPC.Offset == 0)
				break;
			emplace_back((size_t)sf.AddrPC.Offset);
#endif
		}
		LogDebug(L"depth: %Iu\n", size());
	}

}
