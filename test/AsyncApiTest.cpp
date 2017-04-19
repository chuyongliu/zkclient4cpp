//
// Created by liuchuyong on 2017/4/13.
//

#include <iostream>
#include <future>
#include "ZookeeperClient.h"
#include "WatcherEventHandle.h"
#include "CompletionHandler.h"
#include "TestUtil.h"

using namespace std;

int main(int argc , char ** argv)
{
	spdlog::set_async_mode(DEFAULT_QUEUE_SIZE);
	spdlog::set_pattern(DEFAULT_PATTERN);
	string logName = "async_api_test";
	setGlobalLogName(logName);
	auto logger = spdlog::stdout_logger_mt(logName);

	auto zkClientPtr = make_shared<ZookeeperClient>(SERVER);
	zkClientPtr->connectServer();

	string node = "/abc";
	auto stringCompletionHandler = make_shared<SimpleStringCompletionHandler>();
	auto stringHandler = bind(&SimpleStringCompletionHandler::handle,stringCompletionHandler,_1,_2,_3);
	zkClientPtr->asyncCreateNode(node,"",true,false,stringHandler, (void *)"async create");

	this_thread::sleep_for(std::chrono::milliseconds(200));

	string value = "13579";
	auto statCompletionHandler = make_shared<SimpleStatCompletionHandler>();
	auto statHandler = bind(&SimpleStatCompletionHandler::handle,statCompletionHandler,_1,_2,_3);
	string node2 ="/abcde";
	zkClientPtr->asyncSetNodeValue(node2,value,IGNORE_VERSION,statHandler,(void *)"async set");
//	zkClientPtr->asyncSetNodeValue(stringCompletionHandler->getValue(),value,IGNORE_VERSION,statHandler,(void *)"async set");

	auto stringsCompletionHandler = make_shared<SimpleStingsCompletionHandler>();
	auto stringsHandler = bind(&SimpleStingsCompletionHandler::handle,stringsCompletionHandler,_1,_2,_3);
//	zkClientPtr->asyncGetChildrenNodeList("/", stringsHandler,(void*)"async get children");
	zkClientPtr->asyncGetChildrenWithWatcher("/", nullptr, nullptr,stringsHandler,(void*)"async get children with watcher");

	auto stringsStatCompletionHandler = make_shared<SimpleStingsStatCompletionHandler>();
	auto stringsStatHandler = bind(&SimpleStingsStatCompletionHandler::handle,stringsStatCompletionHandler,_1,_2,_3,_4);
//	zkClientPtr->asyncGetChildrenNodeList2("/", stringsStatHandler,(void *)"async get children 2");

	auto childrenWatcher= make_shared<ChildrenWatcherOnceHandler>();
	auto childrenWatchHandler= bind(&ChildrenWatcherOnceHandler::handle,childrenWatcher,_1,_2,_3,_4,_5,_6);
	zkClientPtr->asyncGetChildren2WithWatcher("/", nullptr,nullptr,stringsStatHandler,(void *)"async get children 2");

	auto aclCompletionHandler = make_shared<SimpleACLCompletionHandler>();
	auto aclHandler = bind(&SimpleACLCompletionHandler::handle,aclCompletionHandler,_1,_2,_3,_4);
	zkClientPtr->asyncGetNodeACL("/myzk",aclHandler,(void *)"async get ACL");

	this_thread::sleep_for(std::chrono::seconds(10));
	exit(0);
}