//
// Created by liuchuyong on 2017/3/23.
//
#pragma once

#include <iostream>
#include <string>
#include <future>
#include <functional>
#include "CommonUtil.h"
#include "ZookeeperClient.h"

using  namespace std;

/**
 * 监听事件类型到回调事件的转化映射
 */
class WatcherEventTransfer
{
public:
	static WatcherEventType sessionEvent2WatcherEventType(int type);
	static WatcherEventType existsEvent2WatcherEventType(int type);
	static WatcherEventType getNodeEvent2WatcherEventType(int type);
	static WatcherEventType getChidrenEvent2WatcherEventType(int type);
};

//简单的session事件的处理类(只处理session事件)
class SimpleSessionWatcherHandler
{
public:
	void handle(int type,int state,const ZkClientPtr & zkClientPtr, void * context);
protected:
	std::shared_ptr<spdlog::logger> logger_ = spdlog::get(getGlobalLogName());
};

class PathValueInWatcher;
typedef map<string,PathValueInWatcher> PathValueWatcherMap;


/**
 * 单结点监视处理器基类
 */
class SinglePathWatcherHandler
{
public:
	//获取处理类保存的结点信息，找不到或是不存在表示没有触发过监视
	bool getValueInWatcher(const string & path,PathValueInWatcher & pathValueInWatcher);
protected:
	void eventHandle(int event,const ZkClientPtr & zkClientPtr, const string & path,
	                 string & value, int & nodeVersion,void * context);
	void getNodeData(int event,const ZkClientPtr & zkClientPtr,const string & path,
	                 string & value,int & nodeVersion, void * context);
	void deleteNode(int event, const string & path);
protected:
	PathValueWatcherMap pathValueWatcherMap_;//存放触发监视之后对应结点的信息  结点路径，结点数据，相应的操作
	mutex mutex_;
	std::shared_ptr<spdlog::logger> logger = spdlog::get(getGlobalLogName());
};

/**
 * 结点获取监视处理器类(支持保存变化之后的结点信息)
 */
class GetValueWatcherOnceHandler : public SinglePathWatcherHandler
{
public:
	void handle(int type, int state,const ZkClientPtr & zkClientPtr,const string & path,
	            string & value, int & nodeVersion, void * context);
};

/**
 * 结点监视处理器类(支持保存变化之后的结点信息)
 */
class PathExistsWatcherOnceHandler : public SinglePathWatcherHandler
{
public:
	virtual void handle(int type,int state,const ZkClientPtr &zkClientPtr,const string &path, void *context);
};

/**
 * 结点监视处理器类(持续监视)
 */
class PathExistsWatcherContinuallyHandler : public SinglePathWatcherHandler
{
public:
	virtual void handle(int eventType, const ZkClientPtr &zkClientPtr,const string &path, void *context);
};

/**
 * 子目录监视处理器类(只能监视一次事件发生)
 */
class ChildrenWatcherOnceHandler
{
public:
	void handle(int type, int state, const ZkClientPtr& zkClientPtr,const string& path,
	            VectorString & childrenVec,void* context);

	void printChildrenList();
protected:
	virtual void getChidlren(int eventType, const ZkClientPtr & zkClientPtr,const string & path,
	                         VectorString & childrenVec,void *context);
protected:
	ChildrenList childrenList_;//存放监视结点对应子结点列表的映射
	mutex mutex_;
	std::shared_ptr<spdlog::logger> logger = spdlog::get(getGlobalLogName());
};

/**
 * 子目录监视处理器类(持续监视)
 */
class ChildrenWatcherContinuallyHandler : public ChildrenWatcherOnceHandler
{
protected:
	virtual void getChidlren(int eventType, const ZkClientPtr & zkClientPtr,const string & path,
	                         VectorString & childrenList,void *context);
};
