#include "Redis_Utils.h"

CRedis_Utils::CRedis_Utils()
{
}

CRedis_Utils::~CRedis_Utils()
{
}

bool CRedis_Utils::connect(const char* ip, int port)
{
	this->ip = ip;
	this->port = port;
	timeval t = { 1, 500 };
	pRedisContext = (redisContext *)redisConnectWithTimeout(
		ip, port, t);
	if ((NULL == pRedisContext) || (pRedisContext->err))
	{
		if (pRedisContext)
			cout << "connect error:" << pRedisContext->errstr << endl;
		else
			cout << "connect error: can't allocate redis context." << endl;
		return false;
	}
	
	thread thAsyncSubALL(CRedis_Utils::asyncSubsAll, this);
	thAsyncSubALL.detach();

	return true;
}

void* CRedis_Utils::asyncSubsAll(void *arg)
{
	CRedis_Utils *self = (CRedis_Utils *)arg;
#ifdef _WIN32
	/* For Win32_IOCP the event loop must be created before the async connect */
	self->loop = aeCreateEventLoop(1024 * 10);
#else
	signal(SIGPIPE, SIG_IGN);
	self->base = event_base_new();
#endif
	self->pRedisAsyncContext = redisAsyncConnect(self->ip.c_str(), self->port);
	if (self->pRedisAsyncContext->err) {
		printf("Error: %s\n", self->pRedisAsyncContext->errstr);
		return NULL;
	}
#ifdef _WIN32
	redisAeAttach(self->loop, self->pRedisAsyncContext);
#else
	redisLibeventAttach(self->pRedisAsyncContext, self->base);
#endif

	redisAsyncSetConnectCallback(self->pRedisAsyncContext, CRedis_Utils::connectCallback);
	redisAsyncSetDisconnectCallback(self->pRedisAsyncContext, CRedis_Utils::disconnectCallback);
	redisAsyncCommand(self->pRedisAsyncContext, subCallback, self, "PSUBSCRIBE __keyspace@0__:*");

#ifdef _WIN32
	aeMain(self->loop);
#else
	event_base_dispatch(self->base);
#endif
	return NULL;
}

bool CRedis_Utils::get(const char* _key, char* sRlt)
{
	std::string cmd = std::string(R_GET) + std::string(" ") + std::string(_key);
	if (sendCmd(cmd.c_str(), sRlt))
		return true;
	else
		return false;
}

bool CRedis_Utils::set(const char* _key, const char* _value, char *sRlt)
{
	std::string cmd = std::string(R_SET) + std::string(" ") 
		+ _key + std::string(" ") + _value;
	if (sendCmd(cmd.c_str(), sRlt))
		return true;
	else
		return false;
	return sRlt;
}

bool CRedis_Utils::push(const char* list_name, const char* _value, char *sRlt)
{
	std::string cmd = std::string(R_LPUSH) + std::string(" ") 
		+ list_name + std::string(" ") + _value;
	if (sendCmd(cmd.c_str(), sRlt))
		return true;
	else
		return false;
	return sRlt;
}

bool CRedis_Utils::pop(const char* list_name, char *sRlt)
{
	std::string cmd = std::string(R_RPOP) + std::string(" ") + list_name;
	if (sendCmd(cmd.c_str(), sRlt))
		return true;
	else
		return false;
	return sRlt;
}

void CRedis_Utils::subs(const char *key, subsCallback cb)
{
	mapSubsCB::iterator it = subsKeys.find(std::string(key));
	if (it == subsKeys.end())
	{
		subsKeys.insert(std::pair<std::string, subsCallback>(std::string(key), cb));
		cout << "CRedis_Utils::subs 订阅成功 key = " << key << endl;
	}
	else
		cout << "CRedis_Utils::subs 该键已经订阅 key = " << key << endl;
}

void CRedis_Utils::pull(const char *key, pullCallback cb)
{
	mapPullCB::iterator it = pullKeys.find(std::string(key));
	if (it == pullKeys.end())
		pullKeys.insert(std::pair<std::string, pullCallback>(std::string(key), cb));
}

void CRedis_Utils::close()
{
	//if (pRedisAsyncContext != nullptr)
	redisFree(pRedisContext);
	//if (loop)
	redisAsyncDisconnect(pRedisAsyncContext);
	//sleep(1);
	aeStop(loop);
}

bool CRedis_Utils::sendCmd(const char *cmd, char *sRlt)
{
	redisReply *pRedisReply = (redisReply*)redisCommand(pRedisContext, cmd);  //执行INFO命令
	
	//错误处理!!!
	if (!pRedisReply)
	{
		redisFree(pRedisContext);
		connect("192.168.31.170", 6379);
		pRedisReply = (redisReply*)redisCommand(pRedisContext, cmd);
		if (pRedisReply)
		{
			replyCheck(pRedisReply, sRlt);
			freeReplyObject(pRedisReply);
			return true;
		}
		printf("Error[%d:%s]\n", pRedisContext->err, pRedisContext->errstr);
		memcpy(sRlt, pRedisContext->errstr, strlen(pRedisContext->errstr));
		return false;
	}
	replyCheck(pRedisReply, sRlt);
	freeReplyObject(pRedisReply);
	return true;
}

