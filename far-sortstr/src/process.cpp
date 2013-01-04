#include <process.hpp>
#include <lang.hpp>
#include <guid.hpp>
#include <globalinfo.hpp>

#include <libfar3/plugin.hpp>
#include <libfar3/editor.hpp>
#include <libfar3/dialog_builder.hpp>
#include <libbase/std.hpp>
#include <libbase/str.hpp>
#include <libbase/logger.hpp>

#include <functional>
#include <cmath>
#include <vector>
#include <cstdlib>

PCWSTR EDITOR_EOL = nullptr;

struct SelInfo {
	ssize_t start;
	ssize_t count;
	SelInfo(ssize_t s = -1, ssize_t c = -1):
		start(s),
		count(c) {
	};
};

struct SortInfo {
	size_t line;
	long double num;
	SortInfo(size_t l):
		line(l),
		num(0) {
	};
};

typedef std::pair<ustring, SelInfo> datapair;
typedef std::pair<ustring, SortInfo> sortpair;
typedef std::vector<datapair> data_vector;
typedef std::vector<sortpair> sort_vector;

enum {
	DEL_NO = 0,
	DEL_BLOCK,
	DEL_SPARSE,
};

bool PairEqCI(const sortpair & lhs, const sortpair & rhs) {
	return Base::Str::compare_ci(lhs.first.c_str(), rhs.first.c_str()) == 0;
}

bool PairEqCS(const sortpair & lhs, const sortpair & rhs) {
	return Base::Str::compare_cs(lhs.first.c_str(), rhs.first.c_str()) == 0;
}

bool PairEqCScode(const sortpair & lhs, const sortpair & rhs) {
	return Base::Str::compare(lhs.first.c_str(), rhs.first.c_str()) == 0;
}

bool PairEqNum(const sortpair & lhs, const sortpair & rhs) {
	return lhs.second.num == rhs.second.num;
}

bool PairLessLine(const sortpair & lhs, const sortpair & rhs) {
	return lhs.second.line < rhs.second.line;
}

bool PairLessCI(const sortpair & lhs, const sortpair & rhs) {
	int ret = Base::Str::compare_ci(lhs.first.c_str(), rhs.first.c_str());
	if (ret < 0)
		return true;
	else if (ret == 0)
		return lhs.second.line < rhs.second.line;
	return false;
}

bool PairLessCS(const sortpair & lhs, const sortpair & rhs) {
	int ret = Base::Str::compare_cs(lhs.first.c_str(), rhs.first.c_str());
	if (ret < 0)
		return true;
	else if (ret == 0)
		return lhs.second.line < rhs.second.line;
	return false;
}

bool PairLessCScode(const sortpair & lhs, const sortpair & rhs) {
	int ret = Base::Str::compare(lhs.first.c_str(), rhs.first.c_str());
	if (ret < 0)
		return true;
	else if (ret == 0)
		return lhs.second.line < rhs.second.line;
	return false;
}

bool PairLessNum(const sortpair & lhs, const sortpair & rhs) {
	if (lhs.second.num < rhs.second.num)
		return true;
	else if (lhs.second.num == rhs.second.num)
		return lhs.second.line < rhs.second.line;
	return false;
}

INT_PTR editor_set_string(ssize_t y, ustring const& str, PCWSTR eol)
{
	return Far::Editor::set_string(y, str.c_str(), str.size(), eol);
}

bool is_whitespace(WCHAR ch) {
	return Base::Str::find(get_global_info()->edValue_Whitespaces, ch);
}

long double FindNum(PCWSTR str) {
	long double ret = HUGE_VAL;
	PCWSTR num = (PCWSTR)(str + ::wcscspn(str, L"0123456789"));

	if (*num) {
		WCHAR buf[132]; buf[Base::lengthof(buf) - 1] = 0;
		Base::Str::copy(buf, num, Base::lengthof(buf) - 1);
		for (PWSTR k = buf; *k; ++k) {
			if (*k == L',') {
				*k = L'.';
				break;
			}
		}
		PWSTR k = buf;
		for (PWSTR l = buf; *k && *l ; ++l) {
			if (!is_whitespace(*l)) {
				*k++ = *l;
			}
		}
		*k = 0;
		ret = wcstold(buf, nullptr);
		if (num > str && num[-1] == L'-') {
			ret = -ret;
		}
	}
	return ret;
}

