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
 * 监视器 通用上下文结构
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
	 * 释放资源
	 */
	~ZookeeperClient();

	/**
	 * 连接服务端
	 */
	void connectServer();

	/**
	 * 关闭连接
	 */
	void closeServer();

	/**
	 * 重新连接
	 */
	 void reConnectServer();

	/**
	 * sync api
	 */

	/**
	 * 根据结点路径，获取结点数据
	 * @param path 结点路径
	 * @param value 结点数据
	 * @param version 返回对应的版本号
	 * @param watch 0-不启用监视  1-启用连接的默认监视
	 * @return 返回结果  成功ZOK
	 */
	int getNodeValue(const string & path,string & value,int & version,const int watch = 0);

	/**
	 * 带watcher的getValue(注意:结点不存在的情况,watcher不起作用)
	 * @param path 结点路径
	 * @param value 结点数据
	 * @param defaultWatcher  传true,会使用默认的watcher,传false,需要传自己定义的watcher
	 * @param watcher 当defaultWatcher传false时,需要传
	 * @param context 上下文
	 * @return 返回结果  成功ZOK
	 */
	int getNodeValueWithWatcher(const string & path,string & value, int & version,
	                            GetNodeHandler getNodeHandler = nullptr,void * context = nullptr);

	/**
	 * 获取结点路径对应的ACL权限列表
	 * @param path 结点路径
	 * @param vectorACL ACL权限列表
	 * @return 返回结果  成功ZOK
	 */
	int getNodeACL(const string & path,VectorACL & vectorACL);

	/**
	 * 设置结点数据
	 * @param path 结点路径
	 * @param value 结点数据
	 * @param version 结点版本  -1表示不校验版本,其他表示验证版本，版本正确才能正确修改
	 * @return 返回结果  成功ZOK
	 */
	int setNodeValue(const string & path,const string & value,int version = IGNORE_VERSION,
	                 struct Stat * stat = nullptr);

	/**
	 * 不带监视的结点时候存在的判断
	 * @param path 结点路径
	 * @param stat 支持返回结点的Stat信息
	 * @return 返回结果  成功ZOK
	 */
	int existsNode(const string & path,struct Stat *stat = nullptr);

	/**
	 * 带监视的exists的实现
	 * @param path
	 * @param existsNodeHandler  传入自行定义的session handler
	 * @param context  传入自行定义的session handler
	 * @return 返回结果  成功ZOK
	 */
	int existsNodeWithWatch(const string & path,ExistsNodeHandler existsNodeHandler = nullptr,void * context = nullptr);

	/**
	 * 创建结点(支持创建临时结点/永久结点或序列结点)
	 * @param path 结点路径
	 * @param value 结点数据(传"",表示创建不带data数据的空结点)
	 * @param isTemp  是否临时结点
	 * @param isSequnce 是否序列结点
	 * @return 返回结果  成功ZOK
	 */
	int createNode(const string & path,const string & value,bool isTemp=false,bool isSequence=false);

	/**
	 * 删除结点
	 * @param path 结点路径
	 * @param version  结点版本 -1表示不校验版本,其他表示验证版本，版本正确才能正确删除
	 * @return 返回结果  成功ZOK
	 */
	int deleteNode(const string & path,int version= -1);

	/**
	 * 获取结点的子结点(like ls)
	 * @param path 结点路径
	 * @param childVec 子结点列表
	 * @param getChildrenNodeHandler  传入自行定义的session handler
	 * @param watcher 当defaultWatcher传false时,需要传
	 * @return 返回结果  成功ZOK
	 */
	int getClildren(const string & path,VectorString & childrenVec,
	                GetChildrenHandler getChildrenNodeHandler = nullptr, void * context = nullptr);
	/**
	 * 获取连接状态
	 */
	bool getConectStatus();

	/**
	 * 设置连接状态
	 */
	void setConectStatus(bool connectStatus);

	/**
	 * async api
	 */

	/**
	 * 异步方式创建结点(支持创建临时结点/永久结点或序列结点)
	 * @param path 结点路径
	 * @param value 结点数据(传"",表示创建不带data数据的空结点)
	 * @param isTemp  是否临时结点
	 * @param isSequnce 是否序列结点
	 * @param handler 支持传递completion handler
	 * @param context 上下文信息
	 * @return 返回结果  成功ZOK,其他失败
	 */
	int asyncCreateNode(const string & path,const string & value,
	                    bool isTemp=false,bool isSequence=false,
	                    StringCompletionHandler handler = nullptr, void * context = nullptr);
	/**
	 * 异步方式删除结点
	 * @param path 结点路径
	 * @param version  结点版本 -1表示不校验版本,其他表示验证版本，版本正确才能正确删除
	 * @param handler 支持传递completion handler
	 * @param context 上下文信息
	 * @return 返回结果  成功ZOK,其他失败
	 */
	int asyncDeleteNode(const string & path,int version= IGNORE_VERSION,
	                    VoidCompletionHandler handler = nullptr, void * context = nullptr);
	/**
	 * 异步获取结点数据
	 * @param path 结点路径
	 * @param value 结点数据
	 * @param version 结点版本
	 * @param watch 0-不启用监视  1-启用连接的默认监视
	 * @return 返回结果  成功ZOK,其他失败
	 */
	int asyncGetNodeValue(const string & path,string & value,int & version,const int watch=0,
						  DataCompletionHandler handler = nullptr,void * context = nullptr);

	/**
	 * 异步获取结点数据(带监视器版本)
	 * @param path 结点路径
	 * @param value 结点数据
	 * @param version 结点版本
	 * @param watchHandler 监视处理器
	 * @param watchContext 监视处理器上下文
	 * @param completionHandler 异步完成处理器
	 * @param completionContext 异步完成处理器上下文
	 * @return 返回结果  成功ZOK,其他失败
	 */
	int asyncGetNodeValueWithWatcher(const string & path,string & value, int & version,
	                                 GetNodeHandler watchHandler = nullptr,void * watchContext = nullptr,
	                                 DataCompletionHandler completionHandler = nullptr,void * completionContext = nullptr);
	/**
	 * 异步设置结点信息
	 * @param path 结点路径
	 * @param value 结点数据
	 * @param version 结点版本
	 * @param handler 异步完成处理器
	 * @param context 异步完成处理器上下文
	 * @return 成功ZOK,其他失败
	 */
	int asyncSetNodeValue(const string & path,const string & value,int version = IGNORE_VERSION,
						  StatCompletionHandler handler = nullptr,void * context = nullptr);

	/**
	 * 异步获取子节点信息
	 * @param path 结点路径
	 * @param handler 异步完成处理器
	 * @param context 异步完成处理器上下文
	 * @return 成功ZOK,其他失败
	 */
	int asyncGetChildren(const string & path,StringsCompletionHandler handler = nullptr,void * context = nullptr);

	/**
	 * 异步获取子节点信息(带监视)
	 * @param path 结点路径
	 * @param watchHandler 监视处理器
	 * @param watchContext 监视处理器上下文
	 * @param completionHandler 异步完成处理器
	 * @param completionContext 异步完成处理器上下文
	 * @return 成功ZOK,其他失败
	 */
	int asyncGetChildrenWithWatcher(const string & path, GetChildrenHandler watchHandler = nullptr,
	                                void * watchContext = nullptr,
	                                StringsCompletionHandler completionHandler = nullptr,
	                                void * completionContext = nullptr);

	/**
	 * 异步获取子节点信息(支持带stat版本处理器)
	 * @param path 结点路径
	 * @param handler 异步完成处理器
	 * @param context 异步完成处理器上下文
	 * @return 成功ZOK,其他失败
	 */
	int asyncGetChildren2(const string & path,StringsStatCompletionHandler handler = nullptr,void * context = nullptr);

	/**
	 * 异步获取子节点信息(带监视,支持带stat版本处理器)
	 * @param path 结点路径
	 * @param watchHandler 监视处理器
	 * @param watchContext 监视处理器上下文
	 * @param completionHandler 异步完成处理器
	 * @param completionContext 异步完成处理器上下文
	 * @return 成功ZOK,其他失败
	 */
	int asyncGetChildren2WithWatcher(const string & path, GetChildrenHandler watchHandler = nullptr,
	                                 void * watchContext = nullptr,
	                                 StringsStatCompletionHandler completionHandler = nullptr,
	                                 void * completionContext = nullptr);

	/**
	 * 异步获取结点ACL信息
	 * @param path 结点路径
	 * @param handler 异步完成处理器
	 * @param context 异步完成处理器上下文
	 * @return 成功ZOK,其他失败
	 */
	int asyncGetNodeACL(const string & path,AclCompletionHandler handler = nullptr,void * context = nullptr);

	/**
	 * 异步设置结点ACL信息
	 * @param path 结点路径
	 * @param aclVector ACL信息
	 * @param version 版本
	 * @param handler 异步完成处理器
	 * @param context 异步完成处理器上下文
	 * @return 成功ZOK,其他失败
	 */
	int asyncSetNodeACL(const string & path,ACLVector * aclVector,int version = IGNORE_VERSION,
	                    VoidCompletionHandler handler = nullptr,void * context = nullptr);
	/*
	 *	zoo_async
	 *	zoo_amulti 批处理，即原子性地一次提交多个ZK操作
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


