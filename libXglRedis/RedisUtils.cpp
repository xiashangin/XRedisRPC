#include "RedisUtils.h"

CRedis_Utils::CRedis_Utils(const std::string & strClientID)
{
	this->m_bIsConnected = false;
	//this->m_strLastGetKey = "";
	if (strClientID.length() > 0)
		this->m_strClientId = strClientID;
	else
		this->m_strClientId = DEFAULT_CLIENTID;
	m_redisRPC.setClientId(m_strClientId);
}

CRedis_Utils::~CRedis_Utils()
{
	close();
}

bool CRedis_Utils::connect(const std::string & strIp, int port, bool isSubs)
{
	//DEBUGLOG("connect to redis server ip = " << this->m_strIp.c_str() << ", port = "
	//	<< this->m_iPort << ", isSubs = " << this->m_bNeedSubs);
	if(m_bIsConnected) 
	{
		_DEBUGLOG("redis �����Ѿ����ӳɹ����ظ�����... ip = " << strIp.c_str() 
			<< ", port = " << port);
		return true;
	}
	this->m_strIp = strIp;
	this->m_iPort = port;
	this->m_bNeedSubs = isSubs;
	//m_redisRPC = CRedisRPC(strIp, port);
	m_redisRPC.setRedisAddr(strIp, port);
	//����redis����
	_DEBUGLOG("connect to redis server ip = " << this->m_strIp.c_str() << ", port = "
		<< this->m_iPort << ", isSubs = " << this->m_bNeedSubs);
	if (!_connect(strIp, port))
		return false;
	
	//����redis���ռ�֪ͨ
	if(m_bNeedSubs)
	{
		_DEBUGLOG("subscribe redis keyspace notification!!!");
		thAsyncKeyNotify = std::thread(CRedis_Utils::thAsyncSubsAll, this);
#ifndef _WIN32
		thAsyncKeyNotify.detach();
#endif
	}
	this->m_bIsConnected = true;		//���ӳɹ�
	return true;
}

void CRedis_Utils::disconnect()
{
	//if(this->needSubs && !pRedisAsyncContext)
	//	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	//close();
	//delete this;
	this->m_bIsConnected = false;
	if (this->m_bNeedSubs)
	{
		this->m_bNeedSubs = false;
		m_subsLock.lock(); 
		if (m_mapSubsKeys.size() > 0)
			m_mapSubsKeys.clear();
		m_subsLock.unlock();
		m_pullLock.lock();
		if (m_mapPullKeys.size() > 0)
			m_mapPullKeys.clear();
		m_redisRPC.clearChnl();
		m_pullLock.unlock();
	}
}

void* CRedis_Utils::thAsyncSubsAll(void *arg)
{
	CRedis_Utils *self = (CRedis_Utils *)arg;
#ifdef _WIN32
	/* For Win32_IOCP the event loop must be created before the async connect */
	self->m_loop = aeCreateEventLoop(1024 * 10);
#else
	signal(SIGPIPE, SIG_IGN);
	self->m_base = event_base_new();
#endif
	self->m_pRedisAsyncContext = redisAsyncConnect(self->m_strIp.c_str(), self->m_iPort);
	if (self->m_pRedisAsyncContext->err) {
		printf("Error: %s\n", self->m_pRedisAsyncContext->errstr);
		return NULL;
	}
#ifdef _WIN32
	redisAeAttach(self->m_loop, self->m_pRedisAsyncContext);
#else
	redisLibeventAttach(self->m_pRedisAsyncContext, self->m_base);
#endif

	redisAsyncSetConnectCallback(self->m_pRedisAsyncContext, CRedis_Utils::connectCallback);
	redisAsyncSetDisconnectCallback(self->m_pRedisAsyncContext, CRedis_Utils::disconnectCallback);
	std::string cmd = std::string(R_PSUBS) + std::string(" ") + std::string(R_KEYSPACE) + std::string("*");
	//˫�ر��ϣ������һ��ʧ�ܣ����еڶ��γ��ԡ�
	int iRet = redisAsyncCommand(self->m_pRedisAsyncContext, subsAllCallback, self, cmd.c_str()/*"PSUBSCRIBE __keyspace@0__:*"*/);
	if(iRet == REDIS_OK)
	{ 
		_DEBUGLOG("subscribe redis keyspace notification success!!! cmd = " << cmd.c_str());
	}
	else
	{
		_WARNLOG("subscribe redis keyspace notification failed!!! try again!!! cmd = " << cmd.c_str());
		iRet = redisAsyncCommand(self->m_pRedisAsyncContext, subsAllCallback, self, cmd.c_str());
		if (iRet == REDIS_OK)
		{
			_DEBUGLOG("subscribe redis keyspace notification success!!! cmd = " << cmd.c_str());
		}
		else
		{
			_ERRORLOG("subscribe redis keyspace notification failed!!!");
			return nullptr;
		}
	}
	//_DEBUGLOG("subscribe redis keyspace notification!!! cmd = " << cmd.c_str());

#ifdef _WIN32
	aeMain(self->m_loop);
#else
	event_base_dispatch(self->m_base);
#endif
	_DEBUGLOG("event dispatch done!!! --> " << self->m_strClientId.c_str());
	return nullptr;
}

