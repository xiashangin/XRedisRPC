#include "RedisUtils.h"
#include "ClientCacheUtils.h"


CClientCacheUtils::CClientCacheUtils()
{
	m_redisUtil = new CRedis_Utils("");
	if (g_ECGLogger == nullptr)
		g_ECGLogger = CMyLogger::getInstance();
}


CClientCacheUtils::~CClientCacheUtils()
{
	if (m_redisUtil != nullptr)
	{
		delete (CRedis_Utils *)m_redisUtil;
		m_redisUtil = nullptr;
	}
}

bool CClientCacheUtils::connect(const std::string & strIp, int iPort, bool bNeedSubs /*= false*/)
{
	return ((CRedis_Utils *)m_redisUtil)->connect(strIp, iPort, bNeedSubs);
}

void CClientCacheUtils::disconnect()
{
	((CRedis_Utils *)m_redisUtil)->disconnect();
}

int CClientCacheUtils::get(const std::string & strClientId, const std::string & strInKey, std::string & strOutResult)
{
	((CRedis_Utils *)m_redisUtil)->setClientId(strClientId);
	return ((CRedis_Utils *)m_redisUtil)->get(strInKey, strOutResult);
}

int CClientCacheUtils::set(const std::string & strClientId, const std::string & strInKey, const std::string & strInValue, std::string & strOutResult)
{
	((CRedis_Utils *)m_redisUtil)->setClientId(strClientId);
	return ((CRedis_Utils *)m_redisUtil)->set(strInKey, strInValue, strOutResult);
}

int CClientCacheUtils::push(const std::string & strClientId, const std::string & strInListName, const std::string & strInValue, std::string & strOutResult)
{
	((CRedis_Utils *)m_redisUtil)->setClientId(strClientId);
	return ((CRedis_Utils *)m_redisUtil)->push(strInListName, strInValue, strOutResult);
}

int CClientCacheUtils::pop(const std::string & strClientId, const std::string & strInListName, std::string & strOutResult)
{
	((CRedis_Utils *)m_redisUtil)->setClientId(strClientId);
	return ((CRedis_Utils *)m_redisUtil)->pop(strInListName, strOutResult);
}

int CClientCacheUtils::subs(const std::string & strClientId, const std::string & strInKey, subsCallback cb)
{
	((CRedis_Utils *)m_redisUtil)->setClientId(strClientId);
	return ((CRedis_Utils *)m_redisUtil)->subs(strInKey, cb);
}

bool CClientCacheUtils::unsubs(const std::string & strClientId, const std::string & strInKey)
{
	((CRedis_Utils *)m_redisUtil)->setClientId(strClientId);
	return ((CRedis_Utils *)m_redisUtil)->unsubs(strInKey);
}

int CClientCacheUtils::pull(const std::string & strClientId, const std::string & strInKey, pullCallback cb)
{
	((CRedis_Utils *)m_redisUtil)->setClientId(strClientId);
	return ((CRedis_Utils *)m_redisUtil)->pull(strInKey, cb);
}

bool CClientCacheUtils::unpull(const std::string & strClientId, const std::string & strInKey)
{
	((CRedis_Utils *)m_redisUtil)->setClientId(strClientId);
	return ((CRedis_Utils *)m_redisUtil)->unpull(strInKey);
}

void CClientCacheUtils::log(const int iLogType, const std::string & strLog)
{
	switch (iLogType)
	{
	case LOG_INFO:
		_INFOLOG(strLog.c_str());
		break;
	case LOG_TRACE:
		_TRACELOG(strLog.c_str());
		break;
	case LOG_DEBUG:
		_DEBUGLOG(strLog.c_str());
		break;
	case LOG_WARN:
		_WARNLOG(strLog.c_str());
		break;
	case LOG_ERROR:
		_ERRORLOG(strLog.c_str());
		break;
	case LOG_FATAL:
		_FATALLOG(strLog.c_str());
		break;
	default:
		_DEBUGLOG(strLog.c_str());
		break;
	}
}
