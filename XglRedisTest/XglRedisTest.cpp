// XglRedisTest.cpp : 定义控制台应用程序的入口点。
//

#include "CacheUtils.h"
#pragma comment(lib, "libXglRedis.lib")

#define DEBUGLOG(inf) cout<<inf<<endl



//void test_get(CRedis_Utils& redis);
//void test_set(CRedis_Utils& redis);
//void test_push(CRedis_Utils& redis);
//void test_pop(CRedis_Utils& redis);
//
//void subs_test();
//void pull_test();
void subCBA(const std::string & strKey, const std::string & strValue);
//void subCBB(const std::string & strKey, const std::string & strValue);
//void subCBC(const std::string & strKey, const std::string & strValue);
//void pullCBA(const std::string & strKey, const std::string & strValue);
//void pullCBB(const std::string & strKey, const std::string & strValue);
//void pullCBC(const std::string & strKey, const std::string & strValue);
//
//void subGetOp();
void getCBA(const std::string & strKey, const std::string & strValue);
//void getCBB(const std::string & strKey, const std::string & strValue);
//
//void abnormalTest(CRedis_Utils& redis);
//void performanceTest(int clientId);
//
//#define THREADNUM 10
//static void* multiThread(void *args);
//static void* multiThreadGet(void * lpStrClientId);
//static void* multiThreadSet(void * lpStrClientId);
//static void* multiThreadPush(void * lpStrClientId);
//static void* multiThreadPop(void * lpStrClientId);
#include <iostream>
#include <sstream>
using namespace std;

int main(int argc, char const *argv[])
{
	CCacheUtils cacheUtils("Main");
	int iRlt = cacheUtils.connect("192.168.31.170", 6379, true);
	if (!iRlt)
	{
		cout << "connect to redis failed!!!" << endl;
		getchar();
		return 0;
	}

	cacheUtils.subs("hello", subCBA);
	cacheUtils.subsClientGetOp("hello", getCBA);
	string strRlt;
	cacheUtils.set("hello", "there", strRlt);
	cacheUtils.get("hello", strRlt);
	ostringstream os;
	os << "get Rlt = " << strRlt;
	cacheUtils.log(LOG_DEBUG, os.str());
	DEBUGLOG("get Rlt = " << strRlt);
	//g_ECGLogger = CMyLogger::getInstance();
	
	//CRedis_Utils redis("Main");
	//redis.connect("192.168.31.217", 6379);

	//std::thread t1;
	//t1 = thread(multiThread, &redis);

	//基本操作测试
	//test_set(redis);
	//test_get(redis);
	//test_push(redis);
	//test_pop(redis);

	//订阅发布测试
	//subs_test();
	//pull_test();
	//subGetOp();

	//abnormalTest(redis);
	//performanceTest(1);

	//多线程测试
	//vector<std::thread> thGroup;
	//for (int i = 0; i < THREADNUM; ++i)
	//	thGroup.push_back(std::thread(multiThread, &i));
	//for (int i = 0; i < THREADNUM; ++i)
	//	thGroup[i].join();

	//char * getSetClientId = "GetSetTest";
	//char * PushPopClientId = "PushPopTest";
	//vector<std::thread> thGetGroup, thSetGroup, thPushGroup, thPopGroup;
	//for (int i = 0; i < THREADNUM; ++i)
	//	thGetGroup.push_back(std::thread(multiThreadGet, getSetClientId));
	//for (int i = 0; i < THREADNUM; ++i)
	//	thSetGroup.push_back(std::thread(multiThreadSet, getSetClientId));
	//for (int i = 0; i < THREADNUM; ++i)
	//	thPushGroup.push_back(std::thread(multiThreadPush, PushPopClientId));
	//for (int i = 0; i < THREADNUM; ++i)
	//	thPopGroup.push_back(std::thread(multiThreadPop, PushPopClientId));

	//for (int i = 0; i < THREADNUM; ++i)
	//	thGetGroup[i].join();
	//for (int i = 0; i < THREADNUM; ++i)
	//	thSetGroup[i].join();
	//for (int i = 0; i < THREADNUM; ++i)
	//	thPushGroup[i].join();
	//for (int i = 0; i < THREADNUM; ++i)
	//	thPopGroup[i].join();
	getchar();
	return 0;
}

