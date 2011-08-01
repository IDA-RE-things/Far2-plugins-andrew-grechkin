/**	win_odbc.hpp
 *	@classes (ODBC_Conn, ODBC_Query) to manipulate ODBC
 *	@author GrAnD, 2009
 *	@link (odbc32)
**/
#ifndef WIN_ODBC_HPP
#define WIN_ODBC_HPP

#include <libwin_def/win_def.h>

#include <sql.h>
#include <sqlext.h>

#include <vector>

///===================================================================================== definitions
enum DBServer {
	MSSQL	= 0,	// SQL Server
	MSSQLNative,	// SQL Server Native
	MySQL,	// MySQL ODBC 3.51 Driver
	Oracle,	// Microsoft ODBC for Oracle
	Postgre,
};

struct DataBinding {
	SQLSMALLINT	TargetType;
	SQLPOINTER	TargetValuePtr;
	SQLINTEGER	BufferLength;
	SQLLEN		StrLen_or_Ind;
};
struct ColType {
	ustring		name;
	SQLSMALLINT DataType;
	SQLULEN ColumnSize;
	SQLSMALLINT DecimalDigits;
	SQLSMALLINT Nullable;
};

///======================================================================================= ODBC_base
namespace ODBC_base {
void SetODBCDriver(DBServer type, const ustring &ds);
bool FindAndSetDriver(SQLHENV conn, DBServer type, const ustring &ds);
bool GetStr(SQLHSTMT hstm, size_t col, ustring &out);
ustring GetState(SQLSMALLINT type, SQLHANDLE handle, SQLSMALLINT RecNum, SQLWCHAR *state);
ustring MakeConnStr(const ustring &drv, const ustring &host, const ustring &port, const ustring &schm, const ustring &name, const ustring &pass, const ustring &add = L"");
ustring MakeConnStr(DBServer srv, const ustring &host, const ustring &db, const ustring &name, const ustring &pass, bool tc = false);
}

///======================================================================================= OdbcError
class		OdbcError {
public:
	OdbcError(DWORD code): m_code(code) {
		WinMem::Zero(m_state, sizeof(m_state));
	}
	OdbcError(DWORD code, SQLSMALLINT type, SQLHANDLE hndl): m_code(code) {
		WinMem::Zero(m_state, sizeof(m_state));
		m_msg = CopyAfterLast(ODBC_base::GetState(type, hndl, 1, m_state), L"]");
	}
	ustring	msg(PCWSTR msg) {
		return (m_msg = msg);
	}
	ustring	msg() const {
		return m_msg;
	}
	DWORD	code() const {
		return m_code;
	}
	ustring	state() const {
		return (PCWSTR) m_state;
	}
	void	show() const {
		mbox(m_msg.c_str(), L"OdbcError");
	}
private:
	ustring m_msg;
	DWORD m_code;
	SQLWCHAR m_state[6];
};

inline void OdbcChk(SQLRETURN r) {
	if (!SQL_SUCCEEDED(r)) {
		throw OdbcError(r);
	}
}
inline void OdbcChk(SQLRETURN r, SQLSMALLINT type, SQLHANDLE hndl) {
	if (!SQL_SUCCEEDED(r)) {
		throw OdbcError(r, type, hndl);
	}
}

///======================================================================================= ODBC_Conn
class ODBC_Conn : private Uncopyable {
	SQLHENV	m_henv;
	SQLHDBC	m_hdbc;
	bool	connected;

	void		RegisterODBC();
	void		BindODBC();
	bool		Connect(const ustring &dsn, const ustring &name, const ustring &pass);
	bool		Connect(DBServer srv, const ustring &host, const ustring &schm, const ustring &name, const ustring &pass);
	void		Disconnect();
	void		UnBindODBC();
	void		UnRegisterODBC();

	ODBC_Conn();
public:
	virtual		~ODBC_Conn();
	ODBC_Conn(const ustring &dsn, const ustring &name, const ustring &pass);
	ODBC_Conn(DBServer srv, const ustring &host, const ustring &schm, const ustring &name, const ustring &pass);

	operator	SQLHENV() const {
		return m_henv;
	}
	operator	SQLHDBC() const {
		return m_hdbc;
	}
	SQLHDBC		conn() const {
		return m_hdbc;
	}

	ustring		GetInfo(SQLUSMALLINT type) const;
	ustring		GetStr(const ustring &query, size_t col = 1) const;
	ustring		GetHost() const {
		return GetInfo(SQL_SERVER_NAME);
	}
	ustring		GetUser() const {
		return GetInfo(SQL_USER_NAME);
	}
	ustring		GetVer() const {
		return GetInfo(SQL_ODBC_VER);
	}
	ustring		GetDB() const {
		return GetInfo(SQL_DATABASE_NAME);
	}

	void	Use(const ustring &in) const;
	void	Exec(const ustring &query) const;
	void	ExecAndWait(const ustring &query, DWORD wait = 1000) const;
};

///====================================================================================== ODBC_Query
class ODBC_Query : private Uncopyable {
	ODBC_Conn const	*m_conn;
	SQLHSTMT		m_hstm;

//	StringVector	Fields;
//	StringVector	ShortFields;
	std::vector<ColType> Fields;
	std::vector<ustring> RowData;
	size_t	NumFields;
	size_t	NumRows;
	bool	eof;

	size_t	ColCount() const;
	size_t	RowCount() const;
	void	InitFields();
	bool	IsValidIndex(size_t in) const {
		return (in < RowData.size());
	}

	void	Open();
	void	Close();
	void	ReOpen();

	bool	GetRow(bool prNULL = true);

	ODBC_Query();
public:
	virtual ~ODBC_Query();
	ODBC_Query(const ODBC_Conn &conn);
	ODBC_Query(const ODBC_Conn &conn, const ustring &query);
	ODBC_Query(const ODBC_Conn *conn);
	ODBC_Query(const ODBC_Conn *conn, const ustring &query);

	operator SQLHSTMT() {
		return m_hstm;
	}

	void	Use(const ustring &in) const {
		m_conn->Use(in);
	}
	void	Exec(const ustring &query);
//	bool	printf(PSTR szSQL, PSTR szFmt = "", ...);
	//example:	q.printf("CREATE TABLE users3 (id INTEGER, name CHAR(40), salary FLOAT);");
	//example:	q.printf("INSERT INTO users3 (id, name, salary) VALUES (?, ?, ?);", "%d %s %f", 12, "Bill", 1003.14);
//	bool		scanf(CONSTR &query, CONSTR &fmt, ...);

	ustring	GetDB() const {
		return m_conn->GetDB();
	}
	void	GetDB_All();

	bool	IsNull(size_t index) const;
	int		AsInt(size_t index) const;
	long	AsLong(size_t index) const;
	double	AsDouble(size_t index) const;
	ustring	operator[](size_t index) const;

	ustring	FieldName(int index) const;
	ustring	FieldShortName(int index) const;
	int		FieldIndex(const ustring &name, bool Short = false);

	bool	Eof() const {
		return eof;
	};
	bool	Next();

	size_t	Size() const {
		return NumFields;
	}
	size_t	GetRowCount() const {
		return NumRows;
	}
};

#endif //WIN_ODBC_HPP
