#include "MyLogger.h"
CMyLogger *CMyLogger::m_myLogger = NULL;

CMyLogger::CMyLogger()
{
	//log4cplus::initialize();
	//PropertyConfigurator::doConfigure(LOG4CPLUS_TEXT(MY_LOG__FILE_PATH));
	//logger = Logger::getRoot();
	if(g_pLogger == NULL)
		InitialLog();
	m_globalLogOss = new std::ostringstream;
}


CMyLogger * CMyLogger::getInstance()
{
	if (m_myLogger == NULL)
	{
		m_myLogger = new CMyLogger();
	}
	return m_myLogger;
}

CMyLogger::~CMyLogger()
{
	if (m_myLogger)
	{
		delete m_myLogger;
		m_myLogger = nullptr;
	}
	if (g_pLogger != NULL)
		DeleteAndCloseLog();
	if(m_globalLogOss != nullptr)
	{
		delete m_globalLogOss;
		m_globalLogOss = nullptr;
	}
}