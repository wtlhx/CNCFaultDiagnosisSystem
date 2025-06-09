#include "HttpServer.h"
#include "HttpParse.h"
#include "HttpResponse.h"
#include <muduo/base/Logging.h>
#include <iostream>

HttpServer::HttpServer(int port_, const std::string &name, muduo::net::TcpServer::Option option) :
serverAddress(port_), server(&loop, serverAddress, name, option), 
handleCallback(std::bind(&HttpServer::handleRequest, this, std::placeholders::_1, std::placeholders::_2))
{
    initiate(); // 初始化，设置回调
}

HttpServer::~HttpServer()
{
}

void HttpServer::initiate()
{
    server.setConnectionCallback(std::bind(&HttpServer::onConnect, this, std::placeholders::_1)); // 设置连接回调
    server.setMessageCallback(std::bind(&HttpServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)); // 设置消息回调
}

void HttpServer::start()
{
    LOG_WARN << "HttpServer :" << server.name() << " started at " << serverAddress.toIpPort(); // 日志输出
    server.start(); // 启动服务器
    loop.loop(); // 进入主循环
}

void HttpServer::setHandleCallback(HttpHandleCallback& cb)
{
    handleCallback = cb;
}

void HttpServer::onConnect(const muduo::net::TcpConnectionPtr &conneciton)
{
    if(conneciton->connected()) // 如果有连接
    {
        LOG_INFO << "New connection: " << conneciton->name() << " from " << conneciton->peerAddress().toIpPort(); // 日志输出
        conneciton->setContext(HttpParse()); // 设置解析器
    }
    else
    {
        LOG_INFO << "Connection closed: " << conneciton->name(); // 日志输出
    }
}

void HttpServer::onMessage(const muduo::net::TcpConnectionPtr &conneciton, muduo::net::Buffer* buf, muduo::Timestamp receiveTime)
{
    std::cout << "handle message" << std::endl;
    try
    {
        HttpParse* parser = boost::any_cast<HttpParse>(conneciton->getMutableContext()); // 获取解析器
        if(!parser->parse(buf, receiveTime)) // 解析请求
        {
            LOG_ERROR << "Failed parse request" ;
            conneciton->send("HTTP/1.1 400 Bad Request\r\n\r\n");
            conneciton->shutdown();
        }
        std::cout << (int)parser->status << std::endl;
        if(parser->whetherComplete())
        {
            std::cout << "Parse success" << std::endl;
            onResponse(conneciton, parser->getRequest());
            parser->reset();//重置解析器里的HttpRequst，用于下一次请求的读取
        }
    }
    catch(const std::exception& e)
    {
        LOG_ERROR << "Exception in onMessage" << e.what() << '\n';
        conneciton->send("HTTP/1.1 400 Bad Request\r\n\r\n");
        conneciton->shutdown(); // 关闭连接
    }
}

void HttpServer::onResponse(const muduo::net::TcpConnectionPtr &connection, HttpRequest &request)
{
    std::string reqHeaderConn = request.getHeaders("Connection");
    bool close = (reqHeaderConn == "close" || (request.getVersion() == "Http/1.0" && reqHeaderConn != "Keep-Alive")); // 请求头connetion是close，或者http1.0（无连接）并且请求头不是Keep-Alive
    HttpResponse response;
    response.setKeepAlive(!close);

    handleCallback(request, &response);//处理请求之后，响应体封装在response里
    muduo::net::Buffer buf;
    response.appendToBuffer(&buf); //把response中的数据写入muduo::buf
    LOG_INFO << "Sending response : \n" << buf.toStringPiece().as_string(); // 日志记录发送的响应体，调试用

    connection->send(&buf); //向客户端发送响应

    if(!response.getKeepAlive())
    {
        connection->shutdown();
    }
}

void HttpServer::handleRequest(const HttpRequest& request, HttpResponse* response)
{
    LOG_INFO << "Received request: " << request.getMethodString() << " " << request.getUrl();
    try
    {
        if(!router.route(request, response))
        {
            LOG_INFO << "请求的方法和内容是 : " << request.getMethodString() << " " << request.getUrl();
            LOG_INFO << "response 404 Not FOUND";
            response->setVersion(request.getVersion());
            response->setStatusCode(HttpResponse::HttpStatusCode::NotFound);
            response->setStatusMessage("Not FOUND");
            response->setKeepAlive(false);
        }
    }
    catch(const std::exception& e)
    {
        response->setVersion(request.getVersion());
        response->setStatusCode(HttpResponse::HttpStatusCode::InternalServerError);
        response->setStatusMessage("InternalServerError");
        response->setBody(e.what());
    }
}

//注册路由接口
void HttpServer::Get(const std::string& path, const HttpHandleCallback& cb)
{
    router.registHandlerCallback(HttpRequest::Method::GET, path, cb);
}

void HttpServer::Get(const std::string& path, Router::HandlerPtr handler)
{
    router.registHandler(HttpRequest::Method::GET, path, handler);
}

void HttpServer::POST(const std::string& path, const HttpHandleCallback& cb)
{
    router.registHandlerCallback(HttpRequest::Method::POST, path, cb);
}

void HttpServer::POST(const std::string& path, Router::HandlerPtr handler)
{
    router.registHandler(HttpRequest::Method::POST, path, handler);
}