#pragma once
#include "RouterHandler.h"
#include "IntentRecognizer.h"
#include "HttpResponse.h"
#include "MySQLUtil.h"
#include "Neo4jUtil.h"
#include "LLMInteractor.h"

class ChatHandler : public RouterHandler
{
private:
    IntentRecognizer intentRecognizer; // 意图识别器实例
    // MySQLUtil mySQLUtil; // MySQL工具类实例，用于数据库操作
    Neo4jUtil neo4jutil;
    LLMInteractor llmInteractor; // 大模型交互器实例
public:
    ChatHandler(/* args */);
    ~ChatHandler();
    void handle(const HttpRequest& request, HttpResponse* response) override;
private:
    sql::ResultSet* queryChatHistory(const std::string& usrID); //根据用户ID查询聊天历史
    Json::Value querySolution(const std::string& searchInputs, const std::string& searchType); //根据输入内容和搜索类型查询解决方案
};


