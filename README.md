#**zkclient4cpp** 

**zkclient4cpp 是对zookeeper c api 的简单封装,使得更加易用。主要支持功能包括设定session watcher,watcher handler,completion handler等。**

##安装使用

###**下载**

git clone https://github.com/chuyongliu/zkclient4cpp.git

###**依赖** 

[zookeeper](https://github.com/apache/zookeeper) 编译相应的C源码

[spdlog](https://github.com/gabime/spdlog)

###**编译**

需要支持C++11的编译器，gcc4.8以上  cmake2.8.4以上

mkdir build 

cd build 

cmake .. 

make

