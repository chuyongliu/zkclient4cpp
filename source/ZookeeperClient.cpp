//
// Created by liuchuyong on 2017/3/23.
//
#include <string>
#include <zookeeper/zookeeper.h>
#include <spdlog/spdlog.h>
#include "ZookeeperClient.h"
#include "WatcherEventHandle.h"

using namespace std;

ZookeeperClient::ZookeeperClient(const string & server,SessionWatcherHandler handler,int timeout)
{
	logger_->info("ZookeeperClient::ZookeeperClient");
	server_ = server;
	sessionWatcherHandler_ = handler;
	timeout_ = timeout;

	//!!!!构造函数里面不能调用shared_from_this(),将与服务端建立连接的动作移至connectServer!!!!
	//ZkWatcherOperateContext * context = new ZkWatcherOperateContext(server,(void*)"",shared_from_this());
}
ZookeeperClient::~ZookeeperClient()
{
	logger_->info("ZookeeperClient::~ZookeeperClient()");
	closeServer();
}

void ZookeeperClient::connectServer()
{
	std::lock_guard<std::mutex> lock(mutex_);
	ZkWatcherOperateContext * context = new ZkWatcherOperateContext(server_,(void*)"connect server",shared_from_this());
	context->sessionWatcherHandler_ = sessionWatcherHandler_;

	zkHandle_ = zookeeper_init(server_.data(),sessionWatcher,timeout_, nullptr, context, 0);
	if(zkHandle_)
		connectStatus_ = true;
}

void ZookeeperClient::closeServer()
{
	logger_->info("closing server...");
	std::lock_guard<std::mutex> lock(mutex_);
	zookeeper_close(zkHandle_);
}

void ZookeeperClient::reConnectServer()
{
	{
		std::lock_guard<std::mutex> lockGuard(mutex_);
		if(zkHandle_)
			zookeeper_close(zkHandle_);
	}
	connectServer();
}

int ZookeeperClient::getNodeValue(const string & path,string & value, int & version,const int watch)
{
	char buffer[MAX_BUFF_LEN] = {0};
	int length = sizeof(buffer);
	struct Stat stat;
	int result = zoo_get(zkHandle_,path.data(),watch,buffer,&length,&stat);
	if(result == ZOK)
	{
		value.assign(buffer,length);
		version = stat.version;
	}
	return result;
}

int ZookeeperClient::getNodeValueWithWatcher(const string & path,string & value, int & version,
		GetNodeHandler getNodeHandler,void * context)
{
	char buffer[MAX_BUFF_LEN] = {0};
	int length = sizeof(buffer);
	Stat stat;
	ZkWatcherOperateContext * watchContext = new ZkWatcherOperateContext( path, context, shared_from_this());
	watchContext->getNodeHandler_ = getNodeHandler;

	int result = zoo_wget(zkHandle_,path.data(),getNodeValueWatcher,watchContext,buffer,&length,&stat);
	if(result == ZOK)
	{
		value.assign(buffer,length);
		version = stat.version;
	}
	return result;
}

int ZookeeperClient::getNodeACL(const string & path,VectorACL & vectorACL)
{
	ACL_vector aclVector;
	int result  = zoo_get_acl(zkHandle_,path.data(),&aclVector,0);
	if (result == ZOK)
	{
		for(int i = 0; i < aclVector.count; i++)
			vectorACL.push_back(aclVector.data[i]);
	}
	deallocate_ACL_vector(&aclVector);
	return result;
}

int ZookeeperClient::setNodeValue(const string & path,
		const string & value,int version,struct Stat * stat)
{
	int result = zoo_set2(zkHandle_,path.data(),value.data(),value.length(),version,stat);
	return result;
}

int ZookeeperClient::existsNode(const string & path,struct Stat * stat)
{
	int result = zoo_exists(zkHandle_, path.data(),0,stat);
	return result;
}

