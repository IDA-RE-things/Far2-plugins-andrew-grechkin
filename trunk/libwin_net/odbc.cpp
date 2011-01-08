/** win_odbc.cpp
 *	@classes (ODBC_Conn, ODBC_Query) to manipulate ODBC
 *	@author GrAnD, 2009
 *	@link (odbc32)
**/
#include "odbc.h"

///===================================================================================== definitions

///================================================================================== implementation
AutoUTF		DBServerNames[] = {
	L"SQL Server",
	L"SQL Server Native Client 10.0",
	L"MySQL ODBC 5.1 Driver",
	L"Microsoft ODBC for Oracle",
	L"Postgre SQL",
	L"",
};

///======================================================================================= ODBC_base
void		ODBC_base::SetODBCDriver(DBServer type, const AutoUTF &ds) {
	DBServerNames[type] = ds;
}
bool		ODBC_base::FindAndSetDriver(SQLHENV conn, DBServer type, const AutoUTF &ds) {
	bool	Result = false;

	if (conn) {
		SQLWCHAR	szDriverDesc[SQL_MAX_MESSAGE_LENGTH];
		SQLRETURN	err = ::SQLDriversW(conn, SQL_FETCH_FIRST, szDriverDesc, sizeofa(szDriverDesc) - 1, 0, 0, -1, 0);
		while (SQL_SUCCEEDED(err)) {
			AutoUTF	tmp = szDriverDesc;
			if (tmp.Find(ds)) {
				SetODBCDriver(type, tmp);
				Result = true;
				break;
			}
			err = ::SQLDriversW(conn, SQL_FETCH_NEXT, szDriverDesc, sizeofa(szDriverDesc) - 1, 0, 0, -1, 0);
		}
	}
	return	Result;
}
bool		ODBC_base::GetStr(SQLHSTMT hstm, size_t col, AutoUTF &out) {
	SQLLEN	size = 0;
	WinBuf<WCHAR>	buf(4096);
	SQLRETURN	err = ::SQLGetData(hstm, col, SQL_C_WCHAR, buf.data(), buf.capacity(), &size);
	if (SQL_SUCCEEDED(err)) {
		out = (size == SQL_NULL_DATA) ? L"NULL" : buf.data();
	}
	return	SQL_SUCCEEDED(err);
}
AutoUTF		ODBC_base::GetState(SQLSMALLINT type, SQLHANDLE handle, SQLSMALLINT RecNum, SQLWCHAR *state) {
	SQLWCHAR	Msg[SQL_MAX_MESSAGE_LENGTH];
	SQLINTEGER	NativeError;
	SQLSMALLINT	MsgLen;
	::SQLGetDiagRecW(type, handle, RecNum, state, &NativeError, Msg, sizeofa(Msg), &MsgLen);
	return	Msg;
}
AutoUTF		ODBC_base::MakeConnStr(const AutoUTF &drv, const AutoUTF &host, const AutoUTF &port, const AutoUTF &schm, const AutoUTF &name, const AutoUTF &pass, const AutoUTF &add) {
	AutoUTF	Result(L"Driver");
	Result.Add(drv, L"={");
	Result.Add(L"}");
	if (!host.empty()) {
		Result.Add(L"Server", L";");
		Result.Add(host, L"=");
	}
	if (!port.empty()) {
		Result.Add(port, L",");
	}
	if (!schm.empty()) {
		Result.Add(L"Database", L";");
		Result.Add(schm, L"=");
	}
	if (!name.empty()) {
		Result.Add(L"Uid", L";");
		Result.Add(name, L"=");
	}
	if (!pass.empty()) {
		Result.Add(L"Pwd", L";");
		Result.Add(pass, L"=");
	}
	if (!add.empty()) {
		Result.Add(add, L";");
	}
	return	Result;
}
AutoUTF		ODBC_base::MakeConnStr(DBServer srv, const AutoUTF &host, const AutoUTF &schm, const AutoUTF &name, const AutoUTF &pass, bool tc) {
	AutoUTF	tp(host);
	AutoUTF	th = CutWord(tp, L":");

	AutoUTF Result;
	switch (srv) {
		case Oracle:
			Result = MakeConnStr(DBServerNames[srv], th, tp, schm, name, pass);
			break;
		case MSSQL:
		case MSSQLNative:
			Result = (tc) ?
					 MakeConnStr(DBServerNames[srv], th, tp, schm, "", "", "Trusted_Connection=yes") :
					 MakeConnStr(DBServerNames[srv], th, tp, schm, name, pass);
			break;
		case MySQL:
			Result = MakeConnStr(DBServerNames[srv], th, tp, schm, name, pass);//, "Option=16384;Stmt=");
			break;
		default:
			;
	}
	return	Result;
}