bool CRedis_Utils::replyCheck(redisReply *pRedisReply, char *sReply)
{
	bool bRlt = true;
	switch (pRedisReply->type) {
	case REDIS_REPLY_STATUS:		//表示状态，内容通过str字段查看，字符串长度是len字段
		bRlt = false;
		printf("type:%s, reply->len:%d reply->str:%s\n", "REDIS_REPLY_STATUS", pRedisReply->len, pRedisReply->str);
		memcpy(sReply, pRedisReply->str, pRedisReply->len);
		break;
	case REDIS_REPLY_ERROR:			//表示出错，查看出错信息，如上的str,len字段
		bRlt = false;
		printf("type:%s, reply->len:%d reply->str:%s\n", "REDIS_REPLY_ERROR", pRedisReply->len, pRedisReply->str);
		memcpy(sReply, pRedisReply->str, pRedisReply->len);
		break;
	case REDIS_REPLY_INTEGER:		//返回整数，从integer字段获取值
		printf("type:%s, reply->integer:%lld\n", "REDIS_REPLY_INTEGER", pRedisReply->integer);
		memcpy(sReply, int2str(pRedisReply->integer).c_str(), int2str(pRedisReply->integer).length());
		break;
	case REDIS_REPLY_NIL:			//没有数据返回
		printf("type:%s, no data\n", "REDIS_REPLY_NIL"); 
		break;
	case REDIS_REPLY_STRING:		//返回字符串，查看str,len字段
		printf("type:%s, reply->len:%d reply->str:%s\n", "REDIS_REPLY_STRING", pRedisReply->len, pRedisReply->str);
		memcpy(sReply, pRedisReply->str, pRedisReply->len);
		break;
	case REDIS_REPLY_ARRAY:			//返回一个数组，查看elements的值（数组个数），通过element[index]的方式访问数组元素，每个数组元素是一个redisReply对象的指针
		printf("type:%s, reply->elements:%zd\n", "REDIS_REPLY_ARRAY", pRedisReply->elements);
		for (int i = 0; i < pRedisReply->elements; i++) {
			//printf("%d: %s\n", i, pRedisReply->element[i]->str);
			char sRlt[256];
			memset(sRlt, 0, 256);
			replyCheck(pRedisReply->element[i], sRlt);
			memcpy(sReply + strlen(sReply), sRlt, strlen(sRlt));
			memcpy(sReply + strlen(sReply), "\n", 1);
		}
		cout << sReply << endl;
		break;
	default:
		printf("unkonwn type:%d\n", pRedisReply->type);
		bRlt = false;
		break;
	}
	return bRlt;
}

void CRedis_Utils::connectCallback(const redisAsyncContext *c, int status)
{
	if (status != REDIS_OK) {
		printf("Error: %s\n", c->errstr);
		return;
	}
	printf("Connected...\n");
}

void CRedis_Utils::disconnectCallback(const redisAsyncContext *c, int status)
{
	if (status != REDIS_OK) {
		printf("Connect Error: %s\n", c->errstr);
	}
	printf("Disconnected...\n");
}

void CRedis_Utils::subCallback(redisAsyncContext *c, void *r, void *data)
{
	CRedis_Utils *self = (CRedis_Utils *)data;
	redisReply *reply = (redisReply *)r;
	if (reply == NULL) return;
	if (reply->type == REDIS_REPLY_ARRAY && reply->elements == 3) {
		printf("Received[%s] channel %s: %d\n",
			self->ip.c_str(),
			reply->element[1]->str,
			reply->element[2]->integer);
	}
	else if (reply->type == REDIS_REPLY_ARRAY && reply->elements == 4) {
		printf("Received[%s] channel %s -- %s : %s\n",
			self->ip.c_str(),
			reply->element[1]->str,
			reply->element[2]->str,
			reply->element[3]->str);
		std::string key = split(reply->element[2]->str, "__:")[1];
		self->callSubsCB(key.c_str());
	}
}

void CRedis_Utils::callSubsCB(const char *key)
{
	if (subsKeys.size() > 0)
	{
		mapSubsCB::iterator it_subs = subsKeys.begin();
		for(; it_subs != subsKeys.end(); ++it_subs)
		{
			if (keyMatch(std::string(key), it_subs->first))
				it_subs->second(key);
		}
	}
	if (pullKeys.size() > 0)
	{
		mapPullCB::iterator it_pull = pullKeys.begin();
		for (; it_pull != pullKeys.end(); ++it_pull)
		{
			if (keyMatch(std::string(key), it_pull->first))
				it_pull->second(key);
		}
	}
}
