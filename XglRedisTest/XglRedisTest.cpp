// XglRedisTest.cpp : 定义控制台应用程序的入口点。
//

#include "Redis_Utils.h"
#pragma comment(lib, "libXglRedis.lib")

void subsCb1(const char *key, const char *value)
{
	DEBUGLOG << "key = " << key << ", value = " << value << endl;
}

void subsCb2(const char *key, const char *value)
{
	DEBUGLOG << "key = " << key << ", value = " << value << endl;
}

int main(int argc, char const *argv[])
{
	CRedis_Utils redis("aclient");
	redis.connect("192.168.253.128", 6379, true);
	//redis.disconnect();
	//redis.connect("192.168.253.128", 6379, true);

	//????sub 和 pull 订阅相同的情况！！！！
	//redis.subs("hello", subsCb1);
	redis.pull("hexu", subsCb2);
	std::this_thread::sleep_for(std::chrono::milliseconds(500));

	//正常基本功能
	char *msg = (char *)malloc(REDIS_BUF_SIZE);
	//memset(msg, 0, REDIS_BUF_SIZE);
	//DEBUGLOG << "rlt = " << redis.set("hello", "dsklfjfdisewrkdkjfshf", msg) << "..." << msg << endl;
	//memset(msg, 0, REDIS_BUF_SIZE);
	//DEBUGLOG << "rlt = " << redis.set("hello", "dsklfjfdisewrkdkjfshf", msg) << "..." << msg << endl;
	//memset(msg, 0, REDIS_BUF_SIZE);
	//DEBUGLOG << "rlt = " << redis.get("hexu", msg) << "..." << msg << endl;
	memset(msg, 0, REDIS_BUF_SIZE);
	DEBUGLOG << "rlt = " << redis.push("hexu", "bilibili", msg) << "..." << msg << endl;
	//memset(msg, 0, REDIS_BUF_SIZE);
	//DEBUGLOG << "rlt = " << redis.pop("kdjshfkjsdf1", msg) << "..." << msg << endl;


	//redis.pull("hello", subsCb1);
	//redis.pull("hello", subsCb2);

	getchar();
	return 0;
}