//void test_set(CRedis_Utils& redis)
//{
//	std::string msg;
//	for (int i = 0; i < 10; ++i)
//	{
//		std::string key_ = "!@##%%$" + int2str(i);
//		std::string value = "!@##%%$" + int2str(i);
//		if (redis.set(key_, value, msg))
//			DEBUGLOG("set op succ!!! msg = " << msg.c_str());
//		else
//			ERRORLOG("set op fail!!! err = " << msg.c_str());
//		getchar();
//	}
//}
//void test_get(CRedis_Utils& redis)
//{
//	std::string msg;
//	for (int i = 0; i < 10; ++i)
//	{
//		std::string key_ = "hellosadsdasa";
//		int size = redis.get(key_, msg);
//		if (size > 0)
//			DEBUGLOG("set op succ!!! msg = " << msg.c_str());
//		else if (size == 0)
//			DEBUGLOG("no data... key = " << key_.c_str());
//		else
//			ERRORLOG("get op fail!!! err = " << msg.c_str());
//	}
//}
//void test_push(CRedis_Utils& redis)
//{
//	std::string msg;
//	for (int i = 0; i < 10; ++i)
//	{
//		std::string key_ = "hello";
//		std::string value = "world" + int2str(i);
//		if (redis.push(key_, value, msg))
//			DEBUGLOG("push op succ!!! size = " << msg.c_str());
//		else
//			ERRORLOG("push op fail!!! err = " << msg.c_str());
//	}
//}
//void test_pop(CRedis_Utils& redis)
//{
//	std::string msg;
//	for (int i = 0; i < 11; ++i)
//	{
//		std::string key_ = "hello";
//		int size = redis.pop(key_, msg);
//		if (size > 0)
//			DEBUGLOG("pop op succ!!! value = " << msg.c_str());
//		else if (size == 0)
//			DEBUGLOG("no data... key = " << key_.c_str());
//		else
//			ERRORLOG("pop op fail!!! err = " << msg.c_str());
//	}
//}
//
//void subs_test()
//{
//	CRedis_Utils redisA("A");
//	CRedis_Utils redisB("B");
//	//CRedis_Utils redisC("C");
//	redisA.connect("192.168.31.217", 6379, true);
//	redisB.connect("192.168.31.217", 6379, true);
//	//redisC.connect("192.168.31.217", 6379, true);
//
//	redisA.subs("hello*", subCBA);
//	//redisB.subs("hello*", subCBB);
//	//redisC.subs("hello*", subCBC);
//
//	//只有客户端A收到回调
//	std::string msg;
//	redisA.set("helloA", "worldA", msg);
//	getchar();
//	redisA.unsubs("hello*");		//取消拉取lhello*
//	redisA.set("helloBCD", "worldABCD", msg);
//	//只能收到字符串类型的回调
//	redisA.push("hellol", "1234", msg);
//
//	//只有客户端B收到回调
//	redisB.set("helloA", "worldA", msg);
//	redisB.set("helloBCD", "worldABCD", msg);
//
//	//只有客户端C收到回调
//	//redisC.set("helloA", "worldA", msg);
//	//redisC.set("helloBCD", "worldABCD", msg);
//
//	getchar();
//}
//void pull_test()
//{
//	CRedis_Utils redisA("A");
//	CRedis_Utils redisB("B");
//	CRedis_Utils redisC("C");
//	redisA.connect("192.168.31.217", 6379, true);
//	redisB.connect("192.168.31.217", 6379, true);
//	redisC.connect("192.168.31.217", 6379, true);
//
//	redisA.pull("lhello*", pullCBA);
//	//redisB.pull("lhello*", subCBB);
//	//redisC.pull("lhello*", subCBC);
//	
//	std::string msg;
//	//只有客户端A收到回调
//	redisA.push("lhello123", "1", msg);
//	redisA.set("lhello000", "stringtype", msg);
//	//getchar();
//	redisA.unpull("lhello*");		//取消拉取lhello*
//	redisA.push("lhello456", "2", msg);
//	//只能收到list类型的回调
//	
//	//getchar();
//	//只有客户端B收到回调
//	redisB.push("lhello789", "3", msg);
//	redisB.push("lhello012", "4", msg);
//	//getchar();
//
//	////只有客户端C收到回调
//	redisC.push("lhello345", "5", msg);
//	redisC.push("lhello678", "6", msg);
//	//getchar();
//}
void subCBA(const std::string & strKey, const std::string & strValue)
{
	DEBUGLOG("clientA got subs msg!!!");
	if (strKey.length() == 0)
		DEBUGLOG("key = " << strKey.c_str() << ", was deleted...");
	else
		DEBUGLOG("got subcb msg, key = " << strKey.c_str() << ", value = " << strValue.c_str());
}
//void subCBB(const std::string & strKey, const std::string & strValue)
//{
//	DEBUGLOG("clientB got subs msg!!!");
//	if (strKey.length() == 0)
//		DEBUGLOG("key = " << strKey.c_str() << ", was deleted...");
//	else
//		DEBUGLOG("got subcb msg, key = " << strKey.c_str() << ", value = " << strValue.c_str());
//}
//void subCBC(const std::string & strKey, const std::string & strValue)
//{
//	DEBUGLOG("clientC got subs msg!!!");
//	if (strKey.length() == 0)
//		DEBUGLOG("key = " << strKey.c_str() << ", was deleted...");
//	else
//		DEBUGLOG("got subcb msg, key = " << strKey.c_str() << ", value = " << strValue.c_str());
//}
void pullCBA(const std::string & strKey, const std::string & strValue)
{
	DEBUGLOG("clientA got pull msg!!!");
	if (strKey.length() == 0)
		DEBUGLOG("key = " << strKey.c_str() << ", was deleted...");
	else
		DEBUGLOG("got subcb msg, key = " << strKey.c_str() << ", value = " << strValue.c_str());
}
//void pullCBB(const std::string & strKey, const std::string & strValue)
//{
//	DEBUGLOG("clientB got pull msg!!!");
//	if (strKey.length() == 0)
//		DEBUGLOG("key = " << strKey.c_str() << ", was deleted...");
//	else
//		DEBUGLOG("got subcb msg, key = " << strKey.c_str() << ", value = " << strValue.c_str());
//}
//void pullCBC(const std::string & strKey, const std::string & strValue)
//{
//	DEBUGLOG("clientC got pull msg!!!");
//	if (strKey.length() == 0)
//		DEBUGLOG("key = " << strKey.c_str() << ", was deleted...");
//	else
//		DEBUGLOG("got subcb msg, key = " << strKey.c_str() << ", value = " << strValue.c_str());
//}
//
//void subGetOp()
//{
//	CRedis_Utils redisA("A");
//	redisA.connect("192.168.31.217", 6379, true);
//	redisA.subsClientGetOp("hello", getCBA);
//
//	//CRedis_Utils redisB("A");
//	//redisB.connect("192.168.31.217", 6379, true);
//	//redisB.subsClientGetOp("hello", getCBB);
//	std::string msg;
//	vector<std::thread> thGroup;
//	for (int i = 0; i < THREADNUM; ++i)
//		thGroup.push_back(std::thread(multiThread, &redisA));
//	for (int i = 0; i < THREADNUM; ++i)
//		thGroup[i].join();
//	//redisA.get("hello", msg);
//	//DEBUGLOG("get result = " << msg.c_str());
//	//redisB.get("hello", msg);
//	//DEBUGLOG("get result = " << msg.c_str());
//	//msg.clear();
//	//redisA.get("hello", msg);
//	//redisB.get("hello", msg);
//	//DEBUGLOG("get result = " << msg.c_str());
//	//redisA.unsubClientGetOp("hello");
//	//redisA.subsClientGetOp("hello", getCBA);
//	//redisA.subsClientGetOp("hello123", getCBA);
//	//redisA.unsubClientGetOp("hello");
//	//redisA.unsubClientGetOp("hello123");
//	getchar();
//}
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
//
//void getCBB(const std::string & key, const std::string & value)
//{
//	DEBUGLOG("clientB got get msg!!!");
//	CRedis_Utils redis("A");
//	redis.connect("192.168.31.217", 6379);
//	std::string msg;	//数据处理，处理完成之后调用set接口更新数据
//	std::this_thread::sleep_for(std::chrono::milliseconds(2000));
//	//通知客户端处理完成
//	if (value.length() == 0)
//		redis.notifyRlt(key, "nil");
//	else
//	{
//		std::string value_ = value + std::string("getCBB");
//		redis.notifyRlt(key, value_.c_str());
//	}
//
//}
//
//
//void abnormalTest(CRedis_Utils& redis)
//{
//	std::string strRlt;
//	redis.get("", strRlt);
//	redis.set("", "", strRlt);
//	redis.push("", "", strRlt);
//	redis.pop("", strRlt);
//
//	redis.subs("", nullptr);
//	redis.unsubs("");
//	redis.pull("", nullptr);
//	redis.unpull("");
//
//	redis.subsClientGetOp("", nullptr);
//	redis.unsubClientGetOp("");
//}
//
//void* multiThread(void *args)
//{
//	CRedis_Utils *redis = (CRedis_Utils *)args;
//
//	stringstream ss;
//	std::string strPid;
//	ss << std::this_thread::get_id();
//	ss >> strPid;
//
//	//测试订阅客户端get操作
//	//DEBUGLOG("getOp pid = " << strPid.c_str());
//	//std::string msg;
//	//redis->get("hello", msg);
//	//DEBUGLOG("get result = " << msg.c_str());
//
//
//
//	//std::string key = "hellolist" + strPid;
//	//std::string msg;
//	//for (int i = 0; i < 10; ++i)
//	//{
//	//	if (redis->pop(key.c_str(), msg) >= 0)
//	//		DEBUGLOG("set op succ!!! msg = " << msg.c_str());
//	//	else
//	//		ERRORLOG("set op fail!!! err = " << msg.c_str());
//	//}
//	return nullptr;
//}
//
//void performanceTest(int clientId)
//{
//	CRedis_Utils redis(int2str(clientId).c_str());
//	redis.connect("192.168.31.217", 6379);
//	
//	std::string strRlt;
//	DEBUGLOG("start!!!!");
//	std::vector<int> oneOpTime;
//	for(int i = 0; i < 1000; ++i)
//	{
//		std::string strKey = int2str(clientId) + int2str(i);
//		std::string strValue = generate_uuid();
//		int64_t now = GetSysTimeMicros();
//		DEBUGLOG("start send cmd now = " << now);
//		//redis.set(strKey, strValue, strRlt);
//		//redis.get(strKey, strRlt);
//		//redis.push(strKey, strValue, strRlt);
//		redis.pop(strKey, strRlt);
//		DEBUGLOG("send cmd complete interval = " << GetSysTimeMicros() - now);
//		oneOpTime.push_back(GetSysTimeMicros() - now);
//		DEBUGLOG("set success!!! key = " << strKey.c_str() << ", value = "
//			<< strValue.c_str() << ", rlt = " << strRlt.c_str());
//		strRlt.clear();
//	}
//	int avg = std::accumulate(oneOpTime.begin(), oneOpTime.end(), 0) / oneOpTime.size();
//	DEBUGLOG("end!!!! avg = " << avg
//		<< ", hiredis avg = " << redis.getAvgOpTime());
//}
//
//CRedis_Utils* getRedisUtil(std::string clientId)
//{
//	CRedis_Utils *redis = new CRedis_Utils(clientId);
//	redis->connect("192.168.31.170", 6379);
//	return redis;
//}
//
//std::string getThreadId()
//{
//	stringstream ss;
//	std::string strPid;
//	ss << std::this_thread::get_id();
//	ss >> strPid;
//	return strPid;
//}
//
//#include <random>
//int getRandomNum(int low, int high)
//{
//	std::random_device rd;
//	std::default_random_engine e(rd());
//	std::uniform_int_distribution<> u(low, high);
//	return u(e);
//}
//
//std::vector<std::string> vecThreadId;
//#define OPTIME 100000
//void* multiThreadGet(void * lpStrClientId)
//{
//	CRedis_Utils *redis = getRedisUtil(std::string((char *)lpStrClientId));
//	std::string thread_id = getThreadId();
//
//	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//	std::string strRlt;
//
//	for (int i = 1; i <= OPTIME; ++i)
//	{
//		int num = getRandomNum(0, vecThreadId.size() - 1);
//		int status = redis->get(vecThreadId[num] + int2str(i), strRlt);
//		if (status == 0)
//			DEBUGLOG("get success!!!  rlt = " << strRlt.c_str() << ", key = " << (vecThreadId[num] + int2str(i)).c_str());
//		else
//			DEBUGLOG("get fail!!!  errstatus = " << status << ", errstr = " << strRlt.c_str());
//		strRlt.clear();
//		std::this_thread::sleep_for(std::chrono::milliseconds(getRandomNum(800, 1000)));
//	}
//
//	delete redis;
//	return nullptr;
//}
//
//
//void* multiThreadSet(void * lpStrClientId)
//{
//	CRedis_Utils *redis = getRedisUtil(std::string((char *)lpStrClientId));
//	std::string thread_id = getThreadId();
//	vecThreadId.push_back(thread_id);
//	DEBUGLOG("thread num = " << vecThreadId.size());
//	std::string strRlt;
//
//	for (int i = 1; i <= OPTIME; ++i)
//	{
//		std::string strValue = thread_id + "_" + int2str(i);
//		int status = redis->set(thread_id + int2str(i), strValue, strRlt);
//		if (status == 0)
//			DEBUGLOG("set success!!!  value = " << strValue.c_str() << ", rlt = " << strRlt.c_str());
//		else
//			DEBUGLOG("set fail!!!  errstatus = " << status << ", errstr = " << strRlt.c_str());
//		strRlt.clear();
//		std::this_thread::sleep_for(std::chrono::milliseconds(getRandomNum(500, 900)));
//	}
//
//	delete redis;
//	return nullptr;
//}
//
//
//void* multiThreadPush(void * lpStrClientId)
//{
//	CRedis_Utils *redis = getRedisUtil(std::string((char *)lpStrClientId));
//	std::string thread_id = getThreadId();
//
//	std::string strRlt;
//
//	for (int i = 1; i <= OPTIME; ++i)
//	{
//		std::string strValue = thread_id + "_" + int2str(i);
//		int status = redis->push(int2str(i), strValue, strRlt);
//		if (status == 0)
//			DEBUGLOG("push success!!!  value = " << strValue.c_str() << ", rlt = " << strRlt.c_str());
//		else
//			DEBUGLOG("push fail!!!  errstatus = " << status << ", errstr = " << strRlt.c_str());
//		strRlt.clear();
//		std::this_thread::sleep_for(std::chrono::milliseconds(getRandomNum(500, 900)));
//
//		strRlt.clear();
//	}
//
//	delete redis;
//	return nullptr;
//}
//
//void* multiThreadPop(void * lpStrClientId)
//{
//	CRedis_Utils *redis = getRedisUtil(std::string((char *)lpStrClientId));
//	std::string thread_id = getThreadId();
//	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//
//	std::string strRlt;
//
//	for (int i = 1; i <= OPTIME; ++i)
//	{
//		int status = redis->pop(int2str(i), strRlt);
//		if (status == 0)
//			DEBUGLOG("pop success!!!  rlt = " << strRlt.c_str() << ", key = " << int2str(i).c_str());
//		else
//			DEBUGLOG("pop fail!!!  errstatus = " << status << ", errstr = " << strRlt.c_str());
//		strRlt.clear();
//		std::this_thread::sleep_for(std::chrono::milliseconds(getRandomNum(800, 1000)));
//
//		strRlt.clear();
//	}
//
//	delete redis;
//	return nullptr;
//}