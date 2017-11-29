// XglRedisTest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include "Redis_Utils.h"
#pragma comment(lib, "libXglRedis.lib")

void subsCb1(const char *key)
{
	cout << "subsCb1 get respond-->" << key << endl;
}
void subsCb2(const char *key)
{
	cout << "subsCb2 get respond-->" << key << endl;
}


void pullCb1(const char *key)
{
	cout << "pullCb1 get respond-->" << key << endl;
}

void pullCb2(const char *key)
{
	cout << "pullCb2 get respond-->" << key << endl;
}

int main()
{
	CRedis_Utils redis;
	redis.connect("192.168.31.170", 6379);
	char msg[256];
	memset(msg, 0, 256);

	redis.subs("hello?", subsCb1);
	redis.subs("hello*ac*", subsCb2);

	redis.pull("hello?", pullCb1);
	redis.pull("hello*ac*", pullCb2);

	getchar();
	redis.close();
    return 0;
}