int CRedis_Utils::get(const std::string & strInKey, std::string & strOutResult)
{
	if (!m_bIsConnected)
	{
		_DEBUGLOG("redis ������δ����...������������... ip = " << m_strIp.c_str() 
				<< ", port = " << m_iPort);
		if(!_connect(m_strIp.c_str(), m_iPort))
		{
			_WARNLOG("redis������������ʧ��... ");
			strOutResult = "redis is not connected...";
			return REDIS_CONNFAIL;
		}
	}
	m_getLock.lock();
	bool bRlt = false;
	std::string new_key = genNewKey(strInKey.c_str());
	std::string cmd = std::string(R_GET) + std::string(" ") + std::string(new_key);
	//_DEBUGLOG("get cmd = " << cmd.c_str());
	
	//ͬ������ҵ����ģ���Ѷ��ĵ�����key
	std::string subsKey;
	std::map<std::string, int> allReq = getAllReqs(GLOBALREQKEYS);
	if (allReq.size() > 0)
	{
		std::map<std::string, int>::iterator map_iter = allReq.begin();
		for (; map_iter != allReq.end(); ++map_iter)
		{
			if (map_iter->second > 0)
			{
				std::string strKey = map_iter->first;
				std::string new_key = genNewKey(strKey);				//��
				std::string heartBeat = new_key + HEARTSLOT;			//��������
				std::string reqChnl = new_key + REQSLOT;				//�������
				subsKey = strKey;
				m_redisRPC.syncReqChnl(new_key.c_str(), reqChnl.c_str(), heartBeat.c_str());
			}
			else
			{
				//����Ҫ�������ü���Ϊ0key���Ѿ���ɾ����
			}
		}
	}

	if(!m_redisRPC.isServiceModelAvailable(new_key.c_str()) /*|| !m_redisRPC.isKeySubs(new_key.c_str())*/)
	{
		//getʧ��(nilҲ��ʾʧ��) ��keyֵ����Ҫ�������û�п��÷���
		//ֱ�ӷ�������
		if(subsKey.length() > 0)
		{
			std::string strRlt;
			syncReq(GLOBALREQKEYS, subsKey, false, strRlt);
		}
		_DEBUGLOG("no available service-->" << new_key.c_str());
		bRlt = sendCmd(cmd, strOutResult);
		m_getLock.unlock();
		if (bRlt && strOutResult.length() > 0)
			return 0;
		if (bRlt && strOutResult.length() <= 0)
			return REDIS_KEY_NOT_EXIST;
		return REDIS_NOSERVICE;
	} 



	//Զ�̵��ô�������
	int iRlt = m_redisRPC.processKey(new_key.c_str());

	//��������
	bRlt = sendCmd(cmd, strOutResult);
	m_getLock.unlock();
	if (bRlt && strOutResult.length() <= 0)		//����redis���Ҳ���
		return REDIS_KEY_NOT_EXIST;
	return iRlt;

}

int CRedis_Utils::set(const std::string & strInKey, const std::string & strInValue, std::string & strOutResult)
{
	if (!m_bIsConnected)
	{
		_DEBUGLOG("redis ������δ����...������������... ip = " << m_strIp.c_str()
			<< ", port = " << m_iPort);
		if (!_connect(m_strIp.c_str(), m_iPort))
		{
			_WARNLOG("redis������������ʧ��... ");
			strOutResult = "redis is not connected...";
			return REDIS_CONNFAIL;
		}
	}
	m_setLock.lock();
	bool bRlt = false;
	std::string new_key = genNewKey(strInKey);
	std::string cmd = std::string(R_PUSH) + COMMAND_SPLIT
		+ new_key + SET_KEY_SUFFIX + COMMAND_SPLIT + strInValue;

	//�����ʱ����
	//std::string getHBCmd = std::string(R_GET) + std::string(" ")
	//	+ new_key + SET_KEY_SUFFIX + SET_KEY_TIMESTAMP;
	//std::string strTimestamp = "";
	//sendCmd(cmd, strTimestamp);
	//if((time(nullptr) - atoi(strTimestamp.c_str())) > GET_WAITTIMEOUT)


	int iQueueLen = updateReqHB(new_key + SET_KEY_SUFFIX, true);
	if (iQueueLen >= SETQUEUEMAXSIZE)			//�������������
	{
		strOutResult = "service is busy...";
		m_setLock.unlock();
		return REDIS_SERVICE_BUSY;
	}


	//_DEBUGLOG("set cmd = " << cmd.c_str());
	bRlt = sendCmd(cmd, strOutResult);
	m_setLock.unlock();
	if (bRlt)
		return 0;
	else
		return REDIS_CMD_ERR;
}

int CRedis_Utils::push(const std::string & strInListName, const std::string & strInValue, std::string & strOutResult)
{
	if (!m_bIsConnected)
	{
		_DEBUGLOG("redis ������δ����...������������... ip = " << m_strIp.c_str()
			<< ", port = " << m_iPort);
		if (!_connect(m_strIp.c_str(), m_iPort))
		{
			_WARNLOG("redis������������ʧ��... ");
			strOutResult = "redis is not connected...";
			return REDIS_CONNFAIL;
		}
	}
	m_pushLock.lock();
	std::string new_list_name = genNewKey(strInListName);
	std::string cmd = std::string(R_PUSH) + COMMAND_SPLIT
		+ new_list_name + COMMAND_SPLIT + strInValue;
	//_DEBUGLOG("push cmd = " << cmd.c_str());
	bool bRlt = sendCmd(cmd, strOutResult);
	m_pushLock.unlock();
	if (bRlt)
		return 0;
	else
		return REDIS_CMD_ERR;
}

int CRedis_Utils::pop(const std::string & strInListName, std::string & strOutResult)
{
	if (!m_bIsConnected)
	{
		_DEBUGLOG("redis ������δ����...������������... ip = " << m_strIp.c_str()
			<< ", port = " << m_iPort);
		if (!_connect(m_strIp.c_str(), m_iPort))
		{
			_WARNLOG("redis������������ʧ��... ");
			strOutResult = "redis is not connected...";
			return REDIS_CONNFAIL;
		}
	}
	m_popLock.lock();
	std::string new_list_name = genNewKey(strInListName);
	std::string cmd = std::string(R_POP) + std::string(" ") + new_list_name;
	//_DEBUGLOG("pop cmd = " << cmd.c_str());
	bool bRlt = sendCmd(cmd, strOutResult);
	m_popLock.unlock();
	if (bRlt && strOutResult.length() > 0)
		return 0;
	if (bRlt && strOutResult.length() <= 0)
		return REDIS_KEY_NOT_EXIST;
	return REDIS_CMD_ERR;
}

