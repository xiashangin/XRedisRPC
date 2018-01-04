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
#include "CommonTools.h"
extern "C"
{
#include <hiredis/adapters/ae.h>
}
#else
#include "CommonTools.h"
#include <hiredis/adapters/libevent.h>
#endif


#define _INFOLOG(inf) \
				*g_ECGLogger->m_globalLogOss << inf;\
				MY_Log0(LOGL_INFOR, __FUNCTION__, g_ECGLogger->m_globalLogOss->str().c_str());\
				g_ECGLogger->m_globalLogOss->str("");

#define _DEBUGLOG(inf) \
				*g_ECGLogger->m_globalLogOss << inf;\
				MY_Log0(LOGL_DEBUG, __FUNCTION__, g_ECGLogger->m_globalLogOss->str().c_str());\
				g_ECGLogger->m_globalLogOss->str("");

#define _WARNLOG(inf) \
				*g_ECGLogger->m_globalLogOss << inf;\
				MY_Log0(LOGL_WARN, __FUNCTION__, g_ECGLogger->m_globalLogOss->str().c_str());\
				g_ECGLogger->m_globalLogOss->str("");

#define _ERRORLOG(inf) \
				*g_ECGLogger->m_globalLogOss << inf;\
				MY_Log0(LOGL_ERROR, __FUNCTION__, g_ECGLogger->m_globalLogOss->str().c_str());\
				g_ECGLogger->m_globalLogOss->str("");

//#define _INFOLOG(inf) LOG4CPLUS_INFO(g_ECGLogger->logger, "[" << __FUNCTION__ << "] " << inf)
//#define _TRACELOG(inf) LOG4CPLUS_TRACE(g_ECGLogger->logger, "[" << __FUNCTION__ << "] " << inf)
//#define _DEBUGLOG(inf) LOG4CPLUS_DEBUG(g_ECGLogger->logger, "[" << __FUNCTION__ << "] " << inf)
//#define _WARNLOG(inf) LOG4CPLUS_WARN(g_ECGLogger->logger, "[" << __FUNCTION__ << "] " << inf)
//#define _ERRORLOG(inf) LOG4CPLUS_ERROR(g_ECGLogger->logger, "[" << __FUNCTION__ << "] " << inf)
//#define _FATALLOG(inf) LOG4CPLUS_FATAL(g_ECGLogger->logger, "[" << __FUNCTION__ << "] " << inf)

#define REDIS_BUF_SIZE 1024 * 2

//redis命令
#define R_GET		"GET"				//GET key
#define R_SET		"SET"				//SET key value
#define R_PUSH		"LPUSH"				//SET key value
#define R_POP		"RPOP"				//RPOP key
#define R_LLEN		"LLEN"				//LLEN key
#define R_UNSUBS	"UNSUBSCRIBE"		//UNSUBSCRIBE channel
#define R_SUBS		"SUBSCRIBE"			//SUBSCRIBE channel
#define R_PSUBS		"PSUBSCRIBE"		//PSUBSCRIBE pattern
#define R_RENAME	"RENAME"			//RENAME key newkey
#define R_EXISTS	"EXISTS"			//EXISTS key
#define R_DEL		"DEL"				//DEL key
#define R_HSET		"HSET"				//HSET key field value
#define R_HGET		"HGET"				//HGET key field
#define R_HGETALL 	"HGETALL"			//HGETALL key
#define R_HDEL		"HDEL"				//HDEL key field
#define R_SISMEMBER	"SISMEMBER"			//SISMEMBER key member
#define R_KEYSPACE	"__keyspace@0__:"

//RPC list
#define HEARTSLOT			"_heart_beart_slot"			//心跳信令
#define REQPROCESSING		"_processing?"				//哨兵
#define REQSLOT				"_request_slot"				//请求槽
#define GLOBALREQKEYS		"__GLOBALREQKEYS__"			//请求队列
#define GLOBALREQKEYSREF	"__GLOBALREQKEYSREF__"		//引用计数

#define HEARTBEATTIMEOUT	10		//10s不更新heartbeat
#define HEARTBEATINTERVAL	3
#define GET_WAITTIMEOUT		10000

#define REDIS_TIMEOUT			100		//业务模块处理超时
#define REDIS_NOSERVICE			101		//无业务处理模块
#define REDIS_SENDREQFAIL		102		//发送请求失败
#define REDIS_CONNFAIL			103		//redis服务连接失败
#define REDIS_CMD_ERR			105		//发送命令失败
#define REDIS_KEY_NULL			104		//输入的key值为空
#define REDIS_VALUE_NULL		105		//输入的value值为空
#define REDIS_KEY_EXISTED		106		//key已被订阅
#define REDIS_SUBS_OFF			107		//未开启redis键空间通知功能
#define REDIS_REQ_SYNC_FAIL		108		//请求同步失败
#define	REDIS_KEY_NOT_EXIST		109		//get或pop的key不存在
#define REDIS_SERVICE_BUSY		110		//业务模块繁忙

typedef std::map<std::string, std::string>		mapReqchnl;	//getkey-->requestChnl
typedef std::map<std::string, std::string>		mapHBchnl;	//getkey-->HeartBeatChnl

class CRedisRPC
{
public:
	CRedisRPC();
	//CRedisRPC(std::string lpStrIp, int iPort);
	~CRedisRPC();

	bool connect(const char* lpStrIp, int iPort);		//连接redis服务
	bool isServiceModelAvailable(const char *lpStrKey);					//检查是否有可用服务
	bool isKeySubs(const char *lpStrKey);			//检查key是否需要处理

	int processKey(const char *lpStrKey);			//处理key, timeout--ms
	void syncReqChnl(const char *lpStrKey, const char *reqChlName, const char *heartBeatName);
	
	//业务处理模块
	void subsClientGetOp(const char *lpStrKey, const char *reqChlName,
		const char *heartbeatChnName);		//注册key以及回调函数
	std::string unsubClientGetOp(const char *keys);
	
	std::vector<std::string> clearChnl();
	void setClientId(const std::string & strClientId);
	void setRedisAddr(const std::string & strIp, const int iPort);
private:
	bool sendReq(redisContext *c, std::string strReqCmd, std::string strProcesssCmd);
	static void* thTimeout(void *arg);
	static void* thSetHeartBeat(void *arg);
	std::mutex hbLock;
	std::thread thHeartBeat;
	mapReqchnl m_mapReqChnl;
	mapHBchnl  m_mapHBChnl;

	std::string m_strIp;			//redis ip
	int m_iPort;					//redis 端口
	std::string m_strRequestID;
	redisContext *m_redisContext;
	std::string m_strClientId;
	std::string m_strCurrentProcessKey;

	bool m_bKeyProcessDone;
};

