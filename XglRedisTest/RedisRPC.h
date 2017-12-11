#pragma once

//#ifdef _WIN32
//#include <process.h>
//#define sleep(x) Sleep(x)
//#else
//#include <unistd.h>
//#define sleep(x) usleep((x)*1000)
//#endif

#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include <signal.h> 
#ifdef _WIN32
#include <Win32_Interop/win32fixes.h>
#include "common_tool.h"
extern "C"
{
#include <hiredis/adapters/ae.h>
}
#else
#include "common_tool.h"
#include <hiredis/adapters/libevent.h>
#endif

//#include "easylogging++.h"

//#define INFOLOG LOG(INFO)<<__FUNCTION__<<"***"
//#define TRACELOG LOG(TRACE)<<__FUNCTION__<<"***"
//#define WARNLOG LOG(WARNING)<<__FUNCTION__<<"***"
//#define DEBUGLOG LOG(DEBUG)<<__FUNCTION__<<"***"
//#define ERRORLOG LOG(ERROR)<<__FUNCTION__<<"***"

//#define INFOLOG cout<<endl<<time2str(time(NULL))<<"[info]"<<__FUNCTION__<<"***"
//#define TRACELOG cout<<endl<<time2str(time(NULL))<<"[trace]"<<__FUNCTION__<<"***"
//#define WARNLOG cout<<endl<<time2str(time(NULL))<<"[warn]"<<__FUNCTION__<<"***"
//#define DEBUGLOG cout<<endl<<time2str(time(NULL))<<"[debug]"<<__FUNCTION__<<"***"
//#define ERRORLOG cout<<endl<<time2str(time(NULL))<<"[error]"<<__FUNCTION__<<"***"
//#define FATALLOG cout<<endl<<time2str(time(NULL))<<"[fatal]"<<__FUNCTION__<<"***"

#define INFOLOG(inf) LOG4CPLUS_INFO(g_ECGLogger->logger, "[" << __FUNCTION__ << "] " << inf)
#define TRACELOG(inf) LOG4CPLUS_TRACE(g_ECGLogger->logger, "[" << __FUNCTION__ << "] " << inf)
#define DEBUGLOG(inf) LOG4CPLUS_DEBUG(g_ECGLogger->logger, "[" << __FUNCTION__ << "] " << inf)
#define WARNLOG(inf) LOG4CPLUS_WARN(g_ECGLogger->logger, "[" << __FUNCTION__ << "] " << inf)
#define ERRORLOG(inf) LOG4CPLUS_ERROR(g_ECGLogger->logger, "[" << __FUNCTION__ << "] " << inf)
#define FATALLOG(inf) LOG4CPLUS_FATAL(g_ECGLogger->logger, "[" << __FUNCTION__ << "] " << inf)

#define REDIS_BUF_SIZE 1024 * 2

//redis命令
#define R_GET		"GET"
#define R_SET		"SET"
#define R_PUSH		"LPUSH"
#define R_POP		"RPOP"
#define R_UNSUBS	"UNSUBSCRIBE"
#define R_SUBS		"SUBSCRIBE"
#define R_PSUBS		"PSUBSCRIBE"
#define R_RENAME	"RENAME"
#define R_EXISTS	"EXISTS"
#define R_DEL		"DEL"
#define R_KEYSPACE	"__keyspace@0__:"

//RPC list
#define HEARTLIST	"_heart_beart_list"
#define REQLIST		"_request_list"

#define HEARTBEATTIMEOUT	10		//10s不更新heartbeat
#define HEARTBEATINTERVAL	3


typedef std::map<std::string, std::string>		mapReqchnl;	//getkey-->requestChnl
typedef std::map<std::string, std::string>		mapHBchnl;	//getkey-->HeartBeatChnl

class CRedisRPC
{
public:
	CRedisRPC();
	CRedisRPC(std::string lpStrIp, int iPort);
	~CRedisRPC();

	bool connect(const char* lpStrIp, int iPort);		//连接redis服务
	bool isServiceModelAvailable(const char *lpStrKey);					//检查是否有可用服务
	bool isKeySubs(const char *lpStrKey);			//检查key是否需要处理

	void processKey(const char *lpStrKey, int iTimeout, mutex &processKeyLock);			//处理key, timeout--ms

	//业务处理模块
	void subsClientGetOp(const char *lpStrKey, const char *reqChlName,
		const char *heartbeatChnName);		//注册key以及回调函数
	std::string unsubClientGetOp(const char *keys);
	void clearChnl();
private:
	static void* thTimeout(void *arg);
	static void* thSetHeartBeat(void *arg);
	static std::mutex hbLock;
	mapReqchnl m_mapReqChnl;
	mapHBchnl  m_mapHBChnl;

	std::string m_strIp;				//redis ip
	int m_iPort;					//redis 端口
	std::string m_strRequestID;
	redisContext *m_redisContext;

	static bool m_bKeyProcessDone;
};