int ZookeeperClient::existsNodeWithWatch(const string & path,ExistsNodeHandler existsNodeHandler,void * context)
{
	ZkWatcherOperateContext * watchContext = new ZkWatcherOperateContext(path,context,shared_from_this());
	watchContext->existsNodeHandler_ = existsNodeHandler;

	int result = zoo_wexists(zkHandle_,path.data(),existsNodeWatcher,watchContext,0);
	return result;
}

int ZookeeperClient::createNode(const string & path,const string & value, bool isTemp ,bool isSequence)
{
	int flag = 0;
	if(isTemp)
	{
		flag |= ZOO_EPHEMERAL;//temporary node
	}
	if (isSequence)
	{
		flag |=ZOO_SEQUENCE;
	}
	int result = zoo_create(zkHandle_,path.data(),value.data(), value.length(), &ZOO_OPEN_ACL_UNSAFE, flag, 0, 0);
	return result;
}

int ZookeeperClient::deleteNode(const string & path,int version )
{
	int result = zoo_delete(zkHandle_,path.data(),version);
	return result;//== ZOK || result == ZNONODE;删除成功或者结点不存在，都认为是删除成功
}

bool ZookeeperClient::getConectStatus()
{
	std::lock_guard<std::mutex> lockGuard(mutex_);
	return connectStatus_;
}

void ZookeeperClient::setConectStatus(bool connectStatus)
{
	std::lock_guard<std::mutex> lockGuard(mutex_);
	connectStatus_ = connectStatus;
}

int ZookeeperClient::getClildren(const string & path,VectorString & childrenVec,
		GetChildrenHandler getChildrenNodeHandler,void * context)
{
	ZkWatcherOperateContext * watchContext = new ZkWatcherOperateContext(path,context, shared_from_this());
	watchContext->getChildrenHandler_ = getChildrenNodeHandler;
	/**
	 * zoo_wget_children2将stat传0，会coredump(会自动传回stat,但是并没有事先判断stat是否为空)
	 * int result = zoo_wget_children2(zkHandle_,path.data(),watcher,context,&stringVec,0);
	 */
	ChildrenVector stringVec;
	int result = zoo_wget_children(zkHandle_,path.data(),getChildrenWatcher,watchContext,&stringVec);
	if (result == ZOK)
	{
		transChildVector2VectorString(stringVec,&childrenVec);
		deallocate_String_vector(&stringVec);
	}
	return result;
}

/*********************************************************************************************************/
//各种默认的监视回调函数实现版本
void ZookeeperClient::sessionWatcher(zhandle_t *zh, int type,int state, const char *path,void *watcherCtx)
{
	auto logger = spdlog::get(getGlobalLogName());
	logger->info("[sessionWatcher] type:{} ,state:{}", watcherEventType2String(type),state2String(state));

	ZkWatcherOperateContext * context = (ZkWatcherOperateContext *) watcherCtx;
	if (context->sessionWatcherHandler_)
	{
		context->sessionWatcherHandler_(type,state,context->zkClientPtr_,context->context_);
	}
	destroyPointer(context);
}

void ZookeeperClient::existsNodeWatcher(zhandle_t *zh, int type,int state, const char *path,void *watcherCtx)
{
	auto logger = spdlog::get(getGlobalLogName());
	logger->info("[existsNodeWatcher] type:{} ,state:{}", watcherEventType2String(type),state2String(state));

	ZkWatcherOperateContext * context = (ZkWatcherOperateContext *)watcherCtx;
	if (context->existsNodeHandler_)
	{
		context->existsNodeHandler_(type,state,context->zkClientPtr_,path,context->context_);
	}
	destroyPointer(context);
}

void ZookeeperClient::getNodeValueWatcher(zhandle_t *zh, int type,int state, const char *path,void *watcherCtx)
{
	auto logger = spdlog::get(getGlobalLogName());
	logger->info("[getNodeValueWatcher] type:{} ,state:{}", watcherEventType2String(type),state2String(state));

	ZkWatcherOperateContext * context = (ZkWatcherOperateContext *)watcherCtx;
	if (context->getNodeHandler_)
	{
		string data;
		int version = 0;
		context->getNodeHandler_(type,state,context->zkClientPtr_,path,data,version,context->context_);
	}
	destroyPointer(context);
}

