//
// Created by liuchuyong on 2017/3/20.
//

#pragma once

#include <unistd.h>
#include <iostream>
#include <string>
#include <future>
#include "CommonDef.h"
#include "CommonUtil.h"

using namespace std;

class ZookeeperClient;
typedef shared_ptr<ZookeeperClient> ZkClientPtr;

/**
 * watcher handler
 */
typedef function<void (int type,int state,const ZkClientPtr & zkClientPtr,void * context)>  SessionWatcherHandler;
typedef function<void (int type,int state,const ZkClientPtr & zkClientPtr,
                       const string & path,void * context)>  ExistsNodeHandler;
typedef function<void (int type,int state,const ZkClientPtr & zkClientPtr,
                       const string & path, string & value, int & version, void * context)>  GetNodeHandler;
typedef function<void (int type,int state,const ZkClientPtr & zkClientPtr,
	                   const string & path,const string & value, void * context)>  CreateNodeHandler;
typedef function<void (int type,int state,const ZkClientPtr & zkClientPtr,
                       const string & path, void * context)>  DeleteNodeHandler;
typedef function<void (int type,int state,const ZkClientPtr& zkClientPtr,
                       const string& path, const string& value, int32_t version, void* context)> NodeChangeHandler;
typedef function<void (int type,int state,const ZkClientPtr& zkClientPtr,const string& path,
                       VectorString & childrenList,void* context)> GetChildrenHandler;

/**
 * completion function
 */
typedef function<void (int rc, const char *value, const void *data)> StringCompletionHandler;
typedef function<void (int rc, const void *data)> VoidCompletionHandler;
typedef function<void (int rc, const struct Stat *stat, const void *data)> StatCompletionHandler;
typedef function<void (int rc, const char *value, int value_len,
                       const struct Stat *stat, const void *data)> DataCompletionHandler;
typedef function<void (int rc,const struct String_vector *strings, const void *data)> StringsCompletionHandler ;
typedef function<void (int rc,const struct String_vector *strings,
                       const struct Stat *stat,const void *data)> StringsStatCompletionHandler;
typedef function<void (int rc, struct ACL_vector *acl,struct Stat *stat, const void *data)> AclCompletionHandler;
typedef function<void (int rc, const void *data)> VoidCompletionHandler;


/**
 * ������ ͨ�������Ľṹ
 */
struct  ZkWatcherOperateContext
{
	ZkWatcherOperateContext(const string & path,void * context, const ZkClientPtr & zkClientPtr)
	{
		path_ = path;
		context_ = context;
		zkClientPtr_ = zkClientPtr;
	}
	void* context_;
	string path_;
	ZkClientPtr zkClientPtr_;

	SessionWatcherHandler sessionWatcherHandler_;
	ExistsNodeHandler existsNodeHandler_;
	GetNodeHandler getNodeHandler_;
	CreateNodeHandler createNodeHandler_;
	DeleteNodeHandler deleteNodeHandler_;
	NodeChangeHandler nodeChangeHandler_;
	GetChildrenHandler getChildrenHandler_;
};

struct ZkAsyncCompletionContext
{
	ZkAsyncCompletionContext(const string & path,void * context,const ZkClientPtr & zkClientPtr)
	{
		path_ = path;
		context_ = context;
		zkClientPtr_ = zkClientPtr;
	}
	string path_;
	void * context_;
	ZkClientPtr zkClientPtr_;

	StringCompletionHandler stringCompletionHandler_;
	DataCompletionHandler dataCompletionHandler_;
	StatCompletionHandler statCompletionHandler_;
	StringsCompletionHandler stringsCompletionHandler_;
	StringsStatCompletionHandler stringsStatCompletionHandler_;
	AclCompletionHandler aclCompletionHandler_;
	VoidCompletionHandler voidCompletionHandler_;
};

class ZookeeperClient : public enable_shared_from_this<ZookeeperClient>
{
public:
	ZookeeperClient( const ZookeeperClient &)=delete;
	ZookeeperClient & operator=( const ZookeeperClient &)=delete;

