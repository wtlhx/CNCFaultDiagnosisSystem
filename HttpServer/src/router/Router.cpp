#include "Router.h"
#include <muduo/base/Logging.h>
Router::Router()
{
}

Router::~Router()
{
}

void Router::registHandlerCallback(HttpRequest::Method method, std::string path, const HttpHandleCallback& cb)
{
    RouteKey key {method, path};//列表初始化
    handlerCallbacks[key] = std::move(cb); //移动语义
}

void Router::registHandler(HttpRequest::Method method, std::string path, HandlerPtr handler)
{
    RouteKey key {method, path};
    handlers[key] = std::move(handler);
}

bool Router::route(const HttpRequest& request, HttpResponse* response) // 传递的是response指针，对response的修改直接放入该内存中
{
    RouteKey key {request.getMethod(), request.getUrl()};
    //查找处理器对象
    if(handlers.find(key) != handlers.end())
    {
        auto handler = handlers[key];
        handler->handle(request, response);
        return true;
    }
    LOG_INFO << "router requestbody:" << request.getContentBody();
    //查找回调函数
    if(handlerCallbacks.find(key) != handlerCallbacks.end())
    {
        LOG_INFO << "router requestbody:" << request.getContentBody();
        auto handler = handlerCallbacks[key];
        handler(request, response);
        return true;
    }
    //动态查找处理器对象

    //动态查找回调函数

    return false;
}