#include "FaultDiagnosisServer.h"
#include "ChatHandler.h"

FaultDiagnosisServer::FaultDiagnosisServer(int port_, const std::string& name_, 
                         muduo::net::TcpServer::Option option_) : httpServer(port_, name_, option_)
{
    initialize();
}

FaultDiagnosisServer::~FaultDiagnosisServer()
{
}

void FaultDiagnosisServer::start()
{
    httpServer.start();
}

void FaultDiagnosisServer::initialize()
{
    initializeRouters(); // 初始化路由器，注册路由
}

void FaultDiagnosisServer::initializeRouters()
{
    httpServer.POST("/chat", std::make_shared<ChatHandler>()); // 注册聊天接口
}