#include "HttpServer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "Neo4jUtil.h"
#include <memory>
#include <string>
#include <iostream>

void handle(const HttpRequest& request, HttpResponse* response)
{
    response->setVersion(request.getVersion());
    response->setStatusCode(HttpResponse::HttpStatusCode::OK);
    response->setStatusMessage("OK");
    response->setContentLength(request.getContentLength());
    response->setBody(request.getContentBody());
}

void Neo4jhandler(const HttpRequest& request, HttpResponse* response)
{
    try
    {
        Neo4jUtil neo4jconnection(neo4jDriver::Neo4j::getNeo4j(), "192.168.1.113", "7474", "neo4j", "12345678");
        std::string label = request.getQueryParameters("label");
        std::string properties = request.getQueryParameters("name");
        // std::cout << "label: " << label << ", properties: " << properties << std::endl;
        Json::Value propertiesJson, queryAnswer;
        Json::StyledWriter styledWriter;
        propertiesJson["name"] = properties;
        queryAnswer = neo4jconnection.queryReasonAndSolution(label, propertiesJson);
        if(queryAnswer.empty())
        {
            LOG_INFO << "No data found for label: " << label << " with properties: " << properties;
            response->setVersion(request.getVersion());
            response->setStatusCode(HttpResponse::HttpStatusCode::NotFound);
            response->setStatusMessage("Not Found");
            response->setBody("No data found");
            return;
        }
        std::string answer = styledWriter.write(queryAnswer);
        LOG_INFO << "Query successful for label: " << label << " with properties: " << properties;
        response->setVersion(request.getVersion());
        response->setStatusCode(HttpResponse::HttpStatusCode::OK);
        response->setStatusMessage("OK");
        response->setBody(answer);
        response->setContentType("application/json");
        response->setContentLength(answer.size());
    }
    catch(const std::exception& e)
    {
        LOG_ERROR << "Failed to connect neo4j" << e.what() << '\n';
    }
}
int main()
{
    int port = 80;
    std::string serverName = "testServer";
    HttpServer server(port, serverName);
    server.Get("/", handle);
    server.Get("/neo4j", Neo4jhandler);
    server.start();
}