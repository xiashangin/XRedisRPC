#include "CacheUtils.h"
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
CCacheUtils logUtil("log");
void setLog(int iLogType, std::ostringstream & strLogInfo)
{
	logUtil.log(iLogType, strLogInfo.str());
	strLogInfo.str("");
}

#define REDISIP		"192.168.31.217"
#define REDISPORT	6379

void test_set(CCacheUtils & redis)
{
	std::string msg;
	for (int i = 0; i < 10; ++i)
	{
		std::string key_ = "!@##%%$" + int2str(i);
		std::string value = "!@##%%$" + int2str(i);
		int iRlt = redis.set(key_, value, msg);
		if (iRlt == 0)
			logInfo << "set op succ!!! msg = " << msg.c_str();
		else
			logInfo << "set op fail!!! errType = " << iRlt << "err = " << msg.c_str();
		setLog(LOG_DEBUG, logInfo);
		getchar();
	}
}
void test_get(CCacheUtils & redis)
{
	std::string msg;
	for (int i = 0; i < 10; ++i)
	{
		std::string key_ = "hellosadsdasa";
		int iRlt = redis.get(key_, msg);
		if (iRlt == 0)
			logInfo << "get op succ!!! msg = " << msg.c_str();
		else
			logInfo << "get op fail!!! errType = " << iRlt << "err = " << msg.c_str();
		setLog(LOG_DEBUG, logInfo);
	}
}
void test_push(CCacheUtils & redis)
{
	std::string msg;
	for (int i = 0; i < 10; ++i)
	{
		std::string key_ = "hello";
		std::string value = "world" + int2str(i);
		int iRlt = redis.push(key_, value, msg);
		if (iRlt == 0)
			logInfo << "push op succ!!! size = " << msg.c_str();
		else
			logInfo << "push op fail!!! errType = " << iRlt << "err = " << msg.c_str();
		setLog(LOG_DEBUG, logInfo);
	}
}
void test_pop(CCacheUtils & redis)
{
	std::string msg;
	for (int i = 0; i < 11; ++i)
	{
		std::string key_ = "hello";
		int iRlt = redis.pop(key_, msg);
		if (iRlt == 0)
			logInfo << "pop op succ!!! value = " << msg.c_str();
		else
			logInfo << "pop op fail!!! errType = " << iRlt << "err = " << msg.c_str();
		setLog(LOG_DEBUG, logInfo);
	}
}

void subCBA(const std::string & strKey, const std::string & strValue)
{
	logInfo << "clientA got subs msg!!!";
	setLog(LOG_DEBUG, logInfo);
	if (strKey.length() == 0)
		logInfo << "key = " << strKey.c_str() << ", was deleted...";
	else
		logInfo << "got subcb msg, key = " << strKey.c_str() << ", value = " << strValue.c_str();
	setLog(LOG_DEBUG, logInfo);
}

void pullCBA(const std::string & strKey, const std::string & strValue)
{
	logInfo << "clientA got pull msg!!!";
	setLog(LOG_DEBUG, logInfo);
	if (strKey.length() == 0)
		logInfo << "key = " << strKey.c_str() << ", was deleted...";
	else
		logInfo << "got subcb msg, key = " << strKey.c_str() << ", value = " << strValue.c_str();
	setLog(LOG_DEBUG, logInfo);
}

void getCBA(const std::string & key, const std::string & value)
{
	//DEBUGLOG("clientA got get msg!!!");
	//CRedis_Utils redis("A");
	//redis.connect("192.168.31.217", 6379);
	//std::string msg;	//数据处理，处理完成之后调用set接口更新数据
	//std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	////通知客户端处理完成
	//if (value.length() == 0)
	//	redis.notifyRlt(key, "nil");
	//else
	//{
	//	std::string value_ = value + std::string("getCBA");
	//	
	//	redis.notifyRlt(key, value_.c_str());
	//}
}

void* multiThread(void *args)
{
	CCacheUtils *redis = (CCacheUtils *)args;

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