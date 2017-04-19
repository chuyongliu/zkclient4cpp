//
// Created by liuchuyong on 2017/3/24.
//

#include <iostream>
#include "ZookeeperClient.h"
#include "TestUtil.h"

using namespace std;


int main(int argc , char ** argv)
{
	spdlog::set_async_mode(DEFAULT_QUEUE_SIZE);
	spdlog::set_pattern(DEFAULT_PATTERN);
	string logName = "temp_node_test";
	setGlobalLogName(logName);
	auto logger = spdlog::stdout_logger_mt(logName);
	auto zkClientPtr = make_shared<ZookeeperClient>(SERVER);
	zkClientPtr->connectServer();
	string path = "/temp/sequence";
//	int ret = zkClientPtr->createNode(path,"",true,true);
	int ret = zkClientPtr->asyncCreateNode(path,"", true,true);
	logger->info("create node {} {}",path,ret);

	sleep(10);
	return 0;
}