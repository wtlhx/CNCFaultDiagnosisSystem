#pragma once

#include <mysql_connection.h>
#include <mysql_driver.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <muduo/base/Logging.h>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <chrono>

class MySQLConnection
{
private:
    std::shared_ptr<sql::Connection> connection;
    std::string host;
    std::string user;
    std::string password;
    std::string database;
    std::mutex mtx;
    std::chrono::steady_clock::time_point startTime;
public:
    MySQLConnection(const std::string& host_, const std::string& user_, const std::string& password_, const std::string& database_);
    ~MySQLConnection();

    //执行query语句，模板函数，可以绑定参数包
    template <typename... Args>
    sql::ResultSet* exectuteQuery(sql::SQLString& sql, Args&&... args) //函数模板，&&是万能引用，可以接受左值和右值
    {
        std::lock_guard<std::mutex> lock(mtx);
        try
        {
            std::unique_ptr<sql::PreparedStatement> preStmt(connection->prepareStatement(sql));
            bindParams(preStmt, 1, std::forward<Args>(args)...);
            return preStmt->executeQuery();
        }
        catch(const sql::SQLException& e)
        {
            LOG_ERROR << "Query failed : " << e.what() << ", sql : " << sql;
            throw std::runtime_error(e.what());
        }
    }

    //执行update语句
    template<typename... Args>
    int executeUpdate(const sql::SQLString& sql, Args... agrs)
    {
        try
        {
            std::unique_ptr<sql::PreparedStatement> preStmt(connection->prepareStatement(sql));
            bindParams(preStmt, 1, std::forward<Args>(args)...);
            return preStmt->executeUpdate();
        }
        catch(const sql::SQLException& e)
        {
            LOG_ERROR << "Update failed : " << e.what() << ", sql : " << sql;
            throw std::runtime_error(e.what());
        }
        
    }

private:
    //递归绑定每一个位置上的参数，
    template<typename T, typename... Args>
    void bindParams(sql::PreparedStatement* sql, int index, T&& value, Args&&... args) //sql:查询语句，index:参数下标，value:这次绑定的值，args:还剩下的参数
    {
        sql->setString(index, std::to_string(std::forward<T>(value))); //使用完美转发保留value的左右值属性
        bindParams(sql, index + 1, std::forward<Args>(args)...); //递归调用，每次递归args中的参数会少一个，给到value
    }
    // 递归终止，args为空时会调用，不执行任何操作，递归终止
    void bindParams(sql::PreparedStatement* sql, int) {};

    //特化绑定string的版本，因为to_string不支持接收string，如果value是string，会优先绑定特化版本
    template<typename... Args>
    void bindParams(sql::PreparedStatement* sql, int index, std::string value, Args&&... args)
    {
        sql->setString(index, value);
        bindParams(sql, index + 1, std::forward<Args>(args)...);
    }

public:
    bool ping(); // 检测连接是否有效, 不仅检验能否执行查询，还检验能否收到结果
    void reconnect_(); // 重连
    void cleanup(); // 清理连接，包括未执行的事务，未消费的结果集，在析构函数中调用
    void setStartTime()
    {
        startTime = std::chrono::steady_clock::now(); //获取当前时间戳
    }
    long long GetSqlAliveTime()
    {
        //获取存活的时间
        std::chrono::nanoseconds sqlAliveTime = std::chrono::steady_clock::now() - startTime;
        //纳秒转换为毫秒输出
        return std::chrono::duration_cast<std::chrono::milliseconds>(sqlAliveTime).count();
    }
};
