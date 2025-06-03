#include "Neo4jUtil.h"

Neo4jUtil::Neo4jUtil(
            std::shared_ptr<neo4jDriver::Neo4j> database, 
            std::string host, std::string port, std::string user, std::string password)
: neo4jAPI(neo4jDriver::Neo4jAPI(database, host, port, user, password))
{
    connect();
}
Neo4jUtil::~Neo4jUtil()
{
}

void Neo4jUtil::connect()
{
    try
    {
        neo4jAPI.connectDatabase();
    }
    catch(const std::exception& e)
    {
        LOG_ERROR << "Failed to connect to Neo4j database: " << e.what();
    }
}

void Neo4jUtil::close()
{
    try
    {
        neo4jAPI.closeDatabase();
    }
    catch(const std::exception& e)
    {
        LOG_ERROR << "Failed to close Neo4j database: " << e.what();
    }
}

Json::Value Neo4jUtil::queryReasonAndSolution(std::string linkedLabel, Json::Value &linkedProperties)
{
    try
    {
        //接收reason的查询结果
        Json::Value reason;
        Json::Value nullvalue; // 用于表示空值
        reason = neo4jAPI.selectNodesByAnotherLinkedNode("reason", nullvalue, linkedLabel + "2reason", nullvalue, linkedLabel, linkedProperties);
        //接收solution的查询结果
        Json::Value solution;
        solution = neo4jAPI.selectNodesByAnotherLinkedNode("solution", nullvalue, linkedLabel + "2solution", nullvalue, linkedLabel, linkedProperties);
        //将reason和solution合并到一个Json对象中
        Json::Value result;
        result["reason"] = reason;
        result["solution"] = solution;
        return result; // 返回包含reason和solution的Json对象
    }
    catch(const std::exception& e)
    {
        LOG_ERROR << "Failed to query reason and solution: " << e.what();
        return Json::Value(); // 返回空的Json对象
    }
}

bool Neo4jUtil::addNodeAndrelation(std::string linkedLabel, Json::Value &linkedProperties, const Json::Value &newProperties, const std::string &newLabel)
{
    try
    {
        int linkedID = 0, newID = 0;
        // 检查linkedNode是否存在
        Json::Value linkedNode = neo4jAPI.selectNodesByLabelAndProperties(linkedLabel, linkedProperties);
        if (linkedNode.empty())
        {
            // 如果不存在，则先创建linkedNode节点
            linkedNode = neo4jAPI.createNode(linkedProperties, linkedLabel);
            linkedID = linkedNode["_id"].asInt(); // 获取新创建的linkedNode的ID
        }
        else
        {
            linkedID = linkedNode[0]["_id"].asInt();
        }
        // 增加新节点
        Json::Value newNode = neo4jAPI.createNode(newProperties, newLabel);
        newID = newNode["_id"].asInt(); // 获取新创建的newNode的ID
        // 增加边, fixme：可以获取到新的边的信息
        Json::Value newRelationship;
        neo4jAPI.insertRelationship(linkedID, newID, linkedLabel + "2" + newLabel, newRelationship);
        return true; // 成功添加节点和边
    }
    catch(const std::exception& e)
    {
        LOG_ERROR << "Failed to add node and relation: " << e.what();
        return false; // 添加失败
    }
}

bool Neo4jUtil::deleteNodeAndRelation(Json::Value &deleteProperties, std::string &deleteLabel)
{
    try
    {
        // 查找要删除的节点
        Json::Value deleteNode = neo4jAPI.selectNodesByLabelAndProperties(deleteLabel, deleteProperties);
        if (deleteNode.empty())
        {
            LOG_ERROR << "Node to delete not found";
            return false; // 节点不存在，删除失败
        }
        
        int deleteID = deleteNode[0]["_id"].asInt();
        // 删除这个节点和这个节点的所有边
        return neo4jAPI.deleteNodeAndAllRelationshipsOfTheNode(deleteID);
    }
    catch(const std::exception& e)
    {
        LOG_ERROR << "Failed to delete node and relation: " << e.what();
        return false; // 删除失败
    }
}

bool Neo4jUtil::updateNodeAndRelation(
    std::string linkedLabel, Json::Value &linkedProperties, 
    Json::Value &oldProperties, std::string &oldLabel,
    const Json::Value &newProperties, const std::string &newLabel)
{
    try
    {
        bool d = deleteNodeAndRelation(oldProperties, oldLabel); // 删除旧的节点和边
        bool a = addNodeAndrelation(linkedLabel, linkedProperties, newProperties, newLabel); // 添加新的节点和边
        return a && d; // 返回添加和删除的结果
    }
    catch(const std::exception& e)
    {
        LOG_ERROR << "Failed to update node and relation: " << e.what();
        return false; // 更新失败
    }
}