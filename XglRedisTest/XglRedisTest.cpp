// XglRedisTest.cpp : 定义控制台应用程序的入口点。
//

#include "Redis_Utils.h"
#pragma comment(lib, "libXglRedis.lib")

void test_get(CRedis_Utils& redis);
void test_set(CRedis_Utils& redis);
void test_push(CRedis_Utils& redis);
void test_pop(CRedis_Utils& redis);

void subs_test();
void pull_test();
void subCBA(const std::string & strKey, const std::string & strValue);
void subCBB(const std::string & strKey, const std::string & strValue);
void subCBC(const std::string & strKey, const std::string & strValue);
void pullCBA(const std::string & strKey, const std::string & strValue);
void pullCBB(const std::string & strKey, const std::string & strValue);
void pullCBC(const std::string & strKey, const std::string & strValue);

void subGetOp();
void getCBA(const std::string & strKey, const std::string & strValue);
void getCBB(const std::string & strKey, const std::string & strValue);

#define THREADNUM 10
void abnormalTest(CRedis_Utils& redis);


static void* multiThread(void *args);

int main(int argc, char const *argv[])
{
	g_ECGLogger = CMyLogger::getInstance();
	CRedis_Utils redis("Main");
	redis.connect("192.168.31.217", 6379);

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
	subGetOp();

	//abnormalTest(redis);

	//多线程测试
	//vector<thread> thGroup;
	//for (int i = 0; i < THREADNUM; ++i)
	//	thGroup.push_back(thread(multiThread, &redis));
	//for (int i = 0; i < THREADNUM; ++i)
	//	thGroup[i].join();
	getchar();
	return 0;
}

void test_set(CRedis_Utils& redis)
{
	std::string msg;
	for (int i = 0; i < 10; ++i)
	{
		std::string key_ = "!@##%%$" + int2str(i);
		std::string value = "!@##%%$" + int2str(i);
		if (redis.set(key_, value, msg))
			DEBUGLOG("set op succ!!! msg = " << msg.c_str());
		else
			ERRORLOG("set op fail!!! err = " << msg.c_str());
		getchar();
	}
}
void test_get(CRedis_Utils& redis)
{
	std::string msg;
	for (int i = 0; i < 10; ++i)
	{
		std::string key_ = "hellosadsdasa";
		int size = redis.get(key_, msg);
		if (size > 0)
			DEBUGLOG("set op succ!!! msg = " << msg.c_str());
		else if (size == 0)
			DEBUGLOG("no data... key = " << key_.c_str());
		else
			ERRORLOG("get op fail!!! err = " << msg.c_str());
	}
}
void test_push(CRedis_Utils& redis)
{
	std::string msg;
	for (int i = 0; i < 10; ++i)
	{
		std::string key_ = "hello";
		std::string value = "world" + int2str(i);
		if (redis.push(key_, value, msg))
			DEBUGLOG("push op succ!!! size = " << msg.c_str());
		else
			ERRORLOG("push op fail!!! err = " << msg.c_str());
	}
}
void test_pop(CRedis_Utils& redis)
{
	std::string msg;
	for (int i = 0; i < 11; ++i)
	{
		std::string key_ = "hello";
		int size = redis.pop(key_, msg);
		if (size > 0)
			DEBUGLOG("pop op succ!!! value = " << msg.c_str());
		else if (size == 0)
			DEBUGLOG("no data... key = " << key_.c_str());
		else
			ERRORLOG("pop op fail!!! err = " << msg.c_str());
	}
}

