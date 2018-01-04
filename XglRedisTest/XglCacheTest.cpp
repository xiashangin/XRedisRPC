// XglRedisTest.cpp : 定义控制台应用程序的入口点。
//
#include <iostream>
#include "CacheUtilsTest.h"
//#include "ClientCacheUtilsTest.h"
#pragma comment(lib, "libXglRedis.lib")

//#include "json/ParseHealthData.h"

int main(int argc, char const *argv[])
{
	std::shared_ptr<CCacheUtils> redis = CCacheUtils::createInstance("M");
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

	//CParseHealthData parseUtils;
	//std::string str = parseUtils.readFileIntoString("registerjs_1.json");
	//std::cout << str << std::endl;
	////parseUtils.parseFromFile("registerjs_1.json");
	//parseUtils.parseFromString(str);
	//int iDataType = parseUtils.getDataType();

	//std::string name;

	//if (iDataType == TYPE_PERSONINFO)
	//{
	//	pdPersonInfo personInfo = *(pdPersonInfo *)parseUtils.getHealthData();
	//	name = *personInfo.m_gender;
	//	std::cout << "id-->" << *personInfo.m_id << std::endl;
	//	std::cout << "name-->" << *personInfo.m_name << std::endl;
	//	std::cout << "sex-->" << *personInfo.m_gender << std::endl;
	//	std::cout << "birth-->" << *personInfo.m_birth << std::endl;
	//	std::cout << "phone-->" << *personInfo.m_phone << std::endl;
	//}

	//get test
	//std::shared_ptr<CCacheUtils> redis1 = CCacheUtils::createInstance("M");
	//bRlt = redis1->connect(REDISIP, REDISPORT, true);
	//if (!bRlt)
	//{
	//	logInfo << "connect failed... ip = " << REDISIP << ", port = " << REDISPORT;
	//	setLog(LOG_DEBUG, logInfo);
	//}

	//int callBackCnt = 0;
	//redis1->subsClientGetOp("SGETID_*", [&redis1, &callBackCnt](const std::string & strKey, const std::string & strValue) {
	//	callBackCnt++;
	//	if (strValue.length() > 0)
	//	{
	//		logInfo << "get op callback... key = " << strKey.c_str() <<
	//			", value = " << strValue;
	//		setLog(LOG_DEBUG, logInfo);
	//		std::string value_ = strValue + std::string("1");
	//		redis1->notifyRlt(strKey, int2str(callBackCnt));
	//	}
	//	else
	//	{
	//		logInfo << "get op callback... key was empty key = " << strKey.c_str();
	//		setLog(LOG_DEBUG, logInfo);
	//		redis1->notifyRlt(strKey, int2str(callBackCnt));
	//	}
	//});
	//std::this_thread::sleep_for(std::chrono::milliseconds(100));
	//std::string strRlt;
	//int iRlt;
	//int getOpCnt = 0;
	//while (getOpCnt < 10) {
	//	getOpCnt++;
	//	iRlt = redis->get("SGETID_123", strRlt);
	//	logInfo << "strRlt = " << strRlt.c_str() << "-->" << getOpCnt;
	//	setLog(LOG_DEBUG, logInfo);
	//}

	//setLog(LOG_DEBUG, logInfo);
	//std::shared_ptr<CCacheUtils> redis2 = CCacheUtils::createInstance("1");
	//bRlt = redis2->connect(REDISIP, REDISPORT, true);
	//redis2->subsClientGetOp("hello", [&redis2](const std::string & strKey, const std::string & strValue) {
	//	if (strValue.length() > 0)
	//	{
	//		logInfo << "get op callback... key = " << strKey.c_str() <<
	//			", value = " << strValue;
	//		setLog(LOG_DEBUG, logInfo);
	//		std::string value_ = strValue + std::string("2");
	//		redis2->notifyRlt(strKey, value_);
	//	}
	//	else
	//	{
	//		logInfo << "get op callback... key was empty key = " << strKey.c_str();
	//		setLog(LOG_DEBUG, logInfo);
	//		redis2->notifyRlt(strKey, "1234567");
	//	}
	//});
	//redis1->unsubClientGetOp("hello");
	//redis2->unsubClientGetOp("hello");

	//std::this_thread::sleep_for(std::chrono::milliseconds(100));
	//iRlt = redis->get("hello", strRlt);
	//if (iRlt == 0)
	//{
	//	logInfo << "get success!!! hello --> " << strRlt;
	//	setLog(LOG_DEBUG, logInfo);
	//}
	//else
	//{
	//	logInfo << "get failed!!! iRlt = " << iRlt << "-->" << strRlt;
	//	setLog(LOG_DEBUG, logInfo);
	//}

	//subs test
	int getCnt = 0;
	std::shared_ptr<CCacheUtils> redis1 = CCacheUtils::createInstance("F");
	bRlt = redis1->connect(REDISIP, REDISPORT, true);
	if (!bRlt)
	{
		logInfo << "connect failed... ip = " << REDISIP << ", port = " << REDISPORT;
		setLog(LOG_DEBUG, logInfo);
	}
	redis1->pull("*", [&getCnt](const std::string & strKey, const std::string & strValue) {
		logInfo << "get pull callback... key = " << strKey.c_str() <<
			", value = " << strValue.c_str();
		setLog(LOG_DEBUG, logInfo);
		std::this_thread::sleep_for(std::chrono::milliseconds(10000));
	});
	redis1->subs("*", [&getCnt](const std::string & strKey, const std::string & strValue) {
		logInfo << "get subs callback... key = " << strKey.c_str() <<
			", value = " << strValue.c_str();
		setLog(LOG_DEBUG, logInfo);
		//if (strValue.length() > 0)
		//{
		//	getCnt++;
		//	if (getCnt % 100 == 0)
		//	{
		//		logInfo << "get subs callback... key = " << strKey.c_str() <<
		//			", value = " << getCnt;
		//		setLog(LOG_DEBUG, logInfo);
		//	}
		//}
		//else
		//{
		//	logInfo << "get subs callback... key was empty key = " << strKey.c_str();
		//	setLog(LOG_DEBUG, logInfo);
		//}
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	});
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	std::string strRlt;
	redis1->set("A", "888", strRlt);
	redis1->set("A", "2", strRlt);
	redis1->set("A", "3", strRlt);
	redis1->set("A", "4", strRlt);
	redis1->set("A", "5", strRlt);
	redis1->set("A", "6", strRlt);

	//const int THREADNUM = 10;
	//std::vector<std::thread> thTestGroup;
	//for (int i = 0; i < THREADNUM; ++i)
	//	thTestGroup.push_back(std::thread(multiThread, (void *)&*redis));

	//for (int i = 0; i < THREADNUM; ++i)
	//	thTestGroup[i].join();

	getchar();
	return 0;
}


