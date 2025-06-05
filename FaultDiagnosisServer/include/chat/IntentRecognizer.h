#pragma once
#include <curl/curl.h>
#include <string>
#include "/usr/include/jsoncpp/json/json.h"
#include <muduo/base/Logging.h>
class IntentRecognizer
{
private:
    CURL* curl; // CURL句柄（对象），用于发送HTTP请求
    struct curl_slist* headers; // CURL头部列表，用于设置HTTP请求头
    std::string responseString; // 用于存储HTTP响应的字符串
    std::string responseHeaderString; // 用于存储HTTP响应头的字符串
    std::string host, port; // 意图识别服务的主机和端口
public:
    IntentRecognizer(std::string host_, std::string port_);
    ~IntentRecognizer();

    void connectIR(); // 连接到意图识别服务
    void closeIR(); // 关闭与意图识别服务的连接
    bool recognizeIntent(const std::string& text, Json::Value& Intent); // 识别意图，传入文本和返回的意图Json对象
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


