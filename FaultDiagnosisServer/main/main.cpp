#include "FaultDiagnosisServer.h"

int main()
{
    int port = 80; // 服务器端口
    std::string serverName = "FaultDiagnosisServer"; // 服务器名称
    FaultDiagnosisServer server(port, serverName); // 创建服务器实例
    server.start(); // 启动服务器
    return 0; // 程序结束
}