void ZookeeperClient::getChildrenWatcher(zhandle_t *zh, int type,int state, const char *path,void *watcherCtx)
{
	auto logger = spdlog::get(getGlobalLogName());
	logger->info("[getChildrenWatcher] type:{} ,state:{}", watcherEventType2String(type),state2String(state));

	ZkWatcherOperateContext * context = (ZkWatcherOperateContext *) watcherCtx;
	if (context->getChildrenHandler_)
	{
		VectorString childrenVec;
		context->getChildrenHandler_(type,state,context->zkClientPtr_,path,childrenVec,context->context_);
	}
	destroyPointer(context);
}

/*********************************************************************************************************/
//异步API对应的处理函数
void ZookeeperClient::asyncCreateNodeHandler(int rc, const char *value, const void *data)
{
//	auto logger = spdlog::get(getGlobalLogName());
//	logger->info("asyncCreateNodeHandler");
	ZkAsyncCompletionContext * completionContext = (ZkAsyncCompletionContext *)data;
	if(completionContext->stringCompletionHandler_)
	{
		completionContext->stringCompletionHandler_(rc,value,completionContext);
	}
	destroyPointer(completionContext);
}

void ZookeeperClient::asyncDeleteNodeHandler(int rc, const void *data)
{
//	auto logger = spdlog::get(getGlobalLogName());
//	logger->info("asyncDeleteNodeHandler");
	ZkAsyncCompletionContext * completionContext = (ZkAsyncCompletionContext *) data;
	if(completionContext->voidCompletionHandler_)
	{
		completionContext->voidCompletionHandler_(rc,completionContext);
	}
	destroyPointer(completionContext);
}

void ZookeeperClient::asyncGetNodeHandler(int rc,const char *value, int value_len,const struct Stat *stat, const void *data)
{
//	auto logger = spdlog::get(getGlobalLogName());
//	logger->info("asyncGetNodeHandler");
	ZkAsyncCompletionContext * completionContext = (ZkAsyncCompletionContext *)data;
	if (completionContext->dataCompletionHandler_)
	{
		completionContext->dataCompletionHandler_(rc,value,value_len,stat,completionContext);
	}
	destroyPointer(completionContext);
}

void ZookeeperClient::asyncSetNodeHandler(int rc, const struct Stat *stat, const void *data)
{
//	auto logger = spdlog::get(getGlobalLogName());
//	logger->info("asyncSetNodeHandler");
	ZkAsyncCompletionContext * completionContext = (ZkAsyncCompletionContext *)data;
	if (completionContext->statCompletionHandler_)
	{
		completionContext->statCompletionHandler_(rc,stat,completionContext);
	}
	destroyPointer(completionContext);
}

void ZookeeperClient::asyncGetChildrenHandler(int rc,const struct String_vector *strings, const void *data)
{
//	auto logger = spdlog::get(getGlobalLogName());
//	logger->info("asyncGetChildrenHandler");
	ZkAsyncCompletionContext * completionContext = (ZkAsyncCompletionContext *)data;
	if (completionContext->stringsCompletionHandler_)
	{
		completionContext->stringsCompletionHandler_(rc,strings,completionContext);
	}
	destroyPointer(completionContext);
}

void ZookeeperClient::asyncGetChildrenHandler2(int rc, const struct String_vector *strings,
		const struct Stat *stat,const void *data)
{
//	auto logger = spdlog::get(getGlobalLogName());
//	logger->info("asyncGetChildrenHandler2");
	ZkAsyncCompletionContext * completionContext = (ZkAsyncCompletionContext *)data;
	if (completionContext->stringsStatCompletionHandler_)
	{
		completionContext->stringsStatCompletionHandler_(rc,strings,stat,completionContext);
	}
	destroyPointer(completionContext);
}

