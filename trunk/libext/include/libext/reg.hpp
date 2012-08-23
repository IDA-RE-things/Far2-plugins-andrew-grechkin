/**
	win_reg

	@classes ()
	@author © 2012 Andrew Grechkin
	@link ()
 **/

#ifndef WIN_REG_HPP
#define WIN_REG_HPP

#include <libbase/std.hpp>
#include <libbase/memory.hpp>

namespace Ext {

//	struct WinReg: private Base::Uncopyable {
//		~WinReg() {
//			CloseKey();
//		}
//
//		WinReg():
//			hKeyOpend(0),
//			hKeyReq(HKEY_CURRENT_USER) {
//		}
//
//		WinReg(HKEY hkey, const ustring & path):
//			hKeyOpend(0),
//			hKeyReq(hkey),
//			m_path(path) {
//		}
//
//		WinReg(const ustring & path);
//
//		ustring path() const {
//			return m_path;
//		}
//
//		void path(const ustring & path) {
//			m_path = path;
//		}
//
//		void key(HKEY hkey) {
//			hKeyReq = hkey;
//		}
//
//		bool Add(const ustring & name) const;
//
//		bool Del(const ustring & name) const;
//
//		void Set(const ustring & name, PCWSTR value) const;
//
//		void Set(const ustring & name, const ustring & value) const {
//			Set(name, value.c_str());
//		}
//
//		void Set(const ustring & name, int value) const;
//
//		bool Get(const ustring & name, ustring &value, const ustring & def) const;
//
//		bool Get(const ustring & name, int &value, int def) const;
//
//	private:
//		HKEY mutable hKeyOpend;
//		HKEY hKeyReq;
//		ustring m_path;
//
//		void CloseKey() const;
//
//		bool OpenKey(ACCESS_MASK acc) const {
//			return OpenKey(hKeyReq, m_path, acc);
//		}
//
//		bool OpenKey(HKEY hkey, const ustring & path, ACCESS_MASK acc) const;
//
//		template <typename Type>
//		void SetRaw(const ustring & name, const Type &value, DWORD type = REG_BINARY) const {
//			if (OpenKey(KEY_WRITE)) {
//				::RegSetValueExW(hKeyOpend, name.c_str(), 0, type, (PBYTE)(&value), sizeof(value));
//				CloseKey();
//			}
//		}
//
//		template <typename Type>
//		bool GetRaw(const ustring & name, Type &value, const Type &def) const {
//			bool Result = OpenKey(KEY_READ);
//			value = def;
//			if (Result) {
//				DWORD	size = sizeof(value);
//				Result = ::RegQueryValueExW(hKeyOpend, name.c_str(), nullptr, nullptr, (PBYTE)(&value), &size) == ERROR_SUCCESS;
//				CloseKey();
//			}
//			return Result;
//		}
//	};


	///==================================================================================== Register
	struct Register {
		~Register();

		Register(PCWSTR path, HKEY hkey = HKEY_LOCAL_MACHINE, ACCESS_MASK acc = KEY_READ);

		Register(Register && right);

		Register & operator = (Register & right);

		static bool is_exist_key(PCWSTR path, HKEY hkey = HKEY_LOCAL_MACHINE);

//		bool del(FARSETTINGS_SUBFOLDERS root = FSSF_ROOT);
//
//		bool del(PCWSTR name, FARSETTINGS_SUBFOLDERS root = FSSF_ROOT);
//
		size_t get(PCWSTR name, PVOID value, size_t size) const;

		ustring get(PCWSTR name, PCWSTR def) const;

		uint64_t get(PCWSTR name, uint64_t def) const;

		int64_t get(PCWSTR name, int64_t def) const;

		uint32_t get(PCWSTR name, uint32_t def) const;

		int32_t get(PCWSTR name, int32_t def) const;

		bool get(PCWSTR name, bool def) const;

		void set(PCWSTR name, PCVOID value, size_t size);

		void set(PCWSTR name, PCWSTR value);

		void set(PCWSTR name, uint64_t value);

		operator HKEY () const;

		Register open_subkey(PCWSTR path, ACCESS_MASK acc = 0) const;

		Register open_subkey_stored_in_key(PCWSTR path, ACCESS_MASK acc = 0) const;

	private:
		HKEY m_hndl;
		ACCESS_MASK m_access;

		Register(const Register &);
		Register & operator = (const Register &);
	};

}

#endif
