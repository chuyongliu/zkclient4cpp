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
 * �����¼����͵��ص��¼���ת��ӳ��
 */
class WatcherEventTransfer
{
public:
	static WatcherEventType sessionEvent2WatcherEventType(int type);
	static WatcherEventType existsEvent2WatcherEventType(int type);
	static WatcherEventType getNodeEvent2WatcherEventType(int type);
	static WatcherEventType getChidrenEvent2WatcherEventType(int type);
};

//�򵥵�session�¼��Ĵ�����(ֻ����session�¼�)
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
 * �������Ӵ���������
 */
class SinglePathWatcherHandler
{
public:
	//��ȡ�����ౣ��Ľ����Ϣ���Ҳ������ǲ����ڱ�ʾû�д���������
	bool getValueInWatcher(const string & path,PathValueInWatcher & pathValueInWatcher);
protected:
	void eventHandle(int event,const ZkClientPtr & zkClientPtr, const string & path,
	                 string & value, int & nodeVersion,void * context);
	void getNodeData(int event,const ZkClientPtr & zkClientPtr,const string & path,
	                 string & value,int & nodeVersion, void * context);
	void deleteNode(int event, const string & path);
protected:
	PathValueWatcherMap pathValueWatcherMap_;//��Ŵ�������֮���Ӧ������Ϣ  ���·����������ݣ���Ӧ�Ĳ���
	mutex mutex_;
	std::shared_ptr<spdlog::logger> logger = spdlog::get(getGlobalLogName());
};

/**
 * ����ȡ���Ӵ�������(֧�ֱ���仯֮��Ľ����Ϣ)
 */
class GetValueWatcherOnceHandler : public SinglePathWatcherHandler
{
public:
	void handle(int type, int state,const ZkClientPtr & zkClientPtr,const string & path,
	            string & value, int & nodeVersion, void * context);
};

/**
 * �����Ӵ�������(֧�ֱ���仯֮��Ľ����Ϣ)
 */
class PathExistsWatcherOnceHandler : public SinglePathWatcherHandler
{
public:
	virtual void handle(int type,int state,const ZkClientPtr &zkClientPtr,const string &path, void *context);
};

/**
 * �����Ӵ�������(��������)
 */
class PathExistsWatcherContinuallyHandler : public SinglePathWatcherHandler
{
public:
	virtual void handle(int eventType, const ZkClientPtr &zkClientPtr,const string &path, void *context);
};

/**
 * ��Ŀ¼���Ӵ�������(ֻ�ܼ���һ���¼�����)
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
	ChildrenList childrenList_;//��ż��ӽ���Ӧ�ӽ���б��ӳ��
	mutex mutex_;
	std::shared_ptr<spdlog::logger> logger = spdlog::get(getGlobalLogName());
};

/**
 * ��Ŀ¼���Ӵ�������(��������)
 */
class ChildrenWatcherContinuallyHandler : public ChildrenWatcherOnceHandler
{
protected:
	virtual void getChidlren(int eventType, const ZkClientPtr & zkClientPtr,const string & path,
	                         VectorString & childrenList,void *context);
};