void subs_test()
{
	CRedis_Utils redisA("A");
	CRedis_Utils redisB("B");
	//CRedis_Utils redisC("C");
	redisA.connect("192.168.31.217", 6379, true);
	redisB.connect("192.168.31.217", 6379, true);
	//redisC.connect("192.168.31.217", 6379, true);

	redisA.subs("hello*", subCBA);
	//redisB.subs("hello*", subCBB);
	//redisC.subs("hello*", subCBC);

	//只有客户端A收到回调
	std::string msg;
	redisA.set("helloA", "worldA", msg);
	getchar();
	redisA.unsubs("hello*");		//取消拉取lhello*
	redisA.set("helloBCD", "worldABCD", msg);
	//只能收到字符串类型的回调
	redisA.push("hellol", "1234", msg);

	//只有客户端B收到回调
	redisB.set("helloA", "worldA", msg);
	redisB.set("helloBCD", "worldABCD", msg);

	//只有客户端C收到回调
	//redisC.set("helloA", "worldA", msg);
	//redisC.set("helloBCD", "worldABCD", msg);

	getchar();
}
void pull_test()
{
	CRedis_Utils redisA("A");
	CRedis_Utils redisB("B");
	CRedis_Utils redisC("C");
	redisA.connect("192.168.31.217", 6379, true);
	redisB.connect("192.168.31.217", 6379, true);
	redisC.connect("192.168.31.217", 6379, true);

	redisA.pull("lhello*", pullCBA);
	//redisB.pull("lhello*", subCBB);
	//redisC.pull("lhello*", subCBC);
	
	std::string msg;
	//只有客户端A收到回调
	redisA.push("lhello123", "1", msg);
	redisA.set("lhello000", "stringtype", msg);
	//getchar();
	redisA.unpull("lhello*");		//取消拉取lhello*
	redisA.push("lhello456", "2", msg);
	//只能收到list类型的回调
	
	//getchar();
	//只有客户端B收到回调
	redisB.push("lhello789", "3", msg);
	redisB.push("lhello012", "4", msg);
	//getchar();

	////只有客户端C收到回调
	redisC.push("lhello345", "5", msg);
	redisC.push("lhello678", "6", msg);
	//getchar();
}
void subCBA(const std::string & strKey, const std::string & strValue)
{
	DEBUGLOG("clientA got subs msg!!!");
	if (strKey.length() == 0)
		DEBUGLOG("key = " << strKey.c_str() << ", was deleted...");
	else
		DEBUGLOG("got subcb msg, key = " << strKey.c_str() << ", value = " << strValue.c_str());
}
void subCBB(const std::string & strKey, const std::string & strValue)
{
	DEBUGLOG("clientB got subs msg!!!");
	if (strKey.length() == 0)
		DEBUGLOG("key = " << strKey.c_str() << ", was deleted...");
	else
		DEBUGLOG("got subcb msg, key = " << strKey.c_str() << ", value = " << strValue.c_str());
}
void subCBC(const std::string & strKey, const std::string & strValue)
{
	DEBUGLOG("clientC got subs msg!!!");
	if (strKey.length() == 0)
		DEBUGLOG("key = " << strKey.c_str() << ", was deleted...");
	else
		DEBUGLOG("got subcb msg, key = " << strKey.c_str() << ", value = " << strValue.c_str());
}
void pullCBA(const std::string & strKey, const std::string & strValue)
{
	DEBUGLOG("clientA got pull msg!!!");
	if (strKey.length() == 0)
		DEBUGLOG("key = " << strKey.c_str() << ", was deleted...");
	else
		DEBUGLOG("got subcb msg, key = " << strKey.c_str() << ", value = " << strValue.c_str());
}
void pullCBB(const std::string & strKey, const std::string & strValue)
{
	DEBUGLOG("clientB got pull msg!!!");
	if (strKey.length() == 0)
		DEBUGLOG("key = " << strKey.c_str() << ", was deleted...");
	else
		DEBUGLOG("got subcb msg, key = " << strKey.c_str() << ", value = " << strValue.c_str());
}
void pullCBC(const std::string & strKey, const std::string & strValue)
{
	DEBUGLOG("clientC got pull msg!!!");
	if (strKey.length() == 0)
		DEBUGLOG("key = " << strKey.c_str() << ", was deleted...");
	else
		DEBUGLOG("got subcb msg, key = " << strKey.c_str() << ", value = " << strValue.c_str());
}

void subGetOp()
{
	CRedis_Utils redisA("A");
	redisA.connect("192.168.31.217", 6379, true);
	redisA.subsClientGetOp("hello", getCBA);

	CRedis_Utils redisB("B");
	redisB.connect("192.168.31.217", 6379, true);
	redisB.subsClientGetOp("hello", getCBB);
	//std::string msg;
	//vector<std::thread> thGroup;
	//for (int i = 0; i < THREADNUM; ++i)
	//	thGroup.push_back(std::thread(multiThread, &redisA));
	//for (int i = 0; i < THREADNUM; ++i)
	//	thGroup[i].join();
	//redisA.get("hello", msg);
	//DEBUGLOG("get result = " << msg.c_str());
	//msg.clear();
	//redisA.get("hello", msg);
	//redisB.get("hello", msg);
	//DEBUGLOG("get result = " << msg.c_str());
	//redisA.unsubClientGetOp("hello");
	//redisA.subsClientGetOp("hello", getCBA);
	//redisA.subsClientGetOp("hello123", getCBA);
	//redisA.unsubClientGetOp("hello");
	//redisA.unsubClientGetOp("hello123");
	getchar();
}
void getCBA(const std::string & key, const std::string & value)
{
	DEBUGLOG("clientA got get msg!!!");
	CRedis_Utils redis("A");
	redis.connect("192.168.31.217", 6379);
	std::string msg;	//数据处理，处理完成之后调用set接口更新数据
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	//通知客户端处理完成
	if (value.length() == 0)
		redis.set(key, "nil", msg);
	else
	{
		std::string value_ = value + std::string("getCBA");
		
		redis.set(key, value_.c_str(), msg);
	}
}

void getCBB(const std::string & key, const std::string & value)
{
	DEBUGLOG("clientB got get msg!!!");
	CRedis_Utils redis("B");
	redis.connect("192.168.31.217", 6379);
	std::string msg;	//数据处理，处理完成之后调用set接口更新数据
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	//通知客户端处理完成
	if (value.length() == 0)
		redis.set(key, "nil", msg);
	else
	{
		std::string value_ = value + std::string("getCBA");

		redis.set(key, value_.c_str(), msg);
	}

}


//void abnormalTest(CRedis_Utils& redis)
//{
//	redis.get("", nullptr);
//	redis.set("", "", nullptr);
//	redis.push("", "", nullptr);
//	redis.pop("",  nullptr);
//
//	redis.subs("", nullptr);
//	redis.unsubs("");
//	redis.pull("", nullptr);
//	redis.unpull("");
//
//	redis.subsClientGetOp("", nullptr);
//	redis.unsubClientGetOp("");
//}

void* multiThread(void *args)
{
	CRedis_Utils *redis = (CRedis_Utils *)args;

	stringstream ss;
	std::string strPid;
	ss << std::this_thread::get_id();
	ss >> strPid;

	DEBUGLOG("getOp pid = " << strPid.c_str());
	std::string msg;
	redis->get("hello", msg);
	DEBUGLOG("get result = " << msg.c_str());


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
