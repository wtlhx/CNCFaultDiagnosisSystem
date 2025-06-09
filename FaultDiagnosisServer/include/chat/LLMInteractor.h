#pragma once

#include "CURLUtil.h"

class LLMInteractor : public CURLUtil
{
public:
    using CURLUtil::CURLUtil; // 继承CURLUtil的构造函数
    ~LLMInteractor();

    void connect() override
    {
        CURLUtil::connect(); // 调用基类的connect方法
        headers = curl_slist_append(headers, "Authorization: Bearer ncdc_iss_2025");
        //设置Authorization头部
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    }
};



