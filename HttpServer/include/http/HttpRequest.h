//封装Http请求的内容
#pragma once
#include <string>
#include <unordered_map>
#include <muduo/base/Timestamp.h>
class HttpRequest
{
public:
    enum class Method
    {
        GET, // 请求资源
        POST, // 提交数据
        Valid, // 无效
        PUT,
        DELETE,
        HEAD,
        OPTIONS,
    }; 

private:
    Method method; // 请求方法
    std::string url; // 请求的资源
    std::string version; // Http版本
    std::unordered_map<std::string, std::string> pathParameters; // 路径参数
    std::unordered_map<std::string, std::string> queryParameters; // 查询参数
    std::unordered_map<std::string, std::string> headers; // 请求头
    std::string contentBody; // 请求体
    uint64_t contentLength; // 请求体长度

    muduo::Timestamp receiveTime; // 接受时间
public:
    HttpRequest();
    ~HttpRequest();
    bool setMethod(const char* start, const char* end); // 设置请求方法, [start, end)是指向字符串的指针，表示请求方法的起始和结束位置
    Method getMethod() const { return method; } // 获取请求方法 简单函数直接在头文件中实现（内联）
    std::string getMethodString() const; // 获取请求方法字符串
    
    void setUrl(const char* start, const char* end); // 设置请求资源
    std::string getUrl() const { return url; } // 获取请求资源

    void setVersion(std::string version_) {version = version_;} // 设置Http版本
    std::string getVersion() const { return version; } // 获取Http版本

    void setPathParameters(const std::string& key, const std::string& value) {pathParameters[key] = value;} // 设置路径参数
    std::string getPathParameters(const std::string& key) const; // 获取路径参数

    void setQueryParameters(const char* start, const char* end); // 设置查询参数
    std::string getQueryParameters(const std::string& key) const; // 获取查询参数

    void addHeaders(const char* start, const char* colonPlace, const char* end); // 添加请求头
    std::string getHeaders(const std::string& key) const; // 获取请求头

    void setContentBody(const std::string& contentBody_) {contentBody = contentBody_;} // 设置请求体
    void setContentBody(const char* start, const char* end); // 设置请求体
    std::string getContentBody() const { return contentBody; } // 获取请求体

    void setContentLength(uint64_t contentLength_) {contentLength = contentLength_;} // 设置请求体长度
    uint64_t getContentLength() const { return contentLength; } // 获取请求体长度

    void setReceiveTime(muduo::Timestamp receiveTime_) {receiveTime = receiveTime_;} // 设置接受时间
    muduo::Timestamp getReceiveTime() const { return receiveTime; } // 获取接受时间

    void reset(); // 重置请求体
};