///======================================================================================= ODBC_Conn
//private
void		ODBC_Conn::RegisterODBC() {
	if (m_henv)
		return;
	OdbcChk(::SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_henv));
	OdbcChk(::SQLSetEnvAttr(m_henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER));
}
void		ODBC_Conn::BindODBC() {
	if (m_hdbc)
		return;
	RegisterODBC();
	SQLRETURN err = ::SQLAllocHandle(SQL_HANDLE_DBC, m_henv, &m_hdbc);
	OdbcChk(err);
//		OdbcChk(::SQLSetConnectAttr(m_hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)10, 0));
//		OdbcChk(::SQLSetConnectAttr(m_hdbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_ON, 0));
}
bool		ODBC_Conn::Connect(const AutoUTF &dsn, const AutoUTF &name, const AutoUTF &pass) {
	BindODBC();
	SQLRETURN err = ::SQLConnectW(m_hdbc, (SQLWCHAR*)dsn.c_str(), SQL_NTS,
								  (SQLWCHAR*)((name.empty()) ? NULL : name.c_str()), SQL_NTS,
								  (SQLWCHAR*)((pass.empty()) ? NULL : pass.c_str()), SQL_NTS);
	OdbcChk(err, SQL_HANDLE_DBC, m_hdbc);
	return	true;
}
bool		ODBC_Conn::Connect(DBServer srv, const AutoUTF &host, const AutoUTF &schm, const AutoUTF &name, const AutoUTF &pass) {
	BindODBC();
	AutoUTF		dsn = ODBC_base::MakeConnStr(srv, host, schm, name, pass);
	SQLWCHAR	OutConnStr[1024];
	SQLSMALLINT	OutConnStrLen = 1024;
	SQLRETURN	err = ::SQLDriverConnectW(m_hdbc, NULL, (SQLWCHAR*)dsn.c_str(), dsn.size(), OutConnStr, OutConnStrLen, &OutConnStrLen, SQL_DRIVER_NOPROMPT);
	OdbcChk(err, SQL_HANDLE_DBC, m_hdbc);
	return	true;
}
void		ODBC_Conn::Disconnect() {
	if (!connected)
		return;
	OdbcChk(::SQLDisconnect(m_hdbc));
	connected = false;
}

void		ODBC_Conn::UnBindODBC() {
	if (m_hdbc == SQL_NULL_HANDLE)
		return;
	Disconnect();
	OdbcChk(::SQLFreeHandle(SQL_HANDLE_DBC, m_hdbc));
	m_hdbc = SQL_NULL_HANDLE;
}
void		ODBC_Conn::UnRegisterODBC() {
	if (m_henv == SQL_NULL_HANDLE)
		return;
	UnBindODBC();
	OdbcChk(::SQLFreeHandle(SQL_HANDLE_ENV, m_henv));
	m_henv = SQL_NULL_HANDLE;
}

//public
ODBC_Conn::~ODBC_Conn() {
	try {
		UnRegisterODBC();
	} catch (OdbcError e) {
	}
}
ODBC_Conn::ODBC_Conn(const AutoUTF &dsn, const AutoUTF &name, const AutoUTF &pass): connected(false) {
	m_henv = m_hdbc = SQL_NULL_HANDLE;
	connected = Connect(dsn, name, pass);
}
ODBC_Conn::ODBC_Conn(DBServer srv, const AutoUTF &host, const AutoUTF &schm, const AutoUTF &name, const AutoUTF &pass): connected(false) {
	m_henv = m_hdbc = SQL_NULL_HANDLE;
	connected = Connect(srv, host, schm, name, pass);
}