	/**
	 * ZookeeperClient constructor
	 * @param server e.g. "127.0.0.1" or "127.0.0.1:2181,127.0.0.1:2182,127.0.0.1:2183"
	 * @param handler  session handler defined by yourself
	 * @param timeout default 3000 ms
	 */
	ZookeeperClient(const string & server,SessionWatcherHandler handler = nullptr,int timeout = 3000);

	/**
	 * �ͷ���Դ
	 */
	~ZookeeperClient();

	/**
	 * ���ӷ����
	 */
	void connectServer();

	/**
	 * �ر�����
	 */
	void closeServer();

	/**
	 * ��������
	 */
	 void reConnectServer();

	/**
	 * sync api
	 */

	/**
	 * ���ݽ��·������ȡ�������
	 * @param path ���·��
	 * @param value �������
	 * @param version ���ض�Ӧ�İ汾��
	 * @param watch 0-�����ü���  1-�������ӵ�Ĭ�ϼ���
	 * @return ���ؽ��  �ɹ�ZOK
	 */
	int getNodeValue(const string & path,string & value,int & version,const int watch = 0);

	/**
	 * ��watcher��getValue(ע��:��㲻���ڵ����,watcher��������)
	 * @param path ���·��
	 * @param value �������
	 * @param defaultWatcher  ��true,��ʹ��Ĭ�ϵ�watcher,��false,��Ҫ���Լ������watcher
	 * @param watcher ��defaultWatcher��falseʱ,��Ҫ��
	 * @param context ������
	 * @return ���ؽ��  �ɹ�ZOK
	 */
	int getNodeValueWithWatcher(const string & path,string & value, int & version,
	                            GetNodeHandler getNodeHandler = nullptr,void * context = nullptr);

	/**
	 * ��ȡ���·����Ӧ��ACLȨ���б�
	 * @param path ���·��
	 * @param vectorACL ACLȨ���б�
	 * @return ���ؽ��  �ɹ�ZOK
	 */
	int getNodeACL(const string & path,VectorACL & vectorACL);

	/**
	 * ���ý������
	 * @param path ���·��
	 * @param value �������
	 * @param version ���汾  -1��ʾ��У��汾,������ʾ��֤�汾���汾��ȷ������ȷ�޸�
	 * @return ���ؽ��  �ɹ�ZOK
	 */
	int setNodeValue(const string & path,const string & value,int version = IGNORE_VERSION,
	                 struct Stat * stat = nullptr);

	/**
	 * �������ӵĽ��ʱ����ڵ��ж�
	 * @param path ���·��
	 * @param stat ֧�ַ��ؽ���Stat��Ϣ
	 * @return ���ؽ��  �ɹ�ZOK
	 */
	int existsNode(const string & path,struct Stat *stat = nullptr);

	/**
	 * �����ӵ�exists��ʵ��
	 * @param path
	 * @param existsNodeHandler  �������ж����session handler
	 * @param context  �������ж����session handler
	 * @return ���ؽ��  �ɹ�ZOK
	 */
	int existsNodeWithWatch(const string & path,ExistsNodeHandler existsNodeHandler = nullptr,void * context = nullptr);

	/**
	 * �������(֧�ִ�����ʱ���/���ý������н��)
	 * @param path ���·��
	 * @param value �������(��"",��ʾ��������data���ݵĿս��)
	 * @param isTemp  �Ƿ���ʱ���
	 * @param isSequnce �Ƿ����н��
	 * @return ���ؽ��  �ɹ�ZOK
	 */
	int createNode(const string & path,const string & value,bool isTemp=false,bool isSequence=false);

	/**
	 * ɾ�����
	 * @param path ���·��
	 * @param version  ���汾 -1��ʾ��У��汾,������ʾ��֤�汾���汾��ȷ������ȷɾ��
	 * @return ���ؽ��  �ɹ�ZOK
	 */
	int deleteNode(const string & path,int version= -1);

