#pragma once

#include "RedisRPC.h"
#include <functional>
#define DEFAULT_CLIENTID		"__default__"
#define COMMAND_SPLIT			"&&&_&&&"
#define SET_KEY_SUFFIX			"__SETFLAG__"
#define SET_KEY_TIMESTAMP		"LASTREQ+"
#define SETQUEUEMAXSIZE			3

typedef std::function<void(const std::string & , const std::string & )> subsCallback;
typedef std::function<void(const std::string & , const std::string &)> pullCallback;
typedef std::function<void(const std::string & , const std::string &)> clientOpCallBack;

typedef std::map<std::string, subsCallback> mapSubsCB;		//subkey-->subfunc
typedef std::map<std::string, pullCallback> mapPullCB;		//pullkey-->subfunc
typedef std::map<std::string, clientOpCallBack> mapReqCB;	//getkey-->getfunc

class CRedis_Utils
{
public:
	//如果传入的strClientID是空字符串，那么此字段会有一个默认值：__default__
	CRedis_Utils(const std::string & strClientID);
	~CRedis_Utils();

	//客户端基本操作函数
	/*
	isSubs说明：默认false
		false：不使用redis的键空间通知功能，即subs、pull、subsClientGetOp等接口不起作用，只能进行redis基本操作
		true：启用redis的键空间通知功能
		两个问题无法保证：
			1.  无法检测redis服务是否已经开启键空间通知功能
			2.  无法保证订阅redis键空间命令成功执行（因为此方式使用的是异步接口，只能判断名称进入执行队列了）
				即使此异步接口返回错误，无法将此错误返回给上层。但是程序仍然能正常运行，但是和订阅相关的功能全部无效。
				目前采用重发机制缓解以上问题。
	*/
	bool connect(const std::string & strIp, int iPort, bool bNeedSubs = false);		//连接redis服务并订阅redis键空间通知
	void disconnect();

	//redis基本操作
	/**
		返回值说明：
			0：操作成功
			>0：操作失败，返回状态码
		操作结果通过strOutResult获取
	*/ 
	int get(const std::string & strInKey, std::string & strOutResult);
	int set(const std::string & strInKey, const std::string & strInValue, std::string & strOutResult);
	int push(const std::string & strInListName, const std::string & strInValue, std::string & strOutResult);
	int pop(const std::string & strInListName, std::string & strOutResult);

	//redis订阅功能
	/*
	subs()	pull()
	返回值说明：
		0：操作成功
		>0：操作失败，返回状态码

	subs和pull比较：
	1. subs订阅的是字符串的变化，pull订阅的是list的变化。
	2. 当set操作发生时，subs回调会收到被set的key和value。
	   del操作发生时，subs回调会收到被删除的key，此时value为空字符串。
	3. 当push操作发生时，pull回调会收到被push的listName和value。pop操作发生时，pull回调不会收到消息。
	   del操作发生时，pull回调会收到被删除的listName，此时value为空字符串。

	unsubs()	unpull()
	返回值说明：
		true：操作成功
		false：操作失败
	*/
	int subs(const std::string & strInKey, subsCallback cb);	//subscribe channel
	bool unsubs(const std::string & strInKey);					//unsubscribe channel
	int pull(const std::string & strInKey, pullCallback cb);	//pull list
	bool unpull(const std::string & strInKey);					//unpull list, like unsubs

	//业务处理模块
	/*
	subsClientGetOp()
	返回值说明：
		0：操作成功
		>0：操作失败，返回状态码
	
	notifyRlt()
	返回值说明：
		0：操作成功
		>0：操作失败，返回状态码
	作用：处理完成请求之后通过此函数通知客户端
	*/
	int subsClientGetOp(const std::string & strInKey, clientOpCallBack cb);
	bool unsubClientGetOp(const std::string & strInKey);							//注销监听客户端get key操作
	void stopSubClientGetOp();														//取消监听客户端全部get操作
	int notifyRlt(const std::string & strInKey, const std::string & strInValue);	//通知客户端处理完成

	void setClientId(const std::string & strClientId);

	int getAvgOpTime();
private:
	CRedis_Utils(const CRedis_Utils & c);	//不允许拷贝
	
	void close();
	bool _connect(const std::string & strIp, int iPort);
	int updateReqHB(const std::string & strInKey, bool bType);				//true:set请求，false:处理请求
	//实现业务模块的数据隔离
	std::string genNewKey(const std::string & lpStrOldKey);					//封装用户传入的key				
	std::string getOldKey(const std::string & lpStrNewKey);					//获取客户的原始key
	
	std::map<std::string, int> getAllReqs(const std::string strReqList);
	int syncReq(const std::string & strInKey, const std::string & strInReq, bool syncType, std::string & strOutResult);	//syncType:true-->加refCnt false-->减refCnt
	int getRefCnt(const std::string & strInField, int & refCnt);
	bool isReqExist(const std::string & strInKey, const std::string & strInReq, std::string & strOutResult);
	bool sendCmd(const std::string & strInCmd, std::string & strOutResult);		//发送redis命令
	bool replyCheck(redisReply *rRedisReply, std::string & strOutResult);		//解析redis应答消息

	static void* thAsyncSubsAll(void *arg);							//redis键空间通知

	static void connectCallback(const redisAsyncContext *c, int iStatus);	//redis异步回调函数
	static void disconnectCallback(const redisAsyncContext *c, int iStatus);
	static void subsAllCallback(redisAsyncContext *c, void *r, void *data);
	bool getReq(std::string strInkey);

	void callSubsCB(const std::string & strInKey, const std::string & strInOp);

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
	std::vector<int> hiredisOneOpTime;
	std::string m_strIp;				//redis ip
	int m_iPort;						//redis 端口

	CRedisRPC m_redisRPC;				//redis RPC调用实现

	bool m_bNeedSubs;					//是否需要redis订阅功能
	bool m_bIsConnected;				//连接redis成功标志
	std::string m_strClientId;			//客户端标志
	//std::string m_strLastGetKey;
};