template <typename Type>
void InsertFromVector(const data_vector & data, Type it, Type end) {
	size_t i = get_global_info()->get_first_line(), j = 0;
	for (; it != end; ++i, ++j) {
		if (data[j].second.count == -2 && !get_global_info()->cbValue_AsEmpty) {
			continue;
		}
		if (j == it->second.line) {
			++it;
			continue;
		}
		switch (get_global_info()->cbValue_Operation) {
			case DEL_NO:
				if (get_global_info()->get_block_type() == BTYPE_STREAM) {
					editor_set_string(i, data[it->second.line].first, EDITOR_EOL);
				} else {
					if (get_global_info()->cbValue_Selected) {
						if (data[j].first.size() <= (size_t)data[j].second.start) {
							ustring tmp(data[j].second.start, Base::SPACE_C);
							tmp.replace(0, data[j].first.size(), data[j].first);
							tmp += it->first;
							editor_set_string(i, tmp, EDITOR_EOL);
						} else {
							ustring tmp(data[j].first);
							tmp.replace(data[j].second.start, data[j].second.count, it->first);
							editor_set_string(i, tmp, EDITOR_EOL);
						}
					} else {
						editor_set_string(i, data[it->second.line].first, EDITOR_EOL);
					}
				}
				++it;
				break;
			case DEL_BLOCK:
				editor_set_string(i, data[it->second.line].first, EDITOR_EOL);
				++it;
				break;
			case DEL_SPARSE: {
				if (!data[j].first.empty()) {
					editor_set_string(i, Base::EMPTY_STR, EDITOR_EOL);
				}
				break;
			}
		}
	}
	switch (get_global_info()->cbValue_Operation) {
		case DEL_BLOCK:
			for (; j < data.size(); ++i, ++j)
				editor_set_string(i, Base::EMPTY_STR, EDITOR_EOL);
			break;
		case DEL_SPARSE: {
			for (; j < data.size(); ++i, ++j)
				if (!data[j].first.empty())
					editor_set_string(i, Base::EMPTY_STR, EDITOR_EOL);
		}
		break;
	}
}

bool Execute() {
	LogTrace();
	LogDebug(L"get_first_line(): %Id, get_total_lines(): %Id\n", get_global_info()->get_first_line(), get_global_info()->get_total_lines());
	if (!get_global_info()->get_total_lines())
		return false;

	LogDebug(L"get_block_type(): %Id\n", get_global_info()->get_block_type());
	data_vector	data;
	sort_vector	sortdata;
//	data.reserve(get_global_info()->get_total_lines() - get_global_info()->get_first_line());
//	sortdata.reserve(data.capacity());

	EditorGetString	egs;
	for (size_t y = get_global_info()->get_first_line(); y < get_global_info()->get_total_lines(); ++y) {
		Far::Editor::get_string(y, egs);

		if (y == (get_global_info()->get_total_lines() - 1) && Base::Str::is_empty(egs.StringText))
			break;
		if (get_global_info()->get_block_type() != BTYPE_NONE && (egs.SelStart == -1 || egs.SelStart == egs.SelEnd))
			break;

		ustring	tmp(egs.StringText, egs.StringLength);
//		LogDebug(L"str[%Id]: '%s'\n", y, tmp.c_str());

		ssize_t	SelLen = -2;
		switch (get_global_info()->get_block_type()) {
			case BTYPE_COLUMN: {
				if (egs.SelStart < egs.StringLength) {
					SelLen = std::min(egs.SelEnd, egs.StringLength) - std::min(egs.SelStart, egs.StringLength);
				}
				if (SelLen != -2) {
					sortdata.emplace_back(ustring(egs.StringText + egs.SelStart, SelLen), y - get_global_info()->get_first_line());
//					sortdata.push_back(sortpair(ustring(egs.StringText + egs.SelStart, SelLen), y - get_global_info()->get_first_line()));
					if (get_global_info()->cbValue_Numeric) {
						sortdata.back().second.num = FindNum(sortdata.back().first.c_str());
					}
				} else if (get_global_info()->cbValue_AsEmpty) {
					sortdata.emplace_back(ustring(), y - get_global_info()->get_first_line());
//					sortdata.push_back(sortpair(ustring(), y - get_global_info()->get_first_line()));
				}
				data.emplace_back(tmp, SelInfo(egs.SelStart, SelLen));
//				data.push_back(data_vector::value_type(tmp, SelInfo(egs.SelStart, SelLen)));
				LogDebug(L"egs.SelStart: %Id, egs.StringLength: %Id, SelLen: %Id\n", egs.SelStart, egs.StringLength, SelLen);
				LogDebug(L"data.back().first: '%s', data.back().second.start: %Id, data.back().second.count: %Id\n", data.back().first.c_str(), data.back().second.start, data.back().second.count);
				LogDebug(L"sortdata.back().first: '%s', sortdata.back().second.line: %Id, sortdata.back().second.num: %f\n", sortdata.back().first.c_str(), sortdata.back().second.line, (double)sortdata.back().second.num);
				break;
			}
			case BTYPE_STREAM:
			default: {
				data.emplace_back(tmp, SelInfo(0, egs.StringLength));
//				data.push_back(data_vector::value_type(tmp, SelInfo(0, egs.StringLength)));
				sortdata.emplace_back(tmp, y - get_global_info()->get_first_line());
//				sortdata.push_back(sortpair(tmp, y - get_global_info()->get_first_line()));
				if (get_global_info()->cbValue_Numeric) {
					sortdata.back().second.num = FindNum(sortdata.back().first.c_str());
				}
			}
			break;
		}
//		file << "line: " << i << " sta: " << data[i].second.start << " cnt: " << data[i].second.count << endl;
	}

//	ofstream file1("sortdata1.log");
//	for (sort_vector::iterator it = sortdata.begin(); it != sortdata.end(); ++it) {
//		file1 << "line: " << it->second.line << " num: " << setprecision(16) << it->second.num << " str: '" << oem(it->first).c_str() << "'" << endl;
//	}

	LogTrace();
	std::pointer_to_binary_function<const sortpair &, const sortpair &, bool>
	pfLe(get_global_info()->cbValue_Numeric ? ptr_fun(PairLessNum) : ptr_fun(PairLessCScode)),
	pfEq(get_global_info()->cbValue_Numeric ? ptr_fun(PairEqNum) : ptr_fun(PairEqCScode));

	switch (get_global_info()->cbValue_Sensitive) {
		case 0:
			pfLe = get_global_info()->cbValue_Numeric ? ptr_fun(PairLessNum) : ptr_fun(PairLessCI);
			pfEq = get_global_info()->cbValue_Numeric ? ptr_fun(PairEqNum) : ptr_fun(PairEqCI);
			break;
		case 1:
			pfLe = get_global_info()->cbValue_Numeric ? ptr_fun(PairLessNum) : ptr_fun(PairLessCS);
			pfEq = get_global_info()->cbValue_Numeric ? ptr_fun(PairEqNum) : ptr_fun(PairEqCS);
			break;
	}

	std::sort(sortdata.begin(), sortdata.end(), pfLe);

	if (get_global_info()->cbValue_Operation) {
		sort_vector::iterator it = std::unique(sortdata.begin(), sortdata.end(), pfEq);
		sortdata.erase(it, sortdata.end());
		std::sort(sortdata.begin(), sortdata.end(), ptr_fun(PairLessLine));
	}

//	for (sort_vector::iterator it = sortdata.begin(); it != sortdata.end(); ++it) {
//		LogDebug(L"line: %Iu, str: '%s'\n", it->second.line, it->first.c_str());
//	}

	Far::Editor::start_undo();
	if (get_global_info()->cbValue_Invert && !get_global_info()->cbValue_Operation) {
		InsertFromVector(data, sortdata.rbegin(), sortdata.rend());
	} else {
		InsertFromVector(data, sortdata.begin(), sortdata.end());
	}
	Far::Editor::stop_undo();

	Far::Editor::redraw();

	return true;
}

