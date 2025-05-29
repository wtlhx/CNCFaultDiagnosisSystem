#include "MySQLConnection.h"

MySQLConnection::MySQLConnection(const std::string& host_, const std::string& user_, const std::string& password_, const std::string& database_)
: host(host_), user(user_), password(password_), database(database_)
{
    try
    {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_driver_instance();
        connection.reset(driver->connect(host, user, password)); // reset是shared_ptr的方法，重置其管理的对象
        if(connection)
        {
            connection->setSchema(database);

            // 设置连接属性
            connection->setClientOption("OPT_RECONNECT", "true"); //设置自动重连
            connection->setClientOption("OPT_CONNECT_TIMEOUT", "10"); //设置连接超时时间10s，超过10s，抛出异常
            connection->setClientOption("multi_statements", "false"); //设置禁止多语句，防止sql注入
            
            // 设置字符集
            std::unique_ptr<sql::Statement> stmt(connection->createStatement());
            stmt->execute("SET NAMES utf8mb4"); //设置字符集为utf8mb4
            
            LOG_INFO << "Database connection established";
        }
    }
    catch(const sql::SQLException& e)
    {
        LOG_ERROR << "Failed to connect MySQL : " << e.what();
        throw std::runtime_error(e.what());
    }
}

MySQLConnection::~MySQLConnection()
{
    cleanup();
    LOG_INFO << "MySQLConnection closed";
}

bool MySQLConnection::ping()
{
    try
    {
        std::unique_ptr<sql::Statement> stmt(connection->createStatement());
        std::unique_ptr<sql::ResultSet> result(stmt->executeQuery("SELECT 1"));
        return true;
    }
    catch(const sql::SQLException& e)
    {
        LOG_ERROR << "Ping Failed : " << e.what();
        return false;
    }
}

void MySQLConnection::reconnect_()
{
    try
    {
        if(connection)
        {
            connection->reconnect();
        }
        else
        {
            sql::mysql::MySQL_Driver* driver = sql::mysql::get_driver_instance();
            connection.reset(driver->connect(host, user, password));
            connection->setSchema(database);
        }
    }
    catch(const sql::SQLException& e)
    {
        LOG_ERROR << "Reconnect failed : " << e.what();
        throw std::runtime_error(e.what());        
    }
}

void MySQLConnection::cleanup()
{
    std::lock_guard<std::mutex> lock(mtx);
    try
    {
        if(connection)
        {
            if(!connection->getAutoCommit()) //获取自动提交功能的情况
            {
                connection->rollback(); // 回滚未提交的事务
                connection->setAutoCommit(true); //恢复自动提交功能
            }
            // 清理所有未处理的结果集
            std::unique_ptr<sql::Statement> stmt(connection->createStatement());
            while (stmt->getMoreResults()) //检查是否存在未处理的结果集，防止未处理的结果集占用资源
            {
                auto result = stmt->getResultSet();
                while (result && result->next()) 
                {
                    // 消费所有结果
                }
            }
        }
    } 
    catch (const std::exception& e) 
    {
        LOG_WARN << "Error cleaning up connection: " << e.what();
        try 
        {
            reconnect_();
        } 
        catch (...) 
        {
            // 忽略重连错误
        }
    }
}
    
    