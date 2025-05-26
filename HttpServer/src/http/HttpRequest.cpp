#include "include/http/HttpRequest.h"

HttpRequest::HttpRequest() : method(Method::Valid), url("Unknown"),version("Unknown"), contentLength(0)
{

}

HttpRequest::~HttpRequest()
{
}

bool HttpRequest::setMethod(const char* start, const char* end)
{
    std::string method_(start, end);
    if(method_ == "GET")
    {
        method = Method::GET;
    }
    else if(method_ == "POST")
    {
        method = Method::POST;
    }
    else if(method_ == "PUT")
    {
        method = Method::PUT;
    }
    else if(method_ == "DELETE")
    {
        method = Method::DELETE;
    }
    else if(method_ == "HEAD")
    {
        method = Method::HEAD;
    }
    else if(method_ == "OPTIONS")
    {
        method = Method::OPTIONS;
    }
 
    return method != Method::Valid;
}

std::string HttpRequest::getMethodString() const
{
    switch (method)
    {
    case Method::GET :
        return "GET";
    case Method::POST :
        return "POST";
    case Method::Valid :
        return "Valid";
    case Method::PUT :
        return "PUT";
    case Method::DELETE :
        return "DELETE";
    case Method::HEAD :
        return "HEAD";
    case Method::OPTIONS :
        return "OPTIONS";     
    default:
        return "Method Error";
    }
}

void HttpRequest::setUrl(const char* start, const char* end)
{
    url.assign(start, end); //assign的迭代器用法，范围是[start, end)
}

std::string HttpRequest::getPathParameters(const std::string& key) const
{
    auto it = pathParameters.find(key);
    if(it != pathParameters.end())
    {
        return it->second;
    }
    return "";
}

void HttpRequest::setQueryParameters(const char* start, const char* end)
{
    std::string querys(start, end);//查询参数部分
    size_t andPos = 0; // &符号位置
    size_t prePos = 0; // 前一个&符号位置
    size_t equalPos = 0; // =符号位置
    while((andPos = querys.find('&', prePos)) != std::string::npos)
    {
        equalPos = querys.find('=', prePos);
        if(equalPos != std::string::npos && equalPos < andPos)
        {
            std::string key = querys.substr(prePos, equalPos - prePos);
            std::string value = querys.substr(equalPos + 1, andPos - equalPos - 1);
            queryParameters[key] = value;
        }
        prePos = andPos + 1;
    }
    equalPos = querys.find('=', prePos);
    if(equalPos != std::string::npos)
    {
        std::string key = querys.substr(prePos, equalPos - prePos);
        std::string value = querys.substr(equalPos + 1);
        queryParameters[key] = value;
    }
}

std::string HttpRequest::getQueryParameters(const std::string& key) const
{
    auto it = queryParameters.find(key);
    if(it != queryParameters.end())
    {
        return it->second;
    }
    return "";
}

void HttpRequest::addHeaders(const char* start, const char* colonPlace, const char* end)
{
    std::string key(start, colonPlace);
    colonPlace ++;
    //去掉冒号后面的空格
    while(colonPlace < end && *colonPlace == ' ')
    {
        colonPlace++;
    }
    std::string value(colonPlace, end);
    //去掉末尾的空格
    while(value.size() > 0 && isspace(value[value.size() - 1]))
    {
        value.pop_back();//value.resize(value.size() - 1);
    }
    headers[key] = value;
}

std::string HttpRequest::getHeaders(const std::string& key) const
{
    auto it = headers.find(key);
    if(it != headers.end())
    {
        return it->second;
    }
    return "";
}

void HttpRequest::setContentBody(const char* start, const char* end)
{
    if(end > start)
    {
        contentBody.assign(start, end); //范围[start, end)
    }
}


void HttpRequest::reset()
{
    method = Method::Valid;
    url = "Unknown";
    version = "Unknown";
    pathParameters.clear();
    queryParameters.clear();
    headers.clear();
    contentBody.clear();
    contentLength = 0;
}