#ifndef _LIBBASE_BIT_STR_HPP_
#define _LIBBASE_BIT_STR_HPP_

#include <libbase/std.hpp>
#include <libbase/bit.hpp>
#include <libbase/str.hpp>

namespace Base {

	///========================================================================================= BitMask
	template<typename Type>
	struct BitMask {
		static Type from_str(const ustring & in, size_t lim = 0) {
			// count bits from 1
			Type Result = 0;
			intmax_t bit = 0;
			ustring tmp(in);
			lim = WinBit::Limit<Type>(lim);
			while (Cut(tmp, bit)) {
				if (!WinBit::BadBit<Type>(--bit))
					WinBit::Set(Result, bit);
			}
			return Result;
		}

		static Type from_str_0(const ustring & in, size_t lim = 0) {
			// count bits from zero
			Type Result = 0;
			intmax_t bit = 0;
			ustring tmp(in);
			lim = WinBit::Limit<Type>(lim);
			while (Cut(tmp, bit)) {
				if (!WinBit::BadBit<Type>(bit))
					WinBit::Set(Result, bit);
			}
			return Result;
		}

		static ustring as_str(Type in, size_t lim = 0) {
			// count bits from 1
			ustring Result;
			lim = WinBit::Limit<Type>(lim);
			for (size_t bit = 0; bit < lim; ++bit) {
				if (WinBit::Check(in, bit)) {
					Add(Result, as_str(bit + 1), L",");
				}
			}
			return Result;
		}

		static ustring as_str_0(Type in, size_t lim = 0) {
			// count bits from zero
			ustring Result;
			lim = WinBit::Limit<Type>(lim);
			for (size_t bit = 0; bit < lim; ++bit) {
				if (WinBit::Check(in, bit)) {
					Add(Result, as_str(bit), L",");
				}
			}
			return Result;
		}

		static ustring as_str_bin(Type in, size_t lim = 0) {
			ustring Result;
			uintmax_t flag = (uintmax_t)1 << (WinBit::Limit<Type>(lim) - 1);
			while (flag) {
				Result += WinFlag::Check(in, (Type)flag) ? L'1' : L'0';
				flag >>= 1;
			}
			return Result;
		}

		static ustring as_str_num(Type in, size_t lim = 0) {
			ustring Result;
			uintmax_t flag = (uintmax_t)1 << (WinBit::Limit<Type>(lim) - 1);
			while (flag) {
				if (WinFlag::Check(in, (Type)flag)) {
					Add(Result, Base::as_str(flag), L",");
				}
				flag >>= 1;
			}
			return Result;
		}
	};

}

#endif
