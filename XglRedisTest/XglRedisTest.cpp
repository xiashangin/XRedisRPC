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
void subCBA(const char *key, const char *value);
void subCBB(const char *key, const char *value);
void subCBC(const char *key, const char *value);
void pullCBA(const char *key, const char *value);
void pullCBB(const char *key, const char *value);
void pullCBC(const char *key, const char *value);

void subGetOp();
void getCBA(const char *key, const char *value);
void getCBB(const char *key, const char *value);

#define THREADNUM 10
void abnormalTest(CRedis_Utils& redis);


static void* multiThread(void *args);

int main(int argc, char const *argv[])
{
	CRedis_Utils redis("A");
	redis.connect("192.168.31.217", 6379);
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

	//多线程测试
	vector<thread> thGroup;
	for (int i = 0; i < THREADNUM; ++i)
		thGroup.push_back(thread(multiThread, &redis));
	for (int i = 0; i < THREADNUM; ++i)
		thGroup[i].join();
	getchar();
	return 0;
}

void test_set(CRedis_Utils& redis)
{
	char *msg = (char *)malloc(REDIS_BUF_SIZE);
	for (int i = 0; i < 10; ++i)
	{
		std::string key_ = "hello" + int2str(i);
		std::string value = "world" + int2str(i);
		memset(msg, 0, REDIS_BUF_SIZE);
		if (redis.set(key_.c_str(), value.c_str(), msg))	
			DEBUGLOG << "set op succ!!! msg = " << msg << endl;
		else
			ERRORLOG << "set op fail!!! err = " << msg << endl;
	}
	free(msg);
}
void test_get(CRedis_Utils& redis)
{
	char *msg = (char *)malloc(REDIS_BUF_SIZE);
	for (int i = 0; i < 1; ++i)
	{
		std::string key_ = "helloasd" + int2str(i);
		memset(msg, 0, REDIS_BUF_SIZE);
		int size = redis.get(key_.c_str(), msg);
		if (size > 0)
			DEBUGLOG << "set op succ!!! msg = " << msg << endl;
		else if (size == 0)
			DEBUGLOG << "no data... key = " << key_ << endl;
		else
			ERRORLOG << "get op fail!!! err = " << msg << endl;
	}
	free(msg);
}
void test_push(CRedis_Utils& redis)
{
	char *msg = (char *)malloc(REDIS_BUF_SIZE);
	for (int i = 0; i < 10; ++i)
	{
		std::string key_ = "hello";
		std::string value = "world" + int2str(i);
		memset(msg, 0, REDIS_BUF_SIZE);
		if (redis.push(key_.c_str(), value.c_str(), msg))
			DEBUGLOG << "push op succ!!! size = " << msg << endl;
		else
			ERRORLOG << "push op fail!!! err = " << msg << endl;
	}
	free(msg);
}
void test_pop(CRedis_Utils& redis)
{
	char *msg = (char *)malloc(REDIS_BUF_SIZE);
	for (int i = 0; i < 11; ++i)
	{
		std::string key_ = "hello";
		memset(msg, 0, REDIS_BUF_SIZE);
		int size = redis.pop(key_.c_str(), msg);
		if (size > 0)
			DEBUGLOG << "pop op succ!!! size = " << msg << endl;
		else if (size == 0)
			DEBUGLOG << "no data... key = " << key_ << endl;
		else
			ERRORLOG << "pop op fail!!! err = " << msg << endl;
	}
	free(msg);
}