void process()
{
	get_global_info()->load_settings();
	get_global_info()->load_editor_info();

	FarListItem cbItems[] = {
		{0, Far::get_msg(lbSort), {0}},
		{0, Far::get_msg(lbDelBlock), {0}},
		{0, Far::get_msg(lbDelSparse), {0}},
	};
	static FarList cbList = {
		sizeof(cbList),
		Base::lengthof(cbItems),
		cbItems
	};

	using namespace Far;
	auto Builder = create_dialog_builder(DialogGuid, get_msg(DlgTitle));
	LogTrace();
	Builder->add_item(create_checkbox(&get_global_info()->cbValue_Invert, cbInvert));
	Builder->add_item(create_checkbox(&get_global_info()->cbValue_Sensitive, cbSensitive, DIF_3STATE));
	Builder->add_item(create_separator());
	Builder->add_item(create_checkbox(&get_global_info()->cbValue_Numeric, cbNumeric));
	Builder->add_item(create_label(txWhitespace));
	Builder->add_item_after(create_edit(get_global_info()->edValue_Whitespaces, 10));
	Builder->add_item(create_separator());
	Builder->add_item(create_checkbox(&get_global_info()->cbValue_Selected, cbSelected, (get_global_info()->get_block_type() != BTYPE_COLUMN) ? DIF_DISABLE : 0));
	Builder->add_item(create_checkbox(&get_global_info()->cbValue_AsEmpty, cbAsEmpty, (get_global_info()->get_block_type() != BTYPE_COLUMN) ? DIF_DISABLE : 0));
	Builder->add_item(create_separator());
	Builder->add_item(create_label(txOperation));
	Builder->add_item(create_combobox(&get_global_info()->cbValue_Operation, &cbList, DIF_DROPDOWNLIST | DIF_LISTNOAMPERSAND));
	Builder->add_item(create_separator());
	Builder->add_OKCancel(get_msg(txtBtnOk), get_msg(txtBtnCancel));
	if (Builder->show()) {
		get_global_info()->save_settings();
		Execute();
	}
}