int CRedis_Utils::subs(const std::string & strInKey, subsCallback cb)
{
	if (strInKey.length() == 0 || cb == nullptr)
	{
		_WARNLOG("key or cb is null... subs failed");
		return REDIS_KEY_NULL;
	}

	std::string new_key = genNewKey(strInKey);
	if (!m_bIsConnected)
	{
		_DEBUGLOG("redis ������δ����...������������... ip = " << m_strIp.c_str()
			<< ", port = " << m_iPort);
		if (!_connect(m_strIp.c_str(), m_iPort))
		{
			_WARNLOG("redis������������ʧ��... ");
			return REDIS_CONNFAIL;
		}
	}
	int iRlt = 0;
	if (m_bNeedSubs)
	{
		m_subsLock.lock();
		mapSubsCB::iterator it = m_mapSubsKeys.find(new_key);
		if (it == m_mapSubsKeys.end())
		{
			m_mapSubsKeys.insert(std::pair<std::string, subsCallback>(new_key, cb));
			_DEBUGLOG("CRedis_Utils::subs ���ĳɹ� key = " << new_key.c_str()
				<< ", size = " << m_mapSubsKeys.size());
		}
		else
		{
			iRlt = REDIS_KEY_EXISTED;
			_DEBUGLOG("CRedis_Utils::subs �ü��Ѿ����� key = " << new_key.c_str());
		}
		m_subsLock.unlock();
	}
	else
	{
		iRlt = REDIS_SUBS_OFF;
		_DEBUGLOG("redis subs function is shutdown!!! needSubs = " << m_bNeedSubs);
	}
	return iRlt;
}

bool CRedis_Utils::unsubs(const std::string & strInKey)
{
	if (strInKey.length() == 0)
	{
		_WARNLOG("key or cb is null... subs failed");
		return false;
	}
	std::string new_key = genNewKey(strInKey);
	if (!m_bIsConnected)
	{
		_DEBUGLOG("redis ������δ����...������������... ip = " << m_strIp.c_str()
			<< ", port = " << m_iPort);
		if (!_connect(m_strIp.c_str(), m_iPort))
		{
			_WARNLOG("redis������������ʧ��... ");
			return false;
		}
	}
	bool bRlt = false;
	m_subsLock.lock();
	mapSubsCB::iterator it = m_mapSubsKeys.find(new_key);
	if (it != m_mapSubsKeys.end())
	{
		m_mapSubsKeys.erase(new_key);
		_DEBUGLOG("CRedis_Utils::unsubs ȡ�����ĳɹ� key = " << new_key.c_str()
			<< ", size = " << m_mapSubsKeys.size());
		bRlt = true;
	}
	else
		_DEBUGLOG("CRedis_Utils::unsubs �ü���δ������ key = " << new_key.c_str());
	m_subsLock.unlock();
	return bRlt;
}

int CRedis_Utils::pull(const std::string & strInKey, pullCallback cb)
{
	if (strInKey.length() == 0 || cb == nullptr)
	{
		_WARNLOG("key or cb is null... subs failed");
		return REDIS_KEY_NULL;
	}

	std::string new_key = genNewKey(strInKey);
	if (!m_bIsConnected)
	{
		_DEBUGLOG("redis ������δ����...������������... ip = " << m_strIp.c_str()
			<< ", port = " << m_iPort);
		if (!_connect(m_strIp.c_str(), m_iPort))
		{
			_WARNLOG("redis������������ʧ��... ");
			return REDIS_CONNFAIL;
		}
	}
	bool iRlt = 0;
	if (m_bNeedSubs)
	{
		m_pullLock.lock();
		mapPullCB::iterator it = m_mapPullKeys.find(new_key);
		if (it == m_mapPullKeys.end())
		{
			m_mapPullKeys.insert(std::pair<std::string, pullCallback>(new_key, cb));
			_DEBUGLOG("CRedis_Utils::pull ���ĳɹ� key = " << new_key.c_str()
				<< ", size = " << m_mapPullKeys.size());
		}
		else
		{
			iRlt = REDIS_KEY_EXISTED;
			_DEBUGLOG("CRedis_Utils::pull �ü��Ѿ����� key = " << new_key.c_str());
		}
		m_pullLock.unlock();
	}
	else
	{
		iRlt = REDIS_SUBS_OFF;
		_DEBUGLOG("redis subs function is shutdown!!! needSubs = " << m_bNeedSubs);
	}
	return iRlt;
}

bool CRedis_Utils::unpull(const std::string & strInKey)
{
	if (strInKey.length() == 0)
	{
		_WARNLOG("key is null... unpull failed");
		return false;
	}
	std::string new_key = genNewKey(strInKey);
	if (!m_bIsConnected)
	{
		_DEBUGLOG("redis ������δ����...������������... ip = " << m_strIp.c_str()
			<< ", port = " << m_iPort);
		if (!_connect(m_strIp.c_str(), m_iPort))
		{
			_WARNLOG("redis������������ʧ��... ");
			return false;
		}
	}
	bool bRlt = false;
	m_pullLock.lock();
	mapPullCB::iterator it = m_mapPullKeys.find(new_key);
	if (it != m_mapPullKeys.end())
	{
		m_mapPullKeys.erase(new_key);
		_DEBUGLOG("CRedis_Utils::pull ȡ�����ĳɹ� key = " << new_key.c_str()
			<< ", size = " << m_mapSubsKeys.size());
		bRlt = true;
	}
	else
		_DEBUGLOG("CRedis_Utils::pull �ü��Ѿ����� key = " << new_key.c_str());
	m_pullLock.unlock();
	return bRlt;
}

