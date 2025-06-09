#include "ChatHandler.h"
#include <thread>
#include <future>

ChatHandler::ChatHandler(/* args */) : intentRecognizer(std::string("192.168.1.115"), std::string("8080"), "/api/intent_detection"),
neo4jutil(neo4jDriver::Neo4j::getNeo4j(), "192.168.1.115", "7474", "neo4j", ""),
llmInteractor("192.168.1.115", "9010", "/v1/chat/completions")
{}

ChatHandler::~ChatHandler()
{
}

void ChatHandler::handle(const HttpRequest& request, HttpResponse* response)
{
    try
    {
        intentRecognizer.connect(); // 连接到意图识别服务
        llmInteractor.connect(); // 连接到大模型交互服务
        Json::Reader reader;
        Json::Value requestJson;
        reader.parse(request.getContentBody(), requestJson); // 解析请求体中的JSON内容
        std::string usrinputs = requestJson["inputs"].asString(); // 获取请求体中的内容
        LOG_INFO << "Received user input: " << usrinputs;
        Json::Value intent;
        if(!intentRecognizer.getResult(requestJson, intent)) // 识别意图
        {
            LOG_ERROR << "Intent recognition failed";
            response->setVersion(request.getVersion());
            response->setStatusCode(HttpResponse::HttpStatusCode::InternalServerError);
            response->setStatusMessage("Internal Server Error");
            response->setBody("Intent recognition failed");
            return;
        }

        std::string code = intent["code"].asString(); // 获取意图识别结果中的代码
        std::string msg = intent["msg"].asString(); // 信息
        if(code != "200")
        {
            LOG_ERROR << "Intent recognition failed with code: " << code << ", message: " << msg;
            throw std::runtime_error("Intent recognition failed: " + msg);
        }
        std::string searchInputs = intent["inputs"].asString(); // 筛选之后的输入内容（直接能在知识图谱中进行匹配的内容）
        std::string searchType = intent["search_type"].asString(); // 搜索类型
        LOG_INFO << "Search inputs: " << searchInputs << ", Search type: " << searchType;

        // 封装向大模型输入的prompt
        Json::Value prompt, messages, system, user, assistant;
        // 添加用户当前输入
        user["role"] = "user";
        user["content"] = usrinputs; // 用户输入内容
        messages.append(user);

        assistant["role"] = "assistant";

        // 不管是故障查询还是普通chat，都要查询数据库的聊天历史，所以把聊天历史查询放在这里
        //  查redis/MySQL找到聊天历史。
        // std::future<sql::ResultSet*> chatHistoryResult = std::async(std::launch::async, &ChatHandler::queryChatHistory, this, "1");
        // sql::ResultSet* chatHistory = chatHistoryResult.get(); // 获取聊天历史查询结果
        // 添加聊天历史，如果还没有历史，就会直接返回false，
        // while(chatHistory->next())
        // {
        //     assistant["content"] = chatHistory->getString("gpt_response").asStdString(); // 获取GPT的响应
        //     messages.append(assistant);
        //     user["content"] = chatHistory->getString("user_input").asStdString(); // 获取用户输入
        //     messages.append(user);
        // }

        // 如果是故障查询，再去neo4j中查故障案例，普通聊天chat仅添加system就可以输入大模型了
        /*
        fixme:这里实际的searchType并没有alarm_id,实际逻辑是把报警号的查询走另一条逻辑不走意图识别
        */
        if(searchType == "alarm_id" || searchType == "appear" || searchType == "alarm_info")
        {

            //查redis/Neo4j找到解决方案，生成prompt（看prompt的json格式来构建）
            std::future<Json::Value> SolutionResult = std::async(std::launch::async, &ChatHandler::querySolution, this, searchInputs, searchType);

            Json::Value solution = SolutionResult.get(); // 获取解决方案查询结果

            // 添加系统消息
            system["role"] = "system";
            system["content"] = "You are a helpful assistant for CNC fault diagnosis.";
            messages.append(system); 

            // 如果查询到了解决方案，则将其添加到消息中
            if(!solution.empty())
            {
                // 添加解决方案
                for(const auto& sol : solution)
                {
                    std::string solutionContent = sol["solution"].asString();
                    std::string reasonContent = sol["reason"].asString();
                    user["content"] = "根据查询故障案例，得到以下故障可能的原因和解决方案：\n原因：" + reasonContent + "\n解决方案：" + solutionContent;
                    messages.append(user);
                }
                LOG_INFO << "messages: " << messages.toStyledString();
            }
            else
            {
                LOG_ERROR << "No solution found for search inputs: " << searchInputs;
            }
        }
        else if(searchType == "chat")
        {
            // 添加系统消息
            system["role"] = "system";
            system["content"] = "You are a helpful assistant."; // 对于普通chat就不需要"for CNC fault diagnosis"
            messages.append(system); 
        }
        prompt["messages"] = messages; // 将消息列表添加到prompt中
        prompt["model"] = "local_model"; // 设置使用的模型
        prompt["max_tokens"] = 2048; // 设置最大token数
        prompt["stream"] = false; // 设置是否流式输出
        prompt["temperature"] = 0.5; // 设置温度参数

        // 将构建好的消息发送到大模型
        Json::Value llmResponse;
        if(!llmInteractor.getResult(prompt, llmResponse))
        {
            LOG_ERROR << "Failed to get response from LLM";
            response->setVersion(request.getVersion());
            response->setStatusCode(HttpResponse::HttpStatusCode::InternalServerError);
            response->setStatusMessage("Internal Server Error");
            response->setBody("Failed to get response from LLM");
            return;
        }
        response->setVersion(request.getVersion());
        response->setStatusCode(HttpResponse::HttpStatusCode::OK);
        response->setStatusMessage("OK");
        response->setContentType("application/json");
        response->setBody(llmResponse.toStyledString());
        response->setContentLength(llmResponse.toStyledString().size());

    }
    catch(const std::exception& e)
    {
        LOG_ERROR << "Error in ChatHandler: " << e.what();
    }
    
}

