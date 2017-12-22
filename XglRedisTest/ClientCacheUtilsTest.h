#pragma once
#include "ClientCacheUtils.h"

#include <thread>
#include <sstream>

std::string int2str(const int &int_temp)
{
	std::string str;
	std::stringstream st;
	st << int_temp;
	st >> str;
	return str;
}

std::ostringstream logInfo;
void setLog(int iLogType, std::ostringstream & strLogInfo)
{
	CClientCacheUtils::log(iLogType, strLogInfo.str());
	strLogInfo.str("");
}

#define REDISIP		"192.168.31.217"
#define REDISPORT	6379

void test_set(CClientCacheUtils & redis)
{
	std::string msg;
	for (int i = 0; i < 10; ++i)
	{
		std::string key_ = "!@##%%$" + int2str(i);
		std::string value = "!@##%%$" + int2str(i);
		int iRlt = redis.set(int2str(i), key_, value, msg);
		if (iRlt == 0)
			logInfo << "set op succ!!! msg = " << msg.c_str();
		else
			logInfo << "set op fail!!! errType = " << iRlt << "err = " << msg.c_str();
		setLog(LOG_DEBUG, logInfo);
		getchar();
	}
}
void test_get(CClientCacheUtils & redis)
{
	std::string msg;
	for (int i = 0; i < 10; ++i)
	{
		std::string key_ = "hellosadsdasa";
		int iRlt = redis.get(int2str(i), key_, msg);
		if (iRlt == 0)
			logInfo << "get op succ!!! msg = " << msg.c_str();
		else
			logInfo << "get op fail!!! errType = " << iRlt << "err = " << msg.c_str();
		setLog(LOG_DEBUG, logInfo);
	}
}
void test_push(CClientCacheUtils & redis)
{
	std::string msg;
	for (int i = 0; i < 10; ++i)
	{
		std::string key_ = "hello";
		std::string value = "world" + int2str(i);
		int iRlt = redis.push(int2str(i), key_, value, msg);
		if (iRlt == 0)
			logInfo << "push op succ!!! size = " << msg.c_str();
		else
			logInfo << "push op fail!!! errType = " << iRlt << "err = " << msg.c_str();
		setLog(LOG_DEBUG, logInfo);
	}
}
void test_pop(CClientCacheUtils & redis)
{
	std::string msg;
	for (int i = 0; i < 11; ++i)
	{
		std::string key_ = "hello";
		int iRlt = redis.pop(int2str(i), key_, msg);
		if (iRlt == 0)
			logInfo << "pop op succ!!! value = " << msg.c_str();
		else
			logInfo << "pop op fail!!! errType = " << iRlt << "err = " << msg.c_str();
		setLog(LOG_DEBUG, logInfo);
	}
}

void subCB(const std::string & strKey, const std::string & strValue, void * reids)
{
	CClientCacheUtils *utils = (CClientCacheUtils *)reids;
	std::string str;
	utils->get("A", "hello123", str);
	logInfo << "client got subs msg!!! str = " << str;
	setLog(LOG_DEBUG, logInfo);
	if (strValue.length() == 0)
		logInfo << "key = " << strKey.c_str() << ", was deleted...";
	else
		logInfo << "got subcb msg, key = " << strKey.c_str() << ", value = " << strValue.c_str();
	setLog(LOG_DEBUG, logInfo);
}
void subs_test()
{
	CClientCacheUtils redis;
	redis.connect(REDISIP, REDISPORT, true);
	//redisC.connect("192.168.31.217", 6379, true);

	auto f = [&redis](const std::string & strKey, const std::string & strValue)->void {
		std::string str;
		redis.get("A", "hello123", str);
		logInfo << "client got subs msg!!! str = " << str;
		setLog(LOG_DEBUG, logInfo);
		if (strValue.length() == 0)
			logInfo << "key = " << strKey.c_str() << ", was deleted...";
		else
			logInfo << "got subcb msg, key = " << strKey.c_str() << ", value = " << strValue.c_str();
		setLog(LOG_DEBUG, logInfo);
	};

	redis.subs("A", "hello*", f);
	redis.subs("A", "hello*", f);
	redis.subs("A", "hello*", f);
	redis.subs("B", "hello*", f);
	redis.subs("B", "hello*", f);
	redis.subs("B", "hello*", f);
	getchar();
	redis.unsubs("A", "hello*");
	redis.unsubs("A", "hello*");
	redis.unsubs("A", "hello*");
	redis.unsubs("B", "hello*");
	redis.unsubs("B", "hello*");
	redis.unsubs("B", "hello*");

	////只有客户端A收到回调
	//std::string msg;
	//redis.set("A", "helloA", "worldA", msg);
	//redis.set("B", "helloA", "worldA", msg);
	//redis.set("C", "helloA", "worldA", msg);
	//getchar();

	//redis.unsubs("A", "hello*");		
	//redis.set("A", "helloA", "worldA", msg);
	//redis.set("B", "helloA", "worldA", msg);
	//redis.set("C", "helloA", "worldA", msg);
	//getchar();

	//redis.unsubs("B", "hello*");		
	//redis.set("A", "helloA", "worldA", msg);
	//redis.set("B", "helloA", "worldA", msg);
	//redis.set("C", "helloA", "worldA", msg);
	//getchar();


	////只能收到字符串类型的回调
	//redis.push("A", "hellol", "1234", msg);
	getchar();
}

