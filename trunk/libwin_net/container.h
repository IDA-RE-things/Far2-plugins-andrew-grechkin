/** win_container.hpp
 *	@author GrAnD, 2009
 *	@classes to manipulate Containers
**/
#ifndef WIN_CONTAINER_HPP
#define WIN_CONTAINER_HPP

#include <string>
#include <list>
#include <set>
#include <map>

#ifndef ForEachIn
#define ForEachIn(container)	for ((container->Begin());(!container->End());(container->Next()))
#define ForEachInO(container)	for ((container.Begin()); (!container.End()); (container.Next()))
#define ForEachInEq(container, value)	for ((container->Find((value))); (!container->Differ()); (container->Next()))
#define ForEachInOEq(container, value)	for ((container.Find((value))); (!container.Differ()); (container.Next()))
#endif

using std::string;
using std::wstring;
using std::list;
using std::set;
using std::map;
using std::multimap;

///=================================================================================== ListContainer
template<typename T1>
class ListContainer {
	typedef ListContainer				CurrentContainer;
	typedef list<T1>					_Cont;
	typedef typename _Cont::iterator	_ContIt;
	_Cont	mutable items;
	_ContIt mutable i_;
protected:
	bool		ValidPtr() const {
		return	!End();
	}
	void		ResetPtr() const {
		i_ = items.begin();
	}
public:
	virtual ~ListContainer() {
		Clear();
	}
	ListContainer() {
		ResetPtr();
	}
	ListContainer(const _Cont &in) {
		items.assign(in.begin(), in.end());
		ResetPtr();
	}
	ListContainer(const CurrentContainer &in) {
		items.assign(in.items.begin(), in.items.end());
		ResetPtr();
	}

	void		Clear() {
		items.clear();
		ResetPtr();
	}
	void		Erase() {
		i_ = items.erase(i_);
	}
	void		Erase(const _ContIt &in) {
		i_ = in;
		Erase();
	}
	void		Erase(const _ContIt &in1, const _ContIt &in2) {
		i_ = items.erase(in1, in2);
	}
	bool		Erase(const T1 &in) {
		items.remove(in);
		ResetPtr();
		return	true;
	}
	bool		Insert(const T1 &in) {
		i_ = items.insert(items.end(), in);
		return	true;
	}

	bool		Empty() const {
		return	items.empty();
	}
	bool		Exist(const T1 &in) const {
		return	Find(in);
	}
	bool		Find(const T1 &in) const {
		i_ = find(items.begin(), items.end(), in);
		return	ValidPtr();
	}
	void		Begin() const {
		i_ = items.begin();
	}
	bool		End() const {
		return	i_ == items.end();
	}
	void		Next() const {
		if (ValidPtr())
			++i_;
	}
	int			Size() const {
		return	items.size();
	}

	T1&			Value() {
		if (!ValidPtr())
			throw("Return value error");
		return	(T1&)*i_;
	}
	const T1&	Value() const {
		if (!ValidPtr())
			throw("Return value error");
		return	*i_;
	}
	bool		Value(T1 &out) const {
		if (ValidPtr()) {
			out = *i_;
			return	true;
		}
		return	false;
	}

	const CurrentContainer&	operator=(const _Cont &in) {
		items.assign(in.begin(), in.end());
		ResetPtr();
		return	*this;
	}
	const CurrentContainer&	operator=(const CurrentContainer &in) {
		if (this != &in) {
			operator=(in.items);
		}
		return	*this;
	}
	const CurrentContainer&	operator+=(const CurrentContainer &in) {
		if (this != &in) {
			items.insert(items.end(), in.items.begin(), in.items.end());
			ResetPtr();
		}
		return	*this;
	}
	CurrentContainer		operator+(const CurrentContainer &in) const {
		CurrentContainer Result(items);
		return	Result += in;
	}

	void		Reverse() {
		items.reverse();
	}
	void		Sort() {
		items.sort();
	}
	void		Unique() {
		items.unique();
	}

};

#ifndef WIN_C_MAP_HPP
#define WIN_C_MAP_HPP
///==================================================================================== MapContainer
template<typename T1, typename T2>
class MapContainer {
	typedef MapContainer				CurCont;
	typedef map<T1, T2>					_Cont;
	typedef typename _Cont::iterator	_ContIt;
	_Cont	mutable items;
	_ContIt mutable i_;
protected:
	bool		ValidPtr() const {
		return	!End();
	}
	void		ResetPtr() const {
		i_ = items.begin();
	}
public:
	virtual ~MapContainer() {
		Clear();
	}
	MapContainer() {
		ResetPtr();
	}
	MapContainer(const _Cont &in): items(in.begin(), in.end()) {
		ResetPtr();
	}
	MapContainer(const CurCont &in): items(in.items.begin(), in.items.end()) {
		ResetPtr();
	}

