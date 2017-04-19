//
// Created by liuchuyong on 2017/4/13.
//
#pragma once

#include <iostream>
#include <string>
#include <map>
#include "ZookeeperClient.h"

class BaseCompletionHandler
{
public:
	BaseCompletionHandler() : logger_(spdlog::get(getGlobalLogName()))
	{
	}
protected:
	std::shared_ptr<spdlog::logger> logger_;
	mutex mutex_;
};

class SimpleStringCompletionHandler : public BaseCompletionHandler
{
public:
	SimpleStringCompletionHandler() = default;
	void handle(int rc, const char *value, const void *context);
	string getValue(const string & path);
private:
	NodeValueMap nodeValueMap_;
};

class SimpleVoidCompletionHandler : public BaseCompletionHandler
{
public:
	SimpleVoidCompletionHandler() = default;
	void handle(int rc,const void *context);
private:
	NodeResultMap nodeResultMap_;
};

class SimpleDataCompletionHandler : public BaseCompletionHandler
{
public:
	SimpleDataCompletionHandler() = default;
	void handle(int rc, const char *value, int value_len, const struct Stat *stat, const void *context);
private:
	NodeDataCompletionMap nodeDataCompletionMap_;
};


class SimpleStatCompletionHandler : public BaseCompletionHandler
{
public:
	SimpleStatCompletionHandler() = default;
	void handle(int rc, const struct Stat *stat, const void *context);
private:
	NodeStatMap nodeStatMap_;
};

class SimpleStingsCompletionHandler : public BaseCompletionHandler
{
public:
	SimpleStingsCompletionHandler() = default;
	void handle(int rc,const struct String_vector *strings, const void *context);
private:
	NodeChildrenMap nodeChildrenMap_;
};

class SimpleStingsStatCompletionHandler : public BaseCompletionHandler
{
public:
	SimpleStingsStatCompletionHandler() = default;
	void handle(int rc,const struct String_vector *strings, const struct Stat *stat,const void *context);
private:
	NodeChildrenStatMap nodeChildrenStatMap_;
};


class SimpleACLCompletionHandler : public BaseCompletionHandler
{
public:
	SimpleACLCompletionHandler() = default;
	void handle(int rc, struct ACL_vector *acl,struct Stat *stat, const void *context);
private:
	NodeAclStatMap nodeAclStatMap_;
};