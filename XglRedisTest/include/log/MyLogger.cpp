#include "MyLogger.h"

CMyLogger *CMyLogger::my_logger = NULL;

CMyLogger::CMyLogger()
{
	log4cplus::initialize();
	PropertyConfigurator::doConfigure(LOG4CPLUS_TEXT(MY_LOG__FILE_PATH));
	logger = Logger::getRoot();
}


CMyLogger * CMyLogger::getInstance()
{
	if (my_logger == NULL)
	{
		my_logger = new CMyLogger();
	}
	return my_logger;
}

CMyLogger::~CMyLogger()
{
	if (my_logger)
	{
		delete my_logger;
	}
}