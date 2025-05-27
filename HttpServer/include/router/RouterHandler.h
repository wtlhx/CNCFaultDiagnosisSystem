#pragma once

#include "HttpRequest.h"
class HttpResponse;

// 抽象基类，定义路由处理器的接口
// 处理器的实现类需要继承该类，并实现handle方法
class RouterHandler
{
public:
    virtual void handle(const HttpRequest&, HttpResponse*) = 0; //纯虚函数
    virtual ~RouterHandler() = default;// 默认虚析构函数
};

