#ifdef __PC__

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#undef SQL_NOUNICODEMAP

#include "OdbcInterface.h"

#undef SQLHDBC
#undef SQLHENV
#undef SQLHSTMT
#undef SQLRETURN
#undef SQLSMALLINT
#undef SQLINTEGER
#undef SQLTCHAR


#include <sql.h>
#include <Sqlucode.h>
#include <sqlext.h>
#include <odbcinst.h>
// #include <sqltypes.h>

#include <sqlext.h>
#include <odbcss.h>

#ifdef _DEBUG
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#define MAXBUFLEN 255

OdbcConnection::OdbcConnection(TCHAR* dataSourceName) :
mValid(false),
mSqlEnvHandle(SQL_NULL_HENV),
mSqlDbConnectionHandle(SQL_NULL_HDBC)
{
  SQLRETURN result;


  // Allocate the ODBC Environment and save handle.
  result = SQLAllocHandle (SQL_HANDLE_ENV, SQL_NULL_HANDLE, &mSqlEnvHandle);
  if ((result != SQL_SUCCESS) && (result != SQL_SUCCESS_WITH_INFO))
  {
//    throw new OdbcException( _T( "OdbcConnection::OdbcConnection() - SQLAllochandle failed !!" ), NULL );
    throw _T("OdbcConnection::OdbcConnection(TCHAR*) - SQLAllochandle failed !!");
  }
  
  // Notify ODBC that this is an ODBC 3.0 application.
  result = SQLSetEnvAttr(mSqlEnvHandle, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER);
  if ((result != SQL_SUCCESS) && (result != SQL_SUCCESS_WITH_INFO))
  {
    SQLFreeHandle(SQL_HANDLE_ENV, mSqlEnvHandle);
    throw _T("OdbcConnection::OdbcConnection(TCHAR*) Unable to set ODBC_VERSION Enviroment Attr ");
  }
  
  // Allocate an ODBC connection handle and connect.
  result = SQLAllocHandle(SQL_HANDLE_DBC, mSqlEnvHandle, &mSqlDbConnectionHandle);
  if ((result != SQL_SUCCESS) && (result != SQL_SUCCESS_WITH_INFO))
  {

    SQLFreeHandle(SQL_HANDLE_ENV, mSqlEnvHandle);
    throw _T("OdbcConnection::OdbcConnection(TCHAR*) - SQLAllocHandle failed !!");
  }
  
  result = SQLSetConnectAttr(mSqlDbConnectionHandle, SQL_ATTR_LOGIN_TIMEOUT, (SQLPOINTER)10, SQL_IS_INTEGER);
  if ((result != SQL_SUCCESS) && (result != SQL_SUCCESS_WITH_INFO))
  {
    SQLFreeHandle(SQL_HANDLE_DBC, mSqlDbConnectionHandle);
    SQLFreeHandle(SQL_HANDLE_ENV, mSqlEnvHandle);
    throw _T("OdbcConnection::OdbcConnection(TCHAR*) - SQLSetConnectAttr SQL_ATTR_LOGIN_TIMEOUT failed !!");
  }
  
  SQLSMALLINT   cbConnStrOut = 0;

  result= SQLDriverConnect(mSqlDbConnectionHandle,      // Connection handle
                            NULL,         // Window handle
                            (SQLTCHAR*)(dataSourceName),      // Input connect string
                            SQL_NTS,         // Null-terminated string
                            0,      // Address of output buffer
                            0,      // Size of output buffer
                            &cbConnStrOut,   // Address of output length
                            SQL_DRIVER_NOPROMPT); // Changed by CIM-HM
                            //SQL_DRIVER_COMPLETE);
  if ((result != SQL_SUCCESS) && (result != SQL_SUCCESS_WITH_INFO))
  {
    SQLFreeHandle(SQL_HANDLE_DBC, mSqlDbConnectionHandle);
    SQLFreeHandle(SQL_HANDLE_ENV, mSqlEnvHandle);

    TCHAR szMsg[512];
    _stprintf( szMsg, _T("OdbcConnection::OdbcConnection(TCHAR*) - SQLDriverConnect failed\nMake sure your database is in the correct directory.\nConnection string: %s"), dataSourceName );
    throw szMsg;
  }
  mAutoCommit = true;
  mValid = true;
}