	void		Clear() {
		items.clear();
		ResetPtr();
	}
	void		Erase() {
		items.erase(i_++);
	}
	void		Erase(const _ContIt &in) {
		i_ = in;
		Erase();
	}
	void		Erase(const _ContIt &in1, const _ContIt &in2) {
		items.erase(in1, in2);
		ResetPtr();
	}
	bool		Erase(const T1 &in) {
		bool Result = items.erase(in);
		ResetPtr();
		return	Result;
	}
	bool		Insert(const T1 &in1, const T2 &in2) {
		std::pair<_ContIt, bool> Result = items.insert(std::pair<T1, T2>(in1, in2));
		if (Result.second)
			i_ = Result.first;
		return	Result.second;
	}

	bool		Empty() const {
		return	items.empty();
	}
	bool		Exist(const T1 &in) const {
		return	items.count(in);
	}
	bool		Find(const T1 &in) const {
		i_ = items.find(in);
		return	ValidPtr();
	}
	void		Begin() const {
		i_ = items.begin();
	}
	bool		End() const {
		return	i_ == items.end();
	}
	void		Next() const {
		if (ValidPtr())
			++i_;
	}
	int			Size() const {
		return	items.size();
	}

	T2&			Value() {
		if (!ValidPtr())
			throw("Return value error");
		return	i_->second;
	}
	const T2&	Value() const {
		if (!ValidPtr())
			throw("Return value error");
		return	i_->second;
	}
	bool		Value(T2 &out) const {
		if (ValidPtr()) {
			out = i_->second;
			return	true;
		}
		return	false;
	}

	const CurCont&		operator=(const _Cont &in) {
		items.clear();
		items.insert(in.begin(), in.end());
		ResetPtr();
		return	*this;
	}
	const CurCont&		operator=(const CurCont &in) {
		if (this != &in) {
			operator=(in.items);
		}
		return	*this;
	}
	const CurCont&		operator+=(const CurCont &in) {
		if (this != &in) {
			items.insert(in.items.begin(), in.items.end());
			ResetPtr();
		}
		return	*this;
	}
	const CurCont&		operator-=(const CurCont &in) {
		if (this != &in) {
			for (_ContIt ptr = in.items.begin(); ptr != in.items.end(); ++ptr) {
				items.erase(ptr->first);
			}
			ResetPtr();
		}
		return	*this;
	}
	const CurCont&		operator*=(const CurCont &in) {
		if (this != &in) {
			_Cont tc;
			for (_ContIt ptr = items.begin(); ptr != items.end(); ++ptr) {
				if (in.items.count(ptr->first))
					tc.insert(*ptr);
			}
			items.swap(tc);
			ResetPtr();
		}
		return	*this;
	}
	CurCont				operator+(const CurCont &in) const {
		CurCont Result(*this);
		return	Result += in;
	}
	CurCont				operator-(const CurCont &in) const {
		CurCont Result(*this);
		return	Result -= in;
	}
	CurCont				operator*(const CurCont &in) const {
		CurCont Result(*this);
		return	Result *= in;
	}
	T2&			operator[](const T1 &in) {
		if (!Find(in)) {
			Insert(in, T2());
		}
		return	Value();
	}
	const T1&	Key() const {
		if (!ValidPtr())
			throw("Return value error");
		return	i_->first;
	}
	bool		Key(T1 &out) const {
		if (ValidPtr()) {
			out = i_->first;
			return	true;
		}
		return	false;
	}
	bool		Differ() const {
		return	true;
	}
	int			RangeSize() const {
		return	1;
	}
};

///=============================================================================== MultiMapContainer
template<typename T1, typename T2>
class MultiMapContainer {
	typedef MultiMapContainer			CurrentContainer;
	typedef multimap<T1, T2>			_Cont;
	typedef typename _Cont::iterator	_ContIt;
	_Cont	mutable items;
	_ContIt	mutable i_;
	_ContIt	mutable j_;
protected:
	bool		ValidPtr() const {
		return	!End();
	}
	void		ResetPtr() const {
		i_ = items.begin();
		j_ = items.end();
	}
public:
	virtual ~MultiMapContainer() {
		Clear();
	}
	MultiMapContainer() {
		ResetPtr();
	}
	MultiMapContainer(const _Cont &in) {
		items.insert(in.begin(), in.end());
		ResetPtr();
	}
	MultiMapContainer(const CurrentContainer &in) {
		items.insert(in.items.begin(), in.items.end());
		ResetPtr();
	}

