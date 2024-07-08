#include "httplib.h"

#define SERVER_IP "192.168.211.128"
#define SERVER_PORT 9090

int main()
{
    httplib::Client client(SERVER_IP, SERVER_PORT);

    struct httplib::MultipartFormData item;
    item.name = "file";
    item.filename = "hello.txt";
    item.content = "hello world";
    item.content_type = "text/plain";

    httplib::MultipartFormDataItems items;
    items.push_back(item);

    auto res = client.Post("/multipart", items);

    std::cout << res->status << std::endl;
    std::cout << res->body << std::endl;
    return 0;
}