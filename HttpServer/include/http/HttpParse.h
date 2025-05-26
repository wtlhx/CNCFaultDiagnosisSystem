//解析Http请求，把muduo::net::Buffer中的数据解析成HttpRequest对象
#pragma once

#include <string>
#include <memory>
#include <muduo/net/Buffer.h>
#include <muduo/base/Timestamp.h>
class HttpRequest;
class HttpParse
{
public:
    enum class HttpParseStatus
    {
        ParseResquestLine, // 解析请求行
        ParseResquestHeaders, // 解析请求头
        ParseResquestBody, // 解析请求体
        ParseComplete, // 解析完成
    };
private:
    HttpParseStatus status; // 解析状态
    HttpRequest request; // Http请求对象
public:
    HttpParse();
    ~HttpParse();

    bool parse(muduo::net::Buffer* buf, muduo::Timestamp receiveTime); // 解析Http请求
    bool parseRequestLine(const char* begin, const char* end); // 解析请求行

    bool whetherComplete() const
    {
        return status == HttpParseStatus::ParseComplete; // 判断解析是否完成
    }

    void reset(); // 重置解析状态

    const HttpRequest& getRequest() const //返回常量引用，避免拷贝和外部修改
    {
        return request; // 获取Http请求对象
    }

    HttpRequest& getRequest()
    {
        return request; // 获取Http请求对象
    }
};


