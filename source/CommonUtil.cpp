//
// Created by liuchuyong on 2017/3/23.
//
#include "CommonUtil.h"

void setGlobalLogName(const std::string & logName)
{
	globalLogName = logName;
}
const std::string & getGlobalLogName()
{
	return globalLogName;
}

void transChildVector2VectorString(const ChildrenVector & children, VectorString * vecString)
{
	for (int i = 0; i < children.count; i++)
	{
		vecString->push_back(children.data[i]);
	}
}

void transACLVector2VectorACL(const ACLVector & aclVec, VectorACL * vectorACL)
{
	for (int i = 0; i < aclVec.count; i++)
	{
		vectorACL->push_back(aclVec.data[i]);
	}
}

void printPathList(const PathVector & pathVector)
{
	for_each(pathVector.begin(),pathVector.end(),printString);
}

void printPathValueList(const NodeValueMap & pathValueMap)
{
	for_each(pathValueMap.begin(),pathValueMap.end(),printStringPair);
}

bool isZOK(int result)
{
	return result == ZOK;
}

const char* watcherEventType2String(int type)
{
	switch(type)
	{
		case 0:
			return "ZOO_ERROR_EVENT";
		case CREATED_EVENT_DEF:
			return "ZOO_CREATED_EVENT";
		case DELETED_EVENT_DEF:
			return "ZOO_DELETED_EVENT";
		case CHANGED_EVENT_DEF:
			return "ZOO_CHANGED_EVENT";
		case CHILD_EVENT_DEF:
			return "ZOO_CHILD_EVENT";
		case SESSION_EVENT_DEF:
			return "ZOO_SESSION_EVENT";
		case NOTWATCHING_EVENT_DEF:
			return "ZOO_NOTWATCHING_EVENT";
	}
	return "INVALID_EVENT";
}


const char* state2String(int state)
{
	switch(state)
	{
		case 0:
			return "ZOO_CLOSED_STATE";
		case CONNECTING_STATE_DEF:
			return "ZOO_CONNECTING_STATE";
		case ASSOCIATING_STATE_DEF:
			return "ZOO_ASSOCIATING_STATE";
		case CONNECTED_STATE_DEF:
			return "ZOO_CONNECTED_STATE";
		case EXPIRED_SESSION_STATE_DEF:
			return "ZOO_EXPIRED_SESSION_STATE";
		case AUTH_FAILED_STATE_DEF:
			return "ZOO_AUTH_FAILED_STATE";
	}
	return "INVALID_STATE";
}