int CRedis_Utils::subsClientGetOp(const std::string & strInKey, clientOpCallBack cb)
{
	if (strInKey.length() == 0 || cb == nullptr)
	{
		_WARNLOG("key or cb is null... subs failed");
		return REDIS_KEY_NULL;
	}

	if (!m_bIsConnected)
	{
		_DEBUGLOG("redis ������δ����...������������... ip = " << m_strIp.c_str()
			<< ", port = " << m_iPort);
		if (!_connect(m_strIp.c_str(), m_iPort))
		{
			_WARNLOG("redis������������ʧ��... ");
			return REDIS_CONNFAIL;
		}
	}
	bool iRlt = 0;
	if (m_bNeedSubs)
	{
		std::string new_key = genNewKey(strInKey);				//��
		std::string heartBeat = new_key + HEARTSLOT;			//��������
		std::string reqChnl = new_key + REQSLOT;				//�������
		std::string subsReqChnl = reqChnl;// + std::string("*");	

		mapReqCB::iterator it = m_mapReqChnl.find(subsReqChnl);
		m_reqLock.lock();
		if (it == m_mapReqChnl.end())
		{
			std::string strRlt;
			if(syncReq(GLOBALREQKEYS, strInKey, true, strRlt) == 0)
			{
				m_mapReqChnl.insert(std::pair<std::string, pullCallback>(subsReqChnl, cb));
				_DEBUGLOG("subsClientGetOp ���ĳɹ� key = " << subsReqChnl.c_str()
					<< ", size = " << m_mapReqChnl.size());
				m_redisRPC.subsClientGetOp(new_key.c_str(), reqChnl.c_str(), heartBeat.c_str());
			}
			else
			{
				_WARNLOG("subsClientGetOp ����ʧ�� rlt = " << iRlt << "-->" << strRlt.c_str());
				iRlt = REDIS_REQ_SYNC_FAIL;
			}
		}
		else
		{
			iRlt = REDIS_KEY_EXISTED;
			_DEBUGLOG("subsClientGetOp �ü��Ѿ����� key = " << subsReqChnl.c_str());
		}
		m_reqLock.unlock();
	}
	else
		iRlt = REDIS_SUBS_OFF;
	return iRlt;
}

bool CRedis_Utils::unsubClientGetOp(const std::string & strInKey)
{
	if (strInKey.length() == 0)
	{
		_WARNLOG("key is null... unsubClientGetOp failed");
		return false;
	}
	if (!m_bIsConnected)
	{
		_DEBUGLOG("redis ������δ����...������������... ip = " << m_strIp.c_str()
			<< ", port = " << m_iPort);
		if (!_connect(m_strIp.c_str(), m_iPort))
		{
			_WARNLOG("redis������������ʧ��... ");
			return false;
		}
	}
	bool bRlt = false;
	std::string strRlt;
	if ((syncReq(GLOBALREQKEYS, strInKey, false, strRlt) == 0) && m_mapReqChnl.size() > 0)
	{
		std::string new_key = genNewKey(strInKey);
		m_reqLock.lock();
		std::string reqChnl = m_redisRPC.unsubClientGetOp(new_key.c_str());// +"*";
		if (((reqChnl.length() - 1) > 0) && (m_mapReqChnl.find(reqChnl) != m_mapReqChnl.end()))
		{
			//DEBUGLOG << "before unsubGetOp chnlName = " << reqChnl << ", size = " << m_reqChnl.size();
			m_mapReqChnl.erase(reqChnl);
			_DEBUGLOG("unsubGetOp chnlName = " << reqChnl.c_str() << ", size = " << m_mapReqChnl.size());
			bRlt = true;
		}
		m_reqLock.unlock();
	}
	return bRlt;
}

void CRedis_Utils::stopSubClientGetOp() 
{
	m_reqLock.lock();
	std::vector<std::string> vecReqKeys = m_redisRPC.clearChnl();
	std::vector<std::string>::iterator vec_iter = vecReqKeys.begin();
	for (; vec_iter != vecReqKeys.end(); ++vec_iter)
	{
		std::string strRlt;
		syncReq(GLOBALREQKEYS, getOldKey(*vec_iter), false, strRlt);
	}
	if (m_mapReqChnl.size() > 0)
		m_mapReqChnl.clear();
	m_reqLock.unlock();
}

int CRedis_Utils::notifyRlt(const std::string & strInKey, const std::string & strInValue)
{
	std::string strRlt;
	
	std::string strNewKey = genNewKey(strInKey);
	std::string strReqSlot;
	mapReqCB::iterator iter = m_mapReqChnl.begin();
	for (; iter != m_mapReqChnl.end(); ++iter)
	{
		std::string strSubsKey = iter->first.substr(0, iter->first.length() - strlen(REQSLOT));
		if (keyMatch(strNewKey, strSubsKey))
			strReqSlot = iter->first;
	}


	std::string processURL = strReqSlot + std::string(REQPROCESSING);
	//std::string setProcessCMd = std::string(R_SET) + std::string(" ") + processURL + std::string(" ") + std::string("0");
	strRlt.clear();
	set(getOldKey(processURL), "0", strRlt);	
	strRlt.clear();
	return set(strInKey, strInValue, strRlt);
}

void CRedis_Utils::setClientId(const std::string & strClientId)
{
	m_strClientId = strClientId;
}

int CRedis_Utils::getAvgOpTime()
{
	return std::accumulate(hiredisOneOpTime.begin(), hiredisOneOpTime.end(), 0) / hiredisOneOpTime.size();
}

