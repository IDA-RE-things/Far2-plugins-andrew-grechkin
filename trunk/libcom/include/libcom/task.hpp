#ifndef _LIBCOM_TASK_HPP_
#define _LIBCOM_TASK_HPP_

#include <libcom/win_com.hpp>

#include <vector>

#include <tr1/memory>

///▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ com_task
struct ITask;
struct ITaskTrigger;

///====================================================================================== WinTrigger
struct _TASK_TRIGGER;
struct WinTrigger {
	~WinTrigger();

	WinTrigger(size_t index, const ComObject<ITaskTrigger> &trigger);

	operator _TASK_TRIGGER*() const {
		return m_info.get();
	}

	bool operator==(size_t index) const {
		return m_index == index;
	}

	bool operator==(const WinTrigger & rhs) const {
		return m_index == rhs.m_index;
	}

	bool operator<(const WinTrigger & rhs) const {
		return m_index < rhs.m_index;
	}

	void set_time(DWORD h = 0, DWORD m = 0);

	void set_daily(size_t interval = 1);

	void set_weekly(WORD days_of_week, size_t interval = 1);

	void set_monthly(size_t days_bits, size_t months_bits = 0xFFF);

	void set_repeat(DWORD every = 60, DWORD forwhile = 24 * 60);

	DWORD get_starthour() const;

	DWORD get_startminute() const;

	DWORD get_type() const;

	DWORD get_daily_interval() const;

	DWORD get_weekly_dow() const;

	DWORD get_weekly_interval() const;

	DWORD get_monthly_month() const;

	DWORD get_monthly_dom() const;

	DWORD get_repeat() const;

	DWORD get_repeat_duration() const;

	bool is_repeat() const;

	ustring as_str() const;

private:
	WinTrigger(size_t index, const ComObject<ITask> &task);

	void save();

	ComObject<ITaskTrigger> m_trigger;
	std::tr1::shared_ptr<_TASK_TRIGGER> m_info;
	size_t m_index;

	friend class WinTask;
};

///========================================================================================= WinTask
class WinTask {
public:
	~WinTask();

	explicit WinTask(const ustring &name, bool autocreate = true);

	operator ComObject<ITask>() const;

	ComObject<ITask> operator->() const;

	bool operator==(const ustring & name) const {
		return m_name == name;
	}

	bool operator==(const WinTask & rhs) const {
		return m_name == rhs.m_name;
	}

	bool operator<(const WinTask & rhs) const {
		return m_name < rhs.m_name;
	}

	void run() const;

	void set_flags(DWORD in, bool on = true);

	void disable();

	void enable();

	void set_account(const ustring &name, PCWSTR pass = nullptr);

	void set_application(const ustring &path);

	void set_creator(const ustring &name);

	void set_param(const ustring &in);

	void set_workdir(const ustring &path);

	void set_comment(const ustring &in);

	void set_max_runtime(DWORD in);

	void add_trigger(const WinTrigger &in);

	void set_trigger(DWORD index, const WinTrigger &in);

	void clear_triggers();

	void save() const;

	ustring name() const;

	size_t get_triggers_count() const;

	DWORD get_flags() const;

	HRESULT get_status() const;

	ustring get_account() const;

	ustring get_application() const;

	ustring get_creator() const;

	ustring get_param() const;

	ustring get_workdir() const;

	ustring get_comment() const;

	static DWORD get_flags(const ustring &name);

	static HRESULT get_state(const ustring &name);

	static bool is_exist(const ustring &name);

	static bool is_disabled(const ustring &name);

	static bool is_running(const ustring &name);

	static void del(const ustring &name);

	static ustring parse_state(HRESULT in);

	static void parse_command(const ustring &comm, ustring &app, ustring &par);

private:
	ustring m_name;
	ComObject<ITask> m_task;
};

///===================================================================================== WinTriggers
class WinTriggers: private std::vector<WinTrigger> {
public:
	typedef WinTrigger value_type;
	typedef std::vector<WinTrigger> class_type;
	typedef class_type::iterator iterator;
	typedef class_type::const_iterator const_iterator;
	using class_type::begin;
	using class_type::end;
	using class_type::size;

public:
	void cache();

	ustring AsStrAll(const ustring &delim = L", ");

	iterator find(size_t index);

	void	add(const WinTrigger &in);
	void	del(size_t index);
	void	del(iterator it);

private:
	WinTriggers(const WinTask &task);

	WinTask m_task;

	friend class WinTask;
};

///======================================================================================== WinTasks
class WinTasks: private std::vector<WinTask> {
public:
	typedef WinTask value_type;
	typedef std::vector<WinTask> class_type;
	typedef class_type::iterator iterator;
	typedef class_type::const_iterator const_iterator;
	using class_type::begin;
	using class_type::end;
	using class_type::empty;
	using class_type::size;

public:
	WinTasks(bool autocache = true);

	void cache() {
		cache_by_creator();
	}

	void cache_by_creator(const ustring &creator = ustring());

	iterator find(const ustring &name);

	void Add(const ustring &name);

	void del(const ustring &name);

	void del(iterator it);

	void TriggerAdd(const WinTrigger &in) const;

	void TriggerClean() const;

	ustring GetTriggersString(const ustring &delim = L", ") const;

private:
	ustring m_creator;
};

#endif
