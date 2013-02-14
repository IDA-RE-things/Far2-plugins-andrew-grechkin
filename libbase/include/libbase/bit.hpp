#ifndef _LIBBASE_BIT_HPP_
#define _LIBBASE_BIT_HPP_

#include <libbase/std.hpp>

namespace Base {

	namespace Flags {

		template<typename Type1, typename Type2>
		bool check(Type1 in, Type2 flag)
		{
			return static_cast<Type1>(flag) == (in & static_cast<Type1>(flag));
		}

		template<typename Type1, typename Type2>
		bool check_any(Type1 in, Type2 flag)
		{
			return in & static_cast<Type1>(flag);
		}

		template<typename Type1, typename Type2>
		Type1 & unset(Type1 & in, Type2 flag)
		{
			return in &= ~static_cast<Type1>(flag);
		}

		template<typename Type1, typename Type2>
		Type1 & set(Type1 & in, Type2 flag)
		{
			return in |= static_cast<Type1>(flag);
		}

		template<typename Type1, typename Type2>
		Type1 & set(Type1 & in, Type2 flag, bool sw)
		{
			return (sw) ? set(in, flag) : unset(in, flag);
		}

	}

	///=============================================================================================
	namespace Bits {

		template<typename Type>
		size_t SIZE_IN_BITS()
		{
			return sizeof(Type) * 8;
		}

		template<typename Type>
		bool GOOD_BIT(size_t in)
		{
			return in < SIZE_IN_BITS<Type>();
		}

		template<typename Type>
		size_t Limit(size_t in)
		{
			return (in == 0) ? SIZE_IN_BITS<Type>() : std::min<size_t>(in, SIZE_IN_BITS<Type>());
		}

		template<typename Type>
		bool check(Type in, size_t bit)
		{
			if (!GOOD_BIT<Type>(bit))
				return false;
			Type tmp = 1;
			tmp <<= bit;
			return in & tmp;
		}

		template<typename Type>
		Type & unset(Type & in, size_t bit)
		{
			if (!GOOD_BIT<Type>(bit))
				return in;
			Type tmp = 1;
			tmp <<= bit;
			return in &= ~tmp;
		}

		template<typename Type>
		Type & set(Type & in, size_t bit)
		{
			if (!GOOD_BIT<Type>(bit))
				return in;
			Type tmp = 1;
			tmp <<= bit;
			return in |= tmp;
		}

		template<typename Type>
		Type & set(Type & in, size_t bit, bool sw)
		{
			return (sw) ? set(in, bit) : unset(in, bit);
		}

	}

}

#endif