void ZookeeperClient::asyncGetNodeACLHandler(int rc, struct ACL_vector *acl, struct Stat *stat, const void *data)
{
//	auto logger = spdlog::get(getGlobalLogName());
//	logger->info("asyncGetAclHandler");
	ZkAsyncCompletionContext * completionContext = (ZkAsyncCompletionContext *)data;
	if (completionContext->aclCompletionHandler_)
	{
		completionContext->aclCompletionHandler_(rc,acl,stat,completionContext);
	}
	destroyPointer(completionContext);
}

void ZookeeperClient::asyncSetNodeACLHandler(int rc, const void *data)
{
//	auto logger = spdlog::get(getGlobalLogName());
//	logger->info("asyncSetNodeACLHandler");
	ZkAsyncCompletionContext * completionContext = (ZkAsyncCompletionContext *)data;
	if (completionContext->voidCompletionHandler_)
	{
		completionContext->voidCompletionHandler_(rc,completionContext);
	}
	destroyPointer(completionContext);
}

/*********************************************************************************************************/
//异步API函数封装
int ZookeeperClient::asyncCreateNode(const string & path,const string & value,
		bool isTemp ,bool isSequence, StringCompletionHandler handler, void * context)
{
	int flag = 0;
	if(isTemp)
		flag |= ZOO_EPHEMERAL;//temporary node

	if (isSequence)
		flag |=ZOO_SEQUENCE;

	ZkAsyncCompletionContext * completionContext = new ZkAsyncCompletionContext(path,context,shared_from_this());
	completionContext->stringCompletionHandler_ = handler;

	int result = zoo_acreate(zkHandle_,path.data(),value.data(),value.length(),
		&ZOO_OPEN_ACL_UNSAFE, flag, asyncCreateNodeHandler,completionContext);

	return result;
}

int ZookeeperClient::asyncDeleteNode(const string & path,int version ,
		VoidCompletionHandler handler , void * context)
{
	logger_->info("asyncDeleteNode path[{}]",path);
	ZkAsyncCompletionContext * completionContext = new ZkAsyncCompletionContext(path,context,shared_from_this());
	completionContext->voidCompletionHandler_ = handler;

	int result = zoo_adelete(zkHandle_,path.data(),version,asyncDeleteNodeHandler,completionContext);
	return result;
}

int ZookeeperClient::asyncGetNodeValue(const string &path, string &value, int &version, const int watch,
		DataCompletionHandler handler , void * context )
{
	logger_->info("asyncGetNodeValueWithWatcher path[{}]",path);
	ZkAsyncCompletionContext * completionContext = new ZkAsyncCompletionContext(path,context,shared_from_this());
	completionContext->dataCompletionHandler_  = handler;
	int result = zoo_aget(zkHandle_,path.data(),watch, asyncGetNodeHandler, completionContext);
	return result;
}

int ZookeeperClient::asyncGetNodeValueWithWatcher(const string &path, string &value, int &version,
		GetNodeHandler watchHandler,void * watchContext, DataCompletionHandler completionHandler,void * completionContext)
{
	logger_->info("asyncGetNodeValueWithWatcher path[{}],value[{}]",path,value);
	ZkWatcherOperateContext * watcherOperateContext = new ZkWatcherOperateContext(path,watchContext,shared_from_this());
	watcherOperateContext->getNodeHandler_ = watchHandler;

	ZkAsyncCompletionContext * asyncCompletionContext = new ZkAsyncCompletionContext(path,completionContext,shared_from_this());
	asyncCompletionContext->dataCompletionHandler_ = completionHandler;

	int result = zoo_awget(zkHandle_,path.data(),getNodeValueWatcher,watcherOperateContext,
	                       asyncGetNodeHandler,asyncCompletionContext);
	return result;
}

int ZookeeperClient::asyncSetNodeValue(const string & path, const string & value, int version,
		StatCompletionHandler handler, void *context)
{
	logger_->info("asyncSetNodeValue path[{}],value[{}]",path,value);
	ZkAsyncCompletionContext * completionContext = new ZkAsyncCompletionContext(path,context,shared_from_this());
	completionContext->statCompletionHandler_ = handler;

	int result = zoo_aset(zkHandle_,path.data(),value.data(),value.length(),version,
	                      asyncSetNodeHandler,completionContext);
	return result;
}

