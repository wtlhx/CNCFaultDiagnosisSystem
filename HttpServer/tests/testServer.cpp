#include "HttpServer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include <string>

void handle(const HttpRequest& request, HttpResponse* response)
{
    response->setVersion(request.getVersion());
    response->setStatusCode(HttpResponse::HttpStatusCode::OK);
    response->setStatusMessage("OK");
    response->setContentLength(request.getContentLength());
    response->setBody(request.getContentBody());
}

int main()
{
    int port = 80;
    std::string serverName = "testServer";
    HttpServer server(port, serverName);
    server.Get("/", handle);
    server.start();
}