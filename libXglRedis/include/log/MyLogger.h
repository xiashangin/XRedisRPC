#pragma once

#include <iostream>
#include <string>
#include <log4cplus/logger.h>
#include <log4cplus/configurator.h> 
#include <log4cplus/layout.h> 
#include <log4cplus/loggingmacros.h> 
#include <log4cplus/helpers/stringhelper.h> 

#define MY_LOG_FILE_PATH  "./logconfig.properites"
#define MY_LOG__FILE_PATH  "./log4cplus.properties"
using namespace std;
using namespace log4cplus;
using namespace log4cplus::helpers;

class CMyLogger
{
public:
	

public:
	static CMyLogger * getInstance();
	Logger logger;
private:
	CMyLogger();
	~CMyLogger();
	static CMyLogger * my_logger;
};

