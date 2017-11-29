#pragma once

#include "stdafx.h"

template<class T>
std::string num2str ( const T num );
std::string int2str ( const int &int_temp );
//string long2str ( const unsigned __int64 &double_temp );
std::string double2str ( const double &double_temp );

vector<std::string> split(std::string str, std::string pattern);
bool keyMatch(const std::string srcStr, const std::string & strPattern);