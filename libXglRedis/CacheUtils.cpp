#include "RedisUtils.h"
#include "CacheUtils.h"


CCacheUtils::CCacheUtils(std::string strClientId)
{
	m_redisUtil = new CRedis_Utils(strClientId);
	if (g_ECGLogger == nullptr)
		g_ECGLogger = CMyLogger::getInstance();
}


CCacheUtils::~CCacheUtils()
{
	if (m_redisUtil != nullptr)
	{
		delete (CRedis_Utils *)m_redisUtil;
		m_redisUtil = nullptr;
	}
}

bool CCacheUtils::connect(const std::string & strIp, int iPort, bool bNeedSubs /*= false*/)
{
	return ((CRedis_Utils *)m_redisUtil)->connect(strIp, iPort, bNeedSubs);
}

void CCacheUtils::disconnect()
{
	((CRedis_Utils *)m_redisUtil)->disconnect();
}

int CCacheUtils::get(const std::string & strInKey, std::string & strOutResult)
{
	return ((CRedis_Utils *)m_redisUtil)->get(strInKey, strOutResult);
}

int CCacheUtils::set(const std::string & strInKey, const std::string & strInValue, std::string & strOutResult)
{
	return ((CRedis_Utils *)m_redisUtil)->set(strInKey, strInValue, strOutResult);
}

int CCacheUtils::push(const std::string & strInListName, const std::string & strInValue, std::string & strOutResult)
{
	return ((CRedis_Utils *)m_redisUtil)->push(strInListName, strInValue, strOutResult);
}

int CCacheUtils::pop(const std::string & strInListName, std::string & strOutResult)
{
	return ((CRedis_Utils *)m_redisUtil)->pop(strInListName, strOutResult);
}

int CCacheUtils::subs(const std::string & strInKey, subsCallback cb)
{
	return ((CRedis_Utils *)m_redisUtil)->subs(strInKey, cb);
}

bool CCacheUtils::unsubs(const std::string & strInKey)
{
	return ((CRedis_Utils *)m_redisUtil)->unsubs(strInKey);
}

int CCacheUtils::pull(const std::string & strInKey, pullCallback cb)
{
	return ((CRedis_Utils *)m_redisUtil)->pull(strInKey, cb);
}

bool CCacheUtils::unpull(const std::string & strInKey)
{
	return ((CRedis_Utils *)m_redisUtil)->unpull(strInKey);
}

int CCacheUtils::subsClientGetOp(const std::string & strInKey, clientOpCallBack cb)
{
	return ((CRedis_Utils *)m_redisUtil)->subsClientGetOp(strInKey, cb);
}

bool CCacheUtils::unsubClientGetOp(const std::string & strInKey)
{
	return ((CRedis_Utils *)m_redisUtil)->unsubClientGetOp(strInKey);
}

void CCacheUtils::stopSubClientGetOp()
{
	((CRedis_Utils *)m_redisUtil)->stopSubClientGetOp();
}

int CCacheUtils::notifyRlt(const std::string & strInKey, const std::string & strInValue)
{
	return ((CRedis_Utils *)m_redisUtil)->notifyRlt(strInKey, strInValue);
}

void CCacheUtils::log(const int iLogType, const std::string & strLog)
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

