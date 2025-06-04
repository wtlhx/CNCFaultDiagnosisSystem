#pragma once
#include <Neo4j-cpp-driver-master/api.hpp>
#include "json/json.h"
#include "muduo/base/Logging.h"
class Neo4jUtil
{
private:
    neo4jDriver::Neo4jAPI neo4jAPI;
public:
    Neo4jUtil(
            std::shared_ptr<neo4jDriver::Neo4j> database, 
            std::string host, std::string port, std::string user, std::string password
            );
    ~Neo4jUtil();

    void connect();
    void close();

    //根据label和属性两跳查询出reason和solution
    Json::Value queryReasonAndSolution(std::string linkedLabel, Json::Value &linkedProperties); // 查询的起点标签和属性
    
    //增加节点和边(工程师反馈：xxx故障还有xxx原因/xxx解决方案) //先查linkedNode是否存在，如果不存在，则先创建linkedNode节点，再增加边和新节点
    bool addNodeAndrelation(std::string linkedLabel, Json::Value &linkedProperties, const Json::Value &newProperties, const std::string &newLabel);

    //删除节点和边(工程师反馈：xxx故障的xxx原因/xxx解决方案不存在)
    bool deleteNodeAndRelation(Json::Value &deleteProperties, std::string &deleteLabel);

    //更新节点和边(工程师反馈：xxx故障的xxx原因/xxx解决方案应该是xxx) // 先删除旧的节点和边，再增加新的节点和边
    bool updateNodeAndRelation( std::string linkedLabel, Json::Value &linkedProperties, 
                                Json::Value &oldProperties, std::string &oldLabel,
                                const Json::Value &newProperties, const std::string &newLabel);
};


