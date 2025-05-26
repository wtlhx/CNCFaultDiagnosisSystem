#pragma once;

#include <string>
#include <unordered_map>
#include <memory>
#include <functional>
#include <RouterHandler.h>
#include "http/HttpRequest.h"

class Router
{
public:
    using HandlerPtr = std::shared_ptr<RouterHandler>;
    using HttpHandleCallback = std::function<void(const HttpRequest&, HttpResponse*)>;
    //路由键
    struct RouteKey
    {
        HttpRequest::Method method;
        std::string path;

        bool operator==(const RouteKey &otherKey) const
        {
            return this->method == otherKey.method && this->path == otherKey.path;
        }
    };

    //为路由键定义哈希函数，函数对象
    struct RouteKeyHash
    {
        size_t operator()(const RouteKey key)
        {
            size_t methodHash = std::hash<int>{}(static_cast<int>(key.method)); // 将枚举转换为int
            size_t pathHash = std::hash<std::string>{}(key.path); // 计算路径的哈希值
            return methodHash * 31 + pathHash; // 组合哈希值
        }
    };

private:
    std::unordered_map<RouteKey, HandlerPtr, RouteKeyHash> handlers; //路由表，存储路由键和对应的处理器
    std::unordered_map<RouteKey, HttpHandleCallback, RouteKeyHash> handlerCallbacks; //路由表，存储路由键和对应的回调函数
public:
    Router();
    ~Router();
    void registHandlerCallback(HttpRequest::Method method, std::string path, const HttpHandleCallback& cb);
    void registHandler(HttpRequest::Method method, std::string path, HandlerPtr handler);

    bool route(const HttpRequest& request, HttpResponse* response); //路由，返回是否有匹配的路由

};


