#pragma once
#include <curl/curl.h>
#include <string>
#include "/usr/include/jsoncpp/json/json.h"
#include <muduo/base/Logging.h>
class CURLUtil
{
protected:
    CURL* curl; // CURL句柄（对象），用于发送HTTP请求
    struct curl_slist* headers; // CURL头部列表，用于设置HTTP请求头
    std::string responseString; // 用于存储HTTP响应的字符串
    std::string responseHeaderString; // 用于存储HTTP响应头的字符串
    std::string host, port, path; // 要连接服务的主机和端口
public:
    CURLUtil(std::string host_, std::string port_, std::string path_);
    ~CURLUtil();

    virtual void connect(); // 连接服务
    void close(); // 关闭连接
    bool getResult(const Json::Value& text, Json::Value& result);
private:
    /*
    * 对HTTP协议返回的协议头进行处理的回调方法
    */
    static size_t responseHeaderHandeler(char *buffer, size_t size, size_t nitems, void *userdata);
    
    /*
    * 对HTTP协议返回的数据进行处理的回调方法
    */
    static size_t responseHandeler(char *ptr, size_t size, size_t nmemb, void *userdata);
};