	void		Clear() {
		items.clear();
		ResetPtr();
	}
	void		Erase() {
		items.erase(i_++);
	}
	void		Erase(const _ContIt &in) {
		i_ = in;
		Erase();
	}
	void		Erase(const _ContIt &in1, const _ContIt &in2) {
		items.erase(in1, in2);
		ResetPtr();
	}
	bool		Erase(const T1 &in) {
		bool Result = items.erase(in);
		ResetPtr();
		return	Result;
	}
	bool		Insert(const T1 &in1, const T2 &in2) {
		i_ = items.insert(std::pair<T1, T2>(in1, in2));
		j_ = items.upper_bound(in1);
		return	true;
	}

	bool		Empty() const {
		return	items.empty();
	}
	bool		Exist(const T1 &in) const {
		return	items.count(in);
	}
	bool		Find(const T1 &in) const {
		j_ = items.upper_bound(in);
		i_ = items.lower_bound(in);
		return	i_ != j_;
	}
	void		Begin() const {
		i_ = items.begin();
	}
	bool		End() const {
		return	i_ == items.end();
	}
	void		Next() const {
		if (ValidPtr())
			++i_;
	}
	int			Size() const {
		return	items.size();
	}

	T2&			Value() {
		if (!ValidPtr())
			throw("Return value error");
		return	i_->second;
	}
	const T2&	Value() const {
		if (!ValidPtr())
			throw("Return value error");
		return	i_->second;
	}
	bool		Value(T2 &out) const {
		if (ValidPtr()) {
			out = i_->second;
			return	true;
		}
		return	false;
	}

	const CurrentContainer&		operator=(const _Cont &in) {
		items.clear();
		items.insert(in.begin(), in.end());
		ResetPtr();
		return	*this;
	}
	const CurrentContainer&		operator=(const CurrentContainer &in) {
		if (this != &in) {
			operator=(in.items);
		}
		return	*this;
	}
	const CurrentContainer&		operator+=(const CurrentContainer &in) {
		if (this != &in) {
			items.insert(in.items.begin(), in.items.end());
			ResetPtr();
		}
		return	*this;
	}
	const CurrentContainer&		operator-=(const CurrentContainer &in) {
		if (this != &in) {
			for (_ContIt ptr = in.items.begin(); ptr != in.items.end(); ++ptr) {
				items.erase(ptr->first);
			}
			ResetPtr();
		}
		return	*this;
	}
	const CurrentContainer&		operator*=(const CurrentContainer &in) {
		if (this != &in) {
			_Cont tc;
			for (_ContIt ptr = items.begin(); ptr != items.end(); ++ptr) {
				if (in.items.count(ptr->first))
					tc.insert(*ptr);
			}
			items.swap(tc);
			ResetPtr();
		}
		return	*this;
	}
	CurrentContainer			operator+(const CurrentContainer &in) const {
		CurrentContainer Result(*this);
		return	Result += in;
	}
	CurrentContainer			operator-(const CurrentContainer &in) const {
		CurrentContainer Result(*this);
		return	Result -= in;
	}
	CurrentContainer			operator*(const CurrentContainer &in) const {
		CurrentContainer Result(*this);
		return	Result *= in;
	}
	T2&			operator[](const T1 &in) {
		if (!Find(in));
		Insert(in, T2());
		return	Value();
	}
	const T1&	Key() const {
		if (!ValidPtr())
			throw("Return value error");
		return	i_->first;
	}
	bool		Key(T1 &out) const {
		if (ValidPtr()) {
			out = i_->first;
			return	true;
		}
		return	false;
	}
	bool		Differ() const {
		return	(i_ == items.end()) || (i_ == j_);
	}
	int			RangeSize() const {
		return	distance(i_, j_);
	}
};
#endif

