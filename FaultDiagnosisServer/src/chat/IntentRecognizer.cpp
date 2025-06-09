#include "IntentRecognizer.h"

IntentRecognizer::~IntentRecognizer()
{
    try
    {
        close(); // 在析构函数中关闭意图识别服务连接
    }
    catch(const std::exception& e)
    {
        LOG_ERROR << "Failed to close intentRecognizer:" << e.what() << '\n';
    }
}

