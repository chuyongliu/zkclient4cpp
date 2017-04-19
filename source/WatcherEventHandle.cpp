//
// Created by liuchuyong on 2017/3/23.
//

#include <iostream>
#include "WatcherEventHandle.h"
#include "ZookeeperClient.h"

using namespace std;

WatcherEventType WatcherEventTransfer::sessionEvent2WatcherEventType(int type)
{
	if ( type == ZOO_SESSION_EVENT){
		return WatcherEventType::SessionEvent;//
	}

	return WatcherEventType::NullEvent;
}

WatcherEventType WatcherEventTransfer::existsEvent2WatcherEventType(int type)
{
	if ( type == ZOO_CREATED_EVENT || type == ZOO_CHANGED_EVENT)
	{
		return WatcherEventType::GetNode;
	}
	else if (type == ZOO_DELETED_EVENT)
	{
		return WatcherEventType::DeleteNode;
	}

	return WatcherEventType::NullEvent;
}

WatcherEventType WatcherEventTransfer::getNodeEvent2WatcherEventType(int type)
{
	if (type == ZOO_CHANGED_EVENT)
	{
		return WatcherEventType::GetNode;
	}
	else if (type == ZOO_DELETED_EVENT)
	{
		return WatcherEventType::DeleteNode;
	}
	return WatcherEventType::NullEvent;
}

WatcherEventType WatcherEventTransfer::getChidrenEvent2WatcherEventType(int type)
{
	if (type == ZOO_CHILD_EVENT)
	{
		return WatcherEventType::GetChidren;
	}
	return WatcherEventType::NullEvent;
}

void SimpleSessionWatcherHandler::handle(int type, int state, const ZkClientPtr &zkClientPtr, void *context)
{
	WatcherEventType event = WatcherEventTransfer::sessionEvent2WatcherEventType(type);
	if( event == WatcherEventType::SessionEvent)
	{
		if (state == ZOO_CONNECTED_STATE)
		{
			logger_->info("connect server succ");
		}
		else if(state == ZOO_AUTH_FAILED_STATE)
		{
			logger_->info("Authentication failure.");
			zkClientPtr->closeServer();
		}
		else if(state == ZOO_EXPIRED_SESSION_STATE)
		{
			logger_->info("session expired,reconnecting...");
			zkClientPtr->setConectStatus(false);//设置连接状态
			zkClientPtr->reConnectServer();
		}
	}
}

void SinglePathWatcherHandler::getNodeData(int event,const ZkClientPtr & zkClientPtr,
		const string & path, string & nodeData,int & version, void * context)
{
	int result = zkClientPtr->getNodeValue(path,nodeData,version);
	if(result == ZOK)
	{
		PathValueInWatcher pathValueInWatcher(nodeData,event);
		lock_guard<mutex> lockGuard(mutex_);
		pathValueWatcherMap_[path] = pathValueInWatcher;
	}
}
//获取处理类保存的结点信息，找不到或是不存在表示没有触发过监视
bool SinglePathWatcherHandler::getValueInWatcher(const string & path, PathValueInWatcher & pathValueInWatcher)
{
	lock_guard<mutex> lockGuard(mutex_);
	auto iter = pathValueWatcherMap_.find(path);
	if ( iter != pathValueWatcherMap_.end() )
		pathValueInWatcher = iter->second;
	else
		return false;

	return true;
}

void SinglePathWatcherHandler::deleteNode(int event, const string &path)
{
	PathValueInWatcher pathValueInWatcher("",event);
	lock_guard<mutex> lockGuard(mutex_);
	pathValueWatcherMap_[path] = pathValueInWatcher;//记录的情况内容滞空,同时标记下eventType(DeleteNode)
}

void SinglePathWatcherHandler::eventHandle(int event,const ZkClientPtr & zkClientPtr,
    const string & path,string & value, int & version,void * context)
{
	if (event == WatcherEventType::GetNode)
		getNodeData(event,zkClientPtr,path,value,version,context);
	else if(event == WatcherEventType::DeleteNode)
		deleteNode(event,path);
}
void GetValueWatcherOnceHandler::handle(int type,int state,const ZkClientPtr & zkClientPtr,
	const string & path, string & value,int & version,void * context)
{
	logger->info("GetValueWatcherOnceHandler::handler");
	WatcherEventType event = WatcherEventTransfer::getNodeEvent2WatcherEventType(type);
	eventHandle(event,zkClientPtr,path,value,version,context);
}

void PathExistsWatcherOnceHandler::handle(int type, int state, const ZkClientPtr &zkClientPtr,
	const string &path,void *context)
{
	string nodeData;
	int version = 0;
	WatcherEventType event = WatcherEventTransfer::existsEvent2WatcherEventType(type);
	eventHandle(event,zkClientPtr,path,nodeData,version,context);
}

void PathExistsWatcherContinuallyHandler::handle(int eventType, const ZkClientPtr &zkClientPtr,
     const string &path, void *context)
{
	if (eventType == WatcherEventType::GetNode)
	{
		string nodeData;
		int version = 0;
		getNodeData(eventType,zkClientPtr,path,nodeData,version,context);
	}
	else if( eventType == WatcherEventType::DeleteNode)
	{
		deleteNode(eventType,path);
	}
}

void ChildrenWatcherOnceHandler::printChildrenList()
{
	lock_guard<mutex> lockGuard(mutex_);
	for(auto & iter : childrenList_)
	{
		logger->info("{}:",iter.first);
		printPathList(iter.second);
	}
}

void ChildrenWatcherOnceHandler::handle(int type,int state,const ZkClientPtr& zkClientPtr,
    const string& path, VectorString & childrenVec,void* context)
{
	WatcherEventType event =  WatcherEventTransfer::getChidrenEvent2WatcherEventType(type);
	if (event == WatcherEventType::GetChidren)
	{
		getChidlren(event,zkClientPtr,path,childrenVec,context);
	}
}

void ChildrenWatcherOnceHandler::getChidlren(int eventType,const ZkClientPtr &zkClientPtr,
	const string &path,VectorString & childrenVec, void *context)
{
	logger->info("OnceChildrenWatcherHandler::getChidlren");
	int result = zkClientPtr->getClildren(path,childrenVec);
	if (result == ZOK)
	{
		lock_guard<mutex> lockGuard(mutex_);
		childrenList_[path] = childrenVec;
	}
}

void ChildrenWatcherContinuallyHandler::getChidlren(int eventType, const ZkClientPtr & zkClientPtr,
	const string & path, VectorString & childrenVec,void *context)
{
	logger->info("MultiChildrenWatcherHandler::getChidlren");
	auto handler = bind(&ChildrenWatcherContinuallyHandler::handle,this, _1, _2, _3,_4,_5,_6);

	int result = zkClientPtr->getClildren(path,childrenVec,handler);
	if (result == ZOK)
	{
		lock_guard<mutex> lockGuard(mutex_);
		childrenList_[path] = childrenVec;//vector的operator=操作是先clear,再alloc-on-copy
	}
}