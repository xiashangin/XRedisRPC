#pragma once

#include "RedisRPC.h"
#include <functional>
#define DEFAULT_CLIENTID		"__default__"
#define COMMAND_SPLIT			"&&&_&&&"
#define SET_KEY_SUFFIX			"__SETFLAG__"
#define SET_KEY_TIMESTAMP		"LASTREQ+"
#define SETQUEUEMAXSIZE			3

typedef std::function<void(const std::string & , const std::string & )> subsCallback;
typedef std::function<void(const std::string & , const std::string &)> pullCallback;
typedef std::function<void(const std::string & , const std::string &)> clientOpCallBack;

typedef std::map<std::string, subsCallback> mapSubsCB;		//subkey-->subfunc
typedef std::map<std::string, pullCallback> mapPullCB;		//pullkey-->subfunc
typedef std::map<std::string, clientOpCallBack> mapReqCB;	//getkey-->getfunc

class CRedis_Utils
{
public:
	//��������strClientID�ǿ��ַ�������ô���ֶλ���һ��Ĭ��ֵ��__default__
	CRedis_Utils(const std::string & strClientID);
	~CRedis_Utils();

	//�ͻ��˻�����������
	/*
	isSubs˵����Ĭ��false
		false����ʹ��redis�ļ��ռ�֪ͨ���ܣ���subs��pull��subsClientGetOp�Ƚӿڲ������ã�ֻ�ܽ���redis��������
		true������redis�ļ��ռ�֪ͨ����
		���������޷���֤��
			1.  �޷����redis�����Ƿ��Ѿ��������ռ�֪ͨ����
			2.  �޷���֤����redis���ռ�����ɹ�ִ�У���Ϊ�˷�ʽʹ�õ����첽�ӿڣ�ֻ���ж����ƽ���ִ�ж����ˣ�
				��ʹ���첽�ӿڷ��ش����޷����˴��󷵻ظ��ϲ㡣���ǳ�����Ȼ���������У����ǺͶ�����صĹ���ȫ����Ч��
				Ŀǰ�����ط����ƻ����������⡣
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
	*/
	int subsClientGetOp(const std::string & strInKey, clientOpCallBack cb);
	bool unsubClientGetOp(const std::string & strInKey);							//ע�������ͻ���get key����
	void stopSubClientGetOp();														//ȡ�������ͻ���ȫ��get����
	int notifyRlt(const std::string & strInKey, const std::string & strInValue);	//֪ͨ�ͻ��˴������

	void setClientId(const std::string & strClientId);

	int getAvgOpTime();
private:
	CRedis_Utils(const CRedis_Utils & c);	//��������
	
	void close();
	bool _connect(const std::string & strIp, int iPort);
	int updateReqHB(const std::string & strInKey, bool bType);				//true:set����false:��������
	//ʵ��ҵ��ģ������ݸ���
	std::string genNewKey(const std::string & lpStrOldKey);					//��װ�û������key				
	std::string getOldKey(const std::string & lpStrNewKey);					//��ȡ�ͻ���ԭʼkey
	
	std::map<std::string, int> getAllReqs(const std::string strReqList);
	int syncReq(const std::string & strInKey, const std::string & strInReq, bool syncType, std::string & strOutResult);	//syncType:true-->��refCnt false-->��refCnt
	int getRefCnt(const std::string & strInField, int & refCnt);
	bool isReqExist(const std::string & strInKey, const std::string & strInReq, std::string & strOutResult);
	bool sendCmd(const std::string & strInCmd, std::string & strOutResult);		//����redis����
	bool replyCheck(redisReply *rRedisReply, std::string & strOutResult);		//����redisӦ����Ϣ

	static void* thAsyncSubsAll(void *arg);							//redis���ռ�֪ͨ

	static void connectCallback(const redisAsyncContext *c, int iStatus);	//redis�첽�ص�����
	static void disconnectCallback(const redisAsyncContext *c, int iStatus);
	static void subsAllCallback(redisAsyncContext *c, void *r, void *data);
	bool getReq(std::string strInkey);

	void callSubsCB(const std::string & strInKey, const std::string & strInOp);

	redisContext *m_pRedisContext;			//redisͬ��������
	//std::shared_ptr<redisContext *> m_spRedisContext;
	redisAsyncContext *m_pRedisAsyncContext;	//redis�첽������
	
#ifdef _WIN32				//redis�첽�¼���
	aeEventLoop *m_loop;
#else
	struct event_base *m_base;
#endif

	mapSubsCB m_mapSubsKeys;		//������Ϣ  ��-->�ص�����
	mapPullCB m_mapPullKeys;		//pull��Ϣ  ��-->�ص�����
	mapReqCB  m_mapReqChnl;			//�������	������-->�ص�����

	mutex m_getLock;				//���߳�ͬ����
	mutex m_setLock;
	mutex m_pushLock;
	mutex m_popLock;
	mutex m_subsLock;
	mutex m_pullLock;
	mutex m_reqLock;
	mutex m_aeStopLock;

	std::thread thAsyncKeyNotify;
	std::vector<int> hiredisOneOpTime;
	std::string m_strIp;				//redis ip
	int m_iPort;						//redis �˿�

	CRedisRPC m_redisRPC;				//redis RPC����ʵ��

	bool m_bNeedSubs;					//�Ƿ���Ҫredis���Ĺ���
	bool m_bIsConnected;				//����redis�ɹ���־
	std::string m_strClientId;			//�ͻ��˱�־
	//std::string m_strLastGetKey;
};

