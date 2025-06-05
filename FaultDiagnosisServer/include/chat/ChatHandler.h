#pragma once
#include "RouterHandler.h"
#include "IntentRecognizer.h"
#include "HttpResponse.h"

class ChatHandler : public RouterHandler
{
private:
    IntentRecognizer intentRecognizer; // 意图识别器实例，用于处理聊天请求
public:
    ChatHandler(/* args */);
    ~ChatHandler();
    void handle(const HttpRequest& request, HttpResponse* response) override;
};