OdbcConnection::~OdbcConnection()
{
  if (mValid)
  {
    SQLDisconnect(mSqlDbConnectionHandle);
    SQLFreeHandle(SQL_HANDLE_DBC, mSqlDbConnectionHandle);
    SQLFreeHandle(SQL_HANDLE_ENV, mSqlEnvHandle);
  }
}


SQLHDBC OdbcConnection::GetConnectionHandle()
{
  return mSqlDbConnectionHandle;
}


bool OdbcConnection::IsValid()
{
  return mValid;
}

/**
* @author hm@cim.as
*
* Starts a transaction on the connection, by setting the auto commit,
* connection attribute.
*/
void OdbcConnection::BeginTrans()
{
  if( !mAutoCommit )
  {
    throw _T( "OdbcConnection::BeginTrans() - Error starting a transaction. Cause: A transaction has already been started."); 
  }
  
  SetAutoCommit( false );
}

void OdbcConnection::CommitTrans()
{
  if( mAutoCommit )
  {
    throw _T( "OdbcConnection::CommitTrans() - Error committing transaction. Cause: A transaction has not been started." ); 
  }
  SetAutoCommit( true );
}


/**
* @author hm@cim.as
*
* Rollsback a transaction on the connection, and sets the autocommit state
* to on.
*/
void OdbcConnection::RollbackTrans()
{
  SQLEndTran( SQL_HANDLE_DBC,mSqlDbConnectionHandle, SQL_ROLLBACK);
  SetAutoCommit();
}

/**
* @author hm@cim.as
*
* Sets the auto commit state for the connection. When enabling 
* autocommiting all transaction on all statements are commited
* ( ^ Bill says so him self ^ )
*/
void OdbcConnection::SetAutoCommit( bool enable )
{
  ULONG mode = SQL_AUTOCOMMIT_ON;
  if( !enable )
    mode = SQL_AUTOCOMMIT_OFF;
  
  SQLRETURN rc = SQLSetConnectAttr( mSqlDbConnectionHandle, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)mode, SQL_IS_UINTEGER );
  switch( rc )
  {
  case SQL_SUCCESS:
    
  // fall through
  case SQL_SUCCESS_WITH_INFO :
    mAutoCommit = enable;
    break;
  default:
    throw _T( "OdbcConnection::SetAutoCommit(bool) - Unknown error");
  }
}

OdbcStatement::OdbcStatement(OdbcConnection* pConnection) :
mValid(false),
mpConnection(pConnection),
mSqlStatementHandle(SQL_NULL_HSTMT)
{
  if (!mpConnection) return;
  
  SQLRETURN result;
  
  result = SQLAllocHandle(SQL_HANDLE_STMT, mpConnection->GetConnectionHandle(), &mSqlStatementHandle);
  if ((result != SQL_SUCCESS) && (result != SQL_SUCCESS_WITH_INFO))
  {
    throw _T( "OdbcStatement::OdbcStatement(OdbcConnection*) - SQLAllocHandle failed"); 
  }
  
  mValid = true;
}

OdbcStatement::~OdbcStatement()
{
  SQLFreeHandle(SQL_HANDLE_STMT, mSqlStatementHandle);
}


