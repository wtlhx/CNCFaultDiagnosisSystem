#include "ChatHandler.h"

ChatHandler::ChatHandler(/* args */) : intentRecognizer(std::string("192.168.1.115"), std::string("8080"))
{}

ChatHandler::~ChatHandler()
{
}

void ChatHandler::handle(const HttpRequest& request, HttpResponse* response)
{
    try
    {
        intentRecognizer.connectIR(); // 连接到意图识别服务
        std::string inputs = request.getContentBody(); // 获取请求体中的内容
        Json::Value intent;
        if(intentRecognizer.recognizeIntent(inputs, intent)) // 识别意图
        {
            std::string code = intent["code"].asString(); // 获取意图识别结果中的代码
            std::string msg = intent["msg"].asString(); // 信息
            if(code != "200")
            {
                LOG_ERROR << "Intent recognition failed with code: " << code << ", message: " << msg;
                throw std::runtime_error("Intent recognition failed: " + msg);
            }
            inputs = intent["inputs"].asString(); // 筛选之后的输入内容（直接能在知识图谱中进行匹配的内容）
            std::string searchType = intent["searchType"].asString(); // 搜索类型
            response->setStatusCode(HttpResponse::HttpStatusCode::OK);
            response->setStatusMessage("OK");
            response->setContentType("application/json");
            response->setBody(intent.toStyledString());
            response->setContentLength(intent.toStyledString().size());
        }
        else
        {
            LOG_ERROR << "Intent recognition failed";
            response->setStatusCode(HttpResponse::HttpStatusCode::InternalServerError);
            response->setStatusMessage("Internal Server Error");
            response->setBody("Intent recognition failed");
        }
        response->setVersion(request.getVersion());
    }
    catch(const std::exception& e)
    {
        LOG_ERROR << "Error in ChatHandler: " << e.what();
    }
    
}