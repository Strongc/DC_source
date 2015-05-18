#ifndef gfFramework_OdbcInterface_h
#define gfFramework_OdbcInterface_h
#pragma warning(disable:4786)
#ifdef __PC__


#ifdef _UNICODE
#define _MPCUNICODE
#else
#define _UNICODE
#endif

#ifdef UNICODE
#define MPCUNICODE
#else
#define UNICODE
#endif


#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#undef SQL_NOUNICODEMAP

#include <windows.h>
#include <TCHAR.h>


#define SQLHDBC void*
#define SQLHENV void*
#define SQLHSTMT void*
#define SQLRETURN short
#define SQLSMALLINT short
#define SQLINTEGER long
#define SQLTCHAR wchar_t

class OdbcConnection
{
public:
  OdbcConnection(TCHAR* dataSourceName);
  virtual ~OdbcConnection();
  
  virtual bool IsValid();
  
  virtual SQLHDBC GetConnectionHandle();
  virtual void BeginTrans();
  virtual void CommitTrans();
  virtual void RollbackTrans();
  virtual void SetAutoCommit( bool enable = true );
protected:
  
private:
  bool mValid;
  bool mAutoCommit;
  SQLHENV mSqlEnvHandle;
  SQLHDBC mSqlDbConnectionHandle;
  
  OdbcConnection& operator=(const OdbcConnection& src);
  OdbcConnection(const OdbcConnection& src);
};

class OdbcStatement
{
public:
  OdbcStatement(OdbcConnection* pConnection);
  virtual ~OdbcStatement();
  
  virtual bool IsValid();
  
  virtual bool ExecDirect(TCHAR* statement);
  virtual bool Fetch();
  virtual int GetColCount();
  virtual int GetRowCount();
  virtual int GetData(unsigned int column, unsigned int targetType, void* targetBuf, unsigned int targetBufLen);
  virtual SQLRETURN GetData(unsigned int column, unsigned int targetType, void* targetBuf, unsigned int targetBufLen, SQLINTEGER* lenOrInd);
  
  virtual bool GetString(unsigned int column, TCHAR* dest);
  
  virtual bool GetLong(unsigned int column, long* dest);
  virtual long GetLong(unsigned int column);
  
  virtual bool GetDouble(unsigned int column, double* dest);
  virtual double GetDouble(unsigned int column);
  
  
  virtual bool CloseCursor();
  virtual void GetDiag(SQLTCHAR*  szState, SQLTCHAR* szMessage, SQLSMALLINT messageLen);
  
protected:
  
private:
  bool mValid;
  
  OdbcConnection* mpConnection;
  SQLHSTMT mSqlStatementHandle;
  
  OdbcStatement& operator=(const OdbcStatement& src);
  OdbcStatement(const OdbcStatement& src);
};
#ifndef _MPCUNICODE
#undef _UNICODE
#endif

#ifndef MPCUNICODE
#undef UNICODE
#endif
#endif // __PC__
#pragma warning( default : 4786 )
#endif // gfFramework_OdbcInterface_h
