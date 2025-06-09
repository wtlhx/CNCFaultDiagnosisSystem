#pragma once

#include "CURLUtil.h"

class IntentRecognizer : public CURLUtil
{
public:
    using CURLUtil::CURLUtil; // 继承CURLUtil的构造函数
    ~IntentRecognizer();
};