void CRedis_Utils::close()
{
	_DEBUGLOG("close redis connection ip = " << this->m_strIp.c_str() << ", port = "
		<< this->m_iPort << ", clientID-->" << m_strClientId.c_str());
	std::this_thread::sleep_for(std::chrono::milliseconds(500));	//��ֹ�����˳����죬�����쳣���첽�ص�������������...
	if (m_bNeedSubs)
	{
		stopSubClientGetOp();
		if (m_pRedisAsyncContext) { 
			//m_aeStopLock.lock(); 
			redisAsyncDisconnect(m_pRedisAsyncContext);

#ifdef _WIN32
		aeStop(m_loop);
		//m_loop = nullptr;
		//_DEBUGLOG("stop event dispatch --> " << m_strClientId.c_str());
		_DEBUGLOG("wait for asyncThread quit... id --> " << m_strClientId.c_str());
		if (thAsyncKeyNotify.joinable())
			thAsyncKeyNotify.join();
		_DEBUGLOG("asyncThread quit... id --> " << m_strClientId.c_str());
#else
		event_base_loopbreak(m_base);
		_DEBUGLOG("stop event dispatch --> " << m_strClientId.c_str());
		//event_base_free(m_base);
		//m_base = nullptr;
#endif

		//m_aeStopLock.unlock();
		}
	}

	if (m_pRedisContext != nullptr)
	{
		redisFree(m_pRedisContext);
		m_pRedisContext = nullptr;
	}
}


bool CRedis_Utils::_connect(const std::string & strIp, int port)
{
	timeval t = { 1, 500 };
	m_pRedisContext = (redisContext *)redisConnectWithTimeout(
		strIp.c_str(), port, t);
	if ((NULL == m_pRedisContext) || (m_pRedisContext->err))
	{
		if (m_pRedisContext)
		{
			_ERRORLOG("connect error:" << m_pRedisContext->errstr);
		}
		else
		{
			_ERRORLOG("connect error: can't allocate redis context.");
		}
		return false;
	}
	m_bIsConnected = true;
	return true;
}

int CRedis_Utils::updateReqHB(const std::string & strInKey, bool bType)
{
	std::string strSetHBCmd = std::string(R_SET) + COMMAND_SPLIT
		+ strInKey + SET_KEY_TIMESTAMP
		+ COMMAND_SPLIT + int2str(time(nullptr));
	std::string strDelHBCmd = std::string(R_DEL) + std::string(" ") + strInKey + SET_KEY_TIMESTAMP;
	std::string strGetHBCmd = std::string(R_GET) + std::string(" ") + strInKey + SET_KEY_TIMESTAMP;
	std::string lenCmd = std::string(R_LLEN) + std::string(" ") + strInKey;
	std::string getCmd = std::string(R_POP) + std::string(" ") + strInKey;

	std::string strQueueLen;
	int iQueueLen;
	bool bRlt = sendCmd(lenCmd, strQueueLen);
	if (bRlt)
	{
		iQueueLen = atoi(strQueueLen.c_str());
		if (bType)
		{
			if (iQueueLen <= 0)			//���´�������������ʱ��
			{
				strQueueLen.clear();
				if (!sendCmd(strSetHBCmd, strQueueLen))
					sendCmd(strSetHBCmd, strQueueLen);
			}
			else
			{
				strQueueLen.clear();
				if (!sendCmd(strGetHBCmd, strQueueLen))
					sendCmd(strGetHBCmd, strQueueLen);
				if ((time(nullptr) - atoi(strQueueLen.c_str())) > GET_WAITTIMEOUT / 1000)
				{
					iQueueLen = SETQUEUEMAXSIZE - 1;
				}
			}
		}
		else
		{
			if(iQueueLen > 0)
			{
				strQueueLen.clear();
				if (!sendCmd(strGetHBCmd, strQueueLen))
					sendCmd(strGetHBCmd, strQueueLen);
				if ((time(nullptr) - atoi(strQueueLen.c_str())) > GET_WAITTIMEOUT / 1000)		//������������
				{
					strQueueLen.clear();
					if (!sendCmd(strSetHBCmd, strQueueLen))
						sendCmd(strSetHBCmd, strQueueLen);
					strQueueLen.clear();
					for (int i = 0; i < iQueueLen - 1; ++i)
					{
						if (m_mapSubsKeys.size() > 0)			//����
						{
							mapSubsCB::iterator it_subs = m_mapSubsKeys.begin();
							for (; it_subs != m_mapSubsKeys.end(); ++it_subs)
							{
								if (keyMatch(strInKey, it_subs->first))
								{
									std::string sRlt;
									std::string popCmd = R_POP + std::string(" ") + strInKey;
									std::string old_key = getOldKey(strInKey);
									if (sendCmd(popCmd, sRlt))
										it_subs->second(old_key.substr(0, old_key.length() - strlen(SET_KEY_SUFFIX)), sRlt);
								}
							}
						}
					}
					//iQueueLen = 0;
				}
			}
			if (iQueueLen > 1)			//���´�������������ʱ��
			{
				strQueueLen.clear();
				if (!sendCmd(strSetHBCmd, strQueueLen))
					sendCmd(strSetHBCmd, strQueueLen);
			}
			else						//���������
			{
				strQueueLen.clear();
				if (!sendCmd(strDelHBCmd, strQueueLen))
					sendCmd(strDelHBCmd, strQueueLen);
			}
		}
	}
	else
		iQueueLen = SETQUEUEMAXSIZE;
	return iQueueLen;
}

std::string CRedis_Utils::genNewKey(const std::string & old_key)
{
	return m_strClientId + old_key;
}

