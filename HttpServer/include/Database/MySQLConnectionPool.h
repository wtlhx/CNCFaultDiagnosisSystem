#pragma once

#include <memory>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>
#include "MySQLConnection.h"
#include <jsoncpp/json/json.h>

class MySQLConnectionPool
{
private:
    std::queue<std::shared_ptr<MySQLConnection>> mySQLconnectionPool;
    std::string host;
    std::string user;
    std::string password;
    std::string schema;
    int minSize;
    int maxSize;
    int maxTime;
    std::mutex mtx;
    std::condition_variable cv;
private:
    // 私有化构造函数和析构函数，防止外部构造和析构
    MySQLConnectionPool();
    ~MySQLConnectionPool();

    void createConnetion();
    void deleteConnection();
    void produceConnection();
    void checkConnection();
    void parseConfig();

public:
    std::shared_ptr<MySQLConnection> getConnection();

    static MySQLConnectionPool& getInstance()
    {
        static MySQLConnectionPool instance;
        return instance;
    }


};


