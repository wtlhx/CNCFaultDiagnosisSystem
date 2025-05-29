#include "MySQLConnectionPool.h"
#include <fstream>
MySQLConnectionPool::MySQLConnectionPool()
{
    parseConfig();

    for(int i = 0; i < minSize; i ++)
    {
        createConnetion();
    }

    std::thread produceThread(std::bind(&MySQLConnectionPool::produceConnection, this));
    std::thread deleteThread(std::bind(&MySQLConnectionPool::deleteConnection, this));
    std::thread checkThread(std::bind(&MySQLConnectionPool::checkConnection, this));

    produceThread.detach();
    deleteThread.detach();
    checkThread.detach();
}


MySQLConnectionPool::~MySQLConnectionPool()
{
}

void MySQLConnectionPool::createConnetion()
{
    std::shared_ptr<MySQLConnection> connection = std::make_shared<MySQLConnection>(MySQLConnection(host, user, password, schema));
    connection->setStartTime();
    mySQLconnectionPool.push(connection);
}

//生产者
void MySQLConnectionPool::produceConnection()
{
    while(true)
    {
        std::unique_lock<std::mutex> uniLock(mtx);
        cv.wait(uniLock, [this](){return mySQLconnectionPool.size() < minSize;});
        createConnetion();
        cv.notify_all();
    }
}

//消费者
std::shared_ptr<MySQLConnection> MySQLConnectionPool::getConnection()
{
    std::shared_ptr<MySQLConnection> connection;
    {//锁作用域
        std::unique_lock<std::mutex> uniLock(mtx);
        cv.wait(uniLock, [this](){return !mySQLconnectionPool.empty();});
        connection = mySQLconnectionPool.front();
        mySQLconnectionPool.pop();
        cv.notify_all();
    }

    //锁外检查
    try
    {
        if(!connection->ping())
        {//从连接池拿的连接不能ping通
            LOG_ERROR << "Connection lost, try to reconnect";
            connection->reconnect_();
        }
        
        return std::shared_ptr<MySQLConnection>(connection.get(), 
        [this, connection](){
            std::lock_guard<std::mutex> lock(mtx);
            mySQLconnectionPool.push(connection);
            connection->setStartTime();
            cv.notify_all();
        });
    }
    catch(const std::exception& e)
    {
        LOG_ERROR << "Failed get connection" << e.what();
        {
            std::lock_guard<std::mutex> lock(mtx);
            mySQLconnectionPool.push(connection); // 放回去
            cv.notify_all();
        }
    }
    
}

void MySQLConnectionPool::deleteConnection()
{
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::microseconds(500));
        std::unique_lock<std::mutex> uniLock(mtx);
        while(mySQLconnectionPool.size() > maxSize)
        {
            auto connection = mySQLconnectionPool.front();
            if(connection->GetSqlAliveTime() > maxTime)
            {
                mySQLconnectionPool.pop();
                connection->cleanup(); // connection是共享指针，在离开作用域while循环之后就会自动释放了
            }
            else
            {
                break;
            }
        }
    }
}

void MySQLConnectionPool::checkConnection()
{
    while(true)
    {
        try
        {
            std::vector<std::shared_ptr<MySQLConnection>> connections;
            {
                std::unique_lock<std::mutex> uniLock(mtx);
                if(mySQLconnectionPool.empty())
                {
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    continue;
                }

                auto tem = mySQLconnectionPool;
                while(!tem.empty())
                {
                    connections.push_back(tem.front());
                    tem.pop();
                }
            }

            //锁外检查
            for(auto& conn : connections)
            {
                if(!conn->ping())
                {
                    try
                    {
                        conn->reconnect_();
                    }
                    catch(const std::exception& e)
                    {
                        LOG_ERROR << "Failed to connect : " << e.what();
                    }
                }
            }
            // 间隔1分钟查一次
            std::this_thread::sleep_for(std::chrono::seconds(60));
        }
        catch(const std::exception& e)
        {
            LOG_ERROR << "Error in checkThread : " << e.what();
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }
}

void MySQLConnectionPool::parseConfig()
{
    Json::Reader reader;
    Json::Value root;
    std::ifstream file("/home/luohuaxu/故障诊断系统/CNCFaultDiagnosisSystem/HttpServer/include/Database/config.json", std::ios::binary);
    if (!file.is_open())
    {
        LOG_ERROR << "Open config.json failed";
        return;
    }
    if(!reader.parse(file, root, false))
    {
        LOG_ERROR << "Parse config.json failed";
        return;
    }

    host = root["host"].asString();
    user = root["user"].asString();
    password = root["password"].asString();
    schema = root["schema"].asString();
    minSize = root["minSize"].asUInt();
    maxSize = root["maxSize"].asUInt();
    maxTime = root["maxTime"].asUInt(); 
}