void pullCBA(const std::string & strKey, const std::string & strValue)
{
	logInfo << "clientA got pull msg!!!";
	setLog(LOG_DEBUG, logInfo);
	if (strValue.length() == 0)
		logInfo << "key = " << strKey.c_str() << ", was deleted...";
	else
		logInfo << "got pull msg, key = " << strKey.c_str() << ", value = " << strValue.c_str();
	setLog(LOG_DEBUG, logInfo);
}
void pullCBB(const std::string & strKey, const std::string & strValue)
{
	logInfo << "clientB got pull msg!!!";
	setLog(LOG_DEBUG, logInfo);
	if (strValue.length() == 0)
		logInfo << "key = " << strKey.c_str() << ", was deleted...";
	else
		logInfo << "got pull msg, key = " << strKey.c_str() << ", value = " << strValue.c_str();
	setLog(LOG_DEBUG, logInfo);
}
void pull_test()
{
	CClientCacheUtils redis;
	redis.connect(REDISIP, REDISPORT, true);
	//redisC.connect("192.168.31.217", 6379, true);

	redis.pull("A", "lhello*", pullCBA);
	redis.pull("B", "lhello*", pullCBB);

	//只有客户端A收到回调
	std::string msg;
	redis.push("A", "lhelloA", "worldA", msg);
	redis.push("B", "lhelloA", "worldA", msg);
	redis.push("C", "lhelloA", "worldA", msg);
	//getchar();

	//redis.unpull("A", "lhello*");
	redis.push("A", "lhelloA", "worldA", msg);
	redis.push("B", "lhelloA", "worldA", msg);
	redis.push("C", "lhelloA", "worldA", msg);
	//getchar();

	//redis.unpull("B", "lhello*");
	redis.push("A", "lhelloA", "worldA", msg);
	redis.push("B", "lhelloA", "worldA", msg);
	redis.push("C", "lhelloA", "worldA", msg);
	//getchar();

	redis.set("A", "lhellol", "1234", msg);
	//getchar();

}



void* multiThread(void *args)
{
	CClientCacheUtils *redis = (CClientCacheUtils *)args;

	std::stringstream ss;
	std::string strPid;
	ss << std::this_thread::get_id();
	ss >> strPid;

	//测试订阅客户端get操作
	//DEBUGLOG("getOp pid = " << strPid.c_str());
	//std::string msg;
	//redis->get("hello", msg);
	//DEBUGLOG("get result = " << msg.c_str());

	//std::string key = "hellolist" + strPid;
	//std::string msg;
	//for (int i = 0; i < 10; ++i)
	//{
	//	if (redis->pop(key.c_str(), msg) >= 0)
	//		DEBUGLOG("set op succ!!! msg = " << msg.c_str());
	//	else
	//		ERRORLOG("set op fail!!! err = " << msg.c_str());
	//}
	return nullptr;
}