std::string CRedis_Utils::getOldKey(const std::string & new_key)
{
	if (new_key.length() > m_strClientId.length())
		return new_key.substr(m_strClientId.length());
	else
		return new_key;
}

std::map<std::string, int> CRedis_Utils::getAllReqs(const std::string strReqList)
{
	std::map<std::string, int> mapReqs;
	std::vector<std::string> vecReqs;
	std::vector<std::string>::iterator vecIter;
	_DEBUGLOG("��ȡ�����Ѷ��ĵ�����...");
	if (!m_bIsConnected)
	{
		_DEBUGLOG("redis ������δ����...������������... ip = " << m_strIp.c_str()
			<< ", port = " << m_iPort);
		if (!_connect(m_strIp.c_str(), m_iPort))
		{
			_WARNLOG("redis������������ʧ��... ");
			return mapReqs;
		}
	}
	std::string new_key = genNewKey(strReqList);
	std::string cmd = std::string(R_HGETALL ) + std::string(" ") + new_key;
	
	std::string strOutResult;
	sendCmd(cmd, strOutResult);
	vecReqs = split(strOutResult, COMMAND_SPLIT);
	if(vecReqs.size() > 0)
	{
		vecIter = vecReqs.begin();
		for (; vecIter != vecReqs.end(); ++vecIter)
		{
			std::string strKey = *vecIter;
			vecIter++;
			int refCnt = atoi(vecIter->c_str());
			mapReqs.insert(std::pair<std::string, int>(strKey, refCnt));
		}
	}
	return mapReqs;
}

int CRedis_Utils::syncReq(const std::string & strInKey, const std::string & strInReq, bool syncType, std::string & strOutResult)
{
	if (!m_bIsConnected)
	{
		_DEBUGLOG("redis ������δ����...������������... ip = " << m_strIp.c_str()
			<< ", port = " << m_iPort);
		if (!_connect(m_strIp.c_str(), m_iPort))
		{
			_WARNLOG("redis������������ʧ��... ");
			strOutResult = "redis is not connected...";
			return REDIS_CONNFAIL;
		}
	}
	int refCnt = 0;
	getRefCnt(strInReq, refCnt);
	std::string new_key = genNewKey(strInKey);
	if (syncType)
		refCnt++;
	else
		refCnt--;
	std::string strRefCntUpdateCmd;
	std::string strHBDelCmd;
	std::string strReqDelCmd;
	std::string strReqDefDelCmd;
	if(refCnt > 0)
		strRefCntUpdateCmd = std::string(R_HSET) + std::string(" ")
			+ new_key + std::string(" ") + strInReq + std::string(" ") + int2str(refCnt);
	else
	{
		strRefCntUpdateCmd = std::string(R_HDEL) + std::string(" ")
			+ new_key + std::string(" ") + strInReq;
		strHBDelCmd = std::string(R_DEL) + std::string(" ")
			+ genNewKey(strInReq) + HEARTSLOT;
		strReqDelCmd = std::string(R_DEL) + std::string(" ")
			+ genNewKey(strInReq) + REQSLOT;
		strReqDefDelCmd = std::string(R_DEL) + std::string(" ")
			+ genNewKey(strInReq) + REQSLOT + REQPROCESSING;
		sendCmd(strHBDelCmd, strOutResult);
		sendCmd(strReqDelCmd, strOutResult);
		sendCmd(strReqDefDelCmd, strOutResult);
	}
	strOutResult.clear();
	bool bRlt = sendCmd(strRefCntUpdateCmd, strOutResult);
	if (bRlt)
		return 0;
	else
		return REDIS_CMD_ERR;
}

int CRedis_Utils::getRefCnt(const std::string & strInField, int & refCnt)
{
	refCnt = 0;
	if (!m_bIsConnected)
	{
		_DEBUGLOG("redis ������δ����...������������... ip = " << m_strIp.c_str()
			<< ", port = " << m_iPort);
		if (!_connect(m_strIp.c_str(), m_iPort))
		{
			_WARNLOG("redis������������ʧ��... ");
			return REDIS_CONNFAIL;
		}
	}
	std::string cmd = std::string(R_HGET) + std::string(COMMAND_SPLIT)
		+ genNewKey(GLOBALREQKEYS) + std::string(COMMAND_SPLIT) + strInField;
	std::string strOutResult;
	bool bRlt = sendCmd(cmd, strOutResult);
	if (bRlt)
	{
		refCnt = atoi(strOutResult.c_str());
		return 0;
	}
	else
		return REDIS_CMD_ERR;
}

bool CRedis_Utils::isReqExist(const std::string & strInKey, const std::string & strInReq, std::string & strOutResult)
{
	if (!m_bIsConnected)
	{
		_DEBUGLOG("redis ������δ����...������������... ip = " << m_strIp.c_str()
			<< ", port = " << m_iPort);
		if (!_connect(m_strIp.c_str(), m_iPort))
		{
			_WARNLOG("redis������������ʧ��... ");
			strOutResult = "redis is not connected...";
			return REDIS_CONNFAIL;
		}
	}

	std::string new_key = genNewKey(strInKey);
	std::string cmd = std::string(R_SISMEMBER) + COMMAND_SPLIT
		+ new_key + COMMAND_SPLIT + strInReq;

	bool bRlt = sendCmd(cmd, strOutResult);
	if (bRlt)
		return 0;
	else
		return REDIS_CMD_ERR;
}