void subs_test()
{
	CRedis_Utils redisA("A");
	CRedis_Utils redisB("B");
	CRedis_Utils redisC("C");
	redisA.connect("192.168.31.217", 6379, true);
	redisB.connect("192.168.31.217", 6379, true);
	redisC.connect("192.168.31.217", 6379, true);

	redisA.subs("hello*", subCBA);
	//redisB.subs("hello*", subCBB);
	//redisC.subs("hello*", subCBC);

	//只有客户端A收到回调
	redisA.set("helloA", "worldA", nullptr);
	redisA.unsubs("hello*");		//取消拉取lhello*
	redisA.set("helloBCD", "worldABCD", nullptr);	
	//只能收到字符串类型的回调
	redisA.push("hellol", "1234", nullptr);

	//只有客户端B收到回调
	redisB.set("helloA", "worldA", nullptr);
	redisB.set("helloBCD", "worldABCD", nullptr);

	//只有客户端C收到回调
	redisC.set("helloA", "worldA", nullptr);
	redisC.set("helloBCD", "worldABCD", nullptr);

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

	redisA.pull("lhello*", subCBA);
	redisB.pull("lhello*", subCBB);
	redisC.pull("lhello*", subCBC);
	
	//只有客户端A收到回调
	redisA.push("lhello123", "1", nullptr);
	redisA.unpull("lhello*");		//取消拉取lhello*
	redisA.push("lhello456", "2", nullptr);
	//只能收到list类型的回调
	redisA.set("lhello000", "stringtype", nullptr);
	
	//只有客户端B收到回调
	redisB.push("lhello789", "3", nullptr);
	redisB.push("lhello012", "4", nullptr);

	//只有客户端C收到回调
	redisC.push("lhello345", "5", nullptr);
	redisC.push("lhello678", "6", nullptr);
	getchar();
}
void subCBA(const char *key, const char *value)
{
	DEBUGLOG << "clientA got subs msg!!!";
	if (strlen(value) == 0)
		DEBUGLOG << "key = " << key << ", was deleted..." << endl;
	else
		DEBUGLOG << "got subcb msg, key = " << key << ", value = " << value << endl;
}
void subCBB(const char *key, const char *value)
{
	DEBUGLOG << "clientB got subs msg!!!";
	if (strlen(value) == 0)
		DEBUGLOG << "key = " << key << ", was deleted..." << endl;
	else
		DEBUGLOG << "got subcb msg, key = " << key << ", value = " << value << endl;
}
void subCBC(const char *key, const char *value)
{
	DEBUGLOG << "clientC got subs msg!!!";
	if (strlen(value) == 0)
		DEBUGLOG << "key = " << key << ", was deleted..." << endl;
	else
		DEBUGLOG << "got subcb msg, key = " << key << ", value = " << value << endl;
}
void pullCBA(const char *key, const char *value)
{
	DEBUGLOG << "clientA got pull msg!!!";
	if (strlen(value) == 0)
		DEBUGLOG << "key = " << key << ", was deleted..." << endl;
	else
		DEBUGLOG << "got pullcb msg, key = " << key << ", value = " << value << endl;
}
void pullCBB(const char *key, const char *value)
{
	DEBUGLOG << "clientB got pull msg!!!";
	if (strlen(value) == 0)
		DEBUGLOG << "key = " << key << ", was deleted..." << endl;
	else
		DEBUGLOG << "got pullcb msg, key = " << key << ", value = " << value << endl;
}
void pullCBC(const char *key, const char *value)
{
	DEBUGLOG << "clientC got pull msg!!!";
	if (strlen(value) == 0)
		DEBUGLOG << "key = " << key << ", was deleted..." << endl;
	else
		DEBUGLOG << "got pullcb msg, key = " << key << ", value = " << value << endl;
}

void subGetOp()
{
	CRedis_Utils redisA("A");
	CRedis_Utils redisB("B");
	redisA.connect("192.168.31.217", 6379, true);
	redisB.connect("192.168.31.217", 6379, true);
	char msg[256];
	memset(msg, 0, 256);

	redisA.subsClientGetOp("gethelloOp", "gethelloOpreq",
		"gethelloOphb", getCBA);	//注册
	redisA.set("gethelloOphb", int2str(time(NULL)).c_str(), 
		msg);	//定时发送心跳

	memset(msg, 0, 256);
	redisA.get("gethelloOp", msg);
	//redisB.get("gethelloOp", msg);
	DEBUGLOG << "get result = " << msg;

	getchar();
}
void getCBA(const char *key, const char *value)
{
	DEBUGLOG << "clientA got get msg!!!";
	CRedis_Utils redis("A");
	redis.connect("192.168.31.217", 6379);
	char msg[256];
	memset(msg, 0, 256);
	//数据处理，处理完成之后调用set接口更新数据
	//通知客户端处理完成
	if (strlen(value) == 0)
		redis.set(key, "nil", msg);
	else
	{
		std::string value_ = value + std::string("getCBA");
		
		redis.set(key, value_.c_str(), msg);
	}
}

void abnormalTest(CRedis_Utils& redis)
{
	redis.get("", nullptr);
	redis.set("", "", nullptr);
	redis.push("", "", nullptr);
	redis.pop("",  nullptr);

	redis.subs("", nullptr);
	redis.unsubs("");
	redis.pull("", nullptr);
	redis.unpull("");

	redis.subsClientGetOp("", "", "", nullptr);
	redis.unsubClientGetOp("");
}

void* multiThread(void *args)
{
	CRedis_Utils *redis = (CRedis_Utils *)args;
	stringstream ss;
	std::string str;
	ss << std::this_thread::get_id();
	ss >> str;
	std::string key = "hellolist" + str;
	char *msg = (char *)malloc(REDIS_BUF_SIZE);
	for (int i = 0; i < 10; ++i)
	{
		memset(msg, 0, REDIS_BUF_SIZE);
		if (redis->pop(key.c_str(), msg) >= 0)
			DEBUGLOG << "set op succ!!! msg = " << msg << endl;
		else
			ERRORLOG << "set op fail!!! err = " << msg << endl;
	}
	free(msg);
	return nullptr;
}
