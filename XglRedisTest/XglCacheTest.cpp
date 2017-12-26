// XglRedisTest.cpp : 定义控制台应用程序的入口点。
//
#include "CacheUtilsTest.h"
//#include "ClientCacheUtilsTest.h"
#pragma comment(lib, "libXglRedis.lib")

int main(int argc, char const *argv[])
{
	std::shared_ptr<CCacheUtils> redis = CCacheUtils::createInstance("1");
	//CClientCacheUtils redis;
	bool bRlt = redis->connect(REDISIP, REDISPORT);
	if (!bRlt)
	{
		logInfo << "connect failed... ip = " << REDISIP << ", port = " << REDISPORT;
		setLog(LOG_DEBUG, logInfo);
	}

	//std::shared_ptr<CCacheUtils>utils = CCacheUtils::createInstance(std::string("A"));
	//utils->connect(REDISIP, REDISPORT, true);
	//bool bRlt = redis.connect(REDISIP, REDISPORT);
	//if (!bRlt)
	//{
	//	logInfo << "connect failed... ip = " << REDISIP << ", port = " << REDISPORT;
	//	setLog(LOG_DEBUG, logInfo);
	//}

	//基本操作测试
	//test_set(*redis);
	//test_get(*redis);
	//test_push(redis);
	//test_pop(redis);

	//subs_test();
	//pull_test();


	//get test
	std::shared_ptr<CCacheUtils> redis2 = CCacheUtils::createInstance("1");
	bRlt = redis2->connect(REDISIP, REDISPORT, true);
	if (!bRlt)
	{
		logInfo << "connect failed... ip = " << REDISIP << ", port = " << REDISPORT;
		setLog(LOG_DEBUG, logInfo);
	}

	redis2->subsClientGetOp("hello", [&redis2](const std::string & strKey, const std::string & strValue) {
		if (strValue.length() > 0)
		{
			logInfo << "get op callback... key = " << strKey.c_str() <<
				", value = " << strValue;
			setLog(LOG_DEBUG, logInfo);
			std::string value_ = strValue + std::string("getCBA");
			redis2->notifyRlt(strKey, value_);
		}
		else
		{
			logInfo << "get op callback... key was empty key = " << strKey.c_str();
			setLog(LOG_DEBUG, logInfo);
			redis2->notifyRlt(strKey, "123456");
		}
		
	});
	
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	std::string strRlt;
	int iRlt = redis->get("hello", strRlt);
	if (iRlt == 0)
	{
		logInfo << "get success!!! hello --> " << strRlt;
		setLog(LOG_DEBUG, logInfo);
	}
	else
	{
		logInfo << "get failed!!! iRlt = " << iRlt;
		setLog(LOG_DEBUG, logInfo);
	}

	//subs test
	//int getCnt = 0;
	//std::shared_ptr<CCacheUtils> redis1 = CCacheUtils::createInstance("1");
	//bRlt = redis1->connect(REDISIP, REDISPORT, true);
	//if (!bRlt)
	//{
	//	logInfo << "connect failed... ip = " << REDISIP << ", port = " << REDISPORT;
	//	setLog(LOG_DEBUG, logInfo);
	//}
	//redis1->subs("*", [&getCnt](const std::string & strKey, const std::string & strValue) {
	//	if (strValue.length() > 0)
	//	{
	//		getCnt++;
	//		if (getCnt % 100 == 0)
	//		{
	//			logInfo << "get subs callback... key = " << strKey.c_str() <<
	//				", value = " << getCnt;
	//			setLog(LOG_DEBUG, logInfo);
	//		}
	//	}
	//	else
	//	{
	//		logInfo << "get subs callback... key was empty key = " << strKey.c_str();
	//		setLog(LOG_DEBUG, logInfo);
	//	}
	//});

	//const int THREADNUM = 100;
	//std::vector<std::thread> thTestGroup;
	//for (int i = 0; i < THREADNUM; ++i)
	//	thTestGroup.push_back(std::thread(multiThread, (void *)&*redis));

	//for (int i = 0; i < THREADNUM; ++i)
	//	thTestGroup[i].join();

	getchar();
	return 0;
}


