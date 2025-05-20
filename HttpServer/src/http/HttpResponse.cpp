#include "include/http/HttpResponse.h"

HttpResponse::HttpResponse() : statusCode(HttpStatusCode::UnKnown), keepAlive(false)
{
}
    
HttpResponse::~HttpResponse()
{
}

void HttpResponse::appendToBuffer(muduo::net::Buffer* output) const
{
    // 添加响应行
    // 用格式化字符串添加版本和状态码
    char buf[32];
    snprintf(buf, sizeof(buf), "%s %d ", version.c_str(), static_cast<int>(statusCode));
    output->append(buf);
    output->append(statusMessage);
    output->append("\r\n");
    // 添加连接状态，Connection状态和keepAlive变量直接相关，因此不放入headers里
    if (keepAlive)
    {
        output->append("Connection: keep-alive\r\n");
    }
    else
    {
        output->append("Connection: close\r\n");
    }
    // 添加响应头
    for (const auto& header : headers)
    {
        output->append(header.first);
        output->append(": ");
        output->append(header.second);
        output->append("\r\n");
    }
    // 添加空行
    output->append("\r\n");
    // 添加响应体
    output->append(body);
}

void HttpResponse::setResponseLine(const std::string& version_, HttpStatusCode statusCode_, const std::string& statusMessage_)
{
    version = version_;
    statusCode = statusCode_;
    statusMessage = statusMessage_;
}