bool OdbcStatement::ExecDirect(TCHAR* statement)
{
  if( mSqlStatementHandle == SQL_NULL_HSTMT )
  {
    throw _T( "OdbcStatement::ExecDirect(TCHAR*) - The statement handle is invalid");
  }  
  SQLRETURN result = SQLExecDirect(mSqlStatementHandle, (SQLTCHAR*)(statement), _tcslen(statement));
  
  if (result != SQL_SUCCESS && result != SQL_SUCCESS_WITH_INFO && result != SQL_NO_DATA)
  {
    SQLTCHAR sql_state[7];
    SQLTCHAR message_text[1024];
    GetDiag(sql_state,message_text,1024);
    TCHAR szMsg[1200];
    _stprintf(szMsg, _T( "OdbcStatement::ExecDirect(TCHAR*) - SQLExecDirect failed.\n%s"),message_text);
    throw szMsg;
  }
  return result == SQL_SUCCESS || result == SQL_SUCCESS_WITH_INFO || result == SQL_NO_DATA;
}


bool OdbcStatement::Fetch()
{
  SQLRETURN result = SQLFetch(mSqlStatementHandle);
  if(result == SQL_ERROR)
  {
    SQLTCHAR sql_state[7];
    SQLTCHAR message_text[1024];
    GetDiag(sql_state,message_text,1024);
    TCHAR szMsg[1200];
    _stprintf(szMsg, _T( "OdbcStatement::Fetch() - SQLFetch failed.\n%s"),message_text);
    throw szMsg;
  }
  return result == SQL_SUCCESS || result == SQL_SUCCESS_WITH_INFO;
}

int OdbcStatement::GetColCount()
{
  SQLINTEGER colcount;
  SQLRETURN result = SQLColAttribute(mSqlStatementHandle, 1, SQL_DESC_COUNT, 0, 0, 0, &colcount);
  if (result == SQL_SUCCESS || result == SQL_SUCCESS_WITH_INFO)
  {
    return colcount;
  }
  else
  {
    SQLTCHAR sql_state[7];
    SQLTCHAR message_text[1024];
    GetDiag(sql_state,message_text,1024);
    TCHAR szMsg[1200];
    _stprintf(szMsg, _T( "OdbcStatement::GetColCount() - SQLColAttribute failed.\n%s"),message_text);
    throw szMsg;
  }
}
int OdbcStatement::GetRowCount()
{
  SQLINTEGER rowcount;
  SQLRETURN result = SQLRowCount(mSqlStatementHandle, &rowcount);
  if (result == SQL_SUCCESS || result == SQL_SUCCESS_WITH_INFO)
  {
    return rowcount;
  }
  else
  {
    SQLTCHAR sql_state[7];
    SQLTCHAR message_text[1024];
    GetDiag(sql_state,message_text,1024);
    TCHAR szMsg[1200];
    _stprintf(szMsg, _T( "OdbcStatement::GetRowCount() - SQLRowCount failed.\n%s"),message_text);
    throw szMsg;
  }
}
int OdbcStatement::GetData(unsigned int column, unsigned int targetType, void* targetBuf, unsigned int targetBufLen)
{
  SQLINTEGER total_written;
  
  SQLRETURN result = GetData(column, targetType, targetBuf, targetBufLen, &total_written);
  if (result == SQL_SUCCESS || result == SQL_SUCCESS_WITH_INFO)
  {
    return total_written;
  }
  else
  {
    SQLTCHAR sql_state[7];
    SQLTCHAR message_text[1024];
    GetDiag(sql_state,message_text,1024);
    TCHAR szMsg[1200];
    _stprintf(szMsg, _T( "OdbcStatement::Fetch() - SQLFetch failed.\n%s"),message_text);
    throw szMsg;
  }
}

SQLRETURN OdbcStatement::GetData(unsigned int column, unsigned int targetType, void* targetBuf, unsigned int targetBufLen, SQLINTEGER* lenOrInd)
{
  SQLRETURN result = SQLGetData(mSqlStatementHandle, column, targetType, targetBuf, targetBufLen, lenOrInd);
  if (result != SQL_SUCCESS && result != SQL_SUCCESS_WITH_INFO && result != SQL_NO_DATA)
  {
    SQLTCHAR sql_state[7];
    SQLTCHAR message_text[1024];
    GetDiag(sql_state,message_text,1024);
    TCHAR szMsg[1200];
    _stprintf(szMsg, _T( "OdbcStatement::GetData(unsigned int column, unsigned int targetType, void* targetBuf, unsigned int targetBufLen, SQLINTEGER* lenOrInd) - SQLGetData failed.\n%s"),message_text);
    throw szMsg;
  }
  return result;
}

