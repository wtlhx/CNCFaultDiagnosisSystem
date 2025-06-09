#pragma once

#include "HttpServer.h"
#include "MySQLUtil.h"

class FaultDiagnosisServer
{
private:
    HttpServer httpServer;
public:
    FaultDiagnosisServer(int port_, const std::string& name_, 
                         muduo::net::TcpServer::Option option_ = muduo::net::TcpServer::kNoReusePort);
    ~FaultDiagnosisServer();

    void start(); // 启动服务器

    void initialize();
    void initializeRouters(); // 初始化路由器，注册路由

};


