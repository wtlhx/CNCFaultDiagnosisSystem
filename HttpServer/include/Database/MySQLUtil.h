#pragma once
#include "MySQLConnectionPool.h"

class MySQLUtil
{
private:
    /* data */
public:
    MySQLUtil() = default;
    ~MySQLUtil() = default;

    // 创建MySQL连接池实例
    static void init()
    {
        MySQLConnectionPool::getInstance();
    }

    template<typename... Args>
    sql::ResultSet* executeQuery(const sql::SQLString& sql, Args&&... args)
    {
        auto conn = MySQLConnectionPool::getInstance().getConnection();
        return conn->exectuteQuery(sql, std::forward<Args>(args)...);
    }

    template<typename... Args>
    int executeUpdate(const sql::SQLString& sql, Args&&... args)
    {
        auto conn = MySQLConnectionPool::getInstance().getConnection();
        return conn->executeUpdate(sql, std::forward<Args>(args)...);
    }
};