bool OdbcStatement::GetString(unsigned int column, TCHAR* dest)
{
  SQLRETURN result;
  *dest = 0;
  do
  {
    SQLINTEGER total_written;
    TCHAR buffer[1001];
    buffer[0] = 0;                                            /* V - Yes it should be a subtraction and not a division. It's in bytes */
    result = (SQLRETURN)GetData(column, (unsigned int)SQL_C_TCHAR, buffer, sizeof(buffer) - sizeof(*buffer), &total_written);
    buffer[sizeof(buffer) / sizeof(*buffer) - 1] = 0;
    
    if (result != SQL_SUCCESS && result != SQL_SUCCESS_WITH_INFO && result != SQL_NO_DATA) 
    {
      throw _T("OdbcStatement::GetString - Failed");
    }
    
    _tcscpy( dest + _tcslen(dest), buffer );
  } while (result != SQL_NO_DATA);
  
  return true;
}

bool OdbcStatement::GetLong(unsigned int column, long* dest)
{
  long val;
  if ((unsigned int)GetData(column, (unsigned int)SQL_C_SLONG, &val, sizeof(val)) == sizeof(val))
  {
    *dest = val;
    return true;
  }
  else
  {
    return false;
  }
}

long OdbcStatement::GetLong(unsigned int column)
{
  long val = 0;
  GetLong(column, &val);
  return val;
}

bool OdbcStatement::GetDouble(unsigned int column, double* dest)
{
  double val;
  if (GetData(column, SQL_C_DOUBLE, &val, sizeof(val)) == sizeof(val))
  {
    *dest = val;
    return true;
  }
  else
  {
    return false;
  }
}

double OdbcStatement::GetDouble(unsigned int column)
{
  double val = 0;
  GetDouble(column, &val);
  return val;
}


bool OdbcStatement::IsValid()
{
  return mValid;
}


/**
* @author hm@cim.as
*
* Closes the current cursor if any. A cursor is opened by the Execute or
* Query functions. After a close command the OdbcStatment object can be
* reused for new Queryes.
*/
bool OdbcStatement::CloseCursor()
{
  if( mSqlStatementHandle == SQL_NULL_HSTMT )
    return false;
  
  SQLRETURN result = SQLFreeStmt( mSqlStatementHandle, SQL_CLOSE );
  if (result == SQL_ERROR || result == SQL_SUCCESS_WITH_INFO)
  {
    SQLTCHAR sql_state[7];
    SQLTCHAR message_text[1024];
    GetDiag(sql_state,message_text,1024);
    TCHAR szMsg[1200];
    _stprintf(szMsg, _T( "OdbcStatement::CloseCursor() - SQLFreeStmt failed.\n%s"),message_text);
    throw szMsg;
  }
  
  return result == SQL_SUCCESS || result == SQL_SUCCESS_WITH_INFO || result == SQL_NO_DATA;
}

void OdbcStatement::GetDiag(SQLTCHAR*  szState, SQLTCHAR* szMessage, SQLSMALLINT messageLen)
{
  szMessage[0] = 0;
  szState[0] = 0;
  SQLINTEGER native_error;

  //            SQLRETURN diag_result = SQLGetDiagRec(SQL_HANDLE_STMT, 
  SQLGetDiagRec(  SQL_HANDLE_STMT, 
                  mSqlStatementHandle,
                  1,
                  szState,
                  &native_error,
                  szMessage,
                  messageLen,
                  &messageLen);
}

#ifndef _MPCUNICODE
#undef _UNICODE
#endif

#ifndef MPCUNICODE
#undef UNICODE
#endif

#endif // __PC__


