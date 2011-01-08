/**	win_odbc.hpp
 *	@classes (ODBC_Conn, ODBC_Query) to manipulate ODBC
 *	@author GrAnD, 2009
 *	@link (odbc32)
**/
#ifndef WIN_ODBC_HPP
#define WIN_ODBC_HPP

#include <libwin_def/win_def.h>

#include <windows.h>
#include <sql.h>
#include <sqlext.h>

#include <vector>

///===================================================================================== definitions
enum		DBServer {
	MSSQL	= 0,	// SQL Server
	MSSQLNative,	// SQL Server Native
	MySQL,	// MySQL ODBC 3.51 Driver
	Oracle,	// Microsoft ODBC for Oracle
	Postgre,
};

struct		DataBinding {
	SQLSMALLINT	TargetType;
	SQLPOINTER	TargetValuePtr;
	SQLINTEGER	BufferLength;
	SQLLEN		StrLen_or_Ind;
};
struct		ColType {
	AutoUTF		name;
	SQLSMALLINT DataType;
	SQLUINTEGER ColumnSize;
	SQLSMALLINT DecimalDigits;
	SQLSMALLINT Nullable;
};

///======================================================================================= ODBC_base
namespace	ODBC_base {
void		SetODBCDriver(DBServer type, const AutoUTF &ds);
bool		FindAndSetDriver(SQLHENV conn, DBServer type, const AutoUTF &ds);
bool		GetStr(SQLHSTMT hstm, size_t col, AutoUTF &out);
AutoUTF		GetState(SQLSMALLINT type, SQLHANDLE handle, SQLSMALLINT RecNum, SQLWCHAR *state);
AutoUTF		MakeConnStr(const AutoUTF &drv, const AutoUTF &host, const AutoUTF &port, const AutoUTF &schm, const AutoUTF &name, const AutoUTF &pass, const AutoUTF &add = L"");
AutoUTF		MakeConnStr(DBServer srv, const AutoUTF &host, const AutoUTF &db, const AutoUTF &name, const AutoUTF &pass, bool tc = false);
}

///======================================================================================= OdbcError
class		OdbcError {
	AutoUTF		m_msg;
	DWORD		m_code;
	SQLWCHAR	m_state[6];
public:
	OdbcError(DWORD code): m_code(code) {
		WinMem::Zero(m_state, sizeof(m_state));
	}
	OdbcError(DWORD code, SQLSMALLINT type, SQLHANDLE hndl): m_code(code) {
		WinMem::Zero(m_state, sizeof(m_state));
		m_msg = CopyAfterLast(ODBC_base::GetState(type, hndl, 1, m_state), L"]");
	}
	AutoUTF		msg(PCWSTR msg) {
		return	(m_msg = msg);
	}
	AutoUTF		msg() const {
		return	m_msg;
	}
	DWORD		code() const {
		return	m_code;
	}
	AutoUTF		state() const {
		return	(PCWSTR) m_state;
	}
	void		show() const {
		mbox(m_msg.c_str(), L"OdbcError");
	}
};

inline void	OdbcChk(SQLRETURN r) {
	if (!SQL_SUCCEEDED(r)) {
		throw	OdbcError(r);
	}
}
inline void	OdbcChk(SQLRETURN r, SQLSMALLINT type, SQLHANDLE hndl) {
	if (!SQL_SUCCEEDED(r)) {
		throw	OdbcError(r, type, hndl);
	}
}

///======================================================================================= ODBC_Conn
class		ODBC_Conn : private Uncopyable {
	SQLHENV		m_henv;
	SQLHDBC		m_hdbc;
	bool		connected;

	void		RegisterODBC();
	void		BindODBC();
	bool		Connect(const AutoUTF &dsn, const AutoUTF &name, const AutoUTF &pass);
	bool		Connect(DBServer srv, const AutoUTF &host, const AutoUTF &schm, const AutoUTF &name, const AutoUTF &pass);
	void		Disconnect();
	void		UnBindODBC();
	void		UnRegisterODBC();

