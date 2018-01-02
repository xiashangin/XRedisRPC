#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include "Logger.h"

using namespace std;

class CMyLogger
{
public:
	static CMyLogger * getInstance();
	std::ostringstream *m_globalLogOss;
private:
	CMyLogger();
	~CMyLogger();
	static CMyLogger * m_myLogger;
};

