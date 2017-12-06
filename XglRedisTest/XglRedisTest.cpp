// XglRedisTest.cpp : 定义控制台应用程序的入口点。
//

#include "Redis_Utils.h"
#pragma comment(lib, "libXglRedis.lib")

int main(int argc, char const *argv[])
{
	CRedis_Utils redis("aclient");
	getchar();
	return 0;
}
