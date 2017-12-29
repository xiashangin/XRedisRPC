#include "CacheUtils.h"
#include <thread>
#include <sstream>

std::string int2str(const int &int_temp)
{
	std::string str;
	std::stringstream st;
	st << int_temp;
	st >> str;
	return str;
}

std::ostringstream logInfo;
void setLog(int iLogType, std::ostringstream & strLogInfo)
{
	CCacheUtils::log(iLogType, strLogInfo.str());
	strLogInfo.str("");
}

#define REDISIP		"192.168.31.217"
#define REDISPORT	6379

void test_set(CCacheUtils & redis)
{
	std::string msg;
	for (int i = 0; i < 10; ++i)
	{
		std::string key_ = "!@##%%$" + int2str(i);
		std::string value = "!@##%%$" + int2str(i);
		int iRlt = redis.set(key_, value, msg);
		if (iRlt == 0)
			logInfo << "set op succ!!! msg = " << msg.c_str();
		else
			logInfo << "set op fail!!! errType = " << iRlt << "err = " << msg.c_str();
		setLog(LOG_DEBUG, logInfo);
		getchar();
	}
}
void test_get(CCacheUtils & redis)
{
	std::string msg;
	for (int i = 0; i < 10; ++i)
	{
		std::string key_ = "hellosadsdasa";
		int iRlt = redis.get(key_, msg);
		if (iRlt == 0)
			logInfo << "get op succ!!! msg = " << msg.c_str();
		else
			logInfo << "get op fail!!! errType = " << iRlt << "err = " << msg.c_str();
		setLog(LOG_DEBUG, logInfo);
	}
}
void test_push(CCacheUtils & redis)
{
	std::string msg;
	for (int i = 0; i < 10; ++i)
	{
		std::string key_ = "hello";
		std::string value = "world" + int2str(i);
		int iRlt = redis.push(key_, value, msg);
		if (iRlt == 0)
			logInfo << "push op succ!!! size = " << msg.c_str();
		else
			logInfo << "push op fail!!! errType = " << iRlt << "err = " << msg.c_str();
		setLog(LOG_DEBUG, logInfo);
	}
}
void test_pop(CCacheUtils & redis)
{
	std::string msg;
	for (int i = 0; i < 11; ++i)
	{
		std::string key_ = "hello";
		int iRlt = redis.pop(key_, msg);
		if (iRlt == 0)
			logInfo << "pop op succ!!! value = " << msg.c_str();
		else
			logInfo << "pop op fail!!! errType = " << iRlt << "err = " << msg.c_str();
		setLog(LOG_DEBUG, logInfo);
	}
}

void subCBA(const std::string & strKey, const std::string & strValue)
{
	logInfo << "clientA got subs msg!!!";
	setLog(LOG_DEBUG, logInfo);
	if (strKey.length() == 0)
		logInfo << "key = " << strKey.c_str() << ", was deleted...";
	else
		logInfo << "got subcb msg, key = " << strKey.c_str() << ", value = " << strValue.c_str();
	setLog(LOG_DEBUG, logInfo);
}

void pullCBA(const std::string & strKey, const std::string & strValue)
{
	logInfo << "clientA got pull msg!!!";
	setLog(LOG_DEBUG, logInfo);
	if (strKey.length() == 0)
		logInfo << "key = " << strKey.c_str() << ", was deleted...";
	else
		logInfo << "got subcb msg, key = " << strKey.c_str() << ", value = " << strValue.c_str();
	setLog(LOG_DEBUG, logInfo);
}

void getCBA(const std::string & key, const std::string & value)
{
	//DEBUGLOG("clientA got get msg!!!");
	//CRedis_Utils redis("A");
	//redis.connect("192.168.31.217", 6379);
	//std::string msg;	//数据处理，处理完成之后调用set接口更新数据
	//std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	////通知客户端处理完成
	//if (value.length() == 0)
	//	redis.notifyRlt(key, "nil");
	//else
	//{
	//	std::string value_ = value + std::string("getCBA");
	//	
	//	redis.notifyRlt(key, value_.c_str());
	//}
}


#include "json/ParseHealthData.h"
#define HEALTHDATAFILE	"C:\\workspace\\libXglRedis\\XglRedisTest\\healthData"
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <chrono>
#define LEN 1024
// 深度优先递归遍历目录中所有的文件
//将单字节char*转化为宽字节wchar_t*  
wchar_t* AnsiToUnicode(const char* szStr)
{
	int nLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szStr, -1, NULL, 0);
	if (nLen == 0)
	{
		return NULL;
	}
	wchar_t* pResult = new wchar_t[nLen];
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szStr, -1, pResult, nLen);
	return pResult;
}