///==================================================================================== SetContainer
template<typename T1>
class SetContainer {
	typedef SetContainer				CurrentContainer;
	typedef set<T1>						_Cont;
	typedef typename _Cont::iterator	_ContIt;
	_Cont	mutable items;
	_ContIt mutable i_;
protected:
	bool		ValidPtr() const {
		return	!End();
	}
	void		ResetPtr() const {
		i_ = items.begin();
	}
public:
	virtual ~SetContainer() {
		Clear();
	}
	SetContainer() {
		ResetPtr();
	}
	SetContainer(const _Cont &in) {
		items.insert(in.begin(), in.end());
		ResetPtr();
	}
	SetContainer(const CurrentContainer &in) {
		items.insert(in.items.begin(), in.items.end());
		ResetPtr();
	}

	void		Swap(CurrentContainer &in) {
		items.swap(in.items);
		ResetPtr();
	}
	void		Clear() {
		items.clear();
		ResetPtr();
	}
	void		Erase() {
		items.erase(i_++);
	}
	void		Erase(const _ContIt &in) {
		i_ = in;
		Erase();
	}
	void		Erase(const _ContIt &in1, const _ContIt &in2) {
		items.erase(in1, in2);
		ResetPtr();
	}
	bool		Erase(const T1 &in) {
		bool Result = items.erase(in);
		ResetPtr();
		return	Result;
	}
	bool		Insert(const T1 &in) {
		std::pair<_ContIt, bool> Result = items.insert(in);
		if (Result.second)
			i_ = Result.first;
		return	Result.second;
	}

	bool		Empty() const {
		return	items.empty();
	}
	bool		Exist(const T1 &in) const {
		return	items.count(in);
	}
	bool		Find(const T1 &in) const {
		i_ = items.find(in);
		return	ValidPtr();
	}
	void		Begin() const {
		i_ = items.begin();
	}
	bool		End() const {
		return	i_ == items.end();
	}
	void		Next() const {
		if (ValidPtr())
			++i_;
	}
	int			Size() const {
		return	items.size();
	}

	T1&			Value() {
		if (!ValidPtr())
			throw("Return value error");
		return	(T1&)*i_;
	}
	const T1&	Value() const {
		if (!ValidPtr())
			throw("Return value error");
		return	*i_;
	}
	bool		Value(T1 &out) const {
		if (ValidPtr()) {
			out = *i_;
			return	true;
		}
		return	false;
	}

	const CurrentContainer&		operator=(const _Cont &in) {
		items.clear();
		items.insert(in.begin(), in.end());
		ResetPtr();
		return	*this;
	}
	const CurrentContainer&		operator=(const CurrentContainer &in) {
		if (this != &in) {
			operator=(in.items);
		}
		return	*this;
	}
	const CurrentContainer&		operator+=(const CurrentContainer &in) {
		if (this != &in) {
			items.insert(in.items.begin(), in.items.end());
			ResetPtr();
		}
		return	*this;
	}
	const CurrentContainer&		operator-=(const CurrentContainer &in) {
		if (this != &in) {
			for (_ContIt ptr = in.items.begin(); ptr != in.items.end(); ++ptr) {
				items.erase(*ptr);
			}
			ResetPtr();
		}
		return	*this;
	}
	const CurrentContainer&		operator*=(const CurrentContainer &in) {
		if (this != &in) {
			_Cont tc;
			for (_ContIt ptr = items.begin(); ptr != items.end(); ++ptr) {
				if (in.items.count(*ptr))
					tc.insert(*ptr);
			}
			items.swap(tc);
			ResetPtr();
		}
		return	*this;
	}
	CurrentContainer			operator+(const CurrentContainer &in) const {
		CurrentContainer Result(*this);
		return	Result += in;
	}
	CurrentContainer			operator-(const CurrentContainer &in) const {
		CurrentContainer Result(*this);
		return	Result -= in;
	}
	CurrentContainer			operator*(const CurrentContainer &in) const {
		CurrentContainer Result(*this);
		return	Result *= in;
	}
};

///=================================================================================== SearchPattern
class SearchPattern : private ListContainer<wstring> {
public:
	SearchPattern(const wstring &in) {
		wstring	tmp = in;
		while (true) {
			size_t pos = tmp.find_first_of(L";");
			if (pos != 0)
				Insert(tmp.substr(0, pos));
			if (pos == wstring::npos)
				break;
			tmp.erase(0, pos + 1);
		}
	}
	bool	IsIn(const wstring &in) {
		ForEachIn(this) {
			if (in.find(this->Value()) != wstring::npos) {
				return	(true);
			}
		}
		return	(false);
	}
};

#endif //WIN_CONTAINER_HPP
