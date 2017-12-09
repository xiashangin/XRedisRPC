#pragma once

#include "RedisRPC.h"

//#ifdef _WIN32
//#include <WinSock2.h>
//#else
//#include <sys/time.h>
//#endif // _WIN32


typedef void(*subsCallback)(const std::string & strKey, const std::string & strValue);
typedef void(*pullCallback)(const std::string & strKey, const std::string & strValue);
typedef void(*clientOpCallBack)(const std::string & strKey, const std::string & strValue);

typedef std::map<std::string, subsCallback> mapSubsCB;		//subkey-->subfunc
typedef std::map<std::string, pullCallback> mapPullCB;		//pullkey-->subfunc
typedef std::map<std::string, clientOpCallBack> mapReqCB;	//getkey-->getfunc

class CRedis_Utils
{
public:
	CRedis_Utils(std::string strClientID);
	~CRedis_Utils();

	//客户端基本操作函数
	/*
	isSubs说明：默认false
		false：不使用redis的键空间通知功能，即subs、pull、subsClientGetOp等接口不起作用
		true：启用redis的键空间通知功能
	*/
	bool connect(const std::string & strIp, int iPort, bool bNeedSubs = false);		//连接redis服务并订阅redis键空间通知
	void disconnect();

	//redis基本操作
	/**
		get()    pop()
		返回值说明：
			-1：操作失败，失败原因通过lpStrRlt查看
			 0：lpStrKey对应的值为空
			>0：操作成功，lpStrKey的值通过lpStrRlt查看，返回lpStrRlt的长度

		set()    push()
		返回值说明：
			true：操作成功，结果通过lpStrRlt查看，一般为OK
			false：操作失败，结果通过lpStrRlt查看
	  */ 
	int  get(const std::string & strInKey, std::string & strOutResult);
	bool set(const std::string & strInKey, const std::string & strInValue, std::string & strOutResult);
	bool push(const std::string & strInListName, const std::string & strInValue, std::string & strOutResult);
	int  pop(const std::string & strInListName, std::string & strOutResult);

	//redis订阅功能
	void subs(const std::string & strInKey, subsCallback cb);	//subscribe channel
	void unsubs(const std::string & strInKey);					//unsubscribe channel
	void pull(const std::string & strInKey, pullCallback cb);	//pull list
	void unpull(const std::string & strInKey);					//unpull list, like unsubs

	//业务处理模块
	void subsClientGetOp(const std::string & strInKey, clientOpCallBack cb);
	void unsubClientGetOp(const std::string & strInKey);						//注销监听客户端get key操作
	void stopSubClientGetOp();											//取消监听客户端全部get操作

private:
	void close();
	bool _connect(const std::string & strIp, int iPort);
	//实现业务模块的数据隔离
	std::string genNewKey(const std::string & lpStrOldKey);					//封装用户传入的key				
	std::string getOldKey(const std::string & lpStrNewKey);					//获取客户的原始key
	bool sendCmd(const std::string & strInCmd, std::string & strOutResult);		//发送redis命令
	bool replyCheck(redisReply *rRedisReply, std::string & strOutResult);		//解析redis应答消息

	static void* thAsyncSubsAll(void *arg);							//redis键空间通知

	static void connectCallback(const redisAsyncContext *c, int iStatus);	//redis异步回调函数
	static void disconnectCallback(const redisAsyncContext *c, int iStatus);
	static void subsAllCallback(redisAsyncContext *c, void *r, void *data);

	void callSubsCB(const std::string & strInKey, const std::string & strInOp);
	
	CRedis_Utils(const CRedis_Utils & c);

	redisContext *m_pRedisContext;			//redis同步上下文
	//std::shared_ptr<redisContext *> m_spRedisContext;
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
	mutex m_aeStopLock;

	std::thread thAsyncKeyNotify;

	std::string m_strIp;				//redis ip
	int m_iPort;						//redis 端口

	CRedisRPC m_redisRPC;				//redis RPC调用实现

	bool m_bNeedSubs;					//是否需要redis订阅功能
	bool m_bIsConnected;				//连接redis成功标志
	std::string m_strClientId;			//客户端标志
};