bool CRedis_Utils::sendCmd(const std::string & strInCmd, std::string & strOutResult)
{
	strOutResult.clear();
	bool bRlt = false;
	//m_aeStopLock.lock();
	//int64_t now = GetSysTimeMicros();
	//_DEBUGLOG("start send cmd now = " << now);
	std::vector<std::string> vecStr = split(strInCmd, COMMAND_SPLIT);
	std::string value = "", cmd = "";
	redisReply *pRedisReply = nullptr;
	if (vecStr.size() == 3)
	{
		value = vecStr[2];
		cmd = vecStr[0] + " " + vecStr[1] + " %s";
		pRedisReply = (redisReply*)redisCommand(m_pRedisContext, cmd.c_str(), value.c_str());  //ִ��INFO����
	}
	else
		pRedisReply = (redisReply*)redisCommand(m_pRedisContext, strInCmd.c_str());  //ִ��INFO����
	//_DEBUGLOG("send cmd complete interval = " << GetSysTimeMicros() - now);
	//hiredisOneOpTime.push_back(GetSysTimeMicros() - now);
	//m_aeStopLock.unlock();
	//������!!!
	if (!pRedisReply)
	{
		_WARNLOG("redis cmd = " << strInCmd.c_str() << ", err = " << m_pRedisContext->errstr
			<< ", try to reconnect...");
		m_bIsConnected = false;
		_connect(m_strIp, m_iPort);
		if (vecStr.size() == 3)
			pRedisReply = (redisReply*)redisCommand(m_pRedisContext, cmd.c_str(), value.c_str());  //ִ��INFO����
		else
			pRedisReply = (redisReply*)redisCommand(m_pRedisContext, strInCmd.c_str());  //ִ��INFO����

		if (pRedisReply)
		{
			if (replyCheck(pRedisReply, strOutResult))
				bRlt = true;
			freeReplyObject(pRedisReply);
			//m_bIsConnected = true;
			return bRlt;
		}
		//m_bIsConnected = false;
		_ERRORLOG("Error send cmd[" << m_pRedisContext->err << ":" << m_pRedisContext->errstr << "]");
		strOutResult = m_pRedisContext->errstr;
		return false;
	}
	if (replyCheck(pRedisReply, strOutResult))
		bRlt = true;
	freeReplyObject(pRedisReply);
	//_DEBUGLOG("redis cmd = " << strInCmd.c_str() << ", rlt = " << bRlt);
	return bRlt;
}

bool CRedis_Utils::replyCheck(redisReply *pRedisReply, std::string & strOutResult)
{
	bool bRlt = true;

	switch (pRedisReply->type) {
	case REDIS_REPLY_STATUS:		//��ʾ״̬������ͨ��str�ֶβ鿴���ַ���������len�ֶ�
		//_DEBUGLOG("type:REDIS_REPLY_STATUS, reply->len:" 
		//	<< pRedisReply->len << ", reply->str:" << pRedisReply->str);
		strOutResult = pRedisReply->str;
		break;
	case REDIS_REPLY_ERROR:			//��ʾ�����鿴������Ϣ�����ϵ�str,len�ֶ�
		bRlt = false;
		_WARNLOG("type:REDIS_REPLY_ERROR, reply->len:"
			<< pRedisReply->len << ", reply->str:" << pRedisReply->str);
		strOutResult = pRedisReply->str;
		break;
	case REDIS_REPLY_INTEGER:		//������������integer�ֶλ�ȡֵ
		//_DEBUGLOG("type:REDIS_REPLY_INTEGER, reply->integer:" << pRedisReply->integer);
		strOutResult = int2str(pRedisReply->integer);
		break;
	case REDIS_REPLY_NIL:			//û�����ݷ���
		//bRlt = false;
		_DEBUGLOG("type:REDIS_REPLY_NIL, no data");
		break;
	case REDIS_REPLY_STRING:		//�����ַ������鿴str,len�ֶ�
		//_DEBUGLOG("type:REDIS_REPLY_STRING, reply->len:"
		//	<< pRedisReply->len << ", reply->str:" << pRedisReply->str); 
		strOutResult = pRedisReply->str;
		break;
	case REDIS_REPLY_ARRAY:			//����һ�����飬�鿴elements��ֵ�������������ͨ��element[index]�ķ�ʽ��������Ԫ�أ�ÿ������Ԫ����һ��redisReply�����ָ��
		//_DEBUGLOG("------------------------------------------------------------");
		//_DEBUGLOG("type:REDIS_REPLY_ARRAY, reply->elements:" << pRedisReply->elements);
		for (int i = 0; i < pRedisReply->elements; i++) {
			//printf("%d: %s\n", i, pRedisReply->element[i]->str);
			std::string strRlt;
			replyCheck(pRedisReply->element[i], strRlt);
			strOutResult.append(strRlt);
			strOutResult.append(COMMAND_SPLIT);
		}
		//_DEBUGLOG("------------------------------------------------------------");
		break;
	default:
		_WARNLOG("unkonwn type : " << pRedisReply->type);
		strOutResult = "unkonwn type : " + int2str(pRedisReply->type);
		bRlt = false;
		break;
	}
	
	return bRlt;
}


void CRedis_Utils::connectCallback(const redisAsyncContext *c, int status)
{
	if (status != REDIS_OK) {
		_ERRORLOG("Error: " << c->errstr);
		return;
	}
	_DEBUGLOG("Async Connected...");
}

void CRedis_Utils::disconnectCallback(const redisAsyncContext *c, int status)
{
	if (status != REDIS_OK) {
		_ERRORLOG("Error: " << c->errstr);
		return;
	}
	_DEBUGLOG("Async DisConnected...");
}

