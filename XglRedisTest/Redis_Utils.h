#pragma once

#include "RedisRPC.h"

//#ifdef _WIN32
//#include <WinSock2.h>
//#else
//#include <sys/time.h>
//#endif // _WIN32


typedef void(*subsCallback)(const char *lpStrKey, const char *lpStrValue);
typedef void(*pullCallback)(const char *lpStrKey, const char *lpStrValue);
typedef void(*clientOpCallBack)(const char *lpStrKey, const char *lpStrValue);

typedef std::map<std::string, subsCallback> mapSubsCB;		//subkey-->subfunc
typedef std::map<std::string, pullCallback> mapPullCB;		//pullkey-->subfunc
typedef std::map<std::string, clientOpCallBack> mapReqCB;	//getkey-->getfunc

class CRedis_Utils
{
public:
	CRedis_Utils(std::string lpStrClientID);
	~CRedis_Utils();

	//客户端基本操作函数
	/*
	isSubs说明：默认false
		false：不使用redis的键空间通知功能，即subs、pull、subsClientGetOp等接口不起作用
		true：启用redis的键空间通知功能
	*/
	bool connect(const char* lpStrIp, int iPort, bool bNeedSubs = false);		//连接redis服务并订阅redis键空间通知
	void disconnect();

	//redis基本操作
	/**
		get()    pop()
		返回值说明：
			-1：操作失败，失败原因通过lpStrRlt查看
			 0：lpStrKey对应的值为空
			>0：操作成功，lpStrKey的值通过lpStrRlt查看，返回值的长度

		set()    push()
		返回值说明：
			true：操作成功，结果通过lpStrRlt查看，一般为OK
			false：操作失败，结果通过lpStrRlt查看
	  */ 
	int  get(const char* lpStrKey, char *lpStrRlt);		
	bool set(const char* lpStrKey, const char* lpStrValue, char *lpStrRlt);
	bool push(const char* lpStrListName, const char* lpStrValue, char *lpStrRlt);
	int  pop(const char* lpStrListName, char *lpStrRlt);

	//redis订阅功能
	void subs(const char *lpStrKey, subsCallback cb);	//subscribe channel
	void unsubs(const char *lpStrKey);					//unsubscribe channel
	void pull(const char *lpStrKey, pullCallback cb);	//pull list
	void unpull(const char *lpStrKey);					//unpull list, like unsubs

	//业务处理模块
	void subsClientGetOpSimple(const char *lpStrKey, clientOpCallBack cb);
	void subsClientGetOp(const char *lpStrKey, const char *reqChlName,
		const char *heartbeatChnName, clientOpCallBack cb);				//注册监听客户端get key操作
	void unsubClientGetOp(const char *lpStrKey);						//注销监听客户端get key操作
	void stopSubClientGetOp();											//取消监听客户端全部get操作

private:
	void close();
	//实现业务模块的数据隔离
	std::string genNewKey(std::string lpStrKeyOldKey);					//封装用户传入的key				
	std::string getOldKey(std::string lpStrKeyNewKey);					//获取客户的原始key
	bool sendCmd(const char *lpStrCmd, char *lpStrRlt);					//发送redis命令
	bool replyCheck(redisReply *rRedisReply, char *lpStrReply);		//解析redis应答消息

	static void* thAsyncSubsAll(void *arg);						//redis键空间通知

	static void connectCallback(const redisAsyncContext *c, int iStatus);	//redis异步回调函数
	static void disconnectCallback(const redisAsyncContext *c, int iStatus);
	static void subsAllCallback(redisAsyncContext *c, void *r, void *data);

	void callSubsCB(const char *lpStrKey, const char *lpsStrKeyOp);
	
	redisContext *m_pRedisContext;			//redis同步上下文
	redisAsyncContext *m_pRedisAsyncContext;	//redis异步上下文
	
#ifdef _WIN32				//redis异步事件库
	aeEventLoop *m_loop;
#else
	struct event_base *m_base;
#endif

	mapSubsCB m_mapSubsKeys;		//订阅信息  键-->回调函数
	mapPullCB m_mapPullKeys;		//pull信息  键-->回调函数
	mapReqCB  m_mapReqChnl;			//请求队列	队列名-->回调函数

	mutex m_getLock;				//多线程同步锁
	mutex m_setLock;
	mutex m_pushLock;
	mutex m_popLock;
	mutex m_subsLock;
	mutex m_pullLock;
	mutex m_reqLock;

	std::string m_lpStrIp;				//redis ip
	int m_iPort;					//redis 端口

	CRedisRPC m_redisRPC;			//redis RPC调用实现

	bool m_bNeedSubs;				//是否需要redis订阅功能
	bool m_bIsConnected;			//连接redis成功标志
	std::string m_lpStrClientId;		//客户端标志
};