// sql::ResultSet* ChatHandler::queryChatHistory(const std::string& usrID)
// {
//     try
//     {
//         //fixme: redis
//         sql::SQLString sql = "SELECT user_input, gpt_response FROM conversation_history WHERE user_id = ? ORDER BY created_at DESC LIMIT 10";
//         sql::ResultSet* result = mySQLUtil.executeQuery(sql, usrID);
//         if(result == nullptr)
//         {
//             throw std::runtime_error("Failed to execute query for chat history");
//         }
//         return result; // 返回查询结果集
//     }
//     catch(const std::exception& e)
//     {
//         LOG_ERROR << "Failed to query chat history: " << e.what();
//         return nullptr; // 返回空指针表示查询失败
//     }
// }

Json::Value ChatHandler::querySolution(const std::string& searchInputs, const std::string& searchType)
{
    std::string linkedLabel;
    if(searchType == "alarm_id")
    {
        linkedLabel = "alarm_id";
    }
    else if(searchType == "alarm_info")
    {
        linkedLabel = "alarm_inf";
    }
    else if(searchType == "appear")
    {
        linkedLabel = "appear";
    }
    else
    {
        LOG_ERROR << "Unsupported search type: " << searchType;
        return Json::Value(); // 返回空的Json对象表示不支持的搜索类型
    }

    Json::Value linkedProperties;
    linkedProperties["name"] = searchInputs; // 设置查询条件的属性
    try
    {
        //fixme:redis
        Json::Value result = neo4jutil.queryReasonAndSolution(linkedLabel, linkedProperties); // 查询原因和解决方案
        if(result.empty())
        {
            LOG_INFO << "No data found for label: " << linkedLabel << " with properties: " << searchInputs;
            return Json::Value(); // 返回空的Json对象表示没有找到数据
        }
        return result; // 返回查询结果
    }
    catch(const std::exception& e)
    {
        LOG_ERROR << "Failed to query reason and solution: " << e.what();
        return Json::Value(); // 返回空的Json对象表示查询失败
    }
}