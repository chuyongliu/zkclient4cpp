//
// Created by liuchuyong on 2017/3/21.
//

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <functional>
#include "CommonDef.h"
#include "spdlog/spdlog.h"

static std::string globalLogName;
static const int IGNORE_VERSION = -1;

void setGlobalLogName(const std::string & logName);
const std::string & getGlobalLogName();

#define destroyPointer(pointer)\
	delete pointer;\
	pointer = nullptr;

class PathValueInWatcher
{
public:
	PathValueInWatcher()
	{
	}
	PathValueInWatcher(string value, int event)
	{
		value_ = value;
		event_ = event;
	}
private:
	string value_="";
	int event_=0;//记录下对应的处理的事件类型
};

auto printString = [](const string & name)
{
	auto logger = spdlog::get(getGlobalLogName());
	logger->info("{} ",name);
};

auto printStringPair = [](const pair<string,string> & stringPair)
{
	auto logger = spdlog::get(getGlobalLogName());
	logger->info("{}:{}",stringPair.first,stringPair.second);
};

void transChildVector2VectorString(const ChildrenVector & children, VectorString * vecString);
void printPathList(const PathVector & nodeNameVector);
void printPathValueList(const NodeValueMap & pathValueMap);

void transACLVector2VectorACL(const ACLVector & childs, VectorACL * vectorACL);

const char* watcherEventType2String(int type);
const char* state2String(int state);

bool isZOK(int result);