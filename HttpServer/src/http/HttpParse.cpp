#include "HttpParse.h"
HttpParse::HttpParse(/* args */) : status(HttpParseStatus::ParseResquestLine)
{
}

HttpParse::~HttpParse()
{
}

//解析请求体，并返回是否有格式错误
bool HttpParse::parse(muduo::net::Buffer* buf, muduo::Timestamp receiveTime)
{
    bool correct = true; // 判断报文格式是否正确
    bool finished = false;
    while(!finished)
    {
        if(status == HttpParseStatus::ParseResquestLine)
        {
            const char* crlf = buf->findCRLF();
            if(crlf)
            {
                //解析请求行
                correct = parseRequestLine(buf->peek(), crlf);
                if(correct)
                {
                    request.setReceiveTime(receiveTime);
                    buf->retrieveUntil(crlf + 2); // +2是因为要去掉\r\n
                    status = HttpParseStatus::ParseResquestHeaders;
                }
                else
                {
                    finished = true; // 解析错误，结束解析
                }
            }
            else
            {
                finished = true; // 没有找到\r\n，结束解析
            }
        }
        else if(status == HttpParseStatus::ParseResquestHeaders)
        {
            std::cout << "Parse header" << std::endl;
            const char* crlf = buf->findCRLF();
            if(crlf)
            {
                std::cout << "header.." << std::endl;
                const char* colon = std::find(buf->peek(), crlf, ':'); //找冒号的位置
                if(colon < crlf)
                {
                    request.addHeaders(buf->peek(), colon, crlf);
                }
                else if(buf->peek() == crlf) //空行
                {   //根据请求方法来判断是否继续解析请求体
                    if(request.getMethod() == HttpRequest::Method::POST ||
                        request.getMethod() == HttpRequest::Method::PUT)
                        {
                            std::string contentLength = request.getHeaders("Content-Length");
                            if(contentLength != "")
                            {
                                uint64_t length = request.getContentLength();
                                if(length > 0)
                                {
                                    status = HttpParseStatus::ParseResquestBody;
                                }
                                else
                                {
                                    status = HttpParseStatus::ParseComplete;
                                    finished = true;
                                }
                            }
                            else
                            {
                                correct = false; //POST和PUT方法却没有Content-Length,是格式错误
                                finished = true;
                            }
                        }
                    else //其他方法没有请求体
                    {
                        status = HttpParseStatus::ParseComplete;
                        finished = true;
                    }
                }
                else
                {
                    correct = false; //格式有错误
                    finished = true;
                }
                buf->retrieveUntil(crlf + 2); // 指针指向下一行数据
            }
            else
            {
                std::cout << "Not found CRLF" << std::endl;
                finished = true; //后面没有内容了
            }
        }
        else if(status == HttpParseStatus::ParseResquestBody)
        {
            //检查缓冲区是否有足够的内容，缓冲区中的内容就是收到的请求报文的请求体
            if(buf->readableBytes() < request.getContentLength())
            {
                finished = true;
                return true;
            }
            std::string contentBody(buf->peek(), buf->peek() + request.getContentLength());
            request.setContentBody(contentBody);
            buf->retrieve(request.getContentLength()); //指针向后移动（按长度）

            status = HttpParseStatus::ParseComplete;
            finished = true;
        }
        else if(status == HttpParseStatus::ParseComplete)
        {
            return true; // 解析成功
        }
    }
    return correct;
}

bool HttpParse::parseRequestLine(const char* begin, const char* end)
{
    bool success = false; // 设置请求行是否成功
    const char* start = begin;
    const char* space = std::find(begin, end, ' ');
    if(space != end && request.setMethod(start, space)) // 找到空格并且请求方法设置成功
    {
        start = space + 1;
        space = std::find(start, end, ' ');
        if(space != end)
        {
            //设置路径url和查询参数
            const char* query = std::find(start, space, '?');
            if(query != space)
            {
                request.setUrl(start, query);
                request.setQueryParameters(query + 1, space);
            }
            else
            {
                request.setUrl(start, space);
            }
            //设置Http版本
            start = space + 1;
            success = (end - start == 8) && std::equal(start, end - 1, "HTTP/1.");//version长度是8且以"HTTP/1."开头
            if(success)
            {
                char e = *(end - 1);
                if(e == '0')
                {
                    request.setVersion("HTTP/1.0");
                }
                else if(e == '1')
                {
                    request.setVersion("HTTP/1.1");
                }
                else success = false;
            }
        }
    }
    return success;
}

void HttpParse::reset()
{
    status = HttpParseStatus::ParseResquestLine;
    request.reset();
}