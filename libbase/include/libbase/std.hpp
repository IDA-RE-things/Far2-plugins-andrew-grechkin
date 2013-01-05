#ifndef _LIBBASE_STD_HPP_
#define _LIBBASE_STD_HPP_

#if (_WIN32_WINNT < 0x0600)
#undef _WIN32_WINNT
#endif

#define _WIN32_WINNT 0x0600

#if (WINVER < 0x0600)
#undef WINVER
#endif
#ifndef WINVER
#define WINVER 0x0600
#endif

#if (_WIN32_IE < 0x0600)
#undef _WIN32_IE
#endif
#ifndef _WIN32_IE
#define _WIN32_IE 0x0600
#endif

//#define WIN32_LEAN_AND_MEAN
#define NOMCX
#define NOIME

#include <cstdint>
#include <windows.h>

#include <algorithm>

#ifdef NoStlString
template<typename Type>
struct AutoSTR;

typedef AutoSTR<CHAR> astring;
typedef AutoSTR<WCHAR> ustring;
#else
#include <iosfwd>
typedef std::string astring;
typedef std::wstring ustring;
#endif

typedef const void * PCVOID;

#ifdef NoStdNew
inline void * operator new(size_t size) throw() {
	return ::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, size);
}

inline void * operator new [](size_t size) throw() {
	return ::operator new(size);
}

inline void operator delete(void * in) throw() {
	::HeapFree(::GetProcessHeap(), 0, (PVOID)in);
}

inline void operator delete [](void * ptr) throw() {
	::operator delete(ptr);
}
#endif


namespace Base {

	PCWSTR const EMPTY_STR = L"";
	PCWSTR const SPACE = L" ";
	PCWSTR const PATH_SEPARATOR = L"\\"; // Path separator in the file system
	PCWSTR const PATH_SEPARATORS = L"\\/";
	PCWSTR const PATH_PREFIX_NT = L"\\\\?\\"; // Prefix to put ahead of a long path for Windows API
	PCWSTR const NETWORK_PATH_PREFIX = L"\\\\";

	const WCHAR PATH_SEPARATOR_C = L'\\';
	const WCHAR STR_END = L'\0';
	const WCHAR SPACE_C = L' ';

	const uint32_t BOM_UTF32le = 0x0000FEFF;
	const uint32_t BOM_UTF32be = 0xFFFE0000;
	const uint32_t BOM_UTF16le = 0xFEFF;
	const uint32_t BOM_UTF16be = 0xFFFE;
	const uint32_t BOM_UTF8 = 0xBFBBEF00;

	const size_t MAX_PATH_LEN = 32772;
	const UINT CP_UTF16le = 1200;
	const UINT CP_UTF16be = 1201;
	const UINT CP_UTF32le = 1202;
	const UINT CP_UTF32be = 1203;
//	const size_t CP_AUTODETECT = (UINT)-1;
	const UINT DEFAULT_CP = CP_UTF8;

#define DEFINE_FUNC(name) F##name name
#define GET_DLL_FUNC(name) name = (F##name)get_function(#name)
#define GET_DLL_FUNC_NT(name) name = (F##name)get_function_nt(#name)

#define THIS_FILE Base::filename_only(__FILE__)
#define THIS_PLACE THIS_FILE, __LINE__, __PRETTY_FUNCTION__

#ifndef sizeofa
#define sizeofa(array) (sizeof(array)/sizeof(0[array]))
#endif

#ifndef sizeofe
#define sizeofe(array) (sizeof(0[array]))
#endif

	template<typename Type, size_t N>
	size_t lengthof(Type (&) [N]) {
		return N;
	}

	inline uint64_t make_uint64(uint32_t high, uint32_t low) {
		return ((uint64_t)high) << 32 | low;
	}

	inline uint32_t high_part_64(uint64_t arg64) {
		return (uint32_t)(arg64 >> 32);
	}

	inline uint32_t low_part_64(uint64_t arg64) {
		return (uint32_t)(arg64 & 0xFFFFFFFFULL);
	}

	inline PCSTR filename_only(PCSTR path, char ch = '\\') {
		return (strrchr((path), ch) ? : (path) - 1) + 1;
	}

	inline PCWSTR filename_only(PCWSTR path, WCHAR ch = PATH_SEPARATOR_C) {
		return (wcsrchr((path), ch) ? : (path) - 1) + 1;
	}

	///=============================================================================================
	inline void mbox(PCSTR text, PCSTR capt = "") {
		::MessageBoxA(nullptr, text, capt, MB_OK);
	}

	inline void mbox(PCWSTR text, PCWSTR capt = L"") {
		::MessageBoxW(nullptr, text, capt, MB_OK);
	}

}

//template<class InputIterator, class OutputIterator>
//void PermutationWithRepetition(size_t K, InputIterator start, InputIterator end, OutputIterator out)
//{
//	OperationsContainer pIn(start, end), pOut(K);
//	OperationsContainer::size_type in_size = pIn.size();
//
//	K--;
//	int *stack = new int[K * 2],  // стек псевдорекурсии, глубина рекурсии K - 1
//		*pTop = stack,            // вершина стека
//		k = 0,                    // переменные цикла
//		n = 0,
//		j = 0;
//	for (;;)
//	{
//		while(n < in_size)
//		{
//			pOut[k] = pIn[n++];
//			if (k == K) {
//				++j;
//				*out = pOut;
//								printOperations(j, pOut);
//				//				printf("%02d. %s\n", ++j, pOut);
//			} else {
//				if (n < in_size)
//				{
//					*pTop++ = k;          // сохраняем k и n в стеке
//					*pTop++ = n;
//				}
//				k++;                    // псевдорекурсивный вызов
//				n = 0;
//			}
//		}
//		if (pTop == stack)          // стек пуст, конец цикла
//			break;
//
//		n = *(--pTop);              // выталкиваем k и n из стека
//		k = *(--pTop);
//	}
//	delete[] stack;
//}
//
//Number count_poliz(Operation * op[], size_t size) {
//	Number ret = 0;
//	NumbersContainer stack(size);
//	size_t m = 0;
//	for (size_t i = 0; i < size; ++i)
//	{
//		if (op[i]->get_type() == AdjustNumber) {
//			stack[m] = op[i]->get_value();
//			m++;
//			continue;
//		}
//
//		if (m < 2)
//			return -1;
//
//		switch (op[i]->get_type())
//		{
//			case Addition:
//			{
//				ret = stack[m - 2] + stack[m - 1];
//				break;
//			}
//			case Subtraction:
//			{
//				if (stack[m - 2] < stack[m - 1])
//					return -1;
//				ret = stack[m - 2] - stack[m - 1];
//				break;
//			}
//			case Multiplication:
//			{
//				ret = stack[m - 2] * stack[m - 1];
//				break;
//			}
//			case Division:
//			{
//				if (stack[m - 2] < stack[m - 1])
//					return -1;
//				ret = stack[m - 2] / stack[m - 1];
//				break;
//			}
//			default:
//				break;
//		}
//
//		stack[m - 2] = ret;
//		m--;
//	}
//	return ret;
//}

#endif
