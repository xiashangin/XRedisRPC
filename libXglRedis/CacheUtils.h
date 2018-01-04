#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>

//��־����
#define LOG_INFO	0
#define LOG_DEBUG	1
#define LOG_WARN	2
#define LOG_ERROR	3


//״̬��
#define REDIS_TIMEOUT			100		//ҵ��ģ�鴦��ʱ
#define REDIS_NOSERVICE			101		//��ҵ����ģ��
#define REDIS_SENDREQFAIL		102		//��������ʧ��
#define REDIS_CONNFAIL			103		//redis��������ʧ��
#define REDIS_CMD_ERR			105		//��������ʧ��
#define REDIS_KEY_NULL			104		//�����keyֵΪ��
#define REDIS_VALUE_NULL		105		//�����valueֵΪ��
#define REDIS_KEY_EXISTED		106		//key�ѱ�����
#define REDIS_SUBS_OFF			107		//δ����redis���ռ�֪ͨ����
#define REDIS_REQ_SYNC_FAIL		108		//����ͬ��ʧ��
#define	REDIS_KEY_NOT_EXIST		109		//get��pop��key������
#define REDIS_SERVICE_BUSY		110		//ҵ��ģ�鷱æ

/************************************************************************/
/*  ������صĻص�������
	subs-->subsCallback
	pull-->pullCallback
	subsClientGetOp-->clientOpCallBack
	
	����˵����
	strKey�����ĵļ�
	strValue������Ӧ��ֵ
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
	//��������strClientID�ǿ��ַ�������ô���ֶλ���һ��Ĭ��ֵ��__default__
	static std::shared_ptr<CCacheUtils> createInstance(const std::string &strClientId);
	~CCacheUtils();

	//�ͻ��˻�����������
	/*
	isSubs˵����Ĭ��false
	false����ʹ��redis�ļ��ռ�֪ͨ���ܣ���subs��pull��subsClientGetOp�Ƚӿڲ������ã�ֻ�ܽ���redis��������
	true������redis�ļ��ռ�֪ͨ����
	*/
	bool connect(const std::string & strIp, int iPort, bool bNeedSubs = false);		//����redis���񲢶���redis���ռ�֪ͨ
	void disconnect();

	//redis��������
	/**
	����ֵ˵����
	0�������ɹ�
	>0������ʧ�ܣ�����״̬��
	�������ͨ��strOutResult��ȡ
	*/
	int get(const std::string & strInKey, std::string & strOutResult);
	int set(const std::string & strInKey, const std::string & strInValue, std::string & strOutResult);
	int push(const std::string & strInListName, const std::string & strInValue, std::string & strOutResult);
	int pop(const std::string & strInListName, std::string & strOutResult);

	//redis���Ĺ���
	/*
	subs()	pull()
	����ֵ˵����
	0�������ɹ�
	>0������ʧ�ܣ�����״̬��

	subs��pull�Ƚϣ�
	1. subs���ĵ����ַ����ı仯��pull���ĵ���list�ı仯��
	2. ��set��������ʱ��subs�ص����յ���set��key��value��
	del��������ʱ��subs�ص����յ���ɾ����key����ʱvalueΪ���ַ�����
	3. ��push��������ʱ��pull�ص����յ���push��listName��value��pop��������ʱ��pull�ص������յ���Ϣ��
	del��������ʱ��pull�ص����յ���ɾ����listName����ʱvalueΪ���ַ�����

	unsubs()	unpull()
	����ֵ˵����
	true�������ɹ�
	false������ʧ��
	*/
	int subs(const std::string & strInKey, subsCallback cb);	//subscribe channel
	bool unsubs(const std::string & strInKey);					//unsubscribe channel
	int pull(const std::string & strInKey, pullCallback cb);	//pull list
	bool unpull(const std::string & strInKey);					//unpull list, like unsubs

	//ҵ����ģ��
	/*
	subsClientGetOp()
	����ֵ˵����
	0�������ɹ�
	>0������ʧ�ܣ�����״̬��

	notifyRlt()
	����ֵ˵����
	0�������ɹ�
	>0������ʧ�ܣ�����״̬��
	���ã������������֮��ͨ���˺���֪ͨ�ͻ���
	ʹ��˵����ҵ����ģ���еĻص�����(clientOpCallBack)������ͻ�����֮��ʹ��
		�˺�����������֪ͨ�ͻ��ˡ�
	*/
	int subsClientGetOp(const std::string & strInKey, clientOpCallBack cb);
	bool unsubClientGetOp(const std::string & strInKey);							//ע�������ͻ���get key����
	void stopSubClientGetOp();														//ȡ�������ͻ���ȫ��get����
	int notifyRlt(const std::string & strInKey, const std::string & strInValue);	//֪ͨ�ͻ��˴������

	/**��־�ӿڣ�ilogType�������£�
	LOG_INFO		0		//��ͨ��Ϣ
	LOG_DEBUG		1		//������Ϣ
	LOG_WARN		2		//������Ϣ
	LOG_ERROR		3		//������Ϣ
	**/
	static void log(const int iLogType, const std::string & strLog);
	static void log0(const int iLogType, const char * lpFormatText, ...);
protected:
	CCacheUtils(const std::string & strClientId);
	void * m_redisUtil;
};

