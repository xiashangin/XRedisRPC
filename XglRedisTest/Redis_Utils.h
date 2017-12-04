#pragma once

#include "RedisRPC.h"

#ifdef _WIN32
#include <WinSock2.h>
#else
#include <sys/time.h>
#endif // _WIN32


typedef void(*subsCallback)(const char *key, const char *value);
typedef void(*pullCallback)(const char *key, const char *value);

typedef std::map<std::string, subsCallback> mapSubsCB;	//subkey-->subfunc
typedef std::map<std::string, pullCallback> mapPullCB;	//pullkey-->subfunc

class CRedis_Utils
{
public:
	CRedis_Utils(std::string clientID);
	~CRedis_Utils();

	/////考虑将返回值改为sRlt字符串的长度/////
	//客户端基本操作函数
	/*
	isSubs说明：默认false
		false：不使用redis的键空间功能，即subs、pull、subsClientGetOp等接口不起作用
		true：启用redis的键空间功能
	*/
	bool connect(const char* ip, int port, bool isSubs = false);		//连接redis服务并订阅redis键空间通知
	void disconnect();

	int  get(const char* _key, char *sRlt);		//-1失败  0 key无对应的值
	bool set(const char* _key, const char* _value, char *sRlt);
	bool push(const char* list_name, const char* _value, char *sRlt);
	int pop(const char* list_name, char *sRlt);

	void subs(const char *key, subsCallback cb);	//subscribe channel
	void unsubs(const char *key);					//unsubscribe channel
	void pull(const char *key, pullCallback cb);	//pull list
	void unpull(const char *key);					//unpull list, like unsubs

	//业务处理模块
	void subsClientGetOp(const char *keys, const char *reqChlName,
		const char *heartbeatChnName);

private:
	void close();
	//实现业务模块的数据隔离
	std::string genNewKey(std::string old_key);					//封装用户传入的key				
	std::string getOldKey(std::string new_key);					//获取客户的原始key
	bool sendCmd(const char *cmd, char *sRlt);					//发送redis命令
	bool replyCheck(redisReply *pRedisReply, char *sReply);		//解析redis应答消息

	static void* thAsyncSubsAll(void *arg);						//redis键空间通知

	static void connectCallback(const redisAsyncContext *c, int status);	//redis异步回调函数
	static void disconnectCallback(const redisAsyncContext *c, int status);
	static void subsAllCallback(redisAsyncContext *c, void *r, void *data);

	void callSubsCB(const char *key, const char *key_op);
	
	redisContext *pRedisContext;			//redis同步上下文
	redisAsyncContext *pRedisAsyncContext;	//redis异步上下文
	
#ifdef _WIN32				//redis异步事件库
	aeEventLoop *loop;
#else
	struct event_base *base;
#endif

	mapSubsCB m_subsKeys;		//订阅信息  键-->回调函数
	mapPullCB m_pullKeys;		//pull信息  键-->回调函数
	
	std::string ip;				//redis ip
	int port;					//redis 端口

	CRedisRPC redisRPC;			//redis RPC调用实现

	bool needSubs;				//是否需要redis订阅功能
	bool is_connected;			//连接redis成功标志
	std::string client_id;		//客户端标志
};