	ODBC_Conn();
public:
	virtual		~ODBC_Conn();
	ODBC_Conn(const AutoUTF &dsn, const AutoUTF &name, const AutoUTF &pass);
	ODBC_Conn(DBServer srv, const AutoUTF &host, const AutoUTF &schm, const AutoUTF &name, const AutoUTF &pass);

	operator	SQLHENV() const {
		return	m_henv;
	}
	operator	SQLHDBC() const {
		return	m_hdbc;
	}
	SQLHDBC		conn() const {
		return	m_hdbc;
	}

	AutoUTF		GetInfo(SQLUSMALLINT type) const;
	AutoUTF		GetStr(const AutoUTF &query, size_t col = 1) const;
	AutoUTF		GetHost() const {
		return	GetInfo(SQL_SERVER_NAME);
	}
	AutoUTF		GetUser() const {
		return	GetInfo(SQL_USER_NAME);
	}
	AutoUTF		GetVer() const {
		return	GetInfo(SQL_ODBC_VER);
	}
	AutoUTF		GetDB() const {
		return	GetInfo(SQL_DATABASE_NAME);
	}

	void		Use(const AutoUTF &in) const;
	void		Exec(const AutoUTF &query) const;
	void		ExecAndWait(const AutoUTF &query, DWORD wait = 1000) const;
};

///====================================================================================== ODBC_Query
class		ODBC_Query : private Uncopyable {
	ODBC_Conn const		*m_conn;
	SQLHSTMT			m_hstm;

//	StringVector		Fields;
//	StringVector		ShortFields;
	std::vector<ColType>	Fields;
	std::vector<AutoUTF>	RowData;
	size_t				NumFields;
	size_t				NumRows;
	bool				eof;

	size_t		ColCount() const;
	size_t		RowCount() const;
	void		InitFields();
	bool		IsValidIndex(size_t in) const {
		return	(in < RowData.size());
	}

	void		Open();
	void		Close();
	void		ReOpen();

	bool		GetRow(bool prNULL = true);

	ODBC_Query();
public:
	virtual ~ODBC_Query();
	ODBC_Query(const ODBC_Conn &conn);
	ODBC_Query(const ODBC_Conn &conn, const AutoUTF &query);
	ODBC_Query(const ODBC_Conn *conn);
	ODBC_Query(const ODBC_Conn *conn, const AutoUTF &query);

	operator	SQLHSTMT() {
		return	m_hstm;
	}

	void		Use(const AutoUTF &in) const {
		m_conn->Use(in);
	}
	void		Exec(const AutoUTF &query);
//	bool		printf(PSTR szSQL, PSTR szFmt = "", ...);
	//example:	q.printf("CREATE TABLE users3 (id INTEGER, name CHAR(40), salary FLOAT);");
	//example:	q.printf("INSERT INTO users3 (id, name, salary) VALUES (?, ?, ?);", "%d %s %f", 12, "Bill", 1003.14);
//	bool		scanf(CONSTR &query, CONSTR &fmt, ...);

	AutoUTF		GetDB() const {
		return	m_conn->GetDB();
	}
	void		GetDB_All();

	bool		IsNull(size_t index) const;
	int			AsInt(size_t index) const;
	long		AsLong(size_t index) const;
	double		AsDouble(size_t index) const;
	AutoUTF		operator[](size_t index) const;

	AutoUTF		FieldName(int index) const;
	AutoUTF		FieldShortName(int index) const;
	int			FieldIndex(const AutoUTF &name, bool Short = false);

	bool		Eof() const {
		return	eof;
	};
	bool		Next();

	size_t		Size() const {
		return	NumFields;
	}
	size_t		GetRowCount() const {
		return	NumRows;
	}
};

#endif //WIN_ODBC_HPP