void CRedis_Utils::subsAllCallback(redisAsyncContext *c, void *r, void *data)
{
	CRedis_Utils *self = (CRedis_Utils *)data;
	redisReply *reply = (redisReply *)r;
	if (reply == NULL) return;
	if (reply->type == REDIS_REPLY_ARRAY && reply->elements == 3) {
		_DEBUGLOG("Received[%s " << self->m_strIp.c_str() << "] channel" 
			<< reply->element[1]->str << ": " << reply->element[2]->integer);
	}
	else if (reply->type == REDIS_REPLY_ARRAY && reply->elements == 4) {
		std::string key = split(reply->element[2]->str, "__:")[1];
		_DEBUGLOG("Received[ " << self->m_strIp.c_str() << "] channel"
			<< reply->element[1]->str << " -- " << key.c_str()
			<< " : " << reply->element[3]->str);
		self->callSubsCB(key, reply->element[3]->str);
	}
}

bool CRedis_Utils::getReq(std::string strInKey)
{
	std::string getProcessCmd = R_GET + std::string(" ") + strInKey + std::string(REQPROCESSING);
	std::string setProcessingCmd = R_SET + std::string(COMMAND_SPLIT) + strInKey + std::string(REQPROCESSING)
		+ std::string(COMMAND_SPLIT) + std::string("1");
	std::string getRlt, setRlt;
	redisCommand(m_pRedisContext, "MULTI");
	sendCmd(getProcessCmd, getRlt);
	sendCmd(setProcessingCmd, setRlt);
	//redisCommand(m_pRedisContext, getProcessCmd.c_str());
	//redisCommand(m_pRedisContext, setProcessingCmd.c_str());
	redisReply *reply = (redisReply *)redisCommand(m_pRedisContext, "EXEC");

	std::string strRlt;
	replyCheck(reply, strRlt);
	if (strRlt.length() == (3 + strlen(COMMAND_SPLIT) * 2) && strRlt.find("0") == std::string::npos)
		return false;
	else
		return true;
}

void CRedis_Utils::callSubsCB(const std::string & strInKey, const std::string & strInKeyOp)
{
	std::string getCmd = R_GET + std::string(" ") + strInKey;
	std::string popCmd = R_POP + std::string(" ") + strInKey;
	std::string sRlt;
	std::string old_key = getOldKey(strInKey);

	//�����ƥ�������ĵļ�����ִ�ж��
	m_subsLock.lock(); 
	if (m_mapSubsKeys.size() > 0)			//����
	{
		mapSubsCB::iterator it_subs = m_mapSubsKeys.begin();
		for(; it_subs != m_mapSubsKeys.end(); ++it_subs)
		{
			if (keyMatch(strInKey, it_subs->first))
			{
				//������ֵ�����ظ��ϲ㣬value����Ϊ0��ʾ����del����get rpopʧ��
				if (strcmp(strInKeyOp.c_str(), "lpush") == 0 && strInKey.find(SET_KEY_SUFFIX) != std::string::npos)
				{
					updateReqHB(strInKey, false);		//���´���ʱ��

					std::string lenCmd = std::string(R_LLEN) + std::string(" ") + strInKey + SET_KEY_SUFFIX;
					std::string strQueueLen;
					
					if (sendCmd(popCmd, sRlt))
						it_subs->second(old_key.substr(0, old_key.length() - strlen(SET_KEY_SUFFIX)), sRlt);
				}
				//else if (strcmp(strInKeyOp.c_str(), "del") == 0 && strInKey.find(SET_KEY_SUFFIX) != std::string::npos)
				//	it_subs->second(old_key, "");
			}
		}
	}
	m_subsLock.unlock();
	m_pullLock.lock();
	if (m_mapPullKeys.size() > 0)		//��ȡ
	{
		mapPullCB::iterator it_pull = m_mapPullKeys.begin();
		for (; it_pull != m_mapPullKeys.end(); ++it_pull)
		{
			//������ֵ�����ظ��ϲ㣬value����Ϊ0��ʾ����del����get rpopʧ��
			if (keyMatch(std::string(strInKey), it_pull->first))
			{
				if (strcmp(strInKeyOp.c_str(), "lpush") == 0 && strInKey.find(SET_KEY_SUFFIX) == std::string::npos)
				{
					if (sendCmd(popCmd, sRlt))
						it_pull->second(old_key, sRlt);
				}
				//else if (strcmp(strInKeyOp.c_str(), "del") == 0 && strInKey.find(SET_KEY_SUFFIX) == std::string::npos)
				//	it_pull->second(old_key, "");
			}
			
		}
	}
	m_pullLock.unlock();
	m_reqLock.lock();
	if (this->m_mapReqChnl.size() > 0)	//�������
	{ 
		//_DEBUGLOG("key = " << strInKey.c_str() << ", op = " << strInKeyOp.c_str()
		//	<< ", get reqlist...");
		mapReqCB::iterator it_req = m_mapReqChnl.begin();
		for (; it_req != m_mapReqChnl.end(); ++it_req)
		{
			if (keyMatch(std::string(strInKey), it_req->first))
			{
				if (strcmp(strInKeyOp.c_str(), "set") == 0)
				{
					if (getReq(strInKey))
					{
						std::string get_key = "";
						std::string get_value = "";

						if (sendCmd(getCmd, sRlt))		//��ȡ��������е�ֵ
						{
							get_key = sRlt;

							sRlt.clear();
							getCmd.clear();
							getCmd = R_GET + std::string(" ") + get_key;
							if (sendCmd(getCmd.c_str(), sRlt))		//��ȡ��Ӧ��ֵ
								get_value = sRlt;
							else
								_WARNLOG("get value failed... getCmd = " << getCmd.c_str()
									<< ", errstr = " << sRlt.c_str());
						}
						else
							_WARNLOG("��ȡ���������Ϣʧ��... getCmd = " << getCmd.c_str()
								<< ", errstr = " << sRlt.c_str());
						it_req->second(getOldKey(get_key), get_value);
					}
					else
						_DEBUGLOG("��key�Ѿ�������... key = " << strInKey.c_str());
				}
			}
		}
	}
	m_reqLock.unlock();
}