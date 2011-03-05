/** str.hpp
 *	@author		© 2011 Andrew Grechkin
 **/

#ifndef _WIN_DEF_STR_HPP
#define _WIN_DEF_STR_HPP

#include "std.h"
#include <tr1/functional>
using std::tr1::placeholders::_1;
using std::tr1::placeholders::_2;

template<typename Type>
const Type* find_first_of(const Type *where, const Type *what) {
	//	return (PWSTR)(in + ::wcscspn(in, mask));
	using namespace std;
	typedef const Type * str_t;
	typedef str_t (*func_t)(str_t, Type);
	str_t last1 = &where[Len(where)];
	str_t pos = find_if(&where[0], last1, tr1::bind((func_t)Find, what, _1));
	return (last1 == pos) ? nullptr : pos;
}
template<typename Type>
const Type* find_first_not_of(const Type *where, const Type *what) {
	//	return (PWSTR)(in + ::wcsspn(in, mask));
	using namespace std;
	typedef const Type * str_t;
	typedef str_t (*func_t)(str_t, Type);
	str_t last1 = &where[Len(where)];
	str_t pos = find_if(&where[0], last1, tr1::bind(logical_not<bool>(), tr1::bind((func_t)Find, what, _1)));
	return (last1 == pos) ? nullptr : pos;
}

template<typename Type>
const Type* find_last_of(const Type *where, const Type *what) {
	using namespace std;
	typedef const Type * str_t;
	typedef str_t (*func_t)(str_t, Type);
	reverse_iterator<str_t> first1(&where[Len(where)]);
	reverse_iterator<str_t> last1(&where[0]);
	reverse_iterator<str_t> pos = find_if(first1, last1, tr1::bind((func_t)Find, what, _1));
//	reverse_iterator<str_t> pos = find_if(first1, last1, bind1st(ptr_fun<str_t, Type, str_t>(Find), what));
	return (last1 == pos) ? nullptr : &(*pos);
}
template<typename Type>
const Type* find_last_not_of(const Type *where, const Type *what) {
	using namespace std;
	typedef const Type * str_t;
	typedef str_t (*func_t)(str_t, Type);
	reverse_iterator<str_t> first1(&where[Len(where)]);
	reverse_iterator<str_t> last1(&where[0]);
	reverse_iterator<str_t> pos = find_if(first1, last1, tr1::bind(logical_not<bool>(), tr1::bind((func_t)Find, what, _1)));
//	reverse_iterator<str_t> pos = find_if(first1, last1, not1(bind1st(ptr_fun<str_t, Type, str_t>(Find), what)));
	return (last1 == pos) ? nullptr : &(*pos);
}

#endif //WIN_STD_HPP
