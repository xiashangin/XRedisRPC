// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once


#include <stdio.h>
#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>
#include <signal.h>  
#include <time.h>

#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <deque>
#include <map>
#include <vector>
#include <algorithm>
using namespace std;

#ifdef _WIN32
#include <process.h>
#define sleep(x) Sleep((x)*1000)
#else
#include <unistd.h>
#endif

#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#ifdef _WIN32
#include <Win32_Interop/win32fixes.h>
extern "C"
{
#include <hiredis/adapters/ae.h>
}
#else
#include <hiredis/adapters/libevent.h>
#endif

//客户端指令
#define GET			"GET"
#define SET			"SET"
#define SUBS		"SUBSCRIBE"
#define UNSUB		"UNSUBSCRIBE"
#define PUSH		"PUSH"
#define POP			"POP"
#define PULL		"PULL"

//redis命令
#define R_GET		"GET"
#define R_SET		"SET"
#define R_LPUSH		"LPUSH"
#define R_RPOP		"RPOP"
#define R_SUBS		"PSUBSCRIBE"

#define BUF_SIZE 1000

extern deque<std::string> zmqPubContent;
extern deque<std::string> zmqPushContent;


// TODO:  在此处引用程序需要的其他头文件