AutoUTF		ODBC_Conn::GetInfo(SQLUSMALLINT type) const {
	SQLSMALLINT bufSize = 0;
	SQLRETURN	err = ::SQLGetInfoW(m_hdbc, type, NULL, bufSize, &bufSize);
	OdbcChk(err, SQL_HANDLE_DBC, m_hdbc);
	bufSize = sizeof(SQLWCHAR) * (bufSize + 1);
	WinBuf<SQLWCHAR>	data(bufSize);
	err = ::SQLGetInfoW(m_hdbc, type, data, bufSize, &bufSize);
	OdbcChk(err, SQL_HANDLE_DBC, m_hdbc);
	return	data.data();
}
AutoUTF		ODBC_Conn::GetStr(const AutoUTF &query, size_t col) const {
	AutoUTF		Result;
	SQLHSTMT	hstm;
	SQLRETURN	err = ::SQLAllocHandle(SQL_HANDLE_STMT, m_hdbc, &hstm);
	OdbcChk(err, SQL_HANDLE_DBC, m_hdbc);
	::SQLSetStmtAttr(hstm, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER) - 1, SQL_IS_UINTEGER);
	err = ::SQLExecDirectW(hstm, (SQLWCHAR*)query.c_str(), query.size());
	OdbcChk(err, SQL_HANDLE_STMT, hstm);
	err = ::SQLFetch(hstm);
	if (SQL_SUCCEEDED(err)) {
		ODBC_base::GetStr(hstm, col, Result);
	}
	err = ::SQLCloseCursor(hstm);
	::SQLFreeHandle(SQL_HANDLE_STMT, hstm);
	return	Result;
}

void		ODBC_Conn::Use(const AutoUTF &in) const {
	SQLRETURN	err = ::SQLSetConnectAttrW(m_hdbc, SQL_ATTR_CURRENT_CATALOG, (SQLPOINTER)(in.c_str()), SQL_NTS);
	OdbcChk(err, SQL_HANDLE_DBC, m_hdbc);
}
void		ODBC_Conn::Exec(const AutoUTF &query) const {
	SQLHSTMT	hstm;
	SQLRETURN	err = ::SQLAllocHandle(SQL_HANDLE_STMT, m_hdbc, &hstm);
	OdbcChk(err, SQL_HANDLE_DBC, m_hdbc);
	::SQLSetStmtAttr(hstm, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER) - 1, SQL_IS_UINTEGER);
	err = ::SQLExecDirectW(hstm, (SQLWCHAR*)query.c_str(), query.size());
	if (SQL_SUCCEEDED(err)) {
		SQLWCHAR	errstate[6] = {0};
		SQLWCHAR	Msg[SQL_MAX_MESSAGE_LENGTH] = {0};
		SQLINTEGER	NativeError;
		SQLSMALLINT	MsgLen;
		if ((err == SQL_SUCCESS_WITH_INFO) || (err == SQL_ERROR)) {
			int i = 1;
			while (::SQLGetDiagRecW(SQL_HANDLE_STMT, hstm, i++, errstate, &NativeError, Msg, sizeofa(Msg), &MsgLen) != SQL_NO_DATA) {
//				cout << "errstate: " << errstate << endl;
//				cout << "Msg:      " << Msg << endl;
			}
		}
		do {
			err = ::SQLFetch(hstm);
		} while (err != SQL_ERROR && err != SQL_NO_DATA);
	} else {
		OdbcChk(err, SQL_HANDLE_STMT, hstm);
	}
	OdbcChk(::SQLFreeHandle(SQL_HANDLE_STMT, hstm));
}
void		ODBC_Conn::ExecAndWait(const AutoUTF &query, DWORD wait) const {
	SQLHSTMT	hstm;
	SQLRETURN	err = ::SQLAllocHandle(SQL_HANDLE_STMT, m_hdbc, &hstm);
	OdbcChk(err, SQL_HANDLE_DBC, m_hdbc);
	::SQLSetStmtAttr(hstm, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER) - 1, SQL_IS_UINTEGER);
	err = ::SQLExecDirectW(hstm, (SQLWCHAR*)query.c_str(), query.size());
	if (SQL_SUCCEEDED(err)) {
		SQLWCHAR	errstate[6] = {0};
		SQLWCHAR	Msg[SQL_MAX_MESSAGE_LENGTH] = {0};
		SQLINTEGER	NativeError;
		SQLSMALLINT	MsgLen;
		if ((err == SQL_SUCCESS_WITH_INFO) || (err == SQL_ERROR)) {
			int i = 1;
			while (err != SQL_NO_DATA) {
				err = ::SQLGetDiagRecW(SQL_HANDLE_STMT, hstm, i++, errstate, &NativeError, Msg, sizeofa(Msg), &MsgLen);
//				cout << "errstate: " << errstate << endl;
//				cout << "Msg:      " << Msg << endl;
			}
		}
		do {
			err = ::SQLFetch(hstm);
		} while (err != SQL_ERROR && err != SQL_NO_DATA);
		::Sleep(wait);
	} else {
		OdbcChk(err, SQL_HANDLE_STMT, hstm);
	}
	OdbcChk(::SQLFreeHandle(SQL_HANDLE_STMT, hstm));
}

