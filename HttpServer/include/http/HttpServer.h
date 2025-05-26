//Http服务器整体逻辑，解析请求，处理请求，返回响应
#pragma once

#include <string>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>
#include <muduo/net/InetAddress.h>
#include "HttpRequest.h"
#include "router/Router.h"

class HttpResponse;
class Router;

using HttpHandleCallback = std::function<void(const HttpRequest&, HttpResponse*)>;
class HttpServer
{
private:
    muduo::net::InetAddress serverAddress;// 服务器地址
    muduo::net::TcpServer server; //服务器
    muduo::net::EventLoop loop; // 主循环
    Router router; //路由器
    HttpHandleCallback handleCallback; //业务处理回调
public:
    HttpServer(int port_, const std::string &name, muduo::net::TcpServer::Option option);
    ~HttpServer();

    void initiate(); //初始化，设置回调
    void start(); //启动服务器（主循环）

    void setHandleCallback(HttpHandleCallback& cb); //设置业务回调

    void onConnect(const muduo::net::TcpConnectionPtr &conneciton); // 处理客户端新连接
    void onMessage(const muduo::net::TcpConnectionPtr &conneciton, muduo::net::Buffer* buf, muduo::Timestamp receiveTime); // 处理客户端发来的数据,参数顺序要按照muduo库的要求传递
    void onResponse(const muduo::net::TcpConnectionPtr &conneciton, HttpRequest &request); // 业务处理+发送响应
    
    void handleRequest(const HttpRequest& request, HttpResponse* response); // 处理请求，调用路由器的route方法
    //注册静态路由：严格匹配url和method来找到对应handler
    //Get和POST注册
    void Get(const std::string& path, const HttpHandleCallback& cb); //简单的业务逻辑，用回调函数解决
    void Get(const std::string& path, Router::HandlerPtr handler); //复杂的业务逻辑，可以定义handler来解决

    void POST(const std::string& path, const HttpHandleCallback& cb); //简单的业务逻辑，用回调函数解决
    void POST(const std::string& path, Router::HandlerPtr handler); //复杂的业务逻辑，可以定义handler来解决

    // 用不到
    // //注册动态路由：通过正则表达式匹配
    // void addRoute(HttpRequest::Method method, const std::string& path, const HttpHandleCallback& cb);
    // void addRoute(HttpRequest::Method method, const std::string& path, Router::HandlerPtr handler);
};