int ZookeeperClient::asyncGetChildren(const string &path,StringsCompletionHandler handler, void *context)
{
	logger_->info("asyncGetChildren path[{}]",path);
	ZkAsyncCompletionContext * completionContext = new ZkAsyncCompletionContext(path, context,shared_from_this());
	completionContext->stringsCompletionHandler_ = handler;
	int result = zoo_aget_children(zkHandle_,path.data(),0,asyncGetChildrenHandler,completionContext);
	return result;
}

int ZookeeperClient::asyncGetChildrenWithWatcher(const string &path,GetChildrenHandler watchHandler,
		void * watchContext,StringsCompletionHandler completionHandler, void *context)
{
	logger_->info("asyncGetChildrenWithWatcher path[{}]",path);
	ZkWatcherOperateContext * watcherContext = new ZkWatcherOperateContext(path, context, shared_from_this());
	watcherContext->getChildrenHandler_ = watchHandler;

	ZkAsyncCompletionContext * completionContext = new ZkAsyncCompletionContext(path, context, shared_from_this());
	completionContext->stringsCompletionHandler_ = completionHandler;

	int result = zoo_awget_children(zkHandle_,path.data(),getChildrenWatcher,watcherContext,
	                                asyncGetChildrenHandler,completionContext);
	return result;
}

int ZookeeperClient::asyncGetChildren2(const string &path,StringsStatCompletionHandler handler,void *context)
{
	logger_->info("asyncGetChildren2 path[{}]",path);
	ZkAsyncCompletionContext * completionContext = new ZkAsyncCompletionContext(path, context, shared_from_this());
	completionContext->stringsStatCompletionHandler_ = handler;

	int result = zoo_aget_children2(zkHandle_,path.data(),0,asyncGetChildrenHandler2,completionContext);
	return result;
}

int ZookeeperClient::asyncGetChildren2WithWatcher(const string &path, GetChildrenHandler watchHandler,
		void * watchContext,StringsStatCompletionHandler completionHandler, void *completionContext)
{
	logger_->info("asyncGetChildren2WithWatcher path[{}]",path);
	ZkWatcherOperateContext * watcherContext = new ZkWatcherOperateContext(path,watchContext, shared_from_this());
	watcherContext->getChildrenHandler_ = watchHandler;

	ZkAsyncCompletionContext * asyncCompletionContext = new ZkAsyncCompletionContext(path, completionContext, shared_from_this());
	asyncCompletionContext->stringsStatCompletionHandler_ = completionHandler;

	int result = zoo_awget_children2(zkHandle_,path.data(),getChildrenWatcher,watcherContext,
	                                 asyncGetChildrenHandler2,asyncCompletionContext);
	return result;
}

int ZookeeperClient::asyncGetNodeACL(const string &path,AclCompletionHandler handler,void * context )
{
	logger_->info("asyncGetNodeACL path[{}]",path);
	ZkAsyncCompletionContext * completionContext = new ZkAsyncCompletionContext(path, context, shared_from_this());
	completionContext->aclCompletionHandler_ = handler;

	int result  = zoo_aget_acl(zkHandle_,path.data(),asyncGetNodeACLHandler,completionContext);
	return result;
}

int ZookeeperClient::asyncSetNodeACL(const string &path, ACLVector * aclVector, int version,
		VoidCompletionHandler handler,void *context)
{
	logger_->info("asyncSetNodeACL path[{}]",path);
	ZkAsyncCompletionContext * completionContext = new ZkAsyncCompletionContext(path, context, shared_from_this());
	completionContext->voidCompletionHandler_ = handler;

	int result  = zoo_aset_acl(zkHandle_,path.data(),version,aclVector,asyncSetNodeACLHandler,completionContext);
	return result;
}