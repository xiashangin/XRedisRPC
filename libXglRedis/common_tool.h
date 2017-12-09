#pragma once

#include <time.h>
#include <string.h>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

using namespace std;

extern "C" {
#include "include/uuid/uuid4.h"
}


template<class T>
std::string num2str ( const T num );
std::string time2str(time_t time);
std::string int2str ( const int &int_temp );
//string long2str ( const unsigned __int64 &double_temp );
std::string double2str ( const double &double_temp );

vector<std::string> split(std::string str, std::string pattern);
bool keyMatch(const std::string srcStr, const std::string & strPattern);

std::string generate_uuid();