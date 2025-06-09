#include "LLMInteractor.h"

LLMInteractor::~LLMInteractor()
{
    try
    {
        close(); // 在析构函数中关闭大模型交互服务连接
    }
    catch(const std::exception& e)
    {
        LOG_ERROR << "Failed to close LLM: " << e.what() << '\n';
    }
}