///====================================================================================== ODBC_Query
//private
size_t		ODBC_Query::ColCount() const {
	SQLSMALLINT Result = 0;
	SQLRETURN	err = ::SQLNumResultCols(m_hstm, &Result);
	return	SQL_SUCCEEDED(err) ? Result : 0;
}
size_t		ODBC_Query::RowCount() const {
	SQLLEN Result = 0;
	SQLRETURN	err = ::SQLRowCount(m_hstm, &Result);
	return	SQL_SUCCEEDED(err) ? Result : 0;
}
void		ODBC_Query::InitFields() {
	if (NumFields == 0) {
		NumFields = ColCount();
		NumRows = RowCount();

		if (NumFields) {
			WinBuf<WCHAR> name(1024);
			SQLSMALLINT NameLength;
			for (DWORD i = 0; i < NumFields; ++i) {
				ColType col;
				::SQLDescribeColW(m_hstm, i + 1, (SQLWCHAR*)name.data(), name.capacity(), &NameLength, &col.DataType, &col.ColumnSize, &col.DecimalDigits, &col.Nullable);
				col.name = name.data();
				Fields.push_back(col);
			}
		}
	}
}

void		ODBC_Query::Open() {
	if (m_hstm != SQL_NULL_HANDLE)
		return;
	NumRows		= 0;
	NumFields	= 0;
	SQLRETURN	err = ::SQLAllocHandle(SQL_HANDLE_STMT, m_conn->conn(), &m_hstm);
	OdbcChk(err, SQL_HANDLE_DBC, m_conn->conn());
	::SQLSetStmtAttr(m_hstm, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER) - 1, SQL_IS_UINTEGER);
}
void		ODBC_Query::Close() {
	if (m_hstm == SQL_NULL_HANDLE)
		return;
	SQLRETURN	err = ::SQLCloseCursor(m_hstm);
	err = ::SQLFreeHandle(SQL_HANDLE_STMT, m_hstm);
	OdbcChk(err, SQL_HANDLE_STMT, m_hstm);
	m_hstm = SQL_NULL_HANDLE;
	NumRows		= 0;
	NumFields	= 0;
	Fields.clear();
	RowData.clear();
}
void		ODBC_Query::ReOpen() {
	if (m_hstm != SQL_NULL_HANDLE)
		Close();
	Open();
}

bool		ODBC_Query::GetRow(bool prNULL) {
	SQLRETURN	err = 0;
	if (NumFields) {
		SQLLEN	size = 0;
		WinBuf<WCHAR> buf(4096);
		RowData.clear();
		for (DWORD i = 1; i <= NumFields; ++i) {
			err = ::SQLGetData(m_hstm, i, SQL_C_WCHAR, buf.data(), buf.capacity(), &size);
			if (SQL_SUCCEEDED(err)) {
				(size == SQL_NULL_DATA) ? RowData.push_back((prNULL) ? L"NULL" : L"") : RowData.push_back(buf.data());
			} //else {
//				RowData.push_back(errmsg);
//			}
		}
	}
	return	SQL_SUCCEEDED(err);
}

//public
ODBC_Query::~ODBC_Query() {
	try {
		Close();
	} catch (OdbcError e) {
	}
}
ODBC_Query::ODBC_Query(const ODBC_Conn &conn): m_conn(&conn), m_hstm(SQL_NULL_HANDLE) {
	NumFields = NumRows = 0;
	eof = true;
}
ODBC_Query::ODBC_Query(const ODBC_Conn &conn, const AutoUTF &query): m_conn(&conn), m_hstm(SQL_NULL_HANDLE) {
	NumFields = NumRows = 0;
	eof = true;
	Exec(query);
}
ODBC_Query::ODBC_Query(const ODBC_Conn *conn): m_conn(conn), m_hstm(SQL_NULL_HANDLE) {
	NumFields = NumRows = 0;
	eof = true;
}
ODBC_Query::ODBC_Query(const ODBC_Conn *conn, const AutoUTF &query): m_conn(conn), m_hstm(SQL_NULL_HANDLE) {
	NumFields = NumRows = 0;
	eof = true;
	Exec(query);
}

