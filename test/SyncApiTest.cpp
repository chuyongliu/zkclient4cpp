//
// Created by liuchuyong on 2017/3/20.
//
#include <iostream>
#include <string>
#include <future>
#include <vector>
#include "WatcherEventHandle.h"
#include "ZookeeperClient.h"
#include "TestUtil.h"

int main(int argc,char ** argv)
{
	spdlog::set_async_mode(DEFAULT_QUEUE_SIZE);
	spdlog::set_pattern(DEFAULT_PATTERN);
	string logName = "sync_api_test";
	auto logger = spdlog::stdout_logger_mt(logName);
	setGlobalLogName(logName);

	//connect
	auto sessionHandlerPtr = make_shared<SimpleSessionWatcherHandler>();
	auto handler = bind(&SimpleSessionWatcherHandler::handle,sessionHandlerPtr,_1,_2,_3,_4);
	auto zkClientPtr = make_shared<ZookeeperClient>(SERVER,handler);
	zkClientPtr->connectServer();
	if( !zkClientPtr->getConectStatus() )
	{
		logger->info("连接server[{}]失败,请确认服务是否正常",SERVER);
		exit(1);
	}

	int result= ZOK;
	string path = "/myzk";
	string value;
	int version= 0;
	result = zkClientPtr->getNodeValue(path,value,version);
	if ( isZOK(result) )
	{
		string path1 = "/myzk1";
		string newValue = value;
		newValue = "5678";
		result = zkClientPtr->setNodeValue(path1,newValue,version);
		logger->info("path[{}],value[{},result[{}]]",path1,value,result);
	}

	/*string path1 = "/myzk3";
	result = zkClientPtr->existsNodeWithWatch(path1);
	logger->info("set wexists watch succ");

	//设置同一个节点的不同监视，均会触发
	if (isZOK(result))
	{
		result = zkClientPtr->deleteNode(path1);
	}
	else
	{
		result = zkClientPtr->createNode(path1,string("abcdefg"));
		//result = zkClientPtr->createNode(path1,string("abcdefg"),true);//临时结点,会话终止即消失
	}*/


	//获取子结点
	string parent= "/";
	vector<string> childList;
	result = zkClientPtr->getClildren(parent,childList);
	if (isZOK(result))
	{
		logger->info("child of path[{}]:",parent);
		printPathList(childList);
	}

	//测试下带序列编码结点
	/*string sequencePath = "/myzk/test";
	string sequenceValue = "13579";
	for(int seq = 0; seq <10; seq++){
		result = zkClientPtr->createNode(sequencePath,sequenceValue,false,true);//eg: test0000000000、test0000000001
	}*/

	//获取结点权限
	/*string pathAcl = "/myzk";
	VectorACL vectorACL;
	zkClientPtr->getNodeACL(pathAcl,vectorACL);
	for(auto & acl : vectorACL){
		cout << "id:" << acl.id.id  << ",scheme:" << acl.id.scheme  << ",perms:" << acl.perms << endl;
	}*/

	/*string pathOnWatch = "/abcd";
	string pathOnWatchValue ;
	result = zkClientPtr->getNodeData(pathOnWatch,pathOnWatchValue,version);*/

	sleep(10);

	exit(0);
}