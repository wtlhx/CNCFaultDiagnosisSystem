#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
int main()
{
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(80);
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    if(connect(clientSocket, (sockaddr*) &serverAddress, sizeof(serverAddress)) == -1)
    {
        throw std::runtime_error("Client connect error");
    }
    std::string request = 
                        "GET / HTTP/1.1\r\n"
                        "Host: localhost\r\n"
                        "Connection: close\r\n"
                        "\r\n";

    int writebytes = write(clientSocket, request.c_str(), request.size());
    if(writebytes <= 0)
    {
        throw std::runtime_error("Failed to send request");
    }
    std::cout << "Client write succuss" << std::endl;
    char buf[1024];
    memset(buf, 0, sizeof(buf));
    int readbytes = read(clientSocket, buf, sizeof(buf));
    if(readbytes > 0)
    {
        std::cout << buf;
    }
}