	/**
	 * ��ȡ�����ӽ��(like ls)
	 * @param path ���·��
	 * @param childVec �ӽ���б�
	 * @param getChildrenNodeHandler  �������ж����session handler
	 * @param watcher ��defaultWatcher��falseʱ,��Ҫ��
	 * @return ���ؽ��  �ɹ�ZOK
	 */
	int getClildren(const string & path,VectorString & childrenVec,
	                GetChildrenHandler getChildrenNodeHandler = nullptr, void * context = nullptr);
	/**
	 * ��ȡ����״̬
	 */
	bool getConectStatus();

	/**
	 * ��������״̬
	 */
	void setConectStatus(bool connectStatus);

	/**
	 * async api
	 */

	/**
	 * �첽��ʽ�������(֧�ִ�����ʱ���/���ý������н��)
	 * @param path ���·��
	 * @param value �������(��"",��ʾ��������data���ݵĿս��)
	 * @param isTemp  �Ƿ���ʱ���
	 * @param isSequnce �Ƿ����н��
	 * @param handler ֧�ִ���completion handler
	 * @param context ��������Ϣ
	 * @return ���ؽ��  �ɹ�ZOK,����ʧ��
	 */
	int asyncCreateNode(const string & path,const string & value,
	                    bool isTemp=false,bool isSequence=false,
	                    StringCompletionHandler handler = nullptr, void * context = nullptr);
	/**
	 * �첽��ʽɾ�����
	 * @param path ���·��
	 * @param version  ���汾 -1��ʾ��У��汾,������ʾ��֤�汾���汾��ȷ������ȷɾ��
	 * @param handler ֧�ִ���completion handler
	 * @param context ��������Ϣ
	 * @return ���ؽ��  �ɹ�ZOK,����ʧ��
	 */
	int asyncDeleteNode(const string & path,int version= IGNORE_VERSION,
	                    VoidCompletionHandler handler = nullptr, void * context = nullptr);
	/**
	 * �첽��ȡ�������
	 * @param path ���·��
	 * @param value �������
	 * @param version ���汾
	 * @param watch 0-�����ü���  1-�������ӵ�Ĭ�ϼ���
	 * @return ���ؽ��  �ɹ�ZOK,����ʧ��
	 */
	int asyncGetNodeValue(const string & path,string & value,int & version,const int watch=0,
						  DataCompletionHandler handler = nullptr,void * context = nullptr);

	/**
	 * �첽��ȡ�������(���������汾)
	 * @param path ���·��
	 * @param value �������
	 * @param version ���汾
	 * @param watchHandler ���Ӵ�����
	 * @param watchContext ���Ӵ�����������
	 * @param completionHandler �첽��ɴ�����
	 * @param completionContext �첽��ɴ�����������
	 * @return ���ؽ��  �ɹ�ZOK,����ʧ��
	 */
	int asyncGetNodeValueWithWatcher(const string & path,string & value, int & version,
	                                 GetNodeHandler watchHandler = nullptr,void * watchContext = nullptr,
	                                 DataCompletionHandler completionHandler = nullptr,void * completionContext = nullptr);
	/**
	 * �첽���ý����Ϣ
	 * @param path ���·��
	 * @param value �������
	 * @param version ���汾
	 * @param handler �첽��ɴ�����
	 * @param context �첽��ɴ�����������
	 * @return �ɹ�ZOK,����ʧ��
	 */
	int asyncSetNodeValue(const string & path,const string & value,int version = IGNORE_VERSION,
						  StatCompletionHandler handler = nullptr,void * context = nullptr);

	/**
	 * �첽��ȡ�ӽڵ���Ϣ
	 * @param path ���·��
	 * @param handler �첽��ɴ�����
	 * @param context �첽��ɴ�����������
	 * @return �ɹ�ZOK,����ʧ��
	 */
	int asyncGetChildren(const string & path,StringsCompletionHandler handler = nullptr,void * context = nullptr);

