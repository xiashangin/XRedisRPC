#pragma once

#include "common_tool.h"
#include <hiredis/hiredis.h>

#ifdef _WIN32
#include <WinSock2.h>
#else
#include <sys/time.h>
#endif // _WIN32

typedef void(*subsCallback)(const char *key);
typedef void(*pullCallback)(const char *key);

typedef std::map<std::string, subsCallback> mapSubsCB;
typedef std::map<std::string, pullCallback> mapPullCB;

class CRedis_Utils
{
public:
	CRedis_Utils();
	~CRedis_Utils();

	bool connect(const char* ip, int port);
	bool get(const char* _key, char *sRlt);
	bool set(const char* _key, const char* _value, char *sRlt);
	bool push(const char* list_name, const char* _value, char *sRlt);
	bool pop(const char* list_name, char *sRlt);

	void subs(const char *key, subsCallback cb);	//subscribe channel
	void pull(const char *key, pullCallback cb);	//pull list

	void close();

private:
	bool sendCmd(const char *cmd, char *sRlt);
	bool replyCheck(redisReply *pRedisReply, char *sReply);
	redisContext *pRedisContext;

	static void* asyncSubsAll(void *arg);
	static void connectCallback(const redisAsyncContext *c, int status);
	static void disconnectCallback(const redisAsyncContext *c, int status);
	static void subCallback(redisAsyncContext *c, void *r, void *data);

	void callSubsCB(const char *key);

	redisAsyncContext *pRedisAsyncContext;
	
#ifdef _WIN32
	aeEventLoop *loop;
#else
	struct event_base *base;
#endif

	mapSubsCB subsKeys;
	mapPullCB pullKeys;
	std::string ip;
	int port;
	bool connected;
};

