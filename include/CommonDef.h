//
// Created by liuchuyong on 2017/3/23.
//
#pragma once

#include <string>
#include <map>
#include <vector>
#include <functional>
#include "zookeeper/zookeeper.h"
using namespace std;

#define MAX_BUFF_LEN  1024

typedef vector<ACL> VectorACL;
typedef vector<string> VectorString;
typedef map<string,vector<string> > ChildrenList;
typedef map<string,string> NodeValueMap;
typedef map<string,bool> NodeResultMap;
typedef String_vector ChildrenVector;
typedef vector<string> PathVector;
typedef ACL_vector ACLVector;

struct DataCompletion
{
	string value_;
	int length_;
	Stat stat_;
};

struct ChildrenStat
{
	VectorString vectorString_;
	Stat stat_;
};

struct ACLStat
{
	VectorACL vectorACL_;
	Stat stat_;
};

typedef map<string,DataCompletion> NodeDataCompletionMap;
typedef map<string,Stat> NodeStatMap;
typedef map<string,VectorString> NodeChildrenMap;
typedef map<string,ChildrenStat> NodeChildrenStatMap;
typedef map<string,ACLStat> NodeAclStatMap;

enum WatcherEventType
{
	NullEvent = 0,
	SessionEvent,
	ExistsNode,
	GetNode,
	CreateNode,
	NodeChange,
	DeleteNode,
	GetChidren
};

//摘自zookeeper源文件 zk_adptor.h
/* zookeeper event type constants */
#define CREATED_EVENT_DEF 1
#define DELETED_EVENT_DEF 2
#define CHANGED_EVENT_DEF 3
#define CHILD_EVENT_DEF 4
#define SESSION_EVENT_DEF -1
#define NOTWATCHING_EVENT_DEF -2

//摘自zookeeper源文件 zk_adptor.h
/* zookeeper state constants */
#define EXPIRED_SESSION_STATE_DEF -112
#define AUTH_FAILED_STATE_DEF -113
#define CONNECTING_STATE_DEF 1
#define ASSOCIATING_STATE_DEF 2
#define CONNECTED_STATE_DEF 3
#define NOTCONNECTED_STATE_DEF 999


//为了书写简洁,统一处理
using placeholders::_1;
using placeholders::_2;
using placeholders::_3;
using placeholders::_4;
using placeholders::_5;
using placeholders::_6;
using placeholders::_7;
using placeholders::_8;
using placeholders::_9;
using placeholders::_10;