	/**
	 * �첽��ȡ�ӽڵ���Ϣ(������)
	 * @param path ���·��
	 * @param watchHandler ���Ӵ�����
	 * @param watchContext ���Ӵ�����������
	 * @param completionHandler �첽��ɴ�����
	 * @param completionContext �첽��ɴ�����������
	 * @return �ɹ�ZOK,����ʧ��
	 */
	int asyncGetChildrenWithWatcher(const string & path, GetChildrenHandler watchHandler = nullptr,
	                                void * watchContext = nullptr,
	                                StringsCompletionHandler completionHandler = nullptr,
	                                void * completionContext = nullptr);

	/**
	 * �첽��ȡ�ӽڵ���Ϣ(֧�ִ�stat�汾������)
	 * @param path ���·��
	 * @param handler �첽��ɴ�����
	 * @param context �첽��ɴ�����������
	 * @return �ɹ�ZOK,����ʧ��
	 */
	int asyncGetChildren2(const string & path,StringsStatCompletionHandler handler = nullptr,void * context = nullptr);

	/**
	 * �첽��ȡ�ӽڵ���Ϣ(������,֧�ִ�stat�汾������)
	 * @param path ���·��
	 * @param watchHandler ���Ӵ�����
	 * @param watchContext ���Ӵ�����������
	 * @param completionHandler �첽��ɴ�����
	 * @param completionContext �첽��ɴ�����������
	 * @return �ɹ�ZOK,����ʧ��
	 */
	int asyncGetChildren2WithWatcher(const string & path, GetChildrenHandler watchHandler = nullptr,
	                                 void * watchContext = nullptr,
	                                 StringsStatCompletionHandler completionHandler = nullptr,
	                                 void * completionContext = nullptr);

	/**
	 * �첽��ȡ���ACL��Ϣ
	 * @param path ���·��
	 * @param handler �첽��ɴ�����
	 * @param context �첽��ɴ�����������
	 * @return �ɹ�ZOK,����ʧ��
	 */
	int asyncGetNodeACL(const string & path,AclCompletionHandler handler = nullptr,void * context = nullptr);

	/**
	 * �첽���ý��ACL��Ϣ
	 * @param path ���·��
	 * @param aclVector ACL��Ϣ
	 * @param version �汾
	 * @param handler �첽��ɴ�����
	 * @param context �첽��ɴ�����������
	 * @return �ɹ�ZOK,����ʧ��
	 */
	int asyncSetNodeACL(const string & path,ACLVector * aclVector,int version = IGNORE_VERSION,
	                    VoidCompletionHandler handler = nullptr,void * context = nullptr);
	/*
	 *	zoo_async
	 *	zoo_amulti ��������ԭ���Ե�һ���ύ���ZK����
	 */

private:
	//watch handler
	static void sessionWatcher(zhandle_t *zh, int type,int state, const char *path,void *watcherCtx);
	static void existsNodeWatcher(zhandle_t *zh, int type,int state, const char *path,void *watcherCtx);
	static void getNodeValueWatcher(zhandle_t *zh, int type,int state, const char *path,void *watcherCtx);
	static void getChildrenWatcher(zhandle_t *zh, int type,int state, const char *path,void *watcherCtx);

	//completion handler
	static void asyncCreateNodeHandler(int rc, const char *value, const void *data);
	static void asyncDeleteNodeHandler(int rc, const void *data);
	static void asyncGetNodeHandler(int rc, const char *value, int value_len,
	                                const struct Stat *stat, const void *data);
	static void asyncSetNodeHandler(int rc, const struct Stat *stat, const void *data);
	static void asyncGetChildrenHandler(int rc,const struct String_vector *strings, const void *data);
	static void asyncGetChildrenHandler2(int rc,const struct String_vector *strings,
	                                     const struct Stat *stat,const void *data);
	static void asyncGetNodeACLHandler(int rc, struct ACL_vector *acl,struct Stat *stat, const void *data);
	static void asyncSetNodeACLHandler(int rc, const void *data);

private:
	SessionWatcherHandler sessionWatcherHandler_;
	int timeout_;
	string server_;
	bool connectStatus_ = false;
	zhandle_t *  zkHandle_ = nullptr ;
	std::shared_ptr<spdlog::logger> logger_ = spdlog::get(getGlobalLogName());
	std::mutex mutex_;
};


