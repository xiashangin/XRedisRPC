#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>

//日志类型
#define LOG_INFO		0
#define LOG_TRACE		1
#define LOG_DEBUG		2
#define LOG_WARN		3
#define LOG_ERROR		4
#define	LOG_FATAL		5

//状态码
#define REDIS_TIMEOUT			100		//业务模块处理超时
#define REDIS_NOSERVICE			101		//无业务处理模块
#define REDIS_SENDREQFAIL		102		//发送请求失败
#define REDIS_CONNFAIL			103		//redis服务连接失败
#define REDIS_CMD_ERR			105		//发送命令失败
#define REDIS_KEY_NULL			104		//输入的key值为空
#define REDIS_VALUE_NULL		105		//输入的value值为空
#define REDIS_KEY_EXISTED		106		//key已被订阅
#define REDIS_SUBS_OFF			107		//未开启redis键空间通知功能
#define	REDIS_KEY_NOT_EXIST		108		//get或pop的key不存在

/************************************************************************/
/*  订阅相关的回调函数：
	subs-->subsCallback
	pull-->pullCallback
	subsClientGetOp-->clientOpCallBack
	
	参数说明：
	strKey：订阅的键
	strValue：键对应的值
	*/
/************************************************************************/
typedef std::function<void(const std::string & strKey, const std::string & strValue)> subsCallback;
typedef std::function<void(const std::string & strKey, const std::string & strValue)> pullCallback;
typedef std::function<void(const std::string & strKey, const std::string & strValue)> clientOpCallBack;

typedef std::map<std::string, subsCallback> mapSubsCB;		//subkey-->subfunc
typedef std::map<std::string, pullCallback> mapPullCB;		//pullkey-->subfunc
typedef std::map<std::string, clientOpCallBack> mapReqCB;	//getkey-->getfunc

class CCacheUtils
{
public:
	//如果传入的strClientID是空字符串，那么此字段会有一个默认值：__default__
	static std::shared_ptr<CCacheUtils> createInstance(const std::string &strClientId);
	~CCacheUtils();

	//客户端基本操作函数
	/*
	isSubs说明：默认false
	false：不使用redis的键空间通知功能，即subs、pull、subsClientGetOp等接口不起作用，只能进行redis基本操作
	true：启用redis的键空间通知功能
	*/
	bool connect(const std::string & strIp, int iPort, bool bNeedSubs = false);		//连接redis服务并订阅redis键空间通知
	void disconnect();

	//redis基本操作
	/**
	返回值说明：
	0：操作成功
	>0：操作失败，返回状态码
	操作结果通过strOutResult获取
	*/
	int get(const std::string & strInKey, std::string & strOutResult);
	int set(const std::string & strInKey, const std::string & strInValue, std::string & strOutResult);
	int push(const std::string & strInListName, const std::string & strInValue, std::string & strOutResult);
	int pop(const std::string & strInListName, std::string & strOutResult);

	//redis订阅功能
	/*
	subs()	pull()
	返回值说明：
	0：操作成功
	>0：操作失败，返回状态码

	subs和pull比较：
	1. subs订阅的是字符串的变化，pull订阅的是list的变化。
	2. 当set操作发生时，subs回调会收到被set的key和value。
	del操作发生时，subs回调会收到被删除的key，此时value为空字符串。
	3. 当push操作发生时，pull回调会收到被push的listName和value。pop操作发生时，pull回调不会收到消息。
	del操作发生时，pull回调会收到被删除的listName，此时value为空字符串。

	unsubs()	unpull()
	返回值说明：
	true：操作成功
	false：操作失败
	*/
	int subs(const std::string & strInKey, subsCallback cb);	//subscribe channel
	bool unsubs(const std::string & strInKey);					//unsubscribe channel
	int pull(const std::string & strInKey, pullCallback cb);	//pull list
	bool unpull(const std::string & strInKey);					//unpull list, like unsubs

	//业务处理模块
	/*
	subsClientGetOp()
	返回值说明：
	0：操作成功
	>0：操作失败，返回状态码

	notifyRlt()
	返回值说明：
	0：操作成功
	>0：操作失败，返回状态码
	作用：处理完成请求之后通过此函数通知客户端
	使用说明：业务处理模块中的回调函数(clientOpCallBack)处理完客户请求之后，使用
		此函数将处理结果通知客户端。
	*/
	int subsClientGetOp(const std::string & strInKey, clientOpCallBack cb);
	bool unsubClientGetOp(const std::string & strInKey);							//注销监听客户端get key操作
	void stopSubClientGetOp();														//取消监听客户端全部get操作
	int notifyRlt(const std::string & strInKey, const std::string & strInValue);	//通知客户端处理完成

	/**日志接口，ilogType定义如下：
	LOG_INFO		0
	LOG_TRACE		1
	LOG_DEBUG		2
	LOG_WARN		3
	LOG_ERROR		4
	LOG_FATAL		5

	一个使用例子：
	std::ostringstream logInfo;
	void setLog(int iLogType, std::ostringstream & strLogInfo)
	{
	CClientCacheUtils::log(iLogType, strLogInfo.str());
	strLogInfo.str("");
	}

	std::string result = "log test...";
	logInfo << "log test, result = " << result;
	setLog(LOG_DEBUG, logInfo.str());
	**/
	static void log(const int iLogType, const std::string & strLog);
protected:
	CCacheUtils(const std::string & strClientId);
	void * m_redisUtil;
};

