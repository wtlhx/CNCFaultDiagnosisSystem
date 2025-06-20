#include "CURLUtil.h"

CURLUtil::CURLUtil(std::string host_, std::string port_, std::string path_) : host(host_), port(port_), path(path_)
{

}

CURLUtil::~CURLUtil()
{
    try
    {
        close(); // 在析构函数中关闭意图识别服务连接
    }
    catch(const std::exception& e)
    {
        LOG_ERROR << "Failed to close" << e.what() << '\n';
    }
}

void CURLUtil::connect()
{
    headers = curl_slist_append(headers, "Accept: */*; charset=UTF-8");
    headers = curl_slist_append(headers, "Content-Type: application/json");
    // headers = curl_slist_append(headers, "Host: " + host + ":" + port);
    headers = curl_slist_append(headers, "Connection: keep-alive");

    curl = curl_easy_init();
    if (curl)
    {
        // 设置HTTP头部
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        // 设置回调函数处理响应头
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, responseHeaderHandeler);
        // 设置回调函数处理响应数据
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, responseHandeler);
        // 设置接收数据的字符串
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);
        // 设置接收响应头的字符串
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &responseHeaderString);
        // 设置连接的URL
        std::string httpUrl = "http://" + host + ":" + port + path;
        curl_easy_setopt(curl, CURLOPT_URL, httpUrl.c_str());
        // 设置为POST请求
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");

        // 清空响应字符串
        responseString.clear();
        responseHeaderString.clear();
    }
    else
    {
        LOG_ERROR << "CURL EASY INIT IS FAILED!";
    }
}

bool CURLUtil::getResult(const Json::Value& text, Json::Value& result)
{
    if(!curl)
    {
        LOG_ERROR << "CURL EASY INIT IS FAILED!";
        return false;
    }

    LOG_INFO << "Request body: " << text.toStyledString();
    std::string textStr = text.toStyledString();

    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, textStr.c_str());

    // 开启调试模式
    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    // 执行POST请求
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        LOG_ERROR << "Curl Post error! result : " << curl_easy_strerror(res);
        return false;
    }
    else
    {
        // 检查响应状态码
        long responseCode;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
        if (responseCode != 200)
        {
            LOG_ERROR << "Response code is not 200, but " << responseCode;
            return false;
        }

        // 解析响应内容
        Json::Value responseJson;
        Json::Reader reader;
        if(reader.parse(responseString, responseJson))
        {
            result.clear();
            result = responseJson;
            responseString.clear();
            responseHeaderString.clear();
        }
        else
        {
            responseString.clear();
            responseHeaderString.clear();
            LOG_ERROR << "Failed to parse response JSON: " << reader.getFormattedErrorMessages();
            return false;
        }
    }
    return true;
}

void CURLUtil::close()
{
    responseString.clear();
    responseHeaderString.clear();
    if(curl)
    {
        curl_easy_cleanup(curl);
        curl = nullptr;
    }

    if(headers)
    {
        curl_slist_free_all(headers);
        headers = nullptr;
    }
}


size_t CURLUtil::responseHeaderHandeler(char *buffer, size_t size, size_t nitems, void *userdata)
{
    //计算本次收到的数据长度
    size_t responseHeaderDataLength = size * nitems;
    //根据算出的数据长度建立字节数组用于接收本次数据
    char* responseHeaderData = new char[responseHeaderDataLength];
    
    //若字节数组建立失败，返回0触发接受失败，停止接受
    if(NULL == responseHeaderData)
    {
        return 0;
    }
    
    //获取最终数据存放的位置
    std::string* responseHeaderDataPtr = (std::string*)userdata;
    
    //接收本次数据
    memcpy(responseHeaderData, buffer, responseHeaderDataLength);
    
    //将本次接收到的数据存储至最终的数据位置
    *responseHeaderDataPtr  = *responseHeaderDataPtr + std::string(responseHeaderData, responseHeaderDataLength);
    
    //释放用于存储本次接收到的数据的字节数组
    delete[] responseHeaderData;
    
    //返回本次接收到的数据的长度
    return responseHeaderDataLength;
};

size_t CURLUtil::responseHandeler(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    //计算本次收到的数据长度
    size_t responseDataLength = size * nmemb;
    //根据算出的数据长度建立字节数组用于接收本次数据
    char* responseData = new char[responseDataLength];
    
    //若字节数组建立失败，返回0触发接受失败，停止接受
    if(NULL == responseData)
    {
        return 0;
    }
    
    //获取最终数据存放的位置
    std::string* responseDataPtr = (std::string*)userdata;
    
    //接收本次数据
    memcpy(responseData, ptr, responseDataLength);
    
    //将本次接收到的数据存储至最终的数据位置
    *responseDataPtr  = *responseDataPtr + std::string(responseData, responseDataLength);
    
    //释放用于存储本次接收到的数据的字节数组
    delete[] responseData;
    
    //返回本次接收到的数据的长度
    return responseDataLength;
};