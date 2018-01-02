#pragma once

#include <time.h>
#include <string.h>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <numeric>
#include <sstream>
#include <string>
#include <thread>
#include <vector>


using namespace std;

extern "C" {
#include "include/uuid/uuid4.h"
}


//#include "include/log/Logger.h"
#include "include/log/MyLogger.h"

extern CMyLogger *g_ECGLogger;


template<class T>
std::string num2str ( const T num );
std::string time2str(time_t time);
std::string int2str ( const int &int_temp );
//string long2str ( const unsigned __int64 &double_temp );
std::string double2str ( const double &double_temp );

vector<std::string> split(std::string str, std::string pattern);

// 定义64位整形
#if defined(_WIN32) && !defined(CYGWIN)
typedef __int64 int64_t;
#else
typedef long long int64t;
#endif  // _WIN32
int64_t GetSysTimeMicros();

bool keyMatch(const std::string srcStr, const std::string & strPattern);

std::string generate_uuid();