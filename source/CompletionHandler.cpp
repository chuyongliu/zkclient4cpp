//
// Created by liuchuyong on 2017/4/13.
//

#include "CompletionHandler.h"


string SimpleStringCompletionHandler::getValue(const string &path)
{
	return nodeValueMap_[path];//
}

void SimpleStringCompletionHandler::handle(int rc, const char *value, const void *context)
{
	ZkAsyncCompletionContext * completionContext = (ZkAsyncCompletionContext *)context;
	logger_->info("SimpleStringCompletionHandler rc[{}],value[{}],context[{}]",zerror(rc),value,
	              (char*)(completionContext->context_==nullptr?"null":completionContext->context_));
	if ( isZOK(rc) )
	{
		lock_guard<mutex> lockGuard(mutex_);
		nodeValueMap_[completionContext->path_] = value;
	}
}

void SimpleVoidCompletionHandler::handle(int rc, const void *context)
{
	ZkAsyncCompletionContext * completionContext = (ZkAsyncCompletionContext *)context;
	logger_->info("SimpleVoidCompletionHandler rc[{}],context[{}]",zerror(rc),
	              (char*)(completionContext->context_== nullptr?"null":completionContext->context_));

	lock_guard<mutex> lockGuard(mutex_);
	nodeResultMap_[completionContext->path_] = isZOK(rc);
}

void SimpleDataCompletionHandler::handle(int rc, const char *value, int value_len,
                                          const struct Stat *stat, const void *context)
{
	ZkAsyncCompletionContext * completionContext = (ZkAsyncCompletionContext *)context;
	logger_->info("SimpleDataCompletionHandler rc[{}],value[{}],value_len[{}],context[{}]",zerror(rc),value,value_len,
	              (char *)(completionContext->context_==nullptr?"null":completionContext->context_));
	if(isZOK(rc))
	{
		DataCompletion dataCompletion;
		dataCompletion.value_ = value;
		dataCompletion.length_ = value_len;
		dataCompletion.stat_ = *stat;
		lock_guard<mutex> lockGuard(mutex_);
		nodeDataCompletionMap_[completionContext->path_] = dataCompletion;
	}
}

void SimpleStatCompletionHandler::handle(int rc, const struct Stat *stat, const void *context)
{
	ZkAsyncCompletionContext * completionContext = (ZkAsyncCompletionContext *)context;
	logger_->info("SimpleStatCompletionHandler rc[{}],context[{}]",zerror(rc),
	              (char *)(completionContext->context_==nullptr?"null":completionContext->context_));
	if ( isZOK(rc) )//rc为ZOK的时候，stat为空，直接赋值会coredump
	{
		lock_guard<mutex> lockGuard(mutex_);
		nodeStatMap_[completionContext->path_] = *stat;
	}
}

void SimpleStingsCompletionHandler::handle(int rc, const struct String_vector *strings, const void *context)
{
	ZkAsyncCompletionContext * completionContext = (ZkAsyncCompletionContext *)context;
	logger_->info("SimpleStingsCompletionHandler rc[{}],context[{}]",zerror(rc),
	              (char *)(completionContext->context_==nullptr?"null":completionContext->context_));
	if( isZOK(rc) )
	{
		VectorString  vectorString;
		transChildVector2VectorString(*strings,&vectorString);
		lock_guard<mutex> lockGuard(mutex_);
		nodeChildrenMap_[completionContext->path_] = vectorString;
	}
}

void SimpleStingsStatCompletionHandler::handle(int rc, const struct String_vector *strings, const struct Stat *stat,
                                                const void *context)
{
	ZkAsyncCompletionContext * completionContext = (ZkAsyncCompletionContext *)context;
	logger_->info("SimpleStingsStatCompletionHandler rc[{}],context[{}]",zerror(rc),
	              (char *)(completionContext->context_==nullptr?"null":completionContext->context_));
	if( isZOK(rc) )
	{
		VectorString  vectorString;
		transChildVector2VectorString(*strings,&vectorString);
		ChildrenStat childrenStat;
		childrenStat.vectorString_ == vectorString;
		childrenStat.stat_ = *stat;

		lock_guard<mutex> lockGuard(mutex_);
		nodeChildrenStatMap_[completionContext->path_] = childrenStat;
	}

}

void SimpleACLCompletionHandler::handle(int rc, struct ACL_vector *acl,struct Stat *stat, const void *context)
{
	ZkAsyncCompletionContext * completionContext = (ZkAsyncCompletionContext *)context;
	logger_->info("SimpleACLCompletionHandler rc[{}],context[{}]",zerror(rc),
	              (char *)(completionContext->context_==nullptr?"null":completionContext->context_));
	if( isZOK(rc) )
	{
		VectorACL  vectorACL;
		transACLVector2VectorACL(*acl,&vectorACL);
		ACLStat aclStat;
		aclStat.vectorACL_ = vectorACL;
		aclStat.stat_ = *stat;
		lock_guard<mutex> lockGuard(mutex_);
		nodeAclStatMap_ [ completionContext->path_] =  aclStat;
	}

}