void		ODBC_Query::Exec(const AutoUTF &query) {
	ReOpen();
	SQLRETURN	err = ::SQLExecDirectW(m_hstm, (SQLWCHAR*)query.c_str(), query.size());
	OdbcChk(err, SQL_HANDLE_STMT, m_hstm);
	InitFields();
	Next();
}

/*
bool		ODBC_Query::printf(PSTR szSQL, PSTR szFmt, ...) {
	va_list p_arg;
	ReOpen();
	if (SQL_SUCCEEDED(err = ::SQLPrepareA(m_hstm, (SQLCHAR*)szSQL, SQL_NTS))) {
		int iParam = 0;				// Счётчик параметров
		va_start(p_arg, szFmt);		// Начинаем выборку аргументов со стека
		for (const char *str = szFmt; *str; str++) {
			if (*str == '%') {
				str++;
				PSTR	val1 = va_arg(p_arg, PSTR);
				LONG	val2 = va_arg(p_arg, LONG);
				double	val3 = va_arg(p_arg, double);
				switch (*str) {
					case 's':
						::SQLBindParameter(m_hstm, ++iParam, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_VARCHAR, lstrlenA(val1) + 1, 0, val1, 0, 0);
						break;
					case 'd':
						::SQLBindParameter(m_hstm, ++iParam, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &val2, 0, 0);
						break;
					case 'f':
						::SQLBindParameter(m_hstm, ++iParam, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &val3, 0, 0);
						break;
				}
			}
		}
		va_end(p_arg);
		err = ::SQLExecute(m_hstm);
	}
	return	SQL_SUCCEEDED(err);
}
*/
void		ODBC_Query::GetDB_All() {
	ReOpen();
//	SQLRETURN	err = ::SQLTablesW(m_hstm, (SQLWCHAR*)SQL_ALL_CATALOGS, SQL_NTS, (SQLCHAR*)L"", SQL_NTS, (SQLWCHAR*)L"", SQL_NTS, (SQLWCHAR*)L"", SQL_NTS);
	SQLRETURN	err = ::SQLTablesW(m_hstm, (SQLWCHAR*)L"%", SQL_NTS, (SQLWCHAR*)L"", SQL_NTS, (SQLWCHAR*)L"", SQL_NTS, (SQLWCHAR*)L"", SQL_NTS);
	OdbcChk(err, SQL_HANDLE_STMT, m_hstm);
	InitFields();
	Next();
}
bool		ODBC_Query::IsNull(size_t index) const {
	bool Result = true;
	if (NumFields && index <= NumFields) {
		SQLLEN	size = 5;
		WinBuf<WCHAR> buf(size);
		SQLRETURN	err = ::SQLGetData(m_hstm, index, SQL_C_WCHAR, buf.data(), buf.capacity(), &size);
		if (SQL_SUCCEEDED(err)) {
			Result = (size == SQL_NULL_DATA) ? true : false;
		}
	}
	return	Result;
}
int			ODBC_Query::AsInt(size_t index) const {
	return	IsValidIndex(index) ? _wtoi(RowData[index].c_str()) : 0;
}
long		ODBC_Query::AsLong(size_t index) const {
	return	IsValidIndex(index) ? _wtol(RowData[index].c_str()) : 0;
}
double		ODBC_Query::AsDouble(size_t index) const {
	return	IsValidIndex(index) ? _wtof(RowData[index].c_str()) : 0;
}
AutoUTF		ODBC_Query::operator[](size_t index) const {
	return	IsValidIndex(index) ? RowData[index] : L"";
}

AutoUTF		ODBC_Query::FieldName(int index) const {
	return	Fields[index].name;
}
AutoUTF		ODBC_Query::FieldShortName(int index) const {
	return	Fields[index].name;
}
int			ODBC_Query::FieldIndex(const AutoUTF &name, bool Short) {
	int index = 0;
	if (Short) {
		for (std::vector<ColType>::iterator f = Fields.begin(); f != Fields.end(); f++, index++)
			if (name == f->name)
				return	index;
	} else {
		for (std::vector<ColType>::iterator f = Fields.begin(); f != Fields.end(); f++, index++)
			if (name == f->name)
				return	index;
	}
	return	-1;
}

bool		ODBC_Query::Next() {
	if (m_hstm == SQL_NULL_HANDLE || NumFields == 0) {
		eof = true;
		return	false;
	}
	SQLRETURN	err = ::SQLFetch(m_hstm);
	eof = (err == SQL_NO_DATA);
	if (SQL_SUCCEEDED(err))
		GetRow();
	return	SQL_SUCCEEDED(err) && !eof;
}
