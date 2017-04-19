//
// Created by liuchuyong on 2017/3/23.
//

#include <iostream>
#include <string>
#include <future>
#include <vector>
#include "WatcherEventHandle.h"
#include "ZookeeperClient.h"
#include "TestUtil.h"

using namespace std;

/**
 * ���Զ�·��"/"���ü����ӽڵ���ӣ��ӽڵ㷢���䶯�����Ӵ������ȡ���µ��ӽڵ��б�
 */
int main(int argc,char ** argv)
{
	spdlog::set_async_mode(DEFAULT_QUEUE_SIZE);
	spdlog::set_pattern(DEFAULT_PATTERN);
	auto logger = spdlog::stdout_logger_mt(getGlobalLogName());

	auto zkClientPtr = make_shared<ZookeeperClient>(SERVER);
	zkClientPtr->connectServer();
	if( !zkClientPtr->getConectStatus() )
	{
		logger->info("connect server[{}] fail,please check!",SERVER);
		exit(1);
	}

	string path = "/";
	VectorString childrenVec;
	auto childrenWatcherHandlerPtr = make_shared<ChildrenWatcherContinuallyHandler>();
	auto handler = bind(&ChildrenWatcherContinuallyHandler::handle,childrenWatcherHandlerPtr,_1,_2,_3,_4,_5,_6);
	zkClientPtr->getClildren(path,childrenVec,handler);

	sleep(10);
	childrenWatcherHandlerPtr->printChildrenList();
}
