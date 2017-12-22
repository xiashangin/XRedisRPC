// XglRedisTest.cpp : 定义控制台应用程序的入口点。
//
//#include "CacheUtilsTest.h"
#include "ClientCacheUtilsTest.h"
#pragma comment(lib, "libXglRedis.lib")

int main(int argc, char const *argv[])
{
	//CCacheUtils redis("Main");
	CClientCacheUtils redis;
	bool bRlt = redis.connect(REDISIP, REDISPORT);
	if (!bRlt)
	{
		logInfo << "connect failed... ip = " << REDISIP << ", port = " << REDISPORT;
		setLog(LOG_DEBUG, logInfo);
	}
	//bool bRlt = redis.connect(REDISIP, REDISPORT);
	//if (!bRlt)
	//{
	//	logInfo << "connect failed... ip = " << REDISIP << ", port = " << REDISPORT;
	//	setLog(LOG_DEBUG, logInfo);
	//}

	//基本操作测试
	//test_set(redis);
	//test_get(redis);
	//test_push(redis);
	//test_pop(redis);

	subs_test();
	//pull_test();

	getchar();
	return 0;
}