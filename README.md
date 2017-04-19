# zkclient4cpp

**zkclient4cpp 是对zookeeper c api的简单封装,使得使用zookeeper c开发功能更加简便,代码更加简洁。
封装了主要的同步API及异步API,可以自行设置连接的session watcher,结点设置watcher handler,异步操作设置completion handler。
同时提供了简单的session watcher/watcher handler/completion handler实现样例。**

<<<<<<< HEAD
## ��װʹ��

### ����

git clone https://github.com/chuyongliu/zkclient4cpp.git

### ���� 
=======
## 安装使用

### 下载

git clone https://github.com/chuyongliu/zkclient4cpp.git

### 依赖
>>>>>>> origin/master

[zookeeper](https://github.com/apache/zookeeper) 编译相应的C源码

[spdlog](https://github.com/gabime/spdlog)  日志库支持

<<<<<<< HEAD
### ����
=======
### 编译
>>>>>>> origin/master

需要支持C++11的编译器  gcc 4.8以上  cmake 2.8.4以上

mkdir build 

cd build 

cmake .. 

make