//将宽字节wchar_t*转化为单字节char*  
char* UnicodeToAnsi(const wchar_t* szStr)
{
	int nLen = WideCharToMultiByte(CP_ACP, 0, szStr, -1, NULL, 0, NULL, NULL);
	if (nLen == 0)
	{
		return NULL;
	}
	char* pResult = new char[nLen];
	WideCharToMultiByte(CP_ACP, 0, szStr, -1, pResult, nLen, NULL, NULL);
	return pResult;
}
BOOL DirectoryList(LPCSTR Path, std::vector<std::string> & vecFile)
{
	WIN32_FIND_DATA FindData;
	HANDLE hError;
	int FileCount = 0;
	char FilePathName[LEN];
	// 构造路径
	//char FullPathName[LEN];
	strcpy(FilePathName, Path);
	strcat(FilePathName, "\\*.*");
	hError = FindFirstFile(AnsiToUnicode(FilePathName), &FindData);
	if (hError == INVALID_HANDLE_VALUE)
	{
		printf("搜索失败!");
		return 0;
	}
	while (::FindNextFile(hError, &FindData))
	{
		// 过虑.和..
		if (strcmp(UnicodeToAnsi(FindData.cFileName), ".") == 0
			|| strcmp(UnicodeToAnsi(FindData.cFileName), "..") == 0)
		{
			continue;
		}
		//logInfo << UnicodeToAnsi(FindData.cFileName);
		//setLog(LOG_DEBUG, logInfo);
		vecFile.push_back(UnicodeToAnsi(FindData.cFileName));
		//// 构造完整路径
		//wsprintf(AnsiToUnicode(FullPathName), AnsiToUnicode("%s\\%s"), Path, FindData.cFileName);
		//FileCount++;
		//// 输出本级的文件
		//printf("\n%d %s ", FileCount, FullPathName);

		//if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		//{
		//	printf("<Dir>");
		//	DirectoryList(FullPathName);
		//}
	}
	return true;
}
#include <mutex>
std::mutex testLock;
void parseTest(std::string strFileName, std::string strKey, CCacheUtils & redis);
void* multiThread(void *args)
{
	CCacheUtils *redis = (CCacheUtils *)args;

	std::stringstream ss;
	std::string strPid;
	ss << std::this_thread::get_id();
	ss >> strPid;
	testLock.lock();
	std::vector<std::string> vecFile;
	DirectoryList(HEALTHDATAFILE, vecFile);
	testLock.unlock();
	while (1)
	{
		std::vector<std::string>::iterator iter = vecFile.begin();
		for (; iter != vecFile.end(); ++iter)
		{
			std::string strKey = strPid + *iter;
			std::string strFileName = std::string(HEALTHDATAFILE) + std::string("\\") + *iter;
			parseTest(strFileName, strKey, *redis);
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}
	}
	return nullptr;
}

std::string changePersonId(const std::string & strJson, const std::string & strPersonId)
{
	Json::CharReaderBuilder rBuilder;
	Json::CharReader* reader(rBuilder.newCharReader());
	Json::Value jsonRoot;
	JSONCPP_STRING errs;
	const char* lpStrJson = strJson.c_str();
	bool is_ok = reader->parse(lpStrJson, lpStrJson + strlen(lpStrJson), &jsonRoot, &errs);
	if (!is_ok || errs.size() > 0) //从ifs中读取数据到jsonRoot
	{
		logInfo << "parse Json file failed!!! errstr = " << errs << std::endl;
		setLog(LOG_DEBUG, logInfo);
		return "";
	}

	jsonRoot[PERSON_ID] = strPersonId + strPersonId;
	Json::StreamWriterBuilder wBuilder;
	wBuilder.settings_["indentation"] = "";
	std::unique_ptr<Json::StreamWriter> writer(wBuilder.newStreamWriter());

	std::stringstream ss;
	writer->write(jsonRoot, &ss);

	return ss.str();
}

int succCnt = 0;
int failCnt = 0;
int insertCnt = 0;
void parseTest(std::string strFileName, std::string strKey, CCacheUtils & redis)
{
	//CParseHealthData parseUtil;
	//std::stringstream ss;
	//std::string strPid;
	//ss << std::this_thread::get_id();
	//ss >> strPid;

	//testLock.lock();
	//std::string strJson = parseUtil.readFileIntoString(strFileName.c_str());
	////strJson = changePersonId(strJson, strPid);
	//testLock.unlock();

	//std::string strRlt;
	//int iRlt = redis.set(strKey, strJson, strRlt);
	//
	//insertCnt++;
	//if (iRlt == 0)
	//	succCnt++;
	//else
	//	failCnt++;

	////std::cout << "[" << strKey << "]" << strRlt; 

	//if(insertCnt % 100 == 0)
	//{
	//	logInfo << "strRlt = [" << strKey << "]" << strRlt <<
	//		",succCnt = " << succCnt << ", failCnt = " << failCnt << std::endl;
	//	setLog(LOG_DEBUG, logInfo);
	//}
}