#ifndef _LOGGER__
#define _LOGGER__
#include <mutex>
#include <fstream>
#include <stdarg.h>
#include <string>



enum LogLevel
{
	LOGL_INFOR = 1,    // 普通信息，不宜太多
	LOGL_DEBUG = 2 ,   // 调试信息，可以把详细参数都输出。
	LOGL_WARN  = 4 ,   // 警告信息，不能确认数据是否正确，用这类输出。比如连接不上。
	LOGL_ERROR = 8,    // 代码中确认出错才会进入。比如入参检测不对，比如内存不足，比如系统应该具备的服务无法调用等等。
};

#define LogLevel_ALL (0xFFFFFFFF)


class CLogger
{
public:

	CLogger( );


	// 设置文件名（如果文件名为空，则输出到屏幕。默认情况输出到屏幕）
	bool SetFileName( const char * lpFileName );

	
	~CLogger( );

	// 无格式化，直接输出一句日志，效率较高。
	void Log0( LogLevel lvl , const char * lpModuleName , const char * lpText );
	void Log0( LogLevel lvl , const char * lpModuleName , const std::string & strText );
	// 有格式化，输出效率较低。
	void Log( LogLevel lvl , const char * lpModuleName , const char * lpFormatText , ... );
	//   (对可变参数的处理)有格式化，输出效率较低。
	void Log( LogLevel lvl , const char * lpModuleName , const char * lpFormatText , va_list & vaList );

	// 关闭日志输出
	void LogOff( );
	// 开启日志输出
	void LogOn( );
	// 日志是否开启
	bool IsLogOn( ) const;

	// 获取当前日志级别
	unsigned int GetCurrentLogLevel( )const;
	// 设置日志级别
	void SetCurrentLogLevel( unsigned int uiLogLevel );


	
protected:	



	std::unique_lock<std::mutex> _LockAsUnique( );
	// 是否输出本条日志信息
	bool _LogLvlAvailable( LogLevel lvl );
	// 打印当前时间，以及线程ID
	void _PrintfTimer( LogLevel lvl , const char * lpModuleName , std::ostream & outputStream );

	char m_buf [ 1536 ];
	int m_iSize;

	// 日志级别开关
	unsigned int m_uiLogLevel;
	// 是否开日志
	bool m_bShowLog;
	// 操作锁
	std::mutex m_mtx;

	// 输出日志文件
	std::ofstream * m_ospOutput;

	// 日志、单体
	static CLogger * m_pInstance;
};


extern CLogger * g_pLogger;


//////////////////////////////////////////////////////////////////////////



// 初始化日志、或者修改日志参数。
// 如果参数：lpFileName 是 NULL-> 输出到屏幕；是 "" 空字符串，保持原先输出不变（如果先前没有Initial过，默认输出到屏幕）；如果是有效字符串->日志输出到该字符串命名的文件。
// lvl 可以改变当前日志系统的日志级别。日志级别是一个“或”的关系，只有当对应级别开启，相应日志才会被输出。
void InitialLog( const char * lpFileName = "", unsigned int lvl = ( unsigned int )LogLevel_ALL );

// 删除、关闭日志
void DeleteAndCloseLog( );

// 临时打开、关闭日志
void LogOnOff( bool bOn );

// 判断日志当前是否开启
bool IsLogOn( );

// 全局日志操作函数
// 无格式化，直接输出一句日志，效率较高。
void  MY_Log0( LogLevel lvl , const char * lpModuleName , const char * lpText );
void  MY_Log0( LogLevel lvl , const char * lpModuleName , const std::string &  strText );

// 有格式化，输出效率较低。
void MY_Log( LogLevel lvl , const char *lpModuleName , const char * lpFormatText , ... );
	
// 给外面提供可变参数包装的函数。
void MY_Log_Variable_Num( LogLevel lvl , const char *lpModuleName , const char * lpFormatText , va_list & vaList );

#endif




