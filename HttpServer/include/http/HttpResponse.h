//封装Http响应
#pragma once
#include <string>
#include <unordered_map>
#include <muduo/net/Buffer.h>
class HttpResponse
{
public:
    enum class HttpStatusCode
    {
        UnKnown,
        OK = 200, // 成功
        Created = 201, // 创建成功
        Accepted = 202, // 已接受
        NoContent = 204, // 无内容
        MovedPermanently = 301, // 永久移动
        Found = 302, // 临时移动
        BadRequest = 400, // 错误请求
        Unauthorized = 401, // 未授权
        Forbidden = 403, // 禁止访问
        NotFound = 404, // 未找到
        MethodNotAllowed = 405, // 方法不允许
        InternalServerError = 500 // 内部服务器错误
    };

private:
    std::string version; // HTTP版本
    HttpStatusCode statusCode; // 状态码
    std::string statusMessage; // 状态信息
    std::unordered_map<std::string, std::string> headers; // 响应头
    std::string body; // 响应体
    bool keepAlive; // 是否保持连接
public:
    HttpResponse();
    ~HttpResponse();
    void setVersion(const std::string& version_) { version = version_; } // 设置HTTP版本

    void setStatusCode(HttpStatusCode statusCode_) { statusCode = statusCode_; } // 设置状态码
    HttpStatusCode getStatusCode() const { return statusCode; } // 获取状态码

    void setStatusMessage(const std::string& statusMessage_) { statusMessage = statusMessage_; } // 设置状态信息

    void setHeaders(const std::string& key, const std::string& value) { headers[key] = value; } // 设置响应头
    void setContentType(const std::string& contentType) { headers["Content-Type"] = contentType; } // 设置响应体类型
    void setContentLength(uint64_t contentLength) { headers["Content-Length"] = std::to_string(contentLength); } // 设置响应体长度

    void setBody(const std::string& body_) { body = body_; } // 设置响应体

    void setKeepAlive(bool keepAlive_) { keepAlive = keepAlive_; } // 设置是否保持连接
    bool getKeepAlive() {return keepAlive;};
    
    void appendToBuffer(muduo::net::Buffer* output) const; // 将响应内容添加到缓冲区

    void setResponseLine(const std::string& version, HttpStatusCode statusCode, const std::string& statusMessage); // 设置响应行
};

