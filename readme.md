# webServer
webServer是一个用cpp实现的高性能的轻量级服务器，在[一位前辈的仓库](https://github.com/markparticle/WebServer)基础上进行了大量的框架修改和代码重写，目前只是先传上来顺便记录，后面会慢慢完善

预计这个项目会扩展成一个完整的全栈项目，前端我目前个人的想法是想做游戏/个人网站/故事（偏情感）相关，应该也都会放到这个仓库里~

## 规划
目前是想站在巨人的肩膀上创新，通过结合业界最佳实践，设计出一个高可用、高性能的webServer系统

* 选择合适的工具和技术，并深刻理解其原理
* 展示模块化设计能力，体现清晰的系统架构和技术细节
* 引入工程化特性和生产级优化
* 提供详细的性能分析和优化结果
* 清晰地展示项目的技术决策和解决问题的过程

## 原目录树

    ├── code           源代码
    │   ├── buffer
    │   ├── config
    │   ├── http
    │   ├── log
    │   ├── timer
    │   ├── pool
    │   ├── server
    │   └── main.cpp
    ├── test           单元测试
    │   ├── Makefile
    │   └── test.cpp
    ├── resources      静态资源
    │   ├── index.html
    │   ├── image
    │   ├── video
    │   ├── js
    │   └── css
    ├── bin            可执行文件
    │   └── server
    ├── log            日志文件
    ├── webbench-1.5   压力测试
    ├── build          
    │   └── Makefile
    ├── Makefile
    ├── LICENSE
    └── readme.md

## 现有框架

* 后端
  * main，程序入口
  * server
    * epoller，封装了一个Epoller类，提供Linux中epoll的一些接口
    * webserver
  * timer，
  * pool
  * log
  * http
  * buffer
  * config
* 前端
  * 测试页
* 测试
  * 单元测试
  * webbench
* 资源
  * 测试页资源

## 目标框架
* 并发框架
  * eventLoop
  * channel
  * poller
* 日志系统修改
* 内存池修改
* 线程池修改
* LFU
* 部署
* 性能测试
* 项目介绍